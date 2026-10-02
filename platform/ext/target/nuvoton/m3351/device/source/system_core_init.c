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

/* Forward declaration */
void FMC_NSCBA_Setup(void);

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
    /* Enable PLL */
    CLK->PLLCTL ^= CLK_PLLCTL_PD_Msk;
    CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk) | CLK_CLKSEL0_HCLKSEL_PLL;

    /* Init UART0 to non-secure region */
    SCU_SET_PNSSET(UART0_Attr);
#endif

    /* For regression test, pre-enable clock here, so that CLK driver
     * doesn't need to configure for UNPRIV.
     */
#if defined(TFM_NVT_REG_TEST)
    CLK_EnableModuleClock(TMR0_MODULE);
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);
#endif

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    /* The equivalents are done in target_cfg.c */
    #if 0
    TZ_SAU_Setup();
    SCU_Setup();
    #endif
    /* The equivalent should be done in provisioning. Without provisioning,
     * invoke it for development.
     */
    #if TFM_NVT_INIT_NSCBA
    FMC_NSCBA_Setup();
    #endif
#endif
}


#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
/**
 * @brief    Setup Non-secure boundary
 *
 * @details  This function is used to set Non-secure boundary according to
 *           the configuration of partition header file
 */
void FMC_NSCBA_Setup(void)
{
    uint32_t u32TimeOutCnt;
    uint32_t bForceReset = FALSE;
    uint32_t bUpdateNSCBA = FALSE;

    /* Skip NSCBA Setup according config */
#if (FMC_INIT_NSCBA == 0)
    {
        return;
    }
#endif

    /* Check if NSCBA value with current active NSCBA */
    bUpdateNSCBA = (SCU->FNSADDR != FMC_SECURE_END) ? TRUE : FALSE;

    if (bUpdateNSCBA == FALSE)
    {
        bUpdateNSCBA = ((FMC->ISPSTS & FMC_ISPSTS_MIRBOUND_Msk) != (FMC_INIT_MIRROR_BOUND << FMC_ISPSTS_MIRBOUND_Pos)) ? TRUE : FALSE;
    }

    if (bUpdateNSCBA != FALSE)
    {
        /* Unlock Protected Register */
        SYS_UnlockReg();

        /* Enable FMC ISP and config update */
        FMC->ISPCTL = FMC_ISPCTL_ISPEN_Msk | FMC_ISPCTL_CFGUEN_Msk;

        /* Config Base of NSCBA */
        FMC->ISPADDR = FMC_NSCBA_BASE;

        /* Read Non-secure base address config */
        FMC->ISPCMD = FMC_ISPCMD_READ;
        FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

        u32TimeOutCnt = FMC_TIMEOUT_READ;

        while ((FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) != 0UL)
        {
            if (u32TimeOutCnt == 0UL)
            {
                return ;
            }
            u32TimeOutCnt--;
        }

        /* Setting NSCBA when it is empty */
        if (FMC->ISPDAT != 0xFFFFFFFFUL)
        {
            /* Erase old setting */
            FMC->ISPCMD = FMC_ISPCMD_CFG_ERASE;
            FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

            u32TimeOutCnt = FMC_TIMEOUT_ERASE;

            while ((FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) != 0UL)
            {
                if (u32TimeOutCnt == 0UL)
                {
                    return ;
                }
                u32TimeOutCnt--;
            }
        }

        /* Set new base */
        FMC->ISPDAT = ((uint32_t)(FMC_INIT_MIRROR_BOUND << 31) | FMC_SECURE_END);
        FMC->ISPCMD = FMC_ISPCMD_PROGRAM;
        FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

        u32TimeOutCnt = FMC_TIMEOUT_WRITE;

        while ((FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) != 0UL)
        {
            if (u32TimeOutCnt == 0UL)
            {
                return ;
            }
            u32TimeOutCnt--;
        }

        bForceReset = TRUE;
    }

    /* Skip NSCBA Setup according config */
#if (DFMC_INIT_NSCBA == 1) && (DFMC_SECURE_END > DFMC_DATA_FLASH_BASE)
    uint32_t u32Config14;
    uint32_t u32DFMC_NSCBAConfig = ((DFMC_SECURE_END - DFMC_DATA_FLASH_BASE) / 0x1000UL) - 1UL;

    /* Check if NSCBA value with current active NSCBA */
    if ((SCU->DFNSADDR & 0xFUL) != u32DFMC_NSCBAConfig)
    {
        uint32_t u32DFMC_EnableEEPROM = BIT5;    /* CONFIG14.DFMC_EE[5]: Data Flash EEPROM Function Enable Bit; 0: Enable, 1: Disable */

#if (DFMC_ENABLE_EEPROM == 1UL)
        u32DFMC_EnableEEPROM = 0UL;    /* CONFIG14.DFMC_EE[5]: Data Flash EEPROM Function Enable Bit; 0: Enable, 1: Disable */
#endif

        /* Unlock Protected Register */
        SYS_UnlockReg();

        /* Enable FMC ISP and config update */
        FMC->ISPCTL = FMC_ISPCTL_ISPEN_Msk | FMC_ISPCTL_CFGUEN_Msk;

        /* Config Base of NSCBA */
        FMC->ISPADDR = FMC_USER_CONFIG_14;

        /* Read Non-secure base address config */
        FMC->ISPCMD = FMC_ISPCMD_READ;
        FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

        u32TimeOutCnt = FMC_TIMEOUT_READ;

        while ((FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) != 0UL)
        {
            if (u32TimeOutCnt == 0UL)
            {
                return ;
            }
            u32TimeOutCnt--;
        }

        u32Config14 = FMC->ISPDAT;

        if ((u32Config14 & 0x6FUL) == ((DFMC_SECURE_EEPROM << 6UL) | u32DFMC_EnableEEPROM | u32DFMC_NSCBAConfig))
        {
            return ;
        }

        /* Setting NSCBA when it is empty */
        if (u32Config14 != 0xFFFFFFFFUL)
        {
            /* Erase old setting */
            FMC->ISPCMD = FMC_ISPCMD_CFG_ERASE;
            FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

            u32TimeOutCnt = FMC_TIMEOUT_ERASE;

            while ((FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) != 0UL)
            {
                if (u32TimeOutCnt == 0UL)
                {
                    return ;
                }
                u32TimeOutCnt--;
            }
        }

        /* Set new base */
        FMC->ISPDAT = ~0x6FUL | ((DFMC_SECURE_EEPROM << 6UL) | u32DFMC_EnableEEPROM | u32DFMC_NSCBAConfig);
        FMC->ISPCMD = FMC_ISPCMD_PROGRAM;
        FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

        u32TimeOutCnt = FMC_TIMEOUT_WRITE;

        while ((FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) != 0UL)
        {
            if (u32TimeOutCnt == 0UL)
            {
                return ;
            }
            u32TimeOutCnt--;
        }

        bForceReset = TRUE;
    }

#endif

    if (bForceReset == TRUE)
    {
        /* Force Chip Reset to valid new setting */
        SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;
    }
}
#endif /* defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U) */
