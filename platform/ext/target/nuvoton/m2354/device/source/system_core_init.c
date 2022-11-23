/*
 * Copyright (c) 2009-2019 Arm Limited. All rights reserved.
 * Copyright (c) 2020 Nuvoton Technology Corp. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file is derivative of CMSIS V5.01 \Device\ARM\ARMCM33\Source\system_ARMCM33.c
 * https://github.com/ARM-software/CMSIS_5/tree/5.0.1
 * Git SHA: 8a1d9d6ee18b143ae5befefa14d89fb5b3f99c75
 */

#include <stdint.h>
#include "NuMicro.h"
#include "system_core_init.h"
#include "platform_description.h"

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/

#define  SYSTEM_CLOCK    (64000000UL)

/*----------------------------------------------------------------------------
  Externals
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK;
uint32_t PeripheralClock = SYSTEM_CLOCK;
uint32_t RefClock        = SYSTEM_CLOCK;
uint32_t CyclesPerUs     = SYSTEM_CLOCK / 1000000;
uint32_t PllClock        = SYSTEM_CLOCK;

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)

/**
 * @brief    Setup Non-secure boundary
 *
 * @details  This function is used to set Non-secure boundary according to
 *           the configuration of partition header file
 */
void FMC_NSBA_Setup(void)
{
    /* Skip NSBA Setup according config */
    if(FMC_INIT_NSBA == 0)
        return;

    /* Check if NSBA value with current active NSBA */
    if(SCU->FNSADDR != FMC_SECURE_ROM_SIZE)
    {
        /* Unlock Protected Register */
        SYS_UnlockReg();

        /* Enable ISP and config update */
        FMC->ISPCTL = FMC_ISPCTL_ISPEN_Msk | FMC_ISPCTL_CFGUEN_Msk;

        /* Config Base of NSBA */
        FMC->ISPADDR = FMC_NSCBA_BASE ;

        /* Read Non-secure base address config */
        FMC->ISPCMD = FMC_ISPCMD_READ;
        FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
        while(FMC->ISPTRG);

        //while(PA0);

        /* Setting NSBA when it is empty */
        if(FMC->ISPDAT != 0xfffffffful)
        {
            /* Erase old setting */
            FMC->ISPCMD = FMC_ISPCMD_PAGE_ERASE;
            FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
            while(FMC->ISPTRG);
        }

        /* Set new base */
        FMC->ISPDAT = FMC_SECURE_ROM_SIZE;
        FMC->ISPCMD = FMC_ISPCMD_PROGRAM;
        FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
        while(FMC->ISPTRG);

        /* Force Chip Reset to valid new setting */
        SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;
    }
}

#endif

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3L)
/* Support no SystemCoreClockUpdate(...) in NSPE because CLK driver can call only in SPE. Expect link error
 * when it is referenced in NSPE. */

/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)
{
#if 0
  SystemCoreClock = SYSTEM_CLOCK;
  PeripheralClock = SYSTEM_CLOCK;
  RefClock        = SYSTEM_CLOCK;
#else
    /* Update PLL Clock */
    PllClock = CLK_GetPLLClockFreq();

    /* Update System Core Clock */
    SystemCoreClock = CLK_GetCPUFreq();

    /* Update Cycles per micro second */
    CyclesPerUs = (SystemCoreClock + 500000UL) / 1000000UL;
#endif
}
#endif

/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/

/* Enable initial stack not located in SRAM bank0
 *
 * On reset, only SRAM bank0 is enabled. And SRAM bank1/2 will be enabled in
 * immediately following SystemInit(). When initial stack is located in SRAM
 * bank1/2, we will meet trouble because SystemInit() itself needs to use initial
 * stack. To conquer the dilemma, we add preceding code in front of original
 * Systeminit(), which is responsible for enabling SRAM bank1/2 and guarantees
 * no using initial stack.
 *
 * SystemInit():
 *  Implement in naked inline assembly to avoid use of initial stack
 *  Enable SRAM bank1/2
 *  Jump to SystemInit_1
 *
 * SystemInit_1()
 *  Implement in C and will use initial stack
 *  Do system initialization as original
 */
