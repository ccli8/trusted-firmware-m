/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 * SPDX-FileCopyrightText: Copyright (c) 2024 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#if defined(PSA_CRYPTO_DRIVER_M3351)
#include "m3351_crypto_native.h"
#else
#define MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS
#include "mbedtls/private/sha256.h"
#endif

#include <string.h>
#include <stdbool.h>
#include "tfm_plat_crypto_keys.h"
#include "tfm_builtin_key_ids.h"
#include "tfm_plat_otp.h"
#include "psa_manifest/pid.h"
#include "tfm_builtin_key_loader.h"
#include "region_defs.h"

#define NUMBER_OF_ELEMENTS_OF(x) (sizeof(x) / sizeof(*x))
#define MAPPED_TZ_NS_AGENT_DEFAULT_CLIENT_ID -0x3c000000
#define TFM_NS_PARTITION_ID                  MAPPED_TZ_NS_AGENT_DEFAULT_CLIENT_ID

#define IAK_KDF_LABEL "NUVOTON_M3351_IAK_KDF_V1"

static uint8_t s_huk_buf[32];
static bool s_huk_loaded = false;

static void load_huk_from_sram(void)
{
    if (!s_huk_loaded) {
        volatile uint8_t *sram_huk = (volatile uint8_t *)SBR_HUK_SRAM_BASE;

        /* Copy HUK passed from BL2 via the reserved SRAM mailbox */
        memcpy(s_huk_buf, (const void *)sram_huk, 32);

        /* Zeroize the SRAM location immediately */
        memset((void *)sram_huk, 0, 32);

        s_huk_loaded = true;
    }
}

/**
 * @brief Software HMAC-SHA256 derivation using low-level mbedtls_sha256
 */
static void hmac_sha256(const uint8_t *key, size_t key_len,
                        const uint8_t *msg, size_t msg_len,
                        uint8_t *out)
{
    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    uint8_t buf[128];
    uint8_t inner_hash[32];
    size_t i;

    memset(k_ipad, 0x36, sizeof(k_ipad));
    memset(k_opad, 0x5c, sizeof(k_opad));

    for (i = 0; i < key_len && i < 64; i++) {
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }

    /* Inner hash: SHA256(k_ipad || msg) */
    memcpy(buf, k_ipad, 64);
    memcpy(buf + 64, msg, msg_len);
#if defined(PSA_CRYPTO_DRIVER_M3351)
    m3351_native_sha_compute(SHA_MODE_SHA256, buf, 64 + msg_len, inner_hash, 32);
#else
    mbedtls_sha256(buf, 64 + msg_len, inner_hash, 0);
#endif

    /* Outer hash: SHA256(k_opad || inner_hash) */
    memcpy(buf, k_opad, 64);
    memcpy(buf + 64, inner_hash, 32);
#if defined(PSA_CRYPTO_DRIVER_M3351)
    m3351_native_sha_compute(SHA_MODE_SHA256, buf, 64 + 32, out, 32);
#else
    mbedtls_sha256(buf, 64 + 32, out, 0);
#endif

    /* Zeroize sensitive local buffers */
    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memset(buf, 0, sizeof(buf));
    memset(inner_hash, 0, sizeof(inner_hash));
}

