/*
 * Copyright (c) 2024-2026, Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include "tfm_platform_api.h"
#include "tfm_platform_hal_ioctl.h"

enum tfm_platform_err_t tfm_platform_nuvoton_uart0_init(void)
{
    return tfm_platform_ioctl(TFM_PLATFORM_IOCTL_UART0_INIT, NULL, NULL);
}