void SystemInit_1(void);

/* Add '__attribute__((naked))' here to make sure compiler does not generate prologue and
 * epilogue sequences for SystemInit(). We don't want initial stack is used here.
 *
 * Don't allow extended assembly in naked functions:
 * The compiler only supports basic __asm statements in __attribute__((naked))
 * functions. Using extended assembly, parameter references or mixing C code with
 * __asm statements might not work reliably.
 */
__attribute__((naked)) void SystemInit(void)
{
#if defined(__GNUC__)
    __asm(".syntax  unified                                         \n");
#endif

//#if __DOMAIN_NS == 0
#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3L)
    /* Enable SRAM bank1/2
     *
     * CLK->AHBCLK |= (CLK_AHBCLK_SRAM1CKEN_Msk | CLK_AHBCLK_SRAM2CKEN_Msk)
     */
    __asm("movw     r2, #0x200                                      \n");
    __asm("movt     r2, #0x4000                                     \n");
    __asm("ldr      r0, [r2, #4]                                    \n");
    __asm("movw     r1, #0x0                                        \n");
    __asm("movt     r1, #0x60                                       \n");
    __asm("orrs     r0, r0, r1                                      \n");
    __asm("str      r0, [r2, #4]                                    \n");
#endif

    /* Jump to SystemInit_1() */
    __asm("movw     r0, #:lower16:SystemInit_1                      \n");
    __asm("movt     r0, #:upper16:SystemInit_1                      \n");
    __asm("bx       r0                                              \n");
}

