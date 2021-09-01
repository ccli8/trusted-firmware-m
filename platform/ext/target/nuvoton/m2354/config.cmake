#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited. All rights reserved.
# Copyright (c) 2022 Cypress Semiconductor Corporation (an Infineon company)
# or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(MCUBOOT_IMAGE_NUMBER    2      CACHE STRING    "Whether to combine S and NS into either 1 image, or sign each separately")

# TFM_SP_PS_TEST test will break when PS_NUM_ASSETS is enlarged to some value.
# https://developer.trustedfirmware.org/T956
if (${TEST_S} OR ${TEST_NS})
set(PS_MAX_ASSET_SIZE       1536   CACHE STRING    "The maximum asset size to be stored in the Protected Storage area")
set(PS_NUM_ASSETS           10     CACHE STRING    "The maximum number of assets to be stored in the Protected Storage area")
set(ITS_MAX_ASSET_SIZE      512    CACHE STRING    "The maximum asset size to be stored in the Internal Trusted Storage area")
set(ITS_NUM_ASSETS          15     CACHE STRING    "The maximum number of assets to be stored in the Internal Trusted Storage area")
set(BL2_TRAILER_SIZE        0x800  CACHE STRING    "Trailer size")
else()
set(PS_MAX_ASSET_SIZE       1536   CACHE STRING    "The maximum asset size to be stored in the Protected Storage area")
set(PS_NUM_ASSETS           45     CACHE STRING    "The maximum number of assets to be stored in the Protected Storage area")
set(ITS_MAX_ASSET_SIZE      1536   CACHE STRING    "The maximum asset size to be stored in the Internal Trusted Storage area")
set(ITS_NUM_ASSETS          30     CACHE STRING    "The maximum number of assets to be stored in the Internal Trusted Storage area")
set(BL2_TRAILER_SIZE        0x800  CACHE STRING    "Trailer size")
endif()

set(CONFIG_TFM_USE_TRUSTZONE ON    CACHE BOOL      "Enable use of TrustZone to transition between NSPE and SPE")
set(TFM_MULTI_CORE_TOPOLOGY  OFF   CACHE BOOL      "Whether to build for a dual-cpu architecture")
set(CRYPTO_HW_ACCELERATOR               ON          CACHE BOOL      "Whether to enable the crypto hardware accelerator on supported platforms")
set(TFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH   ${CMAKE_CURRENT_SOURCE_DIR}/platform/ext/target/nuvoton/m2354/accelerator/mbedtls_accelerator_config.h  CACHE PATH "Config to append to standard Mbed Crypto config, used by platforms to cnfigure feature support")
set(CRYPTO_NV_SEED                      OFF         CACHE BOOL      "Use stored NV seed to provide entropy")

# Meet PSA_ERROR_INSUFFICIENT_MEMORY/MBEDTLS_ERR_MPI_ALLOC_FAILED error in mbedtls RSA/ECDSA. Enlarge dedicated heap more for mbedtls_calloc().
# Check mbedtls_memory_buffer_alloc_init() in below:
# trusted-firmware-m/secure_fw/partitions/crypto/crypto_init.c
set(CRYPTO_ENGINE_BUF_SIZE              0x4000      CACHE STRING    "Heap size for the crypto backend")

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

# Target board
set(NU_TGT_NUMAKER_M2354                ON          CACHE BOOL      "Build for NuMaker-M2354 board")
set(NU_TGT_NUMAKER_IOT_M2354            OFF         CACHE BOOL      "Build for NuMaker-IoT-M2354 board")