static enum tfm_plat_err_t tfm_plat_get_huk(const void *ctx,
                                            uint8_t *buf, size_t buf_len,
                                            size_t *key_len,
                                            psa_key_bits_t *key_bits,
                                            psa_algorithm_t *algorithm,
                                            psa_key_type_t *type)
{
    (void)ctx;

    if (buf == NULL || buf_len < 32 || key_len == NULL ||
        key_bits == NULL || algorithm == NULL || type == NULL) {
        return TFM_PLAT_ERR_INVALID_INPUT;
    }

    load_huk_from_sram();
    memcpy(buf, s_huk_buf, 32);

#ifndef TFM_PARTITION_INITIAL_ATTESTATION
    /* If attestation is disabled, we can zeroize s_huk_buf immediately */
    memset(s_huk_buf, 0, sizeof(s_huk_buf));
#endif

    *key_len = 32;
    *key_bits = 256;
    *algorithm = PSA_ALG_HKDF(PSA_ALG_SHA_256);
    *type = PSA_KEY_TYPE_DERIVE;

    return TFM_PLAT_ERR_SUCCESS;
}

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
static enum tfm_plat_err_t tfm_plat_get_iak(const void *ctx,
                                            uint8_t *buf, size_t buf_len,
                                            size_t *key_len,
                                            psa_key_bits_t *key_bits,
                                            psa_algorithm_t *algorithm,
                                            psa_key_type_t *type)
{
    (void)ctx;

    if (buf == NULL || buf_len < 32 || key_len == NULL ||
        key_bits == NULL || algorithm == NULL || type == NULL) {
        return TFM_PLAT_ERR_INVALID_INPUT;
    }

#if defined(PSA_API_TEST_INITIAL_ATTESTATION) && defined(TFM_DUMMY_PROVISIONING)
    /* Standard test dummy IAK matching attest_public_key in pal_attestation_config.h */
    static const uint8_t s_dummy_iak[32] = {
        0xA9, 0xB4, 0x54, 0xB2, 0x6D, 0x6F, 0x90, 0xA4,
        0xEA, 0x31, 0x19, 0x35, 0x64, 0xCB, 0xA9, 0x1F,
        0xEC, 0x6F, 0x9A, 0x00, 0x2A, 0x7D, 0xC0, 0x50,
        0x4B, 0x92, 0xA1, 0x93, 0x71, 0x34, 0x58, 0x5F
    };
    memcpy(buf, s_dummy_iak, 32);
#else
    load_huk_from_sram();

    /* Derive IAK private key scalar (32 bytes) from HUK using HMAC-SHA256 */
    hmac_sha256(s_huk_buf, 32,
                (const uint8_t *)IAK_KDF_LABEL,
                sizeof(IAK_KDF_LABEL) - 1,
                buf);

    /* Zeroize local copy of HUK now that all keys are derived */
    memset(s_huk_buf, 0, sizeof(s_huk_buf));
#endif

    *key_len = 32;
    *key_bits = 256;
    *algorithm = PSA_ALG_ECDSA(PSA_ALG_SHA_256);
    *type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);

    return TFM_PLAT_ERR_SUCCESS;
}
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

#ifdef TFM_PARTITION_INITIAL_ATTESTATION
static const tfm_plat_builtin_key_per_user_policy_t g_iak_per_user_policy[] = {
    {.user = TFM_SP_INITIAL_ATTESTATION,
#ifdef SYMMETRIC_INITIAL_ATTESTATION
        .usage = PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_EXPORT,
#else
        .usage = PSA_KEY_USAGE_SIGN_HASH,
#endif /* SYMMETRIC_INITIAL_ATTESTATION */
    },
#ifdef TEST_S_ATTESTATION
    {.user = TFM_SP_SECURE_TEST_PARTITION, .usage = PSA_KEY_USAGE_VERIFY_HASH},
#endif /* TEST_S_ATTESTATION */
#ifdef TEST_NS_ATTESTATION
    {.user = TFM_NS_PARTITION_ID, .usage = PSA_KEY_USAGE_VERIFY_HASH},
#endif /* TEST_NS_ATTESTATION */
};
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */

static const tfm_plat_builtin_key_policy_t g_builtin_keys_policy[] = {
    {.key_id = TFM_BUILTIN_KEY_ID_HUK, .per_user_policy = 0, .usage = PSA_KEY_USAGE_DERIVE},
#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    {.key_id = TFM_BUILTIN_KEY_ID_IAK,
     .per_user_policy = NUMBER_OF_ELEMENTS_OF(g_iak_per_user_policy),
     .policy_ptr = g_iak_per_user_policy},
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */
};

static const tfm_plat_builtin_key_descriptor_t g_builtin_keys_desc[] = {
    {.key_id = TFM_BUILTIN_KEY_ID_HUK,
     .slot_number = TFM_BUILTIN_KEY_SLOT_HUK,
     .lifetime = TFM_BUILTIN_KEY_LOADER_LIFETIME,
     .loader_key_func = tfm_plat_get_huk,
     .loader_key_ctx = NULL},
#ifdef TFM_PARTITION_INITIAL_ATTESTATION
    {.key_id = TFM_BUILTIN_KEY_ID_IAK,
     .slot_number = TFM_BUILTIN_KEY_SLOT_IAK,
     .lifetime = TFM_BUILTIN_KEY_LOADER_LIFETIME,
     .loader_key_func = tfm_plat_get_iak,
     .loader_key_ctx = NULL},
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */
};

size_t tfm_plat_builtin_key_get_policy_table_ptr(const tfm_plat_builtin_key_policy_t *desc_ptr[])
{
    *desc_ptr = &g_builtin_keys_policy[0];
    return NUMBER_OF_ELEMENTS_OF(g_builtin_keys_policy);
}

size_t tfm_plat_builtin_key_get_desc_table_ptr(const tfm_plat_builtin_key_descriptor_t *desc_ptr[])
{
    *desc_ptr = &g_builtin_keys_desc[0];
    return NUMBER_OF_ELEMENTS_OF(g_builtin_keys_desc);
}
