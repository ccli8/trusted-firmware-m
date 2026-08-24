/**************************************************************************//**
 * @file     NuMicro.h
 * @version  V1.00
 * @brief    NuMicro Peripheral Access Layer Header File
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2025 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/


#ifndef __NUMICRO_H__
#define __NUMICRO_H__

#include "M3351.h"

/*
 * TF-M's shared Nuvoton timer driver predates the M3351 BSP naming update.
 * Keep the aliases local to the M3351 device wrapper.
 */
#define TMR0_IRQn TIMER0_IRQn
#define TMR2_IRQn TIMER2_IRQn
#define TMR01_BASE TIMER0_BASE

#endif  /* __NUMICRO_H__ */
