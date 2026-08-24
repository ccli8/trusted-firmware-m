/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_hal_device_header.h"
#include "target_cfg.h"
#include "tfm_hal_platform.h"
#include "tfm_plat_defs.h"
#include "uart_stdout.h"
#include "rng.h"

#define BOOT_SEED_SIZE 32
uint8_t g_boot_seed[BOOT_SEED_SIZE];

extern const struct memory_region_limits memory_regions;

enum tfm_hal_status_t tfm_hal_platform_init(void)
{
    enum tfm_plat_err_t plat_err = TFM_PLAT_ERR_SYSTEM_ERR;

    plat_err = enable_fault_handlers();
    if (plat_err != TFM_PLAT_ERR_SUCCESS) {
        return TFM_HAL_ERROR_GENERIC;
    }

    plat_err = system_reset_cfg();
    if (plat_err != TFM_PLAT_ERR_SUCCESS) {
        return TFM_HAL_ERROR_GENERIC;
    }

    plat_err = init_debug();
    if (plat_err != TFM_PLAT_ERR_SUCCESS) {
        return TFM_HAL_ERROR_GENERIC;
    }

    __enable_irq();
    stdio_init();

    plat_err = nvic_interrupt_target_state_cfg();
    if (plat_err != TFM_PLAT_ERR_SUCCESS) {
        return TFM_HAL_ERROR_GENERIC;
    }

    plat_err = nvic_interrupt_enable();
    if (plat_err != TFM_PLAT_ERR_SUCCESS) {
        return TFM_HAL_ERROR_GENERIC;
    }

    if (RNG_Open() != 0 || RNG_Random((uint32_t *)g_boot_seed, 8) != 8) {
        return TFM_HAL_ERROR_GENERIC;
    }

    return TFM_HAL_SUCCESS;
}

uint32_t tfm_hal_get_ns_VTOR(void)
{
    return memory_regions.non_secure_code_start;
}

uint32_t tfm_hal_get_ns_MSP(void)
{
    return *((uint32_t *)memory_regions.non_secure_code_start);
}

uint32_t tfm_hal_get_ns_entry_point(void)
{
    return *((uint32_t *)(memory_regions.non_secure_code_start + 4));
}

int32_t tfm_hal_random_init(void)
{
    if (RNG_Open() != 0) {
        return TFM_HAL_ERROR_GENERIC;
    }
    return TFM_HAL_SUCCESS;
}

int32_t tfm_hal_random_generate(uint8_t *rand, size_t size)
{
    if (rand == NULL || size == 0) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    if (RNG_EntropyPoll(rand, (int32_t)size) != (int32_t)size) {
        return TFM_HAL_ERROR_GENERIC;
    }

    return TFM_HAL_SUCCESS;
}
