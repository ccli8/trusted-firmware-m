#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
# Copyright (c) 2023, Nuvoton Technology Corp. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Make FLIH IRQ test as the default IRQ test on M3351
set(TEST_NS_SLIH_IRQ                  OFF   CACHE BOOL    "Whether to build NS regression Second-Level Interrupt Handling tests")

# The plat_test.c code/data (and their dependencies) are not fully allocated
# at UNPRIV. Disable this test for TF-M isolation level 2 temporarily.
if(TFM_ISOLATION_LEVEL EQUAL 1)
set(PLATFORM_SLIH_IRQ_TEST_SUPPORT    ON)
set(PLATFORM_FLIH_IRQ_TEST_SUPPORT    ON)
else()
set(PLATFORM_SLIH_IRQ_TEST_SUPPORT    OFF)
set(PLATFORM_FLIH_IRQ_TEST_SUPPORT    OFF)
endif()

# TFM_S_FWU_TEST_1001/TFM_NS_FWU_TEST_1001 rely on confirmed active image
set(MCUBOOT_CONFIRM_IMAGE               ON          CACHE BOOL      "Whether to confirm the image if REVERT is supported in MCUboot" FORCE)
