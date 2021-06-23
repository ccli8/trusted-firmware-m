/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CONFIG_TFM_TARGET_H__
#define __CONFIG_TFM_TARGET_H__

/* The maximum asset size to be stored in the Internal Trust Storage area. */
#if ITS_MAX_ASSET_SIZE != 512
#pragma message("ITS_MAX_ASSET_SIZE is redefined to 512.")
#undef ITS_MAX_ASSET_SIZE
#endif
#define ITS_MAX_ASSET_SIZE      512

/* The maximum number of assets to be stored in the Internal Trust Storage area. */
#if ITS_NUM_ASSETS != 15
#pragma message("ITS_NUM_ASSETS is redefined to 15.")
#undef ITS_NUM_ASSETS
#endif
#define ITS_NUM_ASSETS          15

/* The maximum asset size to be stored in the Protected Storage area. */
#if PS_MAX_ASSET_SIZE != 1536
#pragma message("PS_MAX_ASSET_SIZE is redefined to 1536.")
#undef PS_MAX_ASSET_SIZE
#endif
#define PS_MAX_ASSET_SIZE       1536

/* The maximum number of assets to be stored in the Protected Storage area. */
#if PS_NUM_ASSETS != 45
#pragma message("PS_NUM_ASSETS is redefined to 45.")
#undef PS_NUM_ASSETS
#endif
#define PS_NUM_ASSETS           45

/* Enlarge dedicated heap more for mbedtls_calloc()
 *
 * Meet PSA_ERROR_INSUFFICIENT_MEMORY/MBEDTLS_ERR_MPI_ALLOC_FAILED error in mbedtls RSA/ECDSA.
 * Check mbedtls_memory_buffer_alloc_init() in below:
 * trusted-firmware-m/secure_fw/partitions/crypto/crypto_init.c
 */
#if CRYPTO_ENGINE_BUF_SIZE != 0x2800
#pragma message("CRYPTO_ENGINE_BUF_SIZE is redefined to 0x2800.")
#undef CRYPTO_ENGINE_BUF_SIZE
#endif
#define CRYPTO_ENGINE_BUF_SIZE  0x2800

/* Without H/W entropy like TRNG, use stored NV seed to provide entropy */
#if CRYPTO_HW_ACCELERATOR
#if CRYPTO_NV_SEED
#pragma message("CRYPTO_NV_SEED is redefined to 0.")
#undef CRYPTO_NV_SEED
#endif
#define CRYPTO_NV_SEED          0
#endif

#endif /* __CONFIG_TFM_TARGET_H__ */
