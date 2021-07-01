/*
 * Copyright (c) 2013-2018 ARM Limited. All rights reserved.
 * Copyright (c) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "Driver_Flash.h"
#include "NuMicro.h"
#include "RTE_Device.h"
#include "flash_layout.h"
#include "region_defs.h"

/* Implementation notes:
 * 1. Add cmake variable NU_SDH_CMSIS_FLASH to determine enable or not. Default is ON.
 * 2. SDH can only be secure for access to secure DMA buffer.
 * 3. Continuing above, NSPE cannot access the secure SDH.
 * 4. On enabled, tweak BSP SDH driver to go non-interrupt through NU_SDH_DIS_INTR.
 *    Secure interrupt needs change to partition manifest file.
 * 5. On enabled, cover upper layer missing doing initializing SDH for read/program/erase operations.
 * 6. On enabled, simulate dummy SD card on probing no SD card for MCUboot because MCUboot will panic on read failure.
 * 7. On enabled, emulate flash program attribute: Transition to 1 from 0 won't occur.
 * 8. With DMA buffer for intermediate, read/program are not limited to sector-aligned.
 */

/* Cover upper layer missing doing initializing SDH for read/program/erase operations */
#ifndef NU_SDH_INIT_GUARD
#define NU_SDH_INIT_GUARD           0
#endif

/* Implement dummy SD card on probe failure */
#ifndef NU_SDH_DUMMY_ON_ABSENT
#define NU_SDH_DUMMY_ON_ABSENT      0
#endif

/* Emulate flash program attribute: 0 doesn't program to 1 */
#ifndef NU_SDH_FLASH_PROG_ATR
#define NU_SDH_FLASH_PROG_ATR       1
#endif

#ifndef ARG_UNUSED
#define ARG_UNUSED(arg)  ((void)arg)
#endif

/* Driver version */
#define ARM_FLASH_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

/**
 * Data width values for ARM_FLASH_CAPABILITIES::data_width
 * \ref ARM_FLASH_CAPABILITIES
 */
 enum {
    DATA_WIDTH_8BIT   = 0u,
    DATA_WIDTH_16BIT,
    DATA_WIDTH_32BIT,
    DATA_WIDTH_ENUM_SIZE
};

static const uint32_t data_width_byte[DATA_WIDTH_ENUM_SIZE] = {
    sizeof(uint8_t),
    sizeof(uint16_t),
    sizeof(uint32_t),
};

#if NU_SDH_INIT_GUARD
#define SDH_FLASH0_INIT_GUARD()                         \
    do {                                                \
        if (!SDH_FLASH0_DEV.data->sector_count) {       \
            ARM_SDH_Flash0_Initialize(0);               \
        }                                               \
    } while(0)
#else
#define SDH_FLASH0_INIT_GUARD()
#endif

/*
 * ARM FLASH device structure
 */
struct sdh_flash_dev_t
{
    SDH_T *         sdh_base;       /*!< SDH base address */
    uint32_t        rsetidx;        /*!< SDH reset index */
    uint32_t        clkidx;         /*!< SDH clock index */
    uint32_t        clksrc;         /*!< SDH clock source */
    uint32_t        clkdiv;         /*!< SDH clock divider */
    SDH_INFO_T *    sdh_info;       /*!< SD card information */
    struct _ARM_FLASH_INFO *data;   /*!< Flash information */
};

