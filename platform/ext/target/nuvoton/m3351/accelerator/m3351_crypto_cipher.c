/*
 * Copyright (c) 2024-2025 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "m3351_crypto_driver.h"
#include <string.h>
#include <stdbool.h>

static psa_status_t m3351_cipher_to_psa_status(int err)
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

static bool m3351_is_supported_cipher_alg(psa_algorithm_t alg)
{
    return (alg == PSA_ALG_ECB_NO_PADDING ||
            alg == PSA_ALG_CBC_NO_PADDING ||
            alg == PSA_ALG_CBC_PKCS7 ||
            alg == PSA_ALG_CTR);
}

psa_status_t m3351_transparent_cipher_encrypt_setup(
    m3351_cipher_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg)
{
    if (operation == NULL || attributes == NULL || key_buffer == NULL) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES ||
        !m3351_is_supported_cipher_alg(alg)) {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    size_t key_bits = psa_get_key_bits(attributes);
    if (key_bits != 128 && key_bits != 192 && key_bits != 256) {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    if (key_buffer_size < (key_bits / 8)) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    memset(operation, 0, sizeof(m3351_cipher_operation_t));
    operation->alg = alg;
    operation->enc_dec = 1;

    int ret = m3351_native_aes_setkey(&operation->native_ctx, key_buffer, key_bits);
    return m3351_cipher_to_psa_status(ret);
}

psa_status_t m3351_transparent_cipher_decrypt_setup(
    m3351_cipher_operation_t *operation,
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg)
{
    if (operation == NULL || attributes == NULL || key_buffer == NULL) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (psa_get_key_type(attributes) != PSA_KEY_TYPE_AES ||
        !m3351_is_supported_cipher_alg(alg)) {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    size_t key_bits = psa_get_key_bits(attributes);
    if (key_bits != 128 && key_bits != 192 && key_bits != 256) {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    if (key_buffer_size < (key_bits / 8)) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    memset(operation, 0, sizeof(m3351_cipher_operation_t));
    operation->alg = alg;
    operation->enc_dec = 0;

    int ret = m3351_native_aes_setkey(&operation->native_ctx, key_buffer, key_bits);
    return m3351_cipher_to_psa_status(ret);
}

psa_status_t m3351_transparent_cipher_set_iv(
    m3351_cipher_operation_t *operation,
    const uint8_t *iv,
    size_t iv_length)
{
    if (operation == NULL || (iv_length > 0 && iv == NULL)) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (operation->alg == PSA_ALG_ECB_NO_PADDING) {
        return PSA_ERROR_NOT_SUPPORTED;
    }

    if (iv_length != 16) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if (operation->iv_set) {
        return PSA_ERROR_BAD_STATE;
    }

    memcpy(operation->iv, iv, 16);
    operation->iv_length = 16;
    operation->iv_set = 1;

    return PSA_SUCCESS;
}

psa_status_t m3351_transparent_cipher_update(
    m3351_cipher_operation_t *operation,
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
    if (operation == NULL || (input_length > 0 && input == NULL) ||
        (output_size > 0 && output == NULL) || output_length == NULL) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    *output_length = 0;
    if (input_length == 0) {
        return PSA_SUCCESS;
    }

    if (operation->alg != PSA_ALG_ECB_NO_PADDING && !operation->iv_set) {
        return PSA_ERROR_BAD_STATE;
    }

    if (operation->alg == PSA_ALG_CTR) {
        if (output_size < input_length) {
            return PSA_ERROR_BUFFER_TOO_SMALL;
        }

        int ret = m3351_native_aes_crypt_ctr(&operation->native_ctx, input_length,
                                             operation->iv, input, output);
        if (ret != M3351_CRYPTO_SUCCESS) {
            return m3351_cipher_to_psa_status(ret);
        }
        *output_length = input_length;
        return PSA_SUCCESS;
    }

    bool is_cbc_pkcs7_decrypt = (operation->alg == PSA_ALG_CBC_PKCS7 &&
                                 operation->enc_dec == 0);

    /* Block ciphers: ECB and CBC */
    size_t total_avail = operation->buffer_len + input_length;
    size_t bytes_to_output;

    if (is_cbc_pkcs7_decrypt) {
        if (total_avail <= 16) {
            bytes_to_output = 0;
        } else {
            bytes_to_output = ((total_avail - 1) / 16) * 16;
        }
    } else {
        bytes_to_output = (total_avail / 16) * 16;
    }

    if (bytes_to_output > 0) {
        if (output == NULL) {
            return PSA_ERROR_INVALID_ARGUMENT;
        }
        if (output_size < bytes_to_output) {
            return PSA_ERROR_BUFFER_TOO_SMALL;
        }
    }

    size_t out_ofs = 0;
    size_t in_ofs = 0;

    if (is_cbc_pkcs7_decrypt) {
        /* If there's not enough data to output anything, just buffer it */
        if (input_length <= 16 - operation->buffer_len) {
            memcpy(&operation->buffer[operation->buffer_len], input, input_length);
            operation->buffer_len += input_length;
            *output_length = 0;
            return PSA_SUCCESS;
        }

        /* If buffer has data, process it first to empty it */
        if (operation->buffer_len != 0) {
            size_t copy_len = 16 - operation->buffer_len;
            if (copy_len > 0) {
                memcpy(&operation->buffer[operation->buffer_len], input, copy_len);
                in_ofs += copy_len;
                input_length -= copy_len;
            }

            int ret = m3351_native_aes_crypt_cbc(&operation->native_ctx, 0,
                                                 16, operation->iv,
                                                 operation->buffer,
                                                 &output[out_ofs]);
            if (ret != M3351_CRYPTO_SUCCESS) {
                return m3351_cipher_to_psa_status(ret);
            }
            out_ofs += 16;
            operation->buffer_len = 0;
        }

        /* Now operation->buffer_len == 0.
         * We must reserve the last 1..16 bytes into operation->buffer.
         */
        if (input_length > 0) {
            size_t copy_len = input_length % 16;
            if (copy_len == 0) {
                copy_len = 16;
            }

            memcpy(operation->buffer, &input[in_ofs + input_length - copy_len], copy_len);
            operation->buffer_len = copy_len;
            input_length -= copy_len;
        }

        /* Process any remaining full blocks */
        if (input_length > 0) {
            int ret = m3351_native_aes_crypt_cbc(&operation->native_ctx, 0,
                                                 input_length, operation->iv,
                                                 &input[in_ofs],
                                                 &output[out_ofs]);
            if (ret != M3351_CRYPTO_SUCCESS) {
                return m3351_cipher_to_psa_status(ret);
            }
            out_ofs += input_length;
        }

        *output_length = out_ofs;
        return PSA_SUCCESS;
    }

    /* Fill and process leftover buffer if available */
    if (operation->buffer_len > 0) {
        size_t needed = 16 - operation->buffer_len;
        if (input_length >= needed) {
            memcpy(&operation->buffer[operation->buffer_len], input, needed);
            in_ofs += needed;
            input_length -= needed;
            operation->buffer_len = 0;

            int ret;
            if (operation->alg == PSA_ALG_ECB_NO_PADDING) {
                ret = m3351_native_aes_crypt_ecb(&operation->native_ctx,
                                                 operation->enc_dec,
                                                 operation->buffer,
                                                 &output[out_ofs]);
            } else {
                ret = m3351_native_aes_crypt_cbc(&operation->native_ctx,
                                                 operation->enc_dec,
                                                 16, operation->iv,
                                                 operation->buffer,
                                                 &output[out_ofs]);
            }

            if (ret != M3351_CRYPTO_SUCCESS) {
                return m3351_cipher_to_psa_status(ret);
            }
            out_ofs += 16;
        } else {
            memcpy(&operation->buffer[operation->buffer_len], input, input_length);
            operation->buffer_len += input_length;
            *output_length = 0;
            return PSA_SUCCESS;
        }
    }

    /* Process remaining whole blocks directly from input */
    size_t remaining_blocks = input_length / 16;
    if (remaining_blocks > 0) {
        size_t block_bytes = remaining_blocks * 16;
        int ret;

        if (operation->alg == PSA_ALG_ECB_NO_PADDING) {
            for (size_t b = 0; b < remaining_blocks; b++) {
                ret = m3351_native_aes_crypt_ecb(&operation->native_ctx,
                                                 operation->enc_dec,
                                                 &input[in_ofs + b * 16],
                                                 &output[out_ofs + b * 16]);
                if (ret != M3351_CRYPTO_SUCCESS) {
                    return m3351_cipher_to_psa_status(ret);
                }
            }
        } else {
            ret = m3351_native_aes_crypt_cbc(&operation->native_ctx,
                                             operation->enc_dec,
                                             block_bytes, operation->iv,
                                             &input[in_ofs],
                                             &output[out_ofs]);
            if (ret != M3351_CRYPTO_SUCCESS) {
                return m3351_cipher_to_psa_status(ret);
            }
        }

        out_ofs += block_bytes;
        in_ofs += block_bytes;
        input_length -= block_bytes;
    }

    /* Save trailing partial block */
    if (input_length > 0) {
        memcpy(operation->buffer, &input[in_ofs], input_length);
        operation->buffer_len = input_length;
    }

    *output_length = out_ofs;
    return PSA_SUCCESS;
}

