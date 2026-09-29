#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
# Copyright (c) 2023, Nuvoton Technology Corp. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(MCUBOOT_IMAGE_NUMBER    2           CACHE STRING    "Whether to combine S and NS into either 1 image, or sign each separately")
set(BL2_TRAILER_SIZE        0x800       CACHE STRING    "Trailer size")
set(MCUBOOT_SIGNATURE_TYPE  "EC-P256"   CACHE STRING    "Algorithm to use for signature validation [RSA-2048, RSA-3072, EC-P256, EC-P384]")
set(MCUBOOT_UPGRADE_STRATEGY "SWAP_USING_SCRATCH" CACHE STRING "Upgrade strategy for images")

# Platform-specific configurations
set(CONFIG_TFM_USE_TRUSTZONE          ON )
set(TFM_MULTI_CORE_TOPOLOGY           OFF)
set(NV_ENABLE_ETM                     OFF)
set(PLATFORM_DEFAULT_UART_STDOUT      OFF)
set(PLATFORM_DEFAULT_NV_COUNTERS      OFF)
set(PLATFORM_DEFAULT_ATTEST_HAL       OFF)
set(PLATFORM_DEFAULT_CRYPTO_KEYS      OFF)
set(PLATFORM_DEFAULT_PROVISIONING     OFF)

set(CRYPTO_HW_ACCELERATOR             OFF CACHE BOOL      "Whether to enable legacy crypto hardware accelerator" FORCE)
set(M3351_CRYPTO_HW_ACCELERATOR       ON  CACHE BOOL      "Whether to enable M3351 crypto hardware acceleration")
set(TFM_TF_PSA_CRYPTO_PLATFORM_EXTRA_CONFIG_PATH ${CMAKE_CURRENT_LIST_DIR}/accelerator/tf_psa_crypto_extra_config.h CACHE PATH "Config to append to standard TF-PSA-Crypto config")

# Firmware Update Partition
set(PLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT ON  CACHE BOOL   "Whether the platform has firmware update support")
set(TFM_PARTITION_FIRMWARE_UPDATE        ON  CACHE BOOL   "Enable firmware update partition")
set(MCUBOOT_DATA_SHARING                 ON  CACHE BOOL   "Enable Data Sharing")

