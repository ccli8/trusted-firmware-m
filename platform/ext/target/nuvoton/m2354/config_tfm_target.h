/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CONFIG_TFM_TARGET_H__
#define __CONFIG_TFM_TARGET_H__

/* Size of output buffer in platform service. */
#undef ITS_NUM_ASSETS
#define ITS_NUM_ASSETS       12

/* The maximum asset size to be stored in the Protected Storage area. */
#undef PS_MAX_ASSET_SIZE
#define PS_MAX_ASSET_SIZE    512

/* The maximum number of assets to be stored in the Protected Storage area. */
#undef PS_NUM_ASSETS
#define PS_NUM_ASSETS        12

/* Without H/W entropy like TRNG, use stored NV seed to provide entropy */
#if CRYPTO_HW_ACCELERATOR
#if CRYPTO_NV_SEED
#pragma message("CRYPTO_NV_SEED is redefined to 0.")
#undef CRYPTO_NV_SEED
#endif
#define CRYPTO_NV_SEED          0
#endif

#endif /* __CONFIG_TFM_TARGET_H__ */
