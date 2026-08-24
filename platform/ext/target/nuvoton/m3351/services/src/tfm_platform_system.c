/*
 * Copyright (c) 2018-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2024-2026, Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_platform_system.h"
#include "tfm_hal_device_header.h"
#include "tfm_platform_hal_ioctl.h"
#include "NuMicro.h"

void tfm_platform_hal_system_reset(void)
{
    /* Reset the system */
    NVIC_SystemReset();
}

static enum tfm_platform_err_t tfm_platform_hal_uart0_init(void)
{
    /* 1. Set UART0 Multi-Function Pins: PB.12 (RXD), PB.13 (TXD) */
    SET_UART0_RXD_PB12();
    SET_UART0_TXD_PB13();

    /* 2. Select UART0 clock source from HIRC and set divider to 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* 3. Enable UART0 module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    return TFM_PLATFORM_ERR_SUCCESS;
}

enum tfm_platform_err_t tfm_platform_hal_ioctl(tfm_platform_ioctl_req_t request,
                                               psa_invec  *in_vec,
                                               psa_outvec *out_vec)
{
    (void)in_vec;
    (void)out_vec;

    switch (request) {
    case TFM_PLATFORM_IOCTL_UART0_INIT:
        return tfm_platform_hal_uart0_init();

    default:
        return TFM_PLATFORM_ERR_NOT_SUPPORTED;
    }
}
