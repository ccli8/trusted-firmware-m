/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 * Copyright (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TFM_PERIPHERALS_DEF_H__
#define __TFM_PERIPHERALS_DEF_H__

#include "NuMicro.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TFM_TIMER0_IRQ    (TMR0_IRQn)
/* Implement TF-M TIMER1 (virtual) with M2354 TIMER2 (real) */
#define TFM_TIMER1_IRQ    (TMR2_IRQn)

struct tfm_spm_partition_platform_data_t;

extern struct tfm_spm_partition_platform_data_t tfm_peripheral_std_uart;
extern struct tfm_spm_partition_platform_data_t tfm_peripheral_uart1;
extern struct tfm_spm_partition_platform_data_t tfm_peripheral_timer0;

#define TFM_PERIPHERAL_STD_UART  (&tfm_peripheral_std_uart)
#define TFM_PERIPHERAL_UART1     (&tfm_peripheral_uart1)
#define TFM_PERIPHERAL_TIMER0    (&tfm_peripheral_timer0)
#define TFM_PERIPHERAL_FPGA_IO   (0)

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PERIPHERALS_DEF_H__ */