/* Flash Status */
static ARM_FLASH_STATUS FlashStatus = {0, 0, 0};

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion =
{
    ARM_FLASH_API_VERSION,
    ARM_FLASH_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_FLASH_CAPABILITIES DriverCapabilities =
{
    0, /* Signal Flash Ready event = No */
    0, /* Data width = 0:8-bit, 1:16-bit, 2:32-bit */
    0  /* Supports EraseChip operation = No */
};

/* SD card sector size */
#define SDH_SECTOR_SIZE     512

/* SDH DMA compatible buffer
 * 
 * SDH DMA buffer location requires to be:
 * (1) Word-aligned
 * (2) Located in 0x2xxxxxxx/0x3xxxxxxx region. Check linker files to ensure global/static
 *     variables are placed in this region.
 *
 * SDH DMA buffer size DMA_BUF_SIZE must be a multiple of 512-byte block size.
 * Its value is estimated to trade memory footprint off against performance.
 *
 */
#define DMA_BUFF_SIZE       SDH_SECTOR_SIZE
__attribute__((aligned(4))) static uint8_t dma_buff[DMA_BUFF_SIZE];

static int32_t is_range_valid(struct sdh_flash_dev_t *flash_dev,
                              uint32_t addr,
                              uint32_t size)
{
    uint32_t sector_size = flash_dev->data->sector_size;
    uint32_t sector_count = flash_dev->data->sector_count;

    return !((sector_size == SDH_SECTOR_SIZE) &&
        ((addr + size) / SDH_SECTOR_SIZE <= sector_count));
}

#if (RTE_SDH_FLASH0)
/* 'ARM_FLASH_INFO' is defined to be const, but we need to probe and fix `sector_count` run-time.
 * Change to non-const 'struct _ARM_FLASH_INFO' instead. */
static struct _ARM_FLASH_INFO SDH_FLASH0_DEV_DATA =
{
    .sector_info  = NULL,               // Uniform sector layout
    .sector_count = 0,                  // Will fix in SD probe
    .sector_size  = SDH_SECTOR_SIZE,
    .page_size    = 4,
    .program_unit = 4,
    .erased_value = 0xFF
};

static struct sdh_flash_dev_t SDH_FLASH0_DEV =
{
    .sdh_base   = SDH0_S,                   // NOTE: Can only be secure for secure DMA buffer
    .rsetidx    = SDH0_RST,
    .clkidx     = SDH0_MODULE,
    .clksrc     = CLK_CLKSEL0_SDH0SEL_HCLK,
    .clkdiv     = CLK_CLKDIV0_SDH0(2),
    .sdh_info   = &SD0,
    .data       = &SDH_FLASH0_DEV_DATA
};

/*
 * Functions
 */

static ARM_DRIVER_VERSION ARM_SDH_Flash0_GetVersion(void)
{
    return DriverVersion;
}

static ARM_FLASH_CAPABILITIES ARM_SDH_Flash0_GetCapabilities(void)
{
    return DriverCapabilities;
}

static int32_t ARM_SDH_Flash0_Initialize(ARM_Flash_SignalEvent_t cb_event)
{
    ARG_UNUSED(cb_event);

    if (DriverCapabilities.data_width >= DATA_WIDTH_ENUM_SIZE) {
        return ARM_DRIVER_ERROR;
    }

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Set multi-function pin for SDH */
    /* CD: PB12(9) */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~SYS_GPB_MFPH_PB12MFP_Msk)) | SD0_nCD_PB12;

    /* CLK: PB1(3), PE6(3) */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB1MFP_Msk)) | SD0_CLK_PB1;
    //SYS->GPE_MFPL = (SYS->GPE_MFPL & (~SYS_GPE_MFPL_PE6MFP_Msk)) | SD0_CLK_PE6;

    /* CMD: PB0(3), PE7(3) */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB0MFP_Msk)) | SD0_CMD_PB0;
    //SYS->GPE_MFPL = (SYS->GPE_MFPL & (~SYS_GPE_MFPL_PE7MFP_Msk)) | SD0_CMD_PE7;

    /* D0: PB2(3), PE2(3) */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB2MFP_Msk)) | SD0_DAT0_PB2;
    //SYS->GPE_MFPL = (SYS->GPE_MFPL & (~SYS_GPE_MFPL_PE2MFP_Msk)) | SD0_DAT0_PE2;

    /* D1: PB3(3), PE3(3) */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB3MFP_Msk)) | SD0_DAT1_PB3;
    //SYS->GPE_MFPL = (SYS->GPE_MFPL & (~SYS_GPE_MFPL_PE3MFP_Msk)) | SD0_DAT1_PE3;

    /* D2: PB4(3), PE4(3) */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB4MFP_Msk)) | SD0_DAT2_PB4;
    //SYS->GPE_MFPL = (SYS->GPE_MFPL & (~SYS_GPE_MFPL_PE4MFP_Msk)) | SD0_DAT2_PE4;

    /* D3: PB5(3)-, PE5(3) */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB5MFP_Msk)) | SD0_DAT3_PB5;
    //SYS->GPE_MFPL = (SYS->GPE_MFPL & (~SYS_GPE_MFPL_PE5MFP_Msk)) | SD0_DAT3_PE5;

    SYS_ResetModule(SDH_FLASH0_DEV.rsetidx);

    CLK_SetModuleClock(SDH_FLASH0_DEV.clkidx, SDH_FLASH0_DEV.clksrc, SDH_FLASH0_DEV.clkdiv);

    CLK_EnableModuleClock(SDH_FLASH0_DEV.clkidx);

    SDH_Open(SDH_FLASH0_DEV.sdh_base, CardDetect_From_GPIO);
    if (SDH_Probe(SDH_FLASH0_DEV.sdh_base) != 0 ||
        SDH_FLASH0_DEV.sdh_info->CardType == SDH_TYPE_UNKNOWN ||
        SDH_FLASH0_DEV.sdh_info->totalSectorN == 0) {
#if !NU_SDH_DUMMY_ON_ABSENT
        return ARM_DRIVER_ERROR;
#endif
    }

    SDH_FLASH0_DEV.data->sector_count = SDH_FLASH0_DEV.sdh_info->totalSectorN;

    return ARM_DRIVER_OK;
}

