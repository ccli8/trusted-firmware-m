/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cmsis.h"
#include "region.h"
#include "flash_map/flash_map.h"
#include "flash_map_backend/flash_map_backend.h"
#include "bootutil/bootutil_log.h"
#include "boot_hal.h"
#include "Driver_Flash.h"

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