psa_status_t m3351_transparent_cipher_finish(
    m3351_cipher_operation_t *operation,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
    if (operation == NULL || output_length == NULL) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    *output_length = 0;

    if (operation->alg == PSA_ALG_CTR) {
        memset(operation, 0, sizeof(m3351_cipher_operation_t));
        return PSA_SUCCESS;
    }

    if (operation->alg == PSA_ALG_ECB_NO_PADDING ||
        operation->alg == PSA_ALG_CBC_NO_PADDING) {
        if (operation->buffer_len != 0) {
            memset(operation, 0, sizeof(m3351_cipher_operation_t));
            return PSA_ERROR_INVALID_ARGUMENT;
        }
        memset(operation, 0, sizeof(m3351_cipher_operation_t));
        return PSA_SUCCESS;
    }

    if (operation->alg == PSA_ALG_CBC_PKCS7) {
        if (operation->enc_dec) {
            /* Encryption: add PKCS#7 padding */
            if (output_size < 16) {
                return PSA_ERROR_BUFFER_TOO_SMALL;
            }
            if (output == NULL) {
                return PSA_ERROR_INVALID_ARGUMENT;
            }
            uint8_t pad = (uint8_t)(16 - operation->buffer_len);
            for (size_t i = operation->buffer_len; i < 16; i++) {
                operation->buffer[i] = pad;
            }

            int ret = m3351_native_aes_crypt_cbc(&operation->native_ctx, 1, 16,
                                                 operation->iv,
                                                 operation->buffer, output);
            memset(operation, 0, sizeof(m3351_cipher_operation_t));
            if (ret != M3351_CRYPTO_SUCCESS) {
                return m3351_cipher_to_psa_status(ret);
            }
            *output_length = 16;
            return PSA_SUCCESS;
        } else {
            /* Decryption */
            if (operation->buffer_len != 16) {
                memset(operation, 0, sizeof(m3351_cipher_operation_t));
                return PSA_ERROR_INVALID_ARGUMENT;
            }

            uint8_t temp_block[16];
            int ret = m3351_native_aes_crypt_cbc(&operation->native_ctx, 0, 16,
                                                 operation->iv,
                                                 operation->buffer,
                                                 temp_block);
            memset(operation, 0, sizeof(m3351_cipher_operation_t));
            if (ret != M3351_CRYPTO_SUCCESS) {
                memset(temp_block, 0, sizeof(temp_block));
                return m3351_cipher_to_psa_status(ret);
            }

            /* Validate PKCS#7 padding */
            uint8_t pad = temp_block[15];
            if (pad == 0 || pad > 16) {
                memset(temp_block, 0, sizeof(temp_block));
                return PSA_ERROR_INVALID_PADDING;
            }
            for (size_t i = 16 - pad; i < 16; i++) {
                if (temp_block[i] != pad) {
                    memset(temp_block, 0, sizeof(temp_block));
                    return PSA_ERROR_INVALID_PADDING;
                }
            }

            size_t plain_len = 16 - pad;
            if (plain_len > 0) {
                if (output_size < plain_len) {
                    memset(temp_block, 0, sizeof(temp_block));
                    return PSA_ERROR_BUFFER_TOO_SMALL;
                }
                if (output == NULL) {
                    memset(temp_block, 0, sizeof(temp_block));
                    return PSA_ERROR_INVALID_ARGUMENT;
                }
                memcpy(output, temp_block, plain_len);
            }
            *output_length = plain_len;
            memset(temp_block, 0, sizeof(temp_block));
            return PSA_SUCCESS;
        }
    }

    memset(operation, 0, sizeof(m3351_cipher_operation_t));
    return PSA_ERROR_NOT_SUPPORTED;
}

