/*
 * Copyright (c) 2018-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_platform_system.h"
#include "cmsis.h"
#include "platform_extra_secure.h"
#include <assert.h>
#include <stdio.h>

void tfm_platform_hal_system_reset(void)
{
    /* Reset the system */
    NVIC_SystemReset();
}

/* Signature of platform extra secure handler (service side) */
typedef enum tfm_platform_err_t (*plat_xtra_sec_hdlr_t)(psa_invec *, psa_outvec *);

/* Table of platform extra secure handlers */
static plat_xtra_sec_hdlr_t plat_xtra_sec_hdlr_tab[] = {
    /* Prefix with "SYS" */
    SYS_ResetModule_S,
    SYS_LockReg_S,
    SYS_UnlockReg_S,

    /* Prefix with "CLK" */
    CLK_SetModuleClock_S,
    CLK_EnableModuleClock_S,
    CLK_DisableModuleClock_S,
    CLK_Idle_S,
    CLK_PowerDown_S,
    CLK_GetHXTFreq_S,
    CLK_GetLXTFreq_S,
    CLK_GetHCLKFreq_S,
    CLK_GetPCLK0Freq_S,
    CLK_GetPCLK1Freq_S,
    CLK_GetCPUFreq_S,
    CLK_GetPLLClockFreq_S,
    CLK_GetModuleClockSource_S,
    CLK_GetModuleClockDivider_S,

    /* Prefix with "RTC" */
    RTC_Open_S,
    RTC_Close_S,
    RTC_WaitAccessEnable_S,
    RTC_GetDateAndTime_S,
    RTC_GetAlarmDateAndTime_S,
    RTC_SetDateAndTime_S,
    RTC_SetAlarmDateAndTime_S,

    /* Prefix with "nu" */
    nu_pin_function_s,
    nu_idle_s,
    nu_powerdown_s,
    nu_rtc_read_spare_register_s,
    nu_rtc_write_spare_register_s,
    nu_rtc_isenabled_s,
};

/* In our implementation, request code also plays as index into handler table
 * to find the corresponding handler. Check these two parts are consistent statically,
 * e.g. max request code = handler table size. */
_Static_assert(NU_PLAT_XTRA_SEC_REQ(MAX) == sizeof(plat_xtra_sec_hdlr_tab)/sizeof(plat_xtra_sec_hdlr_tab[0]),
               "Platform extra secure request code/handler table are not consistent");

enum tfm_platform_err_t tfm_platform_hal_ioctl(tfm_platform_ioctl_req_t request,
                                               psa_invec  *in_vec,
                                               psa_outvec *out_vec)
{
    if (request < 0 || request >= NU_PLAT_XTRA_SEC_REQ(MAX)) {
        //printf("Invalid platform extra request code: %d\r\n", request);
        return TFM_PLATFORM_ERR_NOT_SUPPORTED;
    }

    plat_xtra_sec_hdlr_t hdlr = plat_xtra_sec_hdlr_tab[request];
    return hdlr(in_vec, out_vec);
}
