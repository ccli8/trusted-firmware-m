/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string.h>

#include "Driver_Flash.h"
#include "NuMicro.h"

#define ARM_FLASH_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

static ARM_FLASH_STATUS flash_status;

static const ARM_DRIVER_VERSION driver_version = {
    ARM_FLASH_API_VERSION,
    ARM_FLASH_DRV_VERSION
};

static const ARM_FLASH_CAPABILITIES driver_capabilities = {
    0,
    2,
    1
};

static ARM_FLASH_INFO flash_info = {
    .sector_info = NULL,
    .sector_count = DFMC_DATA_FLASH_SIZE / DFMC_FLASH_PAGE_SIZE,
    .sector_size = DFMC_FLASH_PAGE_SIZE,
    .page_size = DFMC_FLASH_PAGE_SIZE,
    .program_unit = sizeof(uint32_t),
    .erased_value = 0xFF
};

static int32_t is_range_valid(uint32_t addr, uint32_t size)
{
    if ((addr > DFMC_DATA_FLASH_SIZE) || (size > (DFMC_DATA_FLASH_SIZE - addr))) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    return ARM_DRIVER_OK;
}

static ARM_DRIVER_VERSION ARM_Flash_DFMC_GetVersion(void)
{
    return driver_version;
}

static ARM_FLASH_CAPABILITIES ARM_Flash_DFMC_GetCapabilities(void)
{
    return driver_capabilities;
}

static int32_t ARM_Flash_DFMC_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
    (void)cb_event;

    SYS_UnlockReg();
    DFMC_Open();
    DFMC_ENABLE_UPDATE();

    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_DFMC_Uninitialize(void)
{
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_DFMC_PowerControl(ARM_POWER_STATE state)
{
    return state == ARM_POWER_FULL ? ARM_DRIVER_OK :
                                     ARM_DRIVER_ERROR_UNSUPPORTED;
}

static int32_t ARM_Flash_DFMC_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    uint32_t *dest = (uint32_t *)data;
    uint32_t byte_count = cnt * sizeof(uint32_t);
    uint32_t index;

    if (is_range_valid(addr, byte_count) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    SYS_UnlockReg();
    DFMC_ENABLE_UPDATE();
    for (index = 0; index < cnt; index++) {
        dest[index] = DFMC_Read(DFMC_DATA_FLASH_BASE + addr + (index * sizeof(uint32_t)));
    }
    return cnt;
}

static int32_t ARM_Flash_DFMC_ProgramData(uint32_t addr, const void *data,
                                          uint32_t cnt)
{
    const uint8_t *bytes = data;
    uint32_t byte_count = cnt * sizeof(uint32_t);
    uint32_t index;
    uint32_t value;

    if ((is_range_valid(addr, byte_count) != ARM_DRIVER_OK) ||
        ((addr % flash_info.program_unit) != 0) ||
        ((byte_count % flash_info.program_unit) != 0)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    for (index = 0; index < byte_count; index += sizeof(uint32_t)) {
        memcpy(&value, bytes + index, sizeof(value));
        SYS_UnlockReg();
        DFMC_ENABLE_UPDATE();
        if (DFMC_Write(DFMC_DATA_FLASH_BASE + addr + index, value) != 0) {
            return ARM_DRIVER_ERROR;
        }
    }

    return cnt;
}

static int32_t ARM_Flash_DFMC_EraseSector(uint32_t addr)
{
    if ((is_range_valid(addr, flash_info.sector_size) != ARM_DRIVER_OK) ||
        ((addr % flash_info.sector_size) != 0)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    SYS_UnlockReg();
    DFMC_ENABLE_UPDATE();
    return DFMC_Erase(DFMC_DATA_FLASH_BASE + addr) == 0 ?
           ARM_DRIVER_OK : ARM_DRIVER_ERROR;
}

static int32_t ARM_Flash_DFMC_EraseChip(void)
{
    uint32_t addr;

    for (addr = 0; addr < DFMC_DATA_FLASH_SIZE; addr += flash_info.sector_size) {
        if (ARM_Flash_DFMC_EraseSector(addr) != ARM_DRIVER_OK) {
            return ARM_DRIVER_ERROR;
        }
    }

    return ARM_DRIVER_OK;
}

static ARM_FLASH_STATUS ARM_Flash_DFMC_GetStatus(void)
{
    return flash_status;
}

static ARM_FLASH_INFO *ARM_Flash_DFMC_GetInfo(void)
{
    return &flash_info;
}

ARM_DRIVER_FLASH Driver_FLASH2 = {
    ARM_Flash_DFMC_GetVersion,
    ARM_Flash_DFMC_GetCapabilities,
    ARM_Flash_DFMC_Initialize,
    ARM_Flash_DFMC_Uninitialize,
    ARM_Flash_DFMC_PowerControl,
    ARM_Flash_DFMC_ReadData,
    ARM_Flash_DFMC_ProgramData,
    ARM_Flash_DFMC_EraseSector,
    ARM_Flash_DFMC_EraseChip,
    ARM_Flash_DFMC_GetStatus,
    ARM_Flash_DFMC_GetInfo
};
