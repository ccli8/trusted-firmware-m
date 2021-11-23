#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
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
else()
set(PS_MAX_ASSET_SIZE       1536   CACHE STRING    "The maximum asset size to be stored in the Protected Storage area")
set(PS_NUM_ASSETS           45     CACHE STRING    "The maximum number of assets to be stored in the Protected Storage area")
set(ITS_MAX_ASSET_SIZE      1536   CACHE STRING    "The maximum asset size to be stored in the Internal Trusted Storage area")
set(ITS_NUM_ASSETS          30     CACHE STRING    "The maximum number of assets to be stored in the Internal Trusted Storage area")
endif()

set(CRYPTO_HW_ACCELERATOR               ON          CACHE BOOL      "Whether to enable the crypto hardware accelerator on supported platforms")
set(TFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH   ${CMAKE_CURRENT_SOURCE_DIR}/platform/ext/target/nuvoton/m2354/accelerator/mbedtls_accelerator_config.h  CACHE PATH "Config to append to standard Mbed Crypto config, used by platforms to cnfigure feature support")
set(CRYPTO_NV_SEED                      OFF         CACHE BOOL      "Use stored NV seed to provide entropy")

# Meet PSA_ERROR_INSUFFICIENT_MEMORY/MBEDTLS_ERR_MPI_ALLOC_FAILED error in mbedtls RSA/ECDSA. Enlarge dedicated heap more for mbedtls_calloc().
# Check mbedtls_memory_buffer_alloc_init() in below:
# trusted-firmware-m/secure_fw/partitions/crypto/crypto_init.c
set(CRYPTO_ENGINE_BUF_SIZE              0x4000      CACHE STRING    "Heap size for the crypto backend")

# PSA Firmware Update
set(TFM_PARTITION_FIRMWARE_UPDATE       ON          CACHE BOOL      "Enable firmware update partition")
set(MCUBOOT_DATA_SHARING                ON          CACHE BOOL      "Add sharing of application specific data using the same shared data area as for the measured boot")
# Change to "SWAP_USING_SCRATCH" for enabling firmware upgrade rollback
set(MCUBOOT_UPGRADE_STRATEGY            "OVERWRITE_ONLY"    CACHE STRING    "Upgrade strategy for images")
# BL2 can be unset yet at this stage. Default to enabled for below conditional code.
# Can override through command line option or earlier set.
set(BL2                                 ON          CACHE BOOL      "Whether to build BL2")
# Evaluate image trailer size for 'SWAP_USING_SCRATCH' upgrade strategy
#
# Check the link below for necessary trailer size:
# https://www.mcuboot.com/documentation/design/
#
# With the formula:
# Swap status (BOOT_MAX_IMG_SECTORS * min-write-size * 3)
#
# Where for the platform:
# BOOT_MAX_IMG_SECTORS = 512 (= 1MiB / 2KiB)
# min-write-size = 4 bytes (per flash_area_align())
# Swap status = 512 * 4 * 3 = 6KiB
#
# 6KiB plus other fields for image trailer plus TLV, we reserve 8KiB in total.
#
# Notes for above estimation:
# 1. In image signing such as bl2/ext/mcuboot/CMakeLists.txt, `--align` must fix to 4 and `--max-sectors` must specify as 512 to catch trailer size overrun error.
# 2. 2KiB is taken from smaller of internal/external Flash's sector sizes.
# 3. Continuing above, SDH Flash's sector size should have adapted to larger from 512 bytes.
# 4. BL2_TRAILER_SIZE must include TLV area, though not mentioned.
# 5. For consistency, BL2_TRAILER_SIZE doesn't change across 'OVERWRITE_ONLY' and "SWAP_USING_SCRATCH" upgrade strategies.
# 6. For consistency, (BL2_HEADER_SIZE + BL2_TRAILER_SIZE) doesn't change across w/ and w/o BL2.
if(BL2)
set(BL2_HEADER_SIZE                     0x1000 CACHE STRING "Header size")
set(BL2_TRAILER_SIZE                    0x2000 CACHE STRING "Trailer size")
set(MCUBOOT_ALIGN_VAL                   4      CACHE STRING "align option for mcuboot and build image with imgtool [1, 2, 4, 8, 16, 32]")
else()
# No header if no bootloader, but keep IMAGE_CODE_SIZE the same
set(BL2_HEADER_SIZE                     0x0    CACHE STRING "Header size")
set(BL2_TRAILER_SIZE                    0x3000 CACHE STRING "Trailer size")
endif()

set(NU_HXT_PRESENT          OFF     CACHE BOOL      "Whether high-speed external crystal oscillator HXT is present")
set(NU_UPDATE_STAGE_SDH     ON      CACHE BOOL      "Whether enable SDH as update staging area")
set(NU_UPDATE_STAGE_FLASH   OFF     CACHE BOOL      "Whether enable embedded flash as update staging area")

set(TFM_PSA_API                         ON          CACHE BOOL      "Use PSA api (IPC mode) instead of secure library mode")
set(TFM_ISOLATION_LEVEL                 2           CACHE STRING    "Isolation level")
set(MCUBOOT_LOG_LEVEL                   "INFO"      CACHE STRING    "Level of logging to use for MCUboot [OFF, ERROR, WARNING, INFO, DEBUG]" FORCE)
set(TFM_SPM_LOG_LEVEL                   TFM_SPM_LOG_LEVEL_INFO          CACHE STRING    "Set default SPM log level as INFO level" FORCE)
set(TFM_PARTITION_LOG_LEVEL             TFM_PARTITION_LOG_LEVEL_INFO    CACHE STRING    "Set default Secure Partition log level as INFO level" FORCE)

# Target board
set(NU_TGT_NUMAKER_M2354                ON          CACHE BOOL      "Build for NuMaker-M2354 board")
set(NU_TGT_NUMAKER_IOT_M2354            OFF         CACHE BOOL      "Build for NuMaker-IoT-M2354 board")
