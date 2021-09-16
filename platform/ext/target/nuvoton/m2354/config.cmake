#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
# Copyright (c) 2022 Cypress Semiconductor Corporation (an Infineon company)
# or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(MCUBOOT_IMAGE_NUMBER    2      CACHE STRING    "Whether to combine S and NS into either 1 image, or sign each separately")
set(BL2_TRAILER_SIZE        0x800  CACHE STRING    "Trailer size")

# Platform-specific configurations
set(CONFIG_TFM_USE_TRUSTZONE ON)
set(TFM_MULTI_CORE_TOPOLOGY  OFF)

# Enable partitions for default build
# NOTE: Since TF-M v1.7, most partitions default to disabled.
set(TFM_PARTITION_INTERNAL_TRUSTED_STORAGE ON       CACHE BOOL      "Enable Internal Trusted Storage partition")
set(TFM_PARTITION_PROTECTED_STORAGE        ON       CACHE BOOL      "Enable Protected Storage partition")
set(TFM_PARTITION_CRYPTO                   ON       CACHE BOOL      "Enable Crypto partition")
set(TFM_PARTITION_INITIAL_ATTESTATION      ON       CACHE BOOL      "Enable Initial Attestation partition")
set(TFM_PARTITION_PLATFORM                 ON       CACHE BOOL      "Enable Platform partition")

set(CRYPTO_HW_ACCELERATOR               ON          CACHE BOOL      "Whether to enable the crypto hardware accelerator on supported platforms")
set(TFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH   ${CMAKE_CURRENT_SOURCE_DIR}/platform/ext/target/nuvoton/m2354/accelerator/mbedtls_accelerator_config.h  CACHE PATH "Config to append to standard Mbed Crypto config, used by platforms to cnfigure feature support")
set(CRYPTO_NV_SEED                      OFF         CACHE BOOL      "Use stored NV seed to provide entropy")

# PSA Firmware Update
set(PLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT  ON        CACHE BOOL      "Platform supports firmware update, such as network connectivities and bootloader support")
set(TFM_PARTITION_FIRMWARE_UPDATE       ON          CACHE BOOL      "Enable firmware update partition")
set(MCUBOOT_DATA_SHARING                ON          CACHE BOOL      "Add sharing of application specific data using the same shared data area as for the measured boot")

set(NU_HXT_PRESENT          OFF     CACHE BOOL      "Whether high-speed external crystal oscillator HXT is present")
set(NU_UPDATE_STAGE_SDH     ON      CACHE BOOL      "Whether enable SDH as update staging area")
set(NU_UPDATE_STAGE_FLASH   OFF     CACHE BOOL      "Whether enable embedded flash as update staging area")

set(TFM_PSA_API                         ON          CACHE BOOL      "Use PSA api (IPC mode) instead of secure library mode")
set(TFM_ISOLATION_LEVEL                 2           CACHE STRING    "Isolation level")
set(MCUBOOT_LOG_LEVEL                   "INFO"      CACHE STRING    "Level of logging to use for MCUboot [OFF, ERROR, WARNING, INFO, DEBUG]" FORCE)
