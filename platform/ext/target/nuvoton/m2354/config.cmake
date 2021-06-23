#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(MCUBOOT_IMAGE_NUMBER    1      CACHE STRING    "Whether to combine S and NS into either 1 image, or sign each separately")
set(PS_MAX_ASSET_SIZE       1536   CACHE STRING    "The maximum asset size to be stored in the Protected Storage area")
set(PS_NUM_ASSETS           45     CACHE STRING    "The maximum number of assets to be stored in the Protected Storage area")
set(ITS_MAX_ASSET_SIZE      512    CACHE STRING    "The maximum asset size to be stored in the Internal Trusted Storage area")
set(ITS_NUM_ASSETS          15     CACHE STRING    "The maximum number of assets to be stored in the Internal Trusted Storage area")
set(BL2_TRAILER_SIZE        0x800  CACHE STRING    "Trailer size")

set(CRYPTO_HW_ACCELERATOR               ON          CACHE BOOL      "Whether to enable the crypto hardware accelerator on supported platforms")
set(TFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH   ${CMAKE_CURRENT_SOURCE_DIR}/platform/ext/target/nuvoton/m2354/accelerator/mbedtls_accelerator_config.h  CACHE PATH "Config to append to standard Mbed Crypto config, used by platforms to cnfigure feature support")
set(CRYPTO_NV_SEED                      OFF         CACHE BOOL      "Use stored NV seed to provide entropy")

# Meet PSA_ERROR_INSUFFICIENT_MEMORY/MBEDTLS_ERR_MPI_ALLOC_FAILED error in mbedtls ECDSA. Enlarge dedicated heap more for mbedtls_calloc().
# Check mbedtls_memory_buffer_alloc_init() in below:
# trusted-firmware-m/secure_fw/partitions/crypto/crypto_init.c
set(CRYPTO_ENGINE_BUF_SIZE              0x2800      CACHE STRING    "Heap size for the crypto backend")

set(NU_HXT_PRESENT          OFF     CACHE BOOL      "Whether high-speed external crystal oscillator HXT is present")
set(NU_SDH_CMSIS_FLASH      ON      CACHE BOOL      "Whether support SDH as CMSIS Flash driver")
