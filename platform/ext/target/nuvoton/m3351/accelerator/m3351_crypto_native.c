/*
 * Copyright (c) 2024-2025 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "m3351_crypto_native.h"
#include "NuMicro.h"
#include <string.h>
#include <stdbool.h>

#define M3351_CRYPTO_TIMEOUT_CNT  (0x100000)


/* Internal aligned bounce buffer for AES DMA transfers */
__attribute__((aligned(4))) static uint8_t s_aes_dma_in[M3351_AES_BLOCK_SIZE];
__attribute__((aligned(4))) static uint8_t s_aes_dma_out[M3351_AES_BLOCK_SIZE];

static inline const uint8_t *m3351_remap_ns_flash_ptr(const uint8_t *ptr)
{
    uintptr_t addr = (uintptr_t)ptr;
    uint32_t fnsaddr = SCU->FNSADDR;

    if (fnsaddr > 0 && fnsaddr <= 0x00200000UL) {
        if (addr >= fnsaddr && addr < 0x00200000UL) {
            return (const uint8_t *)(addr + 0x10000000UL);
        }
    }
    return ptr;
}

static inline bool m3351_is_dma_accessible(const void *addr, size_t len, uint32_t *dma_addr)
{
    uintptr_t start = (uintptr_t)addr;
    uintptr_t end;

    if (addr == NULL || len == 0 || (start & 0x3) != 0) {
        return false;
    }

    end = start + len;
    if (end < start) {
        return false; /* Integer overflow */
    }

    /* Peripheral memory and PPB cannot be read by DMA */
    if ((start >= 0x40000000UL && start < 0x60000000UL) ||
        (start >= 0xE0000000UL)) {
        return false;
    }

    /* Valid SRAM memory ranges (Non-Secure and Secure SRAM aliases) */
    if ((start >= 0x20000000UL && end <= 0x20040000UL) ||
        (start >= 0x30000000UL && end <= 0x30040000UL)) {
        if (dma_addr != NULL) {
            *dma_addr = (uint32_t)start;
        }
        return true;
    }

    /* Flash memory (APROM):
     * M3351 Crypto DMA can access Flash memory directly via AHB.
     * In TrustZone, Flash below SCU->FNSADDR is Secure, and Flash at/above
     * SCU->FNSADDR is Non-Secure.
     * - Secure Flash is accessible via 0x00000000 ~ SCU->FNSADDR.
     * - Non-Secure Flash MUST be accessed via its Non-Secure alias (0x10000000 | addr),
     *   because accessing Non-Secure Flash via the 0x00000000 Secure alias is blocked
     *   by SCU and returns all zeros (RAZWI).
     */
    uint32_t fnsaddr = SCU->FNSADDR;
    if (fnsaddr == 0 || fnsaddr > 0x00200000UL) {
        fnsaddr = 0x00200000UL;
    }

    /* Case 1: Secure APROM (0x00000000 ~ fnsaddr) */
    if (start < fnsaddr) {
        if (end <= fnsaddr) {
            if (dma_addr != NULL) {
                *dma_addr = (uint32_t)start;
            }
            return true;
        }
        /* Spans boundary between Secure and Non-Secure */
        return false;
    }

    /* Case 2: Non-Secure APROM accessed via Secure address alias (fnsaddr ~ 0x00200000) */
    if (start >= fnsaddr && end <= 0x00200000UL) {
        if (dma_addr != NULL) {
            *dma_addr = (uint32_t)(start + 0x10000000UL); /* Remap to Non-Secure alias */
        }
        return true;
    }

    /* Case 3: Non-Secure APROM native alias (0x10000000 ~ 0x10200000) */
    if (start >= 0x10000000UL && end <= 0x10200000UL) {
        if (dma_addr != NULL) {
            *dma_addr = (uint32_t)start;
        }
        return true;
    }

    /* Case 4: LDROM */
    if ((start >= 0x0F100000UL && end <= 0x0F104000UL) ||
        (start >= 0x1F100000UL && end <= 0x1F104000UL)) {
        if (dma_addr != NULL) {
            *dma_addr = (uint32_t)start;
        }
        return true;
    }

    /* Case 5: DFMC (Data Flash) */
    if ((start >= 0x00100000UL && end <= 0x00110000UL) ||
        (start >= 0x10100000UL && end <= 0x10110000UL) ||
        (start >= 0x0F210000UL && end <= 0x0F220000UL) ||
        (start >= 0x1F210000UL && end <= 0x1F220000UL)) {
        if (dma_addr != NULL) {
            *dma_addr = (uint32_t)start;
        }
        return true;
    }

    /* Case 6: Valid EBI memory ranges */
    if ((start >= 0x60000000UL && end <= 0x60400000UL) ||
        (start >= 0x70000000UL && end <= 0x70400000UL)) {
        if (dma_addr != NULL) {
            *dma_addr = (uint32_t)start;
        }
        return true;
    }

    return false;
}