static int32_t ARM_SDH_Flash0_Uninitialize(void)
{
    CLK_DisableModuleClock(SDH_FLASH0_DEV.clkidx);

    return ARM_DRIVER_OK;
}

static int32_t ARM_SDH_Flash0_PowerControl(ARM_POWER_STATE state)
{
    switch(state)
    {
        case ARM_POWER_FULL:
            /* Nothing to be done */
            return ARM_DRIVER_OK;
            break;

        case ARM_POWER_OFF:
        case ARM_POWER_LOW:
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

static int32_t ARM_SDH_Flash0_ReadData(uint32_t addr, void *data, uint32_t cnt)
{
    /* SDH initialization guard */
    SDH_FLASH0_INIT_GUARD();

    uint32_t size = cnt*data_width_byte[DriverCapabilities.data_width];

    /* Check probe OK */
    if (!SDH_FLASH0_DEV.data->sector_count) {
#if NU_SDH_DUMMY_ON_ABSENT
        memset(data, SDH_FLASH0_DEV.data->erased_value, size);
        return cnt;
#else
        return ARM_DRIVER_ERROR;
#endif
    }

    /* Check validity of access address */
    if (is_range_valid(&SDH_FLASH0_DEV, addr, size) != 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* For easy implementation, we always use SDH DMA-compatible buffer for intermediate. */
    uint8_t *b_pos = data;
    uint32_t addr_pos = addr;
    uint32_t rmn = size;

    while (rmn) {
        uint32_t sector_offset = addr_pos % SDH_SECTOR_SIZE;
        uint32_t todo_size = DMA_BUFF_SIZE - sector_offset;
        todo_size = (todo_size >= rmn) ? rmn : todo_size;
        /* [begin_sector, end_sector) */
        uint32_t begin_sector = addr_pos / SDH_SECTOR_SIZE;
        uint32_t end_sector = (addr_pos + todo_size - 1 + SDH_SECTOR_SIZE) / SDH_SECTOR_SIZE;

        if (SDH_Read(SDH_FLASH0_DEV.sdh_base, dma_buff, begin_sector, end_sector - begin_sector) != 0) {
            return ARM_DRIVER_ERROR;
        }

        memcpy(b_pos, dma_buff + sector_offset, todo_size);

        b_pos += todo_size;
        addr_pos += todo_size;
        rmn -= todo_size;
    }

    return cnt;
}

static int32_t ARM_SDH_Flash0_ProgramData(uint32_t addr, const void *data, uint32_t cnt)
{
    /* SDH initialization guard */
    SDH_FLASH0_INIT_GUARD();

    uint32_t size = cnt*data_width_byte[DriverCapabilities.data_width];

    /* Check probe OK */
    if (!SDH_FLASH0_DEV.data->sector_count) {
#if NU_SDH_DUMMY_ON_ABSENT
        return cnt;
#else
        return ARM_DRIVER_ERROR;
#endif
    }

    /* Check validity of access address */
    if (is_range_valid(&SDH_FLASH0_DEV, addr, size) != 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    /* For easy implementation, we always use SDH DMA-compatible buffer for intermediate. */
    const uint8_t *b_pos = data;
    uint32_t addr_pos = addr;
    uint32_t rmn = size;

    while (rmn) {
        uint32_t sector_offset = addr_pos % SDH_SECTOR_SIZE;
        uint32_t todo_size = DMA_BUFF_SIZE - sector_offset;
        todo_size = (todo_size >= rmn) ? rmn : todo_size;
        /* [begin_sector, end_sector) */
        uint32_t begin_sector = addr_pos / SDH_SECTOR_SIZE;
        uint32_t end_sector = (addr_pos + todo_size - 1 + SDH_SECTOR_SIZE) / SDH_SECTOR_SIZE;

        if (SDH_Read(SDH_FLASH0_DEV.sdh_base, dma_buff, begin_sector, end_sector - begin_sector) != 0) {
            return ARM_DRIVER_ERROR;
        }

#if NU_SDH_FLASH_PROG_ATR
        {
            const uint8_t *b_pos2 = b_pos;
            const uint8_t *b_end2 = b_pos + todo_size;
            uint8_t *dma_buff_pos = dma_buff + sector_offset;
            while (b_pos2 < b_end2) {
                *dma_buff_pos &= *b_pos2;
                dma_buff_pos ++;
                b_pos2 ++;
            }
        }
#else
        memcpy(dma_buff + sector_offset, b_pos, todo_size);
#endif

        if (SDH_Write(SDH_FLASH0_DEV.sdh_base, dma_buff, begin_sector, end_sector - begin_sector) != 0) {
            return ARM_DRIVER_ERROR;
        }

        b_pos += todo_size;
        addr_pos += todo_size;
        rmn -= todo_size;
    }

    return cnt;
}

static int32_t ARM_SDH_Flash0_EraseSector(uint32_t addr)
{
    /* SDH initialization guard */
    SDH_FLASH0_INIT_GUARD();

    /* Check probe OK */
    if (!SDH_FLASH0_DEV.data->sector_count) {
#if NU_SDH_DUMMY_ON_ABSENT
        return ARM_DRIVER_OK;
#else
        return ARM_DRIVER_ERROR;
#endif
    }

    /* Check validity of access address */
    if (is_range_valid(&SDH_FLASH0_DEV, addr, SDH_FLASH0_DEV.data->sector_size) != 0) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    memset(dma_buff, SDH_FLASH0_DEV.data->erased_value, SDH_SECTOR_SIZE);

    if (SDH_Write(SDH_FLASH0_DEV.sdh_base, dma_buff, addr / SDH_SECTOR_SIZE, 1) != 0) {
        return ARM_DRIVER_ERROR;
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_SDH_Flash0_EraseChip(void)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_FLASH_STATUS ARM_SDH_Flash0_GetStatus(void)
{
    return FlashStatus;
}

static ARM_FLASH_INFO * ARM_SDH_Flash0_GetInfo(void)
{
    return SDH_FLASH0_DEV.data;
}

ARM_DRIVER_FLASH Driver_SDH_FLASH0 =
{
    ARM_SDH_Flash0_GetVersion,
    ARM_SDH_Flash0_GetCapabilities,
    ARM_SDH_Flash0_Initialize,
    ARM_SDH_Flash0_Uninitialize,
    ARM_SDH_Flash0_PowerControl,
    ARM_SDH_Flash0_ReadData,
    ARM_SDH_Flash0_ProgramData,
    ARM_SDH_Flash0_EraseSector,
    ARM_SDH_Flash0_EraseChip,
    ARM_SDH_Flash0_GetStatus,
    ARM_SDH_Flash0_GetInfo
};
#endif /* RTE_SDH_FLASH0 */
