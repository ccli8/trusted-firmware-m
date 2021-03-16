/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 * Copyright (c) 2021 Nuvoton Technology Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cmsis.h"
#include <stddef.h>

int mbedtls_hardware_poll( void *data, unsigned char *output, size_t len, size_t *olen )
{
    /* Suppress "unused" warning */
    (void) data;

    /* Check argument validity */
    if (!output && len) {
        return -1;
    }

    /* Activate TRNG */
    CLK_EnableModuleClock(TRNG_MODULE);
    SYS_ResetModule(TRNG_RST);
    TRNG_S->ACT |= TRNG_ACT_ACT_Msk;
    while (!(TRNG_S->CTL & TRNG_CTL_READY_Msk));
    TRNG_S->CTL |= TRNG_CTL_TRNGEN_Msk;

    size_t i;
    for (i = 0; i < len; i ++, output ++) {
        while (!(TRNG_S->CTL & TRNG_CTL_DVIF_Msk));
        *output = TRNG_S->DATA & 0xff;
    }

    if (olen) {
        *olen = len;
    }

    /* De-activate TRNG */
    TRNG_S->CTL &= ~TRNG_CTL_TRNGEN_Msk;
    TRNG_S->ACT &= ~TRNG_ACT_ACT_Msk;
    CLK_DisableModuleClock(TRNG_MODULE);

    return 0;
}