psa_status_t m3351_transparent_cipher_abort(
    m3351_cipher_operation_t *operation)
{
    if (operation != NULL) {
        memset(operation, 0, sizeof(m3351_cipher_operation_t));
    }
    return PSA_SUCCESS;
}

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
    size_t *output_length)
{
    m3351_cipher_operation_t op;
    psa_status_t status;
    size_t update_len = 0, finish_len = 0;

    status = m3351_transparent_cipher_encrypt_setup(&op, attributes,
                                                    key_buffer, key_buffer_size,
                                                    alg);
    if (status != PSA_SUCCESS) {
        return status;
    }

    if (alg != PSA_ALG_ECB_NO_PADDING) {
        status = m3351_transparent_cipher_set_iv(&op, iv, iv_length);
        if (status != PSA_SUCCESS) {
            m3351_transparent_cipher_abort(&op);
            return status;
        }
    }

    status = m3351_transparent_cipher_update(&op, input, input_length,
                                             output, output_size, &update_len);
    if (status != PSA_SUCCESS) {
        m3351_transparent_cipher_abort(&op);
        return status;
    }

    status = m3351_transparent_cipher_finish(&op, &output[update_len],
                                             output_size - update_len,
                                             &finish_len);
    if (status != PSA_SUCCESS) {
        m3351_transparent_cipher_abort(&op);
        return status;
    }

    *output_length = update_len + finish_len;
    return PSA_SUCCESS;
}

