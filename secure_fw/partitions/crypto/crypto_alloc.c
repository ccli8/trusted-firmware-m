/*
 * Copyright (c) 2018-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "tfm_mbedcrypto_include.h"

#include "tfm_crypto_api.h"
#include "tfm_crypto_defs.h"

/**
 * \def TFM_CRYPTO_CONC_OPER_NUM
 *
 * \brief This is the default value for the maximum number of concurrent
 *        operations that can be active (allocated) at any time, supported
 *        by the implementation
 */
#ifndef TFM_CRYPTO_CONC_OPER_NUM
#define TFM_CRYPTO_CONC_OPER_NUM (8)
#endif

struct tfm_crypto_operation_s {
    uint32_t in_use;                /*!< Indicates if the operation is in use */
    int32_t owner;                  /*!< Indicates an ID of the owner of
                                     *   the context
                                     */
    enum tfm_crypto_operation_type type; /*!< Type of the operation */
    union {
        psa_cipher_operation_t cipher;    /*!< Cipher operation context */
        psa_mac_operation_t mac;          /*!< MAC operation context */
        psa_hash_operation_t hash;        /*!< Hash operation context */
        psa_key_derivation_operation_t key_deriv; /*!< Key derivation operation context */
        psa_aead_operation_t aead;        /*!< AEAD operation context */
    } operation;
};

static struct tfm_crypto_operation_s operation[TFM_CRYPTO_CONC_OPER_NUM] ={{0}};

/*
 * \brief Function used to clear the memory associated to a backend context
 *
 * \param[in] index Numerical index in the database of the backend contexts
 *
 * \return None
 *
 */
static void memset_operation_context(uint32_t index)
{
    uint32_t mem_size;

    uint8_t *mem_ptr = (uint8_t *) &(operation[index].operation);

    switch(operation[index].type) {
    case TFM_CRYPTO_CIPHER_OPERATION:
        mem_size = sizeof(psa_cipher_operation_t);
        break;
    case TFM_CRYPTO_MAC_OPERATION:
        mem_size = sizeof(psa_mac_operation_t);
        break;
    case TFM_CRYPTO_HASH_OPERATION:
        mem_size = sizeof(psa_hash_operation_t);
        break;
    case TFM_CRYPTO_KEY_DERIVATION_OPERATION:
        mem_size = sizeof(psa_key_derivation_operation_t);
        break;
    case TFM_CRYPTO_AEAD_OPERATION:
        mem_size = sizeof(psa_aead_operation_t);
        break;
    case TFM_CRYPTO_OPERATION_NONE:
    default:
        mem_size = 0;
        break;
    }

    /* Clear the contents of the backend context */
    (void)memset(mem_ptr, 0, mem_size);
}

/*!
 * \defgroup alloc Function that implement allocation and deallocation of
 *                 contexts to be stored in the secure world for multipart
 *                 operations
 */

/*!@{*/
psa_status_t tfm_crypto_init_alloc(void)
{
    /* Clear the contents of the local contexts */
    (void)memset(operation, 0, sizeof(operation));
    return PSA_SUCCESS;
}

psa_status_t tfm_crypto_operation_alloc(enum tfm_crypto_operation_type type,
                                        uint32_t *handle,
                                        void **ctx)
{
    uint32_t i = 0;
    int32_t partition_id = 0;
    psa_status_t status;

    status = tfm_crypto_get_caller_id(&partition_id);
    if (status != PSA_SUCCESS) {
        return status;
    }

    /* Handle must be initialised before calling a setup function */
    if (*handle != TFM_CRYPTO_INVALID_HANDLE) {
        return PSA_ERROR_BAD_STATE;
    }

    /* Init to invalid values */
    if (ctx == NULL) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }
    *ctx = NULL;

    for (i=0; i<TFM_CRYPTO_CONC_OPER_NUM; i++) {
        if (operation[i].in_use == TFM_CRYPTO_NOT_IN_USE) {
            operation[i].in_use = TFM_CRYPTO_IN_USE;
            operation[i].owner = partition_id;
            operation[i].type = type;
            *handle = i + 1;
            *ctx = (void *) &(operation[i].operation);
            return PSA_SUCCESS;
        }
    }

    return PSA_ERROR_NOT_PERMITTED;
}

psa_status_t tfm_crypto_operation_release(uint32_t *handle)
{
    uint32_t h_val = *handle;
    int32_t partition_id = 0;
    psa_status_t status;

    status = tfm_crypto_get_caller_id(&partition_id);
    if (status != PSA_SUCCESS) {
        return status;
    }

    if ( (h_val != TFM_CRYPTO_INVALID_HANDLE) &&
         (h_val <= TFM_CRYPTO_CONC_OPER_NUM) &&
         (operation[h_val - 1].in_use == TFM_CRYPTO_IN_USE) &&
         (operation[h_val - 1].owner == partition_id)) {

        memset_operation_context(h_val - 1);
        operation[h_val - 1].in_use = TFM_CRYPTO_NOT_IN_USE;
        operation[h_val - 1].type = TFM_CRYPTO_OPERATION_NONE;
        operation[h_val - 1].owner = 0;
        *handle = TFM_CRYPTO_INVALID_HANDLE;
        return PSA_SUCCESS;
    }

    return PSA_ERROR_INVALID_ARGUMENT;
}

psa_status_t tfm_crypto_operation_lookup(enum tfm_crypto_operation_type type,
                                         uint32_t handle,
                                         void **ctx)
{
    int32_t partition_id = 0;
    psa_status_t status;

    status = tfm_crypto_get_caller_id(&partition_id);
    if (status != PSA_SUCCESS) {
        return status;
    }

    if ( (handle != TFM_CRYPTO_INVALID_HANDLE) &&
         (handle <= TFM_CRYPTO_CONC_OPER_NUM) &&
         (operation[handle - 1].in_use == TFM_CRYPTO_IN_USE) &&
         (operation[handle - 1].type == type) &&
         (operation[handle - 1].owner == partition_id)) {

        *ctx = (void *) &(operation[handle - 1].operation);
        return PSA_SUCCESS;
    }

    return PSA_ERROR_BAD_STATE;
}

psa_status_t tfm_crypto_operation_handling(enum tfm_crypto_operation_type type,
                                    enum tfm_crypto_function_type function_type,
                                    uint32_t *handle,
                                    void **ctx)
{
    psa_status_t status = PSA_ERROR_CORRUPTION_DETECTED;

    /* Multipart context retrieving and handling if required */
    switch (function_type) {
    case TFM_CRYPTO_FUNCTION_TYPE_SETUP:
        /* Allocate the operation context in the secure world */
        status = tfm_crypto_operation_alloc(type,
                                            handle,
                                            ctx);
        break;
    case TFM_CRYPTO_FUNCTION_TYPE_LOOKUP:
        /* Look up the corresponding operation context */
        status = tfm_crypto_operation_lookup(type,
                                             *handle,
                                             ctx);
        break;
    /* All the other APIs don't deal with multipart */
    case TFM_CRYPTO_FUNCTION_TYPE_NON_MULTIPART:
        status = PSA_ERROR_INVALID_ARGUMENT;
    default:
        break;
    }

    return status;
}
/*!@}*/
