/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CONFIG_TFM_TARGET_H__
#define __CONFIG_TFM_TARGET_H__

/* The maximum asset size to be stored in the Internal Trust Storage area. */
#if !NU_DONT_ORIDE_ITS_PS_ASSET
#if ITS_MAX_ASSET_SIZE != 1536
#pragma message("ITS_MAX_ASSET_SIZE is redefined to 1536.")
#undef ITS_MAX_ASSET_SIZE
#endif
#define ITS_MAX_ASSET_SIZE      1536
#endif

/* The maximum number of assets to be stored in the Internal Trust Storage area. */
#if !NU_DONT_ORIDE_ITS_PS_ASSET
#if ITS_NUM_ASSETS != 30
#pragma message("ITS_NUM_ASSETS is redefined to 30.")
#undef ITS_NUM_ASSETS
#endif
#define ITS_NUM_ASSETS          30
#endif

/* The maximum asset size to be stored in the Protected Storage area. */
#if !NU_DONT_ORIDE_ITS_PS_ASSET
#if PS_MAX_ASSET_SIZE != 1536
#pragma message("PS_MAX_ASSET_SIZE is redefined to 1536.")
#undef PS_MAX_ASSET_SIZE
#endif
#define PS_MAX_ASSET_SIZE       1536
#else
/* Defaulting to e.g. 2048 and its_flash_fs_validate_config() failure
 *
 * With PS on embedded flash and its block size being only 2KiB on M2354, larger
 * PS_MAX_ASSET_SIZE can cause its_flash_fs_validate_config() to fail. For example,
 * the precondition max file size (derived from PS_MAX_ASSET_SIZE) < flash block size
 * can break. Check PS_MAX_OBJECT_SIZE for details.
 */
#if PS_MAX_ASSET_SIZE != 1536
#pragma message("PS_MAX_ASSET_SIZE is redefined to 1536 to pass its_flash_fs_validate_config().")
#undef PS_MAX_ASSET_SIZE
#endif
#define PS_MAX_ASSET_SIZE       1536
#endif

/* The maximum number of assets to be stored in the Protected Storage area. */
#if !NU_DONT_ORIDE_ITS_PS_ASSET
#if PS_NUM_ASSETS != 45
#pragma message("PS_NUM_ASSETS is redefined to 45.")
#undef PS_NUM_ASSETS
#endif
#define PS_NUM_ASSETS           45
#endif

/* Enlarge dedicated heap more for mbedtls_calloc()
 *
 * Meet PSA_ERROR_INSUFFICIENT_MEMORY/MBEDTLS_ERR_MPI_ALLOC_FAILED error in mbedtls RSA/ECDSA.
 * Check mbedtls_memory_buffer_alloc_init() in below:
 * trusted-firmware-m/secure_fw/partitions/crypto/crypto_init.c
 */
#if CRYPTO_ENGINE_BUF_SIZE != 0x4000
#pragma message("CRYPTO_ENGINE_BUF_SIZE is redefined to 0x4000.")
#undef CRYPTO_ENGINE_BUF_SIZE
#endif
#define CRYPTO_ENGINE_BUF_SIZE  0x4000

/* Without H/W entropy like TRNG, use stored NV seed to provide entropy */
#if CRYPTO_HW_ACCELERATOR
#if CRYPTO_NV_SEED
#pragma message("CRYPTO_NV_SEED is redefined to 0.")
#undef CRYPTO_NV_SEED
#endif
#define CRYPTO_NV_SEED          0
#endif

#endif /* __CONFIG_TFM_TARGET_H__ */
