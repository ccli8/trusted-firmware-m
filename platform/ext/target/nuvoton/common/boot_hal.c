/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cmsis.h"
#include "region.h"
#include "target.h"
#include "flash_map/flash_map.h"
#include "bootutil/bootutil_log.h"
#include "boot_hal.h"
#include "Driver_Flash.h"

/* Flash device name must be specified by target */
extern ARM_DRIVER_FLASH FLASH_DEV_NAME;
#if NU_UPDATE_STAGE_SDH
extern ARM_DRIVER_FLASH SDH_FLASH_DEV_NAME;
#endif

REGION_DECLARE(Image$$, ER_DATA, $$Base)[];
REGION_DECLARE(Image$$, ARM_LIB_HEAP, $$ZI$$Limit)[];

__attribute__((naked)) void boot_clear_bl2_ram_area(void)
{
    __ASM volatile(
#ifndef __ICCARM__
        ".syntax unified                             \n"
#endif
        "movs    r0, #0                              \n"
        "subs    %1, %1, %0                          \n"
        "Loop:                                       \n"
        "subs    %1, #4                              \n"
        "blt     Clear_done                          \n"
        "str     r0, [%0, %1]                        \n"
        "b       Loop                                \n"
        "Clear_done:                                 \n"
        "bx      lr                                  \n"
        :
        : "r" (REGION_NAME(Image$$, ER_DATA, $$Base)),
          "r" (REGION_NAME(Image$$, ARM_LIB_HEAP, $$ZI$$Limit))
        : "r0", "memory"
    );
}

/* bootloader platform-specific hw initialization */
int32_t boot_platform_init(void)
{
    int32_t result;

    result = FLASH_DEV_NAME.Initialize(NULL);
    if (result != ARM_DRIVER_OK) {
        return 1;
    }

#if NU_UPDATE_STAGE_SDH
    result = SDH_FLASH_DEV_NAME.Initialize(NULL);
    if (result != ARM_DRIVER_OK) {
        return 1;
    }
#endif

    return 0;
}

int flash_device_base(uint8_t fd_id, uintptr_t *ret)
{
    switch (fd_id) {
    case FLASH_DEVICE_ID :
        *ret = FLASH_DEVICE_BASE;
        break;

#if NU_UPDATE_STAGE_SDH
    case SDH_FLASH_DEVICE_ID:
        *ret = SDH_FLASH_BASE_ADDRESS;
        break;
#endif

    default:
        BOOT_LOG_ERR("invalid flash ID %d", fd_id);
        *ret = -1;
        return -1;
    }

    return 0;
}
