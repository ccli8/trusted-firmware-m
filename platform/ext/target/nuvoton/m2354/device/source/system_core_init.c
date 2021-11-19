/*
 * Copyright (c) 2009-2022 Arm Limited. All rights reserved.
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
#include "cmsis.h"
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

#ifdef NV_ENABLE_ETM
    /* Init ETM Trace */
    SYS->GPE_MFPH = (SYS->GPE_MFPH & (~(TRACE_CLK_PE12_Msk | TRACE_DATA0_PE11_Msk | TRACE_DATA1_PE10_Msk | TRACE_DATA2_PE9_Msk | TRACE_DATA3_PE8_Msk))) |
        TRACE_CLK_PE12 | TRACE_DATA0_PE11 | TRACE_DATA1_PE10 | TRACE_DATA2_PE9 | TRACE_DATA3_PE8;

    CLK->AHBCLK |= CLK_AHBCLK_TRACECKEN_Msk;
#endif

    /* power gating */
    M32(0x400001f4) = 0xfffffffful;
    M32(0x400000dC) = 0ul;

    /* Enable SRAM Clock */
    CLK->AHBCLK |= CLK_AHBCLK_SRAM0CKEN_Msk | CLK_AHBCLK_SRAM1CKEN_Msk | CLK_AHBCLK_SRAM2CKEN_Msk;

    /* Enable GPIO Clock  */
    CLK->AHBCLK |= CLK_AHBCLK_GPACKEN_Msk | CLK_AHBCLK_GPBCKEN_Msk | CLK_AHBCLK_GPCCKEN_Msk | CLK_AHBCLK_GPDCKEN_Msk |
                   CLK_AHBCLK_GPECKEN_Msk | CLK_AHBCLK_GPFCKEN_Msk | CLK_AHBCLK_GPGCKEN_Msk | CLK_AHBCLK_GPHCKEN_Msk;

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

    /* Force to use HIRC */
    CLK->CLKSEL0 = (CLK->CLKSEL0  & (~CLK_CLKSEL0_HCLKSEL_Msk)) | CLK_CLKSEL0_HCLKSEL_HIRC;

    /* Enable PLL and waiting for stable */
#if NU_HXT_PRESENT
    CLK->PLLCTL = CLK_PLLCTL_96MHz_HXT;
#else
    CLK->PLLCTL = CLK_PLLCTL_96MHz_HIRC;
#endif
    while((CLK->STATUS & CLK_STATUS_PLLSTB_Msk) == 0);

    /* Set flash access delay cycle */
    FMC->CYCCTL = (FMC->CYCCTL & (~FMC_CYCCTL_CYCLE_Msk)) | (4 << FMC_CYCCTL_CYCLE_Pos);

    /* Switch HCLK clock source to PLL */
    CLK->CLKSEL0 = (CLK->CLKSEL0 & (~CLK_CLKSEL0_HCLKSEL_Msk)) | CLK_CLKSEL0_HCLKSEL_PLL;
    CLK->CLKDIV0 = (CLK->CLKDIV0 & (~CLK_CLKDIV0_HCLKDIV_Msk)) | (0 << CLK_CLKDIV0_HCLKDIV_Pos);

#if 0
    /* Enable Crypto Accelerator */
    CLK->AHBCLK |= CLK_AHBCLK_CRPTCKEN_Msk;

    /* Enable SDH for secondary slot */
    CLK->AHBCLK |= CLK_AHBCLK_SDH0CKEN_Msk;

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

    NVIC_DisableIRQ(SDH0_IRQn);
#endif

    /* Enable IP clock */
    CLK->APBCLK0 |= CLK_APBCLK0_UART0CKEN_Msk | CLK_APBCLK0_TMR0CKEN_Msk | CLK_APBCLK0_TMR2CKEN_Msk;

    /* Select UART clock source */
    CLK->CLKSEL2 = (CLK->CLKSEL2 & (~CLK_CLKSEL2_UART0SEL_Msk)) | CLK_CLKSEL2_UART0SEL_HIRC;

    /* Timer clock source */
    CLK->CLKSEL1 = (CLK->CLKSEL1 & (~CLK_CLKSEL1_TMR0SEL_Msk)) | CLK_CLKSEL1_TMR0SEL_HIRC;
#ifdef TFM_ENABLE_IRQ_TEST
    CLK->CLKSEL1 = (CLK->CLKSEL1 & (~CLK_CLKSEL1_TMR2SEL_Msk)) | CLK_CLKSEL1_TMR2SEL_HIRC;
#endif

    /* Set multi-function pins for UART0 RXD and TXD */
    SYS->GPA_MFPL = (SYS->GPA_MFPL & (~(UART0_RXD_PA6_Msk | UART0_TXD_PA7_Msk))) | UART0_RXD_PA6 | UART0_TXD_PA7;

    /* Set UART 0 to Non-secure */
    SCU_SET_PNSSET(UART0_Attr);

    /* Set SAU */
    SAU->RNR = 3;
    SAU->RBAR = 0x50000000;
    SAU->RLAR = (0x5FFFFFFF & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

#ifndef NU_DISABLE_TAMPER
    CLK->APBCLK0 |= CLK_APBCLK0_TAMPERCKEN_Msk;

    /* Reset tamper coreblock */
    TAMPER_CORE_RESET();
    TAMPER_CORE_RELEASE();

    /* Enable voltage glitch detection clock source and select sampling rate */
    TAMPER_ENABLE_HIRC48M();
    TAMPER_VG_SAMPLE_SEL(TAMPER_VG_192M_SAMPLE);

    /* Initialize a reference trim value according to the power level of the system */
    TAMPER_VG_TRIM_INIT();

    /* Enable voltage glitch positive/negative detection interrupt */
    TAMPER_EnableInt(TAMPER_INTEN_VGPIEN_Msk | TAMPER_INTEN_VGNIEN_Msk);

    /* Clear voltage glitch positive/negative interrupt flag */
    TAMPER_CLR_INT_STATUS(TAMPER_INTSTS_VGPEVIF_Msk | TAMPER_INTSTS_VGNEVIF_Msk);

    /* Enable over voltage detector and wait until stable */
    SYS->OVDCTL = SYS_OVDCTL_OVDEN_Msk;
    while(!(SYS->OVDCTL & SYS_OVDCTL_OVDSTB_Msk));

    /* Initialize the trim value of under-shoot and over-shoot detection level */
    TAMPER_TLVD_TRIM_INIT(TAMPER_LBSTRIM_TLVDSEL_0_90V);
    TAMPER_TOVD_TRIM_INIT(TAMPER_LBSTRIM_TOVDSEL_1_40V);

    /* Clear different voltage interrupt flag */
    TAMPER_CLR_INT_STATUS(TAMPER_INTSTS_OVPOUTIF_Msk | TAMPER_INTSTS_BODIF_Msk);

    /* Enable different voltage detection interrupt */
    TAMPER_EnableInt(TAMPER_INTEN_OVPIEN_Msk | TAMPER_INTEN_BODIEN_Msk);

    /* Enable to trigger chip reset */
    TAMPER_ENABLE_CHIPRST();
#endif

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

#endif




  SystemCoreClock = SYSTEM_CLOCK;
}