/* Do system initialization as original SystemInit() */
void SystemInit_1(void)
{
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
  extern uint32_t __Vectors;
  SCB->VTOR = (uint32_t) &__Vectors;
#endif

//#if __DOMAIN_NS == 0
#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3L)

  /* Initial the system */
  SYS_UnlockReg();

  /* Get around BL2 linker error on GNUARM
   *
   * ld.exe: no address assigned to the veneers output section .gnu.sgstubs
   *
   * BL2 linker file doesn't enable .gnu.sgstubs for SG. We remove all SG in StdDriver sys.c/clk.c
   * and so we can invoke SYS/CLK driver API.
   *
   * TODO: replace direct access to CLK registers with CLK driver API
   */

  /* Enable HIRC and waiting for stable */
  CLK->PWRCTL |= CLK_PWRCTL_HIRCEN_Msk;
  while((CLK->STATUS & CLK_STATUS_HIRCSTB_Msk) == 0);

#if NU_HXT_PRESENT
  /* Enable HXT and waiting for stable */
  CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;
  while((CLK->STATUS & CLK_STATUS_HXTSTB_Msk) == 0);
#else
  CLK->PWRCTL &= ~CLK_PWRCTL_HXTEN_Msk;
#endif

  /* Enable LIRC and waiting for stable */
  CLK->PWRCTL |= CLK_PWRCTL_LIRCEN_Msk;
  while((CLK->STATUS & CLK_STATUS_LIRCSTB_Msk) == 0);

  /* Enable LXT and waiting for stable */
  CLK->PWRCTL |= CLK_PWRCTL_LXTEN_Msk;
  while((CLK->STATUS & CLK_STATUS_LXTSTB_Msk) == 0);

  /* Enable HIRC48 and waiting for stable */
  CLK->PWRCTL |= CLK_PWRCTL_HIRC48EN_Msk;
  while((CLK->STATUS & CLK_STATUS_HIRC48STB_Msk) == 0);

  /* Enable PLL and waiting for stable */
#if NU_HXT_PRESENT
  CLK->PLLCTL = CLK_PLLCTL_96MHz_HXT;
#else
  CLK->PLLCTL = CLK_PLLCTL_96MHz_HIRC;
#endif
  while((CLK->STATUS & CLK_STATUS_PLLSTB_Msk) == 0);

  /* Set flash access delay cycle */
  FMC->CYCCTL = 3;

  /* Switch HCLK clock source to PLL */
  CLK->CLKSEL0 = (CLK->CLKSEL0 & (~CLK_CLKSEL0_HCLKSEL_Msk)) | CLK_CLKSEL0_HCLKSEL_PLL;
  CLK->CLKDIV0 = 0;

  /* Enable IP clock */
#if NU_TGT_NUMAKER_M2354 || NU_TGT_NUMAKER_IOT_M2354
  CLK->APBCLK0 |= CLK_APBCLK0_UART0CKEN_Msk | CLK_APBCLK0_TMR0CKEN_Msk | CLK_APBCLK0_TMR2CKEN_Msk;
#endif

  /* Select UART clock source */
#if NU_TGT_NUMAKER_M2354 || NU_TGT_NUMAKER_IOT_M2354
  CLK->CLKSEL2 = (CLK->CLKSEL2 & (~CLK_CLKSEL2_UART0SEL_Msk)) | CLK_CLKSEL2_UART0SEL_HIRC;
#endif

  /* Timer clock source */
  CLK->CLKSEL1 = (CLK->CLKSEL1 & (~CLK_CLKSEL1_TMR0SEL_Msk)) | CLK_CLKSEL1_TMR0SEL_HIRC;
#ifdef TFM_ENABLE_IRQ_TEST
  CLK->CLKSEL1 = (CLK->CLKSEL1 & (~CLK_CLKSEL1_TMR2SEL_Msk)) | CLK_CLKSEL1_TMR2SEL_HIRC;
#endif

  /* Set multi-function pins for UART0 RXD and TXD */
#if NU_TGT_NUMAKER_M2354
  SYS->GPA_MFPL = (SYS->GPA_MFPL & (~(UART0_RXD_PA6_Msk | UART0_TXD_PA7_Msk))) | UART0_RXD_PA6 | UART0_TXD_PA7;
#elif NU_TGT_NUMAKER_IOT_M2354
  SYS->GPB_MFPH = (SYS->GPB_MFPH & (~(UART0_RXD_PB8_Msk | UART0_TXD_PB9_Msk))) | UART0_RXD_PB8 | UART0_TXD_PB9;
#endif

#if NU_TGT_NUMAKER_M2354 || NU_TGT_NUMAKER_IOT_M2354
  /* Set UART 0 to Non-secure */
  SCU_SET_PNSSET(UART0_Attr);
#endif

  /* Set SAU */
  SAU->RNR = 3;
  SAU->RBAR = 0x50000000;
  SAU->RLAR = (0x5FFFFFFF & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

  /* Init ETM Trace */
  SYS->GPE_MFPH = (SYS->GPE_MFPH & (~(TRACE_CLK_PE12_Msk | TRACE_DATA0_PE11_Msk | TRACE_DATA1_PE10_Msk | TRACE_DATA2_PE9_Msk | TRACE_DATA3_PE8_Msk))) |
      TRACE_CLK_PE12 | TRACE_DATA0_PE11 | TRACE_DATA1_PE10 | TRACE_DATA2_PE9 | TRACE_DATA3_PE8;

  /* power gating */
  M32(0x400001f4) = 0xfffffffful;
  M32(0x400000dC) = 0ul;
  /* GPIO clk */
  CLK->AHBCLK |= (0xffful << 20) | (1ul << 14);

  /* Configure NSCBA programmatic
   *
   * This is done at first boot so that we needn't external programming tool extra.
   */
  FMC_NSBA_Setup();

  /* Default to (shallow) sleep/idle mode configuration to enable entering (shallow) sleep/idle
   * mode without crossing SPE/NSPE boundary */
    SYS_UnlockReg();
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    CLK->PWRCTL &= ~CLK_PWRCTL_PDEN_Msk;

    // Default role of USB to On-The-Go device mode
    SYS->USBPHY = (SYS->USBPHY & ~SYS_USBPHY_USBROLE_Msk) | SYS_USBPHY_OTGPHYEN_Msk | SYS_USBPHY_SBO_Msk;

#endif

      

  SystemCoreClock = SYSTEM_CLOCK;
}