static inline uint32_t nu_get32_le(const uint8_t *pos)
{
    return ((uint32_t)pos[0]) |
           (((uint32_t)pos[1]) << 8) |
           (((uint32_t)pos[2]) << 16) |
           (((uint32_t)pos[3]) << 24);
}

static inline void nu_set32_le(uint8_t *pos, uint32_t val)
{
    pos[0] = (uint8_t)(val & 0xFF);
    pos[1] = (uint8_t)((val >> 8) & 0xFF);
    pos[2] = (uint8_t)((val >> 16) & 0xFF);
    pos[3] = (uint8_t)((val >> 24) & 0xFF);
}

void m3351_crypto_native_init(void)
{
    /* Enable CRYPTO module clock */
    CLK_EnableModuleClock(CRPT_MODULE);

    /* Reset CRYPTO module */
    SYS_UnlockReg();
    SYS_ResetModule(CRPT_RST);
    SYS_LockReg();
}

void m3351_crypto_native_free(void)
{
    /* Force stop any active operations */
    CRYPTO->HMAC_CTL = CRYPTO_HMAC_CTL_STOP_Msk;
    CRYPTO->AES_CTL  = CRYPTO_AES_CTL_STOP_Msk;
}

/* ========================================================================= */
/* Native SHA implementation                                                 */
/* ========================================================================= */

int m3351_native_sha_init(m3351_native_sha_ctx_t *ctx, uint32_t u32OpMode)
{
    if (ctx == NULL) {
        return M3351_CRYPTO_ERR_PARAM;
    }

    /* Ensure CRYPTO clock is enabled before register access */
    CLK_EnableModuleClock(CRPT_MODULE);

    memset(ctx, 0, sizeof(m3351_native_sha_ctx_t));

    ctx->first = 1;
    if (u32OpMode == SHA_MODE_SHA224) {
        ctx->is224 = 1;
    } else {
        ctx->is224 = 0;
        u32OpMode = SHA_MODE_SHA256;
    }

    /* Stop any previous SHA operation */
    CRYPTO->HMAC_CTL = CRYPTO_HMAC_CTL_STOP_Msk;

    /* Base control register settings */
    ctx->ctl = CRYPTO_HMAC_CTL_DMAEN_Msk |
               (u32OpMode << CRYPTO_HMAC_CTL_OPMODE_Pos) |
               CRYPTO_HMAC_CTL_INSWAP_Msk |
               CRYPTO_HMAC_CTL_OUTSWAP_Msk;

    return M3351_CRYPTO_SUCCESS;
}

static int m3351_sha_wait_done(uint32_t dmacnt)
{
    /* Scale timeout count with DMA byte count: base timeout + extra for large blocks */
    int32_t timeout = M3351_CRYPTO_TIMEOUT_CNT + (int32_t)(dmacnt * 16U);

    while ((CRYPTO->INTSTS & CRYPTO_INTSTS_HMACIF_Msk) == 0) {
        if (--timeout <= 0) {
            return M3351_CRYPTO_ERR_TIMEOUT;
        }
    }

    CRYPTO->INTSTS = CRYPTO_INTSTS_HMACIF_Msk;
    return M3351_CRYPTO_SUCCESS;
}

static int m3351_sha_dma_cascade(m3351_native_sha_ctx_t *ctx, uint32_t saddr, uint32_t dmacnt)
{
    CRYPTO->HMAC_SADDR = saddr;
    CRYPTO->HMAC_DMACNT = dmacnt;

    if (ctx->first) {
        CRYPTO->HMAC_CTL = ctx->ctl | CRYPTO_HMAC_CTL_START_Msk |
                           CRYPTO_HMAC_CTL_DMACSCAD_Msk | CRYPTO_HMAC_CTL_DMAFIRST_Msk;
        ctx->first = 0;
    } else {
        CRYPTO->HMAC_CTL = ctx->ctl | CRYPTO_HMAC_CTL_START_Msk |
                           CRYPTO_HMAC_CTL_DMACSCAD_Msk;
    }

    return m3351_sha_wait_done(dmacnt);
}

