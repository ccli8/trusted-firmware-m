/*
 * Copyright (c) 2024-2025 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef M3351_CRYPTO_NATIVE_H
#define M3351_CRYPTO_NATIVE_H

#include <stdint.h>
#include <stddef.h>

#ifndef SHA_MODE_SHA1
#define SHA_MODE_SHA1           0UL
#endif
#ifndef SHA_MODE_SHA224
#define SHA_MODE_SHA224         5UL
#endif
#ifndef SHA_MODE_SHA256
#define SHA_MODE_SHA256         4UL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define M3351_SHA_BLOCK_SIZE     (64U)
#define M3351_AES_BLOCK_SIZE     (16U)

/* Return codes for native driver operations */
#define M3351_CRYPTO_SUCCESS      (0)
#define M3351_CRYPTO_ERR_PARAM   (-1)
#define M3351_CRYPTO_ERR_TIMEOUT (-2)
#define M3351_CRYPTO_ERR_HW      (-3)

/**
 * \brief Native SHA context structure for M3351 hardware engine
 */
typedef struct {
    uint32_t ctl;
    uint32_t buffer_len;
    uint8_t first;
    uint8_t is224;
    uint8_t reserved[2];
    __attribute__((aligned(4))) uint8_t buffer[M3351_SHA_BLOCK_SIZE];
} m3351_native_sha_ctx_t;

/**
 * \brief Native AES context structure for M3351 hardware engine
 */
typedef struct {
    uint32_t key_size;      /* 16, 24, 32 bytes */
    uint32_t key_size_op;   /* Register setting for key size */
    uint32_t enc_dec;       /* CRYPTO_AES_CTL_ENCRYPTO_Msk or 0 */
    uint32_t op_mode;       /* AES_MODE_ECB, AES_MODE_CBC, etc. */
    __attribute__((aligned(4))) uint32_t keys[8];
    __attribute__((aligned(4))) uint32_t iv[4];
} m3351_native_aes_ctx_t;

/**
 * \brief Initialize M3351 crypto hardware IP (clock, reset)
 */
void m3351_crypto_native_init(void);

/**
 * \brief Free/disable M3351 crypto hardware IP
 */
void m3351_crypto_native_free(void);

/* ========================================================================= */
/* Native SHA functions                                                      */
/* ========================================================================= */

int m3351_native_sha_init(m3351_native_sha_ctx_t *ctx, uint32_t u32OpMode);
int m3351_native_sha_update(m3351_native_sha_ctx_t *ctx, const uint8_t *input, size_t ilen);
int m3351_native_sha_finish(m3351_native_sha_ctx_t *ctx, uint8_t *output, size_t digest_len);
int m3351_native_sha_clone(const m3351_native_sha_ctx_t *src, m3351_native_sha_ctx_t *dst);
int m3351_native_sha_compute(uint32_t u32OpMode, const uint8_t *input, size_t ilen,
                             uint8_t *output, size_t digest_len);

/* ========================================================================= */
/* Native AES functions                                                      */
/* ========================================================================= */

int m3351_native_aes_setkey(m3351_native_aes_ctx_t *ctx, const uint8_t *key, size_t keybits);
int m3351_native_aes_crypt_ecb(m3351_native_aes_ctx_t *ctx, int mode,
                               const uint8_t input[16], uint8_t output[16]);
int m3351_native_aes_crypt_cbc(m3351_native_aes_ctx_t *ctx, int mode, size_t length,
                               uint8_t iv[16], const uint8_t *input, uint8_t *output);
int m3351_native_aes_crypt_ctr(m3351_native_aes_ctx_t *ctx, size_t length,
                               uint8_t nonce_counter[16], const uint8_t *input,
                               uint8_t *output);

#ifdef __cplusplus
}
#endif

#endif /* M3351_CRYPTO_NATIVE_H */
