/**************************************************************************//**
 * @file     system_M3351.c
 * @version  V1.00
 * @brief    CMSIS Device System Source File for NuMicro M3351
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2025 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)       // ARM Compiler 6
    #include <arm_cmse.h>
#endif

#include "NuMicro.h"

#ifndef FMC_INIT_MIRROR_BOUND
    #define FMC_INIT_MIRROR_BOUND       0x0UL
#endif

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
/**
 * @static_deviation
 * <b>Rule:</b>          MISRA C:2012 Rule 5.5<br>
 * <b>Justification:</b> __VECTOR_TABLE is a CMSIS/toolchain-mandated vector table
 *                       symbol name shared with startup_M3351.c; its final expanded
 *                       symbol differs across ARMCLANG/GNUC/IAR toolchains (e.g.
 *                       __Vectors, __vector_table), so it must keep this exact name
 *                       to remain compatible with CMSIS startup flow and linker scripts.
 */
/* cppcheck-suppress misra-c2012-5.5 */
extern const VECTOR_TABLE_Type __VECTOR_TABLE[FMC_VECMAP_SIZE / 4U];

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = __HSI;                /*!< System Clock Frequency (Core Clock) */
uint32_t CyclesPerUs     = (__HSI / 1000000UL);  /*!< Cycles per micro second             */
uint32_t PllClock        = __HSI;                /*!< PLL Output Clock Frequency          */

/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate(void)
{
    /* Update PLL Clock */
    PllClock = CLK_GetPLLClockFreq();

    /* Update System Core Clock */
    SystemCoreClock = CLK_GetCPUFreq();

    /* Update Cycles per micro second */
    CyclesPerUs = (SystemCoreClock + 500000UL) / 1000000UL;
}


/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/
//__attribute__((constructor))
void SystemInit(void)
{
#if !defined(DOMAIN_NS) || (DOMAIN_NS == 0)
    SYS_UnlockReg();
#endif

#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
    SCB->VTOR = (uint32_t)(&__VECTOR_TABLE);
#endif

#ifdef UNALIGNED_SUPPORT_DISABLE
    SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif

#if defined(BL2)
    /* Init UART0 to non-secure region */
    SCU_SET_PNSSET(UART0_Attr);
#endif

}