int m3351_native_sha_update(m3351_native_sha_ctx_t *ctx, const uint8_t *input, size_t ilen)
{
    int ret;

    if (ctx == NULL || (ilen > 0 && input == NULL)) {
        return M3351_CRYPTO_ERR_PARAM;
    }

    if (ilen == 0) {
        return M3351_CRYPTO_SUCCESS;
    }

    /* Step 1: If there is pending data in ctx->buffer, fill it to a full 64-byte block */
    if (ctx->buffer_len > 0) {
        size_t needed = M3351_SHA_BLOCK_SIZE - ctx->buffer_len;
        if (ilen < needed) {
            /* Still not enough for a full block, keep in buffer */
            memcpy(&ctx->buffer[ctx->buffer_len], m3351_remap_ns_flash_ptr(input), ilen);
            ctx->buffer_len += ilen;
            return M3351_CRYPTO_SUCCESS;
        }

        /* Fill the buffer to 64 bytes and process via DMA cascade */
        memcpy(&ctx->buffer[ctx->buffer_len], m3351_remap_ns_flash_ptr(input), needed);
        ret = m3351_sha_dma_cascade(ctx, (uint32_t)&ctx->buffer[0], M3351_SHA_BLOCK_SIZE);
        if (ret != M3351_CRYPTO_SUCCESS) {
            return ret;
        }

        ctx->buffer_len = 0;
        input += needed;
        ilen -= needed;
    }

    /* Step 2: Process all full blocks (N * 64 bytes) */
    size_t full_blocks_len = ilen & ~(size_t)(M3351_SHA_BLOCK_SIZE - 1);
    if (full_blocks_len > 0) {
        uint32_t dma_saddr = 0;
        if (m3351_is_dma_accessible(input, full_blocks_len, &dma_saddr)) {
            /* Fast path: 4-byte aligned and DMA-accessible, process N * 64 bytes in one shot */
            ret = m3351_sha_dma_cascade(ctx, dma_saddr, full_blocks_len);
            if (ret != M3351_CRYPTO_SUCCESS) {
                return ret;
            }
            input += full_blocks_len;
            ilen -= full_blocks_len;
        } else {
            /* Unaligned or non-DMA memory: bounce buffer 64 bytes at a time */
            while (ilen >= M3351_SHA_BLOCK_SIZE) {
                memcpy(&ctx->buffer[0], m3351_remap_ns_flash_ptr(input), M3351_SHA_BLOCK_SIZE);
                ret = m3351_sha_dma_cascade(ctx, (uint32_t)&ctx->buffer[0], M3351_SHA_BLOCK_SIZE);
                if (ret != M3351_CRYPTO_SUCCESS) {
                    return ret;
                }
                input += M3351_SHA_BLOCK_SIZE;
                ilen -= M3351_SHA_BLOCK_SIZE;
            }
        }
    }

    /* Step 3: Retain trailing leftover bytes (< 64 bytes) in context buffer */
    if (ilen > 0) {
        memcpy(&ctx->buffer[0], m3351_remap_ns_flash_ptr(input), ilen);
        ctx->buffer_len = ilen;
    }

    return M3351_CRYPTO_SUCCESS;
}

int m3351_native_sha_finish(m3351_native_sha_ctx_t *ctx, uint8_t *output, size_t digest_len)
{
    int ret;
    size_t copy_len;

    if (ctx == NULL || output == NULL) {
        return M3351_CRYPTO_ERR_PARAM;
    }

    copy_len = ctx->is224 ? 28 : 32;
    if (digest_len < copy_len) {
        return M3351_CRYPTO_ERR_PARAM;
    }

    CRYPTO->HMAC_SADDR = (uint32_t)&ctx->buffer[0];
    CRYPTO->HMAC_DMACNT = ctx->buffer_len;

    if (ctx->first) {
        CRYPTO->HMAC_CTL = ctx->ctl | CRYPTO_HMAC_CTL_START_Msk |
                           CRYPTO_HMAC_CTL_DMALAST_Msk;
    } else {
        CRYPTO->HMAC_CTL = ctx->ctl | CRYPTO_HMAC_CTL_START_Msk |
                           CRYPTO_HMAC_CTL_DMACSCAD_Msk | CRYPTO_HMAC_CTL_DMALAST_Msk;
    }

    ret = m3351_sha_wait_done(ctx->buffer_len);
    if (ret != M3351_CRYPTO_SUCCESS) {
        return ret;
    }

    ctx->buffer_len = 0;

    /* Read result from hardware digest registers */
    memcpy(output, (const void *)&CRYPTO->HMAC_DGST[0], copy_len);

    return M3351_CRYPTO_SUCCESS;
}

