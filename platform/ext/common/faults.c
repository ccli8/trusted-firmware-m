/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "config_tfm.h"

#include "tfm_hal_device_header.h"
#include "utilities.h"
#include "tfm_log.h"
/* "exception_info.h" must be the last include because of the IAR pragma */
#include "exception_info.h"

void C_HardFault_Handler(uint32_t *sp, uint32_t exc_return)
{
    ERROR("[FAULT] HardFault! EXC_RETURN=0x%x, SP=0x%x\n", exc_return, (uint32_t)sp);
    if (sp) {
        ERROR("  R0=0x%x, R1=0x%x, R2=0x%x, R3=0x%x\n", sp[0], sp[1], sp[2], sp[3]);
        ERROR("  R12=0x%x, LR=0x%x, PC=0x%x, XPSR=0x%x\n", sp[4], sp[5], sp[6], sp[7]);
    }
    uint32_t psp_ns = __TZ_get_PSP_NS();
    uint32_t msp_ns = __TZ_get_MSP_NS();
    ERROR("  PSP_NS=0x%x, MSP_NS=0x%x\n", psp_ns, msp_ns);
    if (psp_ns >= 0x30000000 && psp_ns < 0x30030000) {
        uint32_t *ns_sp = (uint32_t *)psp_ns;
        ERROR("  NS stacked: PC=0x%x, LR=0x%x\n", ns_sp[6], ns_sp[5]);
    }
    tfm_core_panic();
}

EXCEPTION_INFO_IAR_REQUIRED
__attribute__((naked)) void HardFault_Handler(void)
{
    EXCEPTION_INFO();

    __ASM volatile(
        "mov     r1, lr                  \n"
        "tst     lr, #4                  \n"
        "ite     eq                      \n"
        "mrseq   r0, msp                 \n"
        "mrsne   r0, psp                 \n"
        "b       C_HardFault_Handler     \n"
    );
}

void C_MemManage_Handler(uint32_t *sp, uint32_t exc_return)
{
    ERROR("[FAULT] MemManageFault! CFSR=0x%x, MMAR=0x%x, EXC_RETURN=0x%x\n",
          *(volatile uint32_t *)0xE000ED28, *(volatile uint32_t *)0xE000ED34, exc_return);
    if (sp) {
        ERROR("  R0=0x%x, R1=0x%x, R2=0x%x, R3=0x%x\n", sp[0], sp[1], sp[2], sp[3]);
        ERROR("  R12=0x%x, LR=0x%x, PC=0x%x, XPSR=0x%x\n", sp[4], sp[5], sp[6], sp[7]);
    }
    tfm_core_panic();
}

EXCEPTION_INFO_IAR_REQUIRED
__attribute__((naked)) void MemManage_Handler(void)
{
    EXCEPTION_INFO();

    __ASM volatile(
        "mov     r1, lr                  \n"
        "tst     lr, #4                  \n"
        "ite     eq                      \n"
        "mrseq   r0, msp                 \n"
        "mrsne   r0, psp                 \n"
        "b       C_MemManage_Handler     \n"
    );
}

void C_BusFault_Handler(uint32_t *sp, uint32_t exc_return)
{
    ERROR("[FAULT] BusFault! CFSR=0x%x, BFAR=0x%x, EXC_RETURN=0x%x\n",
          *(volatile uint32_t *)0xE000ED28, *(volatile uint32_t *)0xE000ED38, exc_return);
    if (sp) {
        ERROR("  R0=0x%x, R1=0x%x, R2=0x%x, R3=0x%x\n", sp[0], sp[1], sp[2], sp[3]);
        ERROR("  R12=0x%x, LR=0x%x, PC=0x%x, XPSR=0x%x\n", sp[4], sp[5], sp[6], sp[7]);
    }
    tfm_core_panic();
}

EXCEPTION_INFO_IAR_REQUIRED
__attribute__((naked)) void BusFault_Handler(void)
{
    EXCEPTION_INFO();

    __ASM volatile(
        "mov     r1, lr                  \n"
        "tst     lr, #4                  \n"
        "ite     eq                      \n"
        "mrseq   r0, msp                 \n"
        "mrsne   r0, psp                 \n"
        "b       C_BusFault_Handler      \n"
    );
}

void C_SecureFault_Handler(uint32_t *sp, uint32_t exc_return)
{
    ERROR("[FAULT] SecureFault! SFSR=0x%x, SFAR=0x%x, CFSR=0x%x, EXC_RETURN=0x%x\n",
          *(volatile uint32_t *)0xE000EDE4, *(volatile uint32_t *)0xE000EDE8,
          *(volatile uint32_t *)0xE000ED28, exc_return);
    if (sp) {
        ERROR("  R0=0x%x, R1=0x%x, R2=0x%x, R3=0x%x\n", sp[0], sp[1], sp[2], sp[3]);
        ERROR("  R12=0x%x, LR=0x%x, PC=0x%x, XPSR=0x%x\n", sp[4], sp[5], sp[6], sp[7]);
    }
    uint32_t psp_ns = __TZ_get_PSP_NS();
    uint32_t msp_ns = __TZ_get_MSP_NS();
    ERROR("  PSP_NS=0x%x, MSP_NS=0x%x\n", psp_ns, msp_ns);
    if (psp_ns >= 0x30000000 && psp_ns < 0x30030000) {
        uint32_t *ns_sp = (uint32_t *)psp_ns;
        ERROR("  NS stacked: PC=0x%x, LR=0x%x\n", ns_sp[6], ns_sp[5]);
    }
    tfm_core_panic();
}

EXCEPTION_INFO_IAR_REQUIRED
__attribute__((naked)) void SecureFault_Handler(void)
{
    EXCEPTION_INFO();

    __ASM volatile(
        "mov     r1, lr                  \n"
        "tst     lr, #4                  \n"
        "ite     eq                      \n"
        "mrseq   r0, msp                 \n"
        "mrsne   r0, psp                 \n"
        "b       C_SecureFault_Handler   \n"
    );
}

void C_UsageFault_Handler(void)
{
    ERROR("[FAULT] UsageFault! CFSR=0x%x\n", *(volatile uint32_t *)0xE000ED28);
    tfm_core_panic();
}

EXCEPTION_INFO_IAR_REQUIRED
__attribute__((naked)) void UsageFault_Handler(void)
{
    EXCEPTION_INFO();

    __ASM volatile(
        "bl        C_UsageFault_Handler   \n"
        "b         .                      \n"
    );
}