psa_status_t m3351_transparent_cipher_decrypt(
    const psa_key_attributes_t *attributes,
    const uint8_t *key_buffer,
    size_t key_buffer_size,
    psa_algorithm_t alg,
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
    m3351_cipher_operation_t op;
    psa_status_t status;
    size_t update_len = 0, finish_len = 0;
    const uint8_t *actual_input = input;
    size_t actual_input_len = input_length;

    status = m3351_transparent_cipher_decrypt_setup(&op, attributes,
                                                    key_buffer, key_buffer_size,
                                                    alg);
    if (status != PSA_SUCCESS) {
        return status;
    }

    if (alg != PSA_ALG_ECB_NO_PADDING) {
        if (input_length < 16) {
            m3351_transparent_cipher_abort(&op);
            return PSA_ERROR_INVALID_ARGUMENT;
        }
        /* In PSA one-shot decrypt, IV is prepended to input */
        status = m3351_transparent_cipher_set_iv(&op, input, 16);
        if (status != PSA_SUCCESS) {
            m3351_transparent_cipher_abort(&op);
            return status;
        }
        actual_input += 16;
        actual_input_len -= 16;
    }

    status = m3351_transparent_cipher_update(&op, actual_input, actual_input_len,
                                             output, output_size, &update_len);
    if (status != PSA_SUCCESS) {
        m3351_transparent_cipher_abort(&op);
        return status;
    }

    status = m3351_transparent_cipher_finish(&op, &output[update_len],
                                             output_size - update_len,
                                             &finish_len);
    if (status != PSA_SUCCESS) {
        m3351_transparent_cipher_abort(&op);
        return status;
    }

    *output_length = update_len + finish_len;
    return PSA_SUCCESS;
}