int m3351_native_sha_clone(const m3351_native_sha_ctx_t *src, m3351_native_sha_ctx_t *dst)
{
    if (src == NULL || dst == NULL) {
        return M3351_CRYPTO_ERR_PARAM;
    }
    memcpy(dst, src, sizeof(m3351_native_sha_ctx_t));
    return M3351_CRYPTO_SUCCESS;
}

int m3351_native_sha_compute(uint32_t u32OpMode, const uint8_t *input, size_t ilen,
                             uint8_t *output, size_t digest_len)
{
    m3351_native_sha_ctx_t ctx;
    int ret;

    ret = m3351_native_sha_init(&ctx, u32OpMode);
    if (ret != M3351_CRYPTO_SUCCESS) {
        return ret;
    }

    ret = m3351_native_sha_update(&ctx, input, ilen);
    if (ret != M3351_CRYPTO_SUCCESS) {
        return ret;
    }

    return m3351_native_sha_finish(&ctx, output, digest_len);
}

/* ========================================================================= */
/* Native AES implementation                                                 */
/* ========================================================================= */

int m3351_native_aes_setkey(m3351_native_aes_ctx_t *ctx, const uint8_t *key, size_t keybits)
{
    if (ctx == NULL || key == NULL) {
        return M3351_CRYPTO_ERR_PARAM;
    }

    switch (keybits) {
    case 128:
        ctx->key_size = 16;
        ctx->key_size_op = 0;
        break;
    case 192:
        ctx->key_size = 24;
        ctx->key_size_op = (1U << CRYPTO_AES_CTL_KEYSZ_Pos);
        break;
    case 256:
        ctx->key_size = 32;
        ctx->key_size_op = (2U << CRYPTO_AES_CTL_KEYSZ_Pos);
        break;
    default:
        return M3351_CRYPTO_ERR_PARAM;
    }

    memcpy((uint8_t *)&ctx->keys[0], key, ctx->key_size);
    return M3351_CRYPTO_SUCCESS;
}

static int m3351_aes_crypt_raw_block(m3351_native_aes_ctx_t *ctx,
                                     const uint8_t input[16], uint8_t output[16])
{
    int32_t timeout;
    uint32_t wcnt, i;

    /* Ensure CRYPTO clock is enabled before register access */
    CLK_EnableModuleClock(CRPT_MODULE);

    /* Force AES stop */
    CRYPTO->AES_CTL = CRYPTO_AES_CTL_STOP_Msk;

    /* Copy to word-aligned DMA input buffer */
    memcpy(s_aes_dma_in, input, M3351_AES_BLOCK_SIZE);

    /* Set AES control */
    CRYPTO->AES_CTL = ctx->enc_dec | ctx->op_mode | ctx->key_size_op |
                      (AES_IN_OUT_SWAP << CRYPTO_AES_CTL_OUTSWAP_Pos) |
                      CRYPTO_AES_CTL_KINSWAP_Msk | CRYPTO_AES_CTL_DMAEN_Msk;

    /* Set IV */
    CRYPTO->AES_IV[0] = ctx->iv[0];
    CRYPTO->AES_IV[1] = ctx->iv[1];
    CRYPTO->AES_IV[2] = ctx->iv[2];
    CRYPTO->AES_IV[3] = ctx->iv[3];

    /* Set Keys */
    wcnt = ctx->key_size / 4U;
    for (i = 0; i < wcnt; i++) {
        CRYPTO->AES_KEY[i] = ctx->keys[i];
    }

    /* Set DMA source, destination and length */
    CRYPTO->AES_SADDR = (uint32_t)s_aes_dma_in;
    CRYPTO->AES_DADDR = (uint32_t)s_aes_dma_out;
    CRYPTO->AES_CNT = M3351_AES_BLOCK_SIZE;

    /* Clear finish flag */
    CRYPTO->INTSTS = CRYPTO_INTSTS_AESIF_Msk;

    /* Start AES engine */
    CRYPTO->AES_CTL |= CRYPTO_AES_CTL_START_Msk;

    timeout = M3351_CRYPTO_TIMEOUT_CNT;
    while ((CRYPTO->INTSTS & CRYPTO_INTSTS_AESIF_Msk) == 0) {
        if (--timeout <= 0) {
            return M3351_CRYPTO_ERR_TIMEOUT;
        }
    }

    CRYPTO->INTSTS = CRYPTO_INTSTS_AESIF_Msk;

    /* Copy result from word-aligned DMA output buffer */
    memcpy(output, s_aes_dma_out, M3351_AES_BLOCK_SIZE);

    return M3351_CRYPTO_SUCCESS;
}

