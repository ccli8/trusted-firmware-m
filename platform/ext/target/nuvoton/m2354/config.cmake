#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
# Copyright (c) 2022 Cypress Semiconductor Corporation (an Infineon company)
# or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(MCUBOOT_IMAGE_NUMBER    1      CACHE STRING    "Whether to combine S and NS into either 1 image, or sign each separately")
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
