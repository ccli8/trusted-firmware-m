/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 * SPDX-FileCopyrightText: Copyright (c) 2024 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "region_defs.h"
#include "boot_hal.h"
#include "psa/crypto.h"
#include "flash_map/flash_map.h"

#define HUK_KDF_LABEL "NUVOTON_M3351_HUK_KDF_V1"

int flash_device_base(uint8_t fd_id, uintptr_t *ret)
{
    if (ret == NULL) {
        return -1;
    }
    if (fd_id == FLASH_DEVICE_ID) {
        *ret = FLASH_BASE_ADDRESS; /* 0x00000000 (APROM) */
        return 0;
    }
#if defined(FLASH_DEVICE_ID_SCRATCH)
    if (fd_id == FLASH_DEVICE_ID_SCRATCH) {
        *ret = 0x0F100000UL; /* LDROM Base */
        return 0;
    }
#endif
    return -1;
}

/**
 * @brief Software HMAC-SHA256 derivation using PSA Crypto SHA256
 */
static int hmac_sha256(const uint8_t *key, size_t key_len,
                       const uint8_t *msg, size_t msg_len,
                       uint8_t *out)
{
    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    uint8_t buf[128];
    uint8_t inner_hash[32];
    size_t out_len;
    size_t i;
    psa_status_t status;

    if (key_len != 32 || msg_len > 64) {
        return -1;
    }

    memset(k_ipad, 0x36, sizeof(k_ipad));
    memset(k_opad, 0x5c, sizeof(k_opad));

    for (i = 0; i < key_len; i++) {
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }

    /* Inner hash: SHA256(k_ipad || msg) */
    memcpy(buf, k_ipad, 64);
    memcpy(buf + 64, msg, msg_len);
    status = psa_hash_compute(PSA_ALG_SHA_256, buf, 64 + msg_len, inner_hash, sizeof(inner_hash), &out_len);
    if (status != PSA_SUCCESS) {
        goto cleanup;
    }

    /* Outer hash: SHA256(k_opad || inner_hash) */
    memcpy(buf, k_opad, 64);
    memcpy(buf + 64, inner_hash, 32);
    status = psa_hash_compute(PSA_ALG_SHA_256, buf, 64 + 32, out, 32, &out_len);

cleanup:
    /* Zeroize sensitive local buffers */
    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memset(buf, 0, sizeof(buf));
    memset(inner_hash, 0, sizeof(inner_hash));

    return (status == PSA_SUCCESS) ? 0 : -1;
}

/**
 * @brief Overrides weak hook in boot_hal_bl2.c.
 *        Called after an image is loaded and verified.
 *        Image 0 is the last loaded image (TF-M Secure image).
 */
int boot_platform_post_load(uint32_t image_id)
{
    if (image_id == 0) {
        volatile uint8_t *cdi_ptr = (volatile uint8_t *)SBR_CDI_SRAM_BASE;
        uint8_t cdi[32];
        uint8_t huk[32];

        /* Read CDI from MKROM mailbox (0x20000020) */
        memcpy(cdi, (const void *)cdi_ptr, sizeof(cdi));

        /* Derive HUK from CDI */
        if (hmac_sha256(cdi, sizeof(cdi),
                        (const uint8_t *)HUK_KDF_LABEL,
                        sizeof(HUK_KDF_LABEL) - 1,
                        huk) == 0) {
            /* Write HUK to reserved SRAM mailbox (0x20000020) */
            memcpy((void *)cdi_ptr, huk, sizeof(huk));
        }

        /* Wipe sensitive local copies */
        memset(cdi, 0, sizeof(cdi));
        memset(huk, 0, sizeof(huk));
    }

    return 0;
}
