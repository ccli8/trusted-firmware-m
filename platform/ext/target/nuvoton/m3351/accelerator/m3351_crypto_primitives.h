/*
 * Copyright (c) 2024-2025 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef M3351_CRYPTO_PRIMITIVES_H
#define M3351_CRYPTO_PRIMITIVES_H

#include <stdint.h>
#include <stddef.h>
#include "psa/crypto_types.h"
#include "m3351_crypto_native.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief M3351 transparent hash operation context
 */
typedef struct {
    psa_algorithm_t alg;
    m3351_native_sha_ctx_t native_ctx;
} m3351_hash_operation_t;

#define M3351_TRANSPARENT_HASH_OPERATION_INIT { 0, { 0, 0, 1, 0, {0}, {0} } }

/**
 * \brief M3351 transparent cipher operation context
 */
typedef struct {
    psa_algorithm_t alg;
    uint32_t enc_dec;               /* 1: Encrypt, 0: Decrypt */
    m3351_native_aes_ctx_t native_ctx;
    uint8_t iv[16];
    size_t iv_length;
    uint8_t iv_set;
    uint8_t reserved[3];
    uint8_t buffer[16];             /* Unprocessed leftover input block */
    size_t buffer_len;
} m3351_cipher_operation_t;

#define M3351_TRANSPARENT_CIPHER_OPERATION_INIT \
    { 0, 0, { 0, 0, 0, 0, {0}, {0} }, {0}, 0, 0, {0}, {0}, 0 }

#ifdef __cplusplus
}
#endif

#endif /* M3351_CRYPTO_PRIMITIVES_H */
