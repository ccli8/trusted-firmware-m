/*
 * Copyright (c) 2024-2025 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "m3351_crypto_driver.h"
#include <string.h>

static psa_status_t m3351_to_psa_status(int err)
{
    switch (err) {
    case M3351_CRYPTO_SUCCESS:
        return PSA_SUCCESS;
    case M3351_CRYPTO_ERR_PARAM:
        return PSA_ERROR_INVALID_ARGUMENT;
    case M3351_CRYPTO_ERR_TIMEOUT:
    case M3351_CRYPTO_ERR_HW:
        return PSA_ERROR_HARDWARE_FAILURE;
    default:
        return PSA_ERROR_GENERIC_ERROR;
    }
}

psa_status_t m3351_crypto_init(void)
{
    m3351_crypto_native_init();
    return PSA_SUCCESS;
}

psa_status_t m3351_crypto_free(void)
{
    m3351_crypto_native_free();
    return PSA_SUCCESS;
}

psa_status_t m3351_transparent_hash_setup(
    m3351_hash_operation_t *operation,
    psa_algorithm_t alg)
{
    uint32_t mode;

    if (operation == NULL) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (alg == PSA_ALG_SHA_256) {
        mode = SHA_MODE_SHA256;
    } else if (alg == PSA_ALG_SHA_224) {
        mode = SHA_MODE_SHA224;
    } else {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    operation->alg = alg;
    int ret = m3351_native_sha_init(&operation->native_ctx, mode);
    return m3351_to_psa_status(ret);
}

psa_status_t m3351_transparent_hash_clone(
    const m3351_hash_operation_t *source_operation,
    m3351_hash_operation_t *target_operation)
{
    if (source_operation == NULL || target_operation == NULL) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    /*
     * M3351 single-instance hardware engine maintains intermediate hash state in
     * internal registers without software context-switching.
     * If full blocks have already been streamed to the hardware accelerator (first == 0),
     * intermediate state cannot be cloned without hardware feedback, so return
     * PSA_ERROR_NOT_SUPPORTED to prevent silent state corruption.
     * If no full blocks have been streamed yet (first == 1), all state is in the software
     * context buffer and can be safely cloned.
     */
    if (!source_operation->native_ctx.first) {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    target_operation->alg = source_operation->alg;
    int ret = m3351_native_sha_clone(&source_operation->native_ctx,
                                     &target_operation->native_ctx);
    return m3351_to_psa_status(ret);
}

psa_status_t m3351_transparent_hash_update(
    m3351_hash_operation_t *operation,
    const uint8_t *input,
    size_t input_length)
{
    if (operation == NULL || (input_length > 0 && input == NULL)) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    int ret = m3351_native_sha_update(&operation->native_ctx, input, input_length);
    return m3351_to_psa_status(ret);
}

psa_status_t m3351_transparent_hash_finish(
    m3351_hash_operation_t *operation,
    uint8_t *hash,
    size_t hash_size,
    size_t *hash_length)
{
    size_t expected_len;

    if (operation == NULL || hash == NULL || hash_length == NULL) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    expected_len = (operation->alg == PSA_ALG_SHA_224) ? 28 : 32;
    if (hash_size < expected_len) {
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    int ret = m3351_native_sha_finish(&operation->native_ctx, hash, hash_size);
    if (ret == M3351_CRYPTO_SUCCESS) {
        *hash_length = expected_len;
    }

    memset(operation, 0, sizeof(m3351_hash_operation_t));
    return m3351_to_psa_status(ret);
}

psa_status_t m3351_transparent_hash_abort(
    m3351_hash_operation_t *operation)
{
    if (operation != NULL) {
        memset(operation, 0, sizeof(m3351_hash_operation_t));
    }
    return PSA_SUCCESS;
}

psa_status_t m3351_transparent_hash_compute(
    psa_algorithm_t alg,
    const uint8_t *input,
    size_t input_length,
    uint8_t *hash,
    size_t hash_size,
    size_t *hash_length)
{
    uint32_t mode;
    size_t expected_len;

    if (hash == NULL || hash_length == NULL || (input_length > 0 && input == NULL)) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (alg == PSA_ALG_SHA_256) {
        mode = SHA_MODE_SHA256;
        expected_len = 32;
    } else if (alg == PSA_ALG_SHA_224) {
        mode = SHA_MODE_SHA224;
        expected_len = 28;
    } else {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    if (hash_size < expected_len) {
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    int ret = m3351_native_sha_compute(mode, input, input_length, hash, hash_size);
    if (ret == M3351_CRYPTO_SUCCESS) {
        *hash_length = expected_len;
    }

    return m3351_to_psa_status(ret);
}
