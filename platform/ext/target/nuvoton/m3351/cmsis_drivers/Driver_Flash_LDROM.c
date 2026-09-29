/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
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
    0, /* event_ready */
    2, /* data_width = 32-bit */
    1  /* erase_chip */
};

static ARM_FLASH_INFO flash_info = {
    .sector_info = NULL,
    .sector_count = FMC_LDROM_SIZE / FMC_FLASH_PAGE_SIZE,
    .sector_size = FMC_FLASH_PAGE_SIZE,
    .page_size = FMC_FLASH_PAGE_SIZE,
    .program_unit = 8,
    .erased_value = 0xFF
};

static int32_t is_range_valid(uint32_t addr, uint32_t size)
{
    if ((addr > FMC_LDROM_SIZE) || (size > (FMC_LDROM_SIZE - addr))) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    return ARM_DRIVER_OK;
}

static ARM_DRIVER_VERSION ARM_Flash_LDROM_GetVersion(void)
{
    return driver_version;
}

static ARM_FLASH_CAPABILITIES ARM_Flash_LDROM_GetCapabilities(void)
{
    return driver_capabilities;
}

static int32_t ARM_Flash_LDROM_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
    (void)cb_event;

    SYS_UnlockReg();
    FMC_Open();
    FMC_ENABLE_LD_UPDATE();

    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_LDROM_Uninitialize(void)
{
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_LDROM_PowerControl(ARM_POWER_STATE state)
{
    return state == ARM_POWER_FULL ? ARM_DRIVER_OK :
                                     ARM_DRIVER_ERROR_UNSUPPORTED;
}

static int32_t ARM_Flash_LDROM_ReadData(uint32_t addr, void *data,
                                        uint32_t cnt)
{
    uint32_t byte_count = cnt * sizeof(uint32_t);

    if (is_range_valid(addr, byte_count) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    memcpy(data, (const void *)(FMC_LDROM_BASE + addr), byte_count);
    return cnt;
}

static int32_t ARM_Flash_LDROM_ProgramData(uint32_t addr, const void *data,
                                           uint32_t cnt)
{
    const uint8_t *bytes = data;
    uint32_t byte_count = cnt * sizeof(uint32_t);
    uint32_t index;
    uint32_t u32Data0, u32Data1;

    if ((is_range_valid(addr, byte_count) != ARM_DRIVER_OK) ||
        ((addr % flash_info.program_unit) != 0) ||
        ((byte_count % flash_info.program_unit) != 0)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    SYS_UnlockReg();
    FMC_Open();
    FMC_ENABLE_LD_UPDATE();
    FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;

    for (index = 0; index < byte_count; index += 8) {
        memcpy(&u32Data0, bytes + index, sizeof(uint32_t));
        memcpy(&u32Data1, bytes + index + 4, sizeof(uint32_t));
        if (FMC_Write8Bytes(FMC_LDROM_BASE + addr + index, u32Data0, u32Data1) != FMC_OK) {
            return ARM_DRIVER_ERROR;
        }
        if (*(const uint32_t *)(FMC_LDROM_BASE + addr + index) != u32Data0 ||
            *(const uint32_t *)(FMC_LDROM_BASE + addr + index + 4) != u32Data1) {
            return ARM_DRIVER_ERROR;
        }
    }

    return cnt;
}

static int32_t ARM_Flash_LDROM_EraseSector(uint32_t addr)
{
    if ((is_range_valid(addr, flash_info.sector_size) != ARM_DRIVER_OK) ||
        ((addr % flash_info.sector_size) != 0)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    SYS_UnlockReg();
    FMC_Open();
    FMC_ENABLE_LD_UPDATE();
    FMC->ISPSTS |= FMC_ISPSTS_ISPFF_Msk;

    if (FMC_Erase(FMC_LDROM_BASE + addr) != FMC_OK) {
        return ARM_DRIVER_ERROR;
    }
    return ARM_DRIVER_OK;
}

static int32_t ARM_Flash_LDROM_EraseChip(void)
{
    uint32_t addr;

    SYS_UnlockReg();
    FMC_Open();
    FMC_ENABLE_LD_UPDATE();

    for (addr = 0; addr < FMC_LDROM_SIZE; addr += flash_info.sector_size) {
        FMC_Erase(FMC_LDROM_BASE + addr);
    }

    return ARM_DRIVER_OK;
}

static ARM_FLASH_STATUS ARM_Flash_LDROM_GetStatus(void)
{
    return flash_status;
}

static ARM_FLASH_INFO *ARM_Flash_LDROM_GetInfo(void)
{
    return &flash_info;
}

ARM_DRIVER_FLASH Driver_FLASH1 = {
    ARM_Flash_LDROM_GetVersion,
    ARM_Flash_LDROM_GetCapabilities,
    ARM_Flash_LDROM_Initialize,
    ARM_Flash_LDROM_Uninitialize,
    ARM_Flash_LDROM_PowerControl,
    ARM_Flash_LDROM_ReadData,
    ARM_Flash_LDROM_ProgramData,
    ARM_Flash_LDROM_EraseSector,
    ARM_Flash_LDROM_EraseChip,
    ARM_Flash_LDROM_GetStatus,
    ARM_Flash_LDROM_GetInfo
};
