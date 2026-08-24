/*
 * Copyright (c) 2024-2026, Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TFM_PLATFORM_HAL_IOCTL_H__
#define __TFM_PLATFORM_HAL_IOCTL_H__

#include <stdint.h>
#include "tfm_platform_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \enum tfm_platform_ioctl_nuvoton_req_t
 *
 * \brief Nuvoton platform specific IOCTL request types.
 *
 */
enum tfm_platform_ioctl_nuvoton_req_t {
    TFM_PLATFORM_IOCTL_UART0_INIT = 0,
};

/*!
 * \brief Request UART0 initialization (MFP to PB.12/PB.13, Clock Enable, Clock Source to HIRC, Clock Div to 1).
 *
 * \return Returns values as specified by the \ref tfm_platform_err_t
 */
enum tfm_platform_err_t tfm_platform_nuvoton_uart0_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PLATFORM_HAL_IOCTL_H__ */
