/*
 * Copyright (c) 2024-2025 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef M3351_CRYPTO_DRIVER_H
#define M3351_CRYPTO_DRIVER_H

#include <psa/crypto.h>
#include <psa/crypto_driver_common.h>
#include "m3351_crypto_primitives.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Driver lifecycle */
psa_status_t m3351_crypto_init(void);
psa_status_t m3351_crypto_free(void);

/* ========================================================================= */
/* Hash Driver Entry Points                                                  */
/* ========================================================================= */

psa_status_t m3351_transparent_hash_setup(
    m3351_hash_operation_t *operation,
    psa_algorithm_t alg);

psa_status_t m3351_transparent_hash_clone(
    const m3351_hash_operation_t *source_operation,
    m3351_hash_operation_t *target_operation);

psa_status_t m3351_transparent_hash_update(
    m3351_hash_operation_t *operation,
    const uint8_t *input,
    size_t input_length);

psa_status_t m3351_transparent_hash_finish(
    m3351_hash_operation_t *operation,
    uint8_t *hash,
    size_t hash_size,
    size_t *hash_length);

psa_status_t m3351_transparent_hash_abort(
    m3351_hash_operation_t *operation);

psa_status_t m3351_transparent_hash_compute(
    psa_algorithm_t alg,
    const uint8_t *input,
    size_t input_length,
    uint8_t *hash,
    size_t hash_size,
    size_t *hash_length);

/* ========================================================================= */
/* Cipher Driver Entry Points                                                */
/* ========================================================================= */

psa_status_t m3351_transparent_cipher_encrypt(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *iv,
    size_t iv_length,
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length);

psa_status_t m3351_transparent_cipher_decrypt(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length);

psa_status_t m3351_transparent_cipher_encrypt_setup(
    m3351_cipher_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg);

psa_status_t m3351_transparent_cipher_decrypt_setup(
    m3351_cipher_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg);

psa_status_t m3351_transparent_cipher_set_iv(
    m3351_cipher_operation_t *operation,
    const uint8_t *iv,
    size_t iv_length);

psa_status_t m3351_transparent_cipher_update(
    m3351_cipher_operation_t *operation,
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length);

psa_status_t m3351_transparent_cipher_finish(
    m3351_cipher_operation_t *operation,
    uint8_t *output,
    size_t output_size,
    size_t *output_length);

psa_status_t m3351_transparent_cipher_abort(
    m3351_cipher_operation_t *operation);

#ifdef __cplusplus
}
#endif

#endif /* M3351_CRYPTO_DRIVER_H */