int m3351_native_aes_crypt_ecb(m3351_native_aes_ctx_t *ctx, int mode,
                               const uint8_t input[16], uint8_t output[16])
{
    if (ctx == NULL || input == NULL || output == NULL) {
        return M3351_CRYPTO_ERR_PARAM;
    }

    ctx->enc_dec = mode ? CRYPTO_AES_CTL_ENCRYPTO_Msk : 0;
    ctx->op_mode = (AES_MODE_ECB << CRYPTO_AES_CTL_OPMODE_Pos);
    ctx->iv[0] = 0;
    ctx->iv[1] = 0;
    ctx->iv[2] = 0;
    ctx->iv[3] = 0;

    return m3351_aes_crypt_raw_block(ctx, input, output);
}

int m3351_native_aes_crypt_cbc(m3351_native_aes_ctx_t *ctx, int mode, size_t length,
                               uint8_t iv[16], const uint8_t *input, uint8_t *output)
{
    uint8_t tmp_in[16];
    int ret;

    if (ctx == NULL || iv == NULL || input == NULL || output == NULL) {
        return M3351_CRYPTO_ERR_PARAM;
    }

    if ((length % M3351_AES_BLOCK_SIZE) != 0) {
        return M3351_CRYPTO_ERR_PARAM;
    }

    ctx->enc_dec = mode ? CRYPTO_AES_CTL_ENCRYPTO_Msk : 0;
    ctx->op_mode = (AES_MODE_CBC << CRYPTO_AES_CTL_OPMODE_Pos);

    ctx->iv[0] = nu_get32_le(iv);
    ctx->iv[1] = nu_get32_le(iv + 4);
    ctx->iv[2] = nu_get32_le(iv + 8);
    ctx->iv[3] = nu_get32_le(iv + 12);

    while (length > 0) {
        memcpy(tmp_in, input, M3351_AES_BLOCK_SIZE);

        ret = m3351_aes_crypt_raw_block(ctx, input, output);
        if (ret != M3351_CRYPTO_SUCCESS) {
            return ret;
        }

        if (mode) {
            /* Encryption: next IV is ciphertext */
            ctx->iv[0] = nu_get32_le(output);
            ctx->iv[1] = nu_get32_le(output + 4);
            ctx->iv[2] = nu_get32_le(output + 8);
            ctx->iv[3] = nu_get32_le(output + 12);
        } else {
            /* Decryption: next IV is previous ciphertext input */
            ctx->iv[0] = nu_get32_le(tmp_in);
            ctx->iv[1] = nu_get32_le(tmp_in + 4);
            ctx->iv[2] = nu_get32_le(tmp_in + 8);
            ctx->iv[3] = nu_get32_le(tmp_in + 12);
        }

        length -= M3351_AES_BLOCK_SIZE;
        input  += M3351_AES_BLOCK_SIZE;
        output += M3351_AES_BLOCK_SIZE;
    }

    /* Save updated IV */
    nu_set32_le(iv, ctx->iv[0]);
    nu_set32_le(iv + 4, ctx->iv[1]);
    nu_set32_le(iv + 8, ctx->iv[2]);
    nu_set32_le(iv + 12, ctx->iv[3]);

    return M3351_CRYPTO_SUCCESS;
}

int m3351_native_aes_crypt_ctr(m3351_native_aes_ctx_t *ctx, size_t length,
                               uint8_t nonce_counter[16], const uint8_t *input,
                               uint8_t *output)
{
    uint8_t stream_block[16];
    int ret, i;

    if (ctx == NULL || nonce_counter == NULL || input == NULL || output == NULL) {
        return M3351_CRYPTO_ERR_PARAM;
    }

    while (length > 0) {
        ret = m3351_native_aes_crypt_ecb(ctx, 1, nonce_counter, stream_block);
        if (ret != M3351_CRYPTO_SUCCESS) {
            return ret;
        }

        /* Increment 128-bit big-endian counter */
        for (i = 16; i > 0; i--) {
            if (++nonce_counter[i - 1] != 0) {
                break;
            }
        }

        size_t block_chunk = (length > 16) ? 16 : length;
        for (size_t k = 0; k < block_chunk; k++) {
            *output++ = *input++ ^ stream_block[k];
        }
        length -= block_chunk;
    }

    return M3351_CRYPTO_SUCCESS;
}
