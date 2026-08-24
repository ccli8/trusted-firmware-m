/**************************************************************************//**
 * @file     partition_M3351.h
 * @version  V3.00
 * @brief    SCU and memory partition settings.
 *           (Note: SAU configuration is managed in target_cfg.c via sau_cfg)
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

#ifndef PARTITION_M3351
#define PARTITION_M3351

#ifndef NS_OFFSET
#define NS_OFFSET                 (0x10000000UL)
#endif

/*
//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
*/


/*
    SRAMNSSET
*/
/*
// Bit 0..18
// <o.0..18> Secure SRAM Size              <0=> 0 KB
//                                         <0x4000=> 16KB
//                                         <0x8000=> 32KB
//                                         <0xc000=> 48KB
//                                         <0x10000=> 64KB
//                                         <0x14000=> 80KB
//                                         <0x18000=> 96KB
//                                         <0x1C000=> 112KB
//                                         <0x20000=> 128KB
//                                         <0x24000=> 144KB
//                                         <0x28000=> 160KB
//                                         <0x2C000=> 176KB
//                                         <0x30000=> 192KB
//                                         <0x34000=> 208KB
//                                         <0x38000=> 224KB
//                                         <0x3C000=> 240KB
//                                         <0x40000=> 256KB
*/
#define SCU_SECURE_SRAM_SIZE      0x10000
#define NON_SECURE_SRAM_BASE      (0x30000000 + SCU_SECURE_SRAM_SIZE)



/*--------------------------------------------------------------------------------------------------------*/

/*
    NSBA
*/
#define FMC_INIT_NSCBA         1
/*
//     <o>Secure Flash ROM Size <0x800-0x80000:0x800>
*/

#define FMC_SECURE_ROM_SIZE      (0x80000)

#define FMC_NON_SECURE_BASE     (FMC_SECURE_ROM_SIZE+NS_OFFSET)

/*--------------------------------------------------------------------------------------------------------*/

/*
// <h> Peripheral Secure Attribution Configuration
*/

/*
    PNSSET0
*/
/*
// Module 0..31
//   <o.9>  USBH       <0=> Secure <1=> Non-Secure
//   <o.13>  SD0   <0=> Secure <1=> Non-Secure
//   <o.16>  EBI    <0=> Secure <1=> Non-Secure
//   <o.24>  PDMA1      <0=> Secure <1=> Non-Secure
*/
#define SCU_INIT_PNSSET0_VAL      0x80000
/*
    PNSSET1
*/
/*
// Module 0..31
//   <o.17>  CRC       <0=> Secure <1=> Non-Secure
//   <o.18>  CRPT   <0=> Secure <1=> Non-Secure
*/
#define SCU_INIT_PNSSET1_VAL      0x00000
/*
    PNSSET2
*/
/*
// Module 0..31
//   <o.2>  EWDT        <0=> Secure <1=> Non-Secure
//   <o.3>  EADC        <0=> Secure <1=> Non-Secure
//   <o.5>  ACMP01      <0=> Secure <1=> Non-Secure
//
//   <o.7>  DAC         <0=> Secure <1=> Non-Secure
//   <o.8>  I2S0        <0=> Secure <1=> Non-Secure
//   <o.13>  OTG        <0=> Secure <1=> Non-Secure
//   <h> TIMER
//   <o.17>  TMR23      <0=> Secure <1=> Non-Secure
//   <o.18>  TMR45      <0=> Secure <1=> Non-Secure
//   </h>
//   <h> EPWM
//   <o.24>  EPWM0      <0=> Secure <1=> Non-Secure
//   <o.25>  EPWM1      <0=> Secure <1=> Non-Secure
//   </h>
//   <h> BPWM
//   <o.26>  BPWM0      <0=> Secure <1=> Non-Secure
//   <o.27>  BPWM1      <0=> Secure <1=> Non-Secure
//   </h>
*/
#define SCU_INIT_PNSSET2_VAL      0x0
/*
    PNSSET3
*/
/*
// Module 0..31
//   <h>  SPI
//   <o.0>  QSPI0       <0=> Secure <1=> Non-Secure
//   <o.1>  SPI0   <0=> Secure <1=> Non-Secure
//   <o.2>  SPI1      <0=> Secure <1=> Non-Secure
//   <o.3>  SPI2    <0=> Secure <1=> Non-Secure
//   <o.4>  SPI3      <0=> Secure <1=> Non-Secure
//   </h>
//   <h> UART
//   <o.16>  UART0      <0=> Secure <1=> Non-Secure
//   <o.17>  UART1      <0=> Secure <1=> Non-Secure
//   <o.18>  UART2      <0=> Secure <1=> Non-Secure
//   <o.19>  UART3      <0=> Secure <1=> Non-Secure
//   <o.20>  UART4      <0=> Secure <1=> Non-Secure
//   <o.21>  UART5      <0=> Secure <1=> Non-Secure
//   </h>
*/
#define SCU_INIT_PNSSET3_VAL      0x10000
/*
    PNSSET4
*/
/*
// Module 0..31
//   <h> I2C
//   <o.0>  I2C0       <0=> Secure <1=> Non-Secure
//   <o.1>  I2C1   <0=> Secure <1=> Non-Secure
//   <o.2>  I2C2      <0=> Secure <1=> Non-Secure
//   </h>
//   <h> Smart Card
//   <o.16>  SC0      <0=> Secure <1=> Non-Secure
//   <o.17>  SC1      <0=> Secure <1=> Non-Secure
//   <o.18>  SC2      <0=> Secure <1=> Non-Secure
//   </h>
*/
#define SCU_INIT_PNSSET4_VAL      0x0
/*
    PNSSET5
*/
/*
// Module 0..31
//   <o.0>  CAN0        <0=> Secure <1=> Non-Secure
//   <h> QEI
//   <o.16>  QEI0       <0=> Secure <1=> Non-Secure
//   <o.17>  QEI1       <0=> Secure <1=> Non-Secure
//   </h>
//   <h> ECAP
//   <o.20>  ECAP0      <0=> Secure <1=> Non-Secure
//   <o.21>  ECAP1      <0=> Secure <1=> Non-Secure
//   </h>
//   <o.25>  TRNG       <0=> Secure <1=> Non-Secure
//   <o.27>  LCD        <0=> Secure <1=> Non-Secure
*/
#define SCU_INIT_PNSSET5_VAL      0x0
/*
    PNSSET6
*/
/*
// Module 0..31
//   <o.0>  USBD       <0=> Secure <1=> Non-Secure
//   <h> USCI
//   <o.16>  USCI0   <0=> Secure <1=> Non-Secure
//   <o.17>  USCI1      <0=> Secure <1=> Non-Secure
//   </h>
*/
#define SCU_INIT_PNSSET6_VAL      0x0
/*
// </h>
*/



/*
// <h> GPIO Secure Attribution Configuration
*/

/*
    IONSSET
*/

/*
// Bit 0..31
//   <h> PA
//   <o.0>  PA0       <0=> Secure <1=> Non-Secure
//   <o.1>  PA1       <0=> Secure <1=> Non-Secure
//   <o.2>  PA2       <0=> Secure <1=> Non-Secure
//   <o.3>  PA3       <0=> Secure <1=> Non-Secure
//   <o.4>  PA4       <0=> Secure <1=> Non-Secure
//   <o.5>  PA5       <0=> Secure <1=> Non-Secure
//   <o.6>  PA6       <0=> Secure <1=> Non-Secure
//   <o.7>  PA7       <0=> Secure <1=> Non-Secure
//   <o.8>  PA8       <0=> Secure <1=> Non-Secure
//   <o.7>  PA9       <0=> Secure <1=> Non-Secure
//   <o.10>  PA10       <0=> Secure <1=> Non-Secure
//   <o.11>  PA11       <0=> Secure <1=> Non-Secure
//   <o.12>  PA12       <0=> Secure <1=> Non-Secure
//   <o.13>  PA13       <0=> Secure <1=> Non-Secure
//   <o.14>  PA14       <0=> Secure <1=> Non-Secure
//   <o.15>  PA15      <0=> Secure <1=> Non-Secure
//   </h>

*/
#define SCU_INIT_IONSSET0_VAL      0x00000000

/*
// Bit 0..31
//   <h> PB
//   <o.0>  PB0       <0=> Secure <1=> Non-Secure
//   <o.1>  PB1       <0=> Secure <1=> Non-Secure
//   <o.2>  PB2       <0=> Secure <1=> Non-Secure
//   <o.3>  PB3       <0=> Secure <1=> Non-Secure
//   <o.4>  PB4       <0=> Secure <1=> Non-Secure
//   <o.5>  PB5       <0=> Secure <1=> Non-Secure
//   <o.6>  PB6       <0=> Secure <1=> Non-Secure
//   <o.7>  PB7       <0=> Secure <1=> Non-Secure
//   <o.8>  PB8       <0=> Secure <1=> Non-Secure
//   <o.9>  PB9       <0=> Secure <1=> Non-Secure
//   <o.10>  PB10       <0=> Secure <1=> Non-Secure
//   <o.11>  PB11       <0=> Secure <1=> Non-Secure
//   <o.12>  PB12       <0=> Secure <1=> Non-Secure
//   <o.13>  PB13       <0=> Secure <1=> Non-Secure
//   <o.14>  PB14       <0=> Secure <1=> Non-Secure
//   <o.15>  PB15       <0=> Secure <1=> Non-Secure
//   </h>
*/
#define SCU_INIT_IONSSET1_VAL      0x00003000


/*
// Bit 0..31
//   <h> PC
//   <o.0>  PC0       <0=> Secure <1=> Non-Secure
//   <o.1>  PC1       <0=> Secure <1=> Non-Secure
//   <o.2>  PC2       <0=> Secure <1=> Non-Secure
//   <o.3>  PC3       <0=> Secure <1=> Non-Secure
//   <o.4>  PC4       <0=> Secure <1=> Non-Secure
//   <o.5>  PC5       <0=> Secure <1=> Non-Secure
//   <o.6>  PC6       <0=> Secure <1=> Non-Secure
//   <o.7>  PC7       <0=> Secure <1=> Non-Secure
//   <o.8>  PC8       <0=> Secure <1=> Non-Secure
//   <o.9>  PC9       <0=> Secure <1=> Non-Secure
//   <o.10>  PC10       <0=> Secure <1=> Non-Secure
//   <o.11>  PC11       <0=> Secure <1=> Non-Secure
//   <o.12>  PC12       <0=> Secure <1=> Non-Secure
//   <o.13>  PC13       <0=> Secure <1=> Non-Secure
//   </h>
*/
#define SCU_INIT_IONSSET2_VAL      0x00000003

/*
// Bit 0..31
//   <h> PD
//   <o.0>  PD0       <0=> Secure <1=> Non-Secure
//   <o.1>  PD1       <0=> Secure <1=> Non-Secure
//   <o.2>  PD2       <0=> Secure <1=> Non-Secure
//   <o.3>  PD3       <0=> Secure <1=> Non-Secure
//   <o.4>  PD4       <0=> Secure <1=> Non-Secure
//   <o.5>  PD5       <0=> Secure <1=> Non-Secure
//   <o.6>  PD6       <0=> Secure <1=> Non-Secure
//   <o.7>  PD7       <0=> Secure <1=> Non-Secure
//   <o.8>  PD8       <0=> Secure <1=> Non-Secure
//   <o.9>  PD9       <0=> Secure <1=> Non-Secure
//   <o.10>  PD10       <0=> Secure <1=> Non-Secure
//   <o.11>  PD11       <0=> Secure <1=> Non-Secure
//   <o.12>  PD12       <0=> Secure <1=> Non-Secure
//   <o.14>  PD14       <0=> Secure <1=> Non-Secure
//   </h>
*/
#define SCU_INIT_IONSSET3_VAL      0x00000000


/*
// Bit 0..31
//   <h> PE
//   <o.0>  PE0       <0=> Secure <1=> Non-Secure
//   <o.1>  PE1       <0=> Secure <1=> Non-Secure
//   <o.2>  PE2       <0=> Secure <1=> Non-Secure
//   <o.3>  PE3       <0=> Secure <1=> Non-Secure
//   <o.4>  PE4       <0=> Secure <1=> Non-Secure
//   <o.5>  PE5       <0=> Secure <1=> Non-Secure
//   <o.6>  PE6       <0=> Secure <1=> Non-Secure
//   <o.7>  PE7       <0=> Secure <1=> Non-Secure
//   <o.8>  PE8       <0=> Secure <1=> Non-Secure
//   <o.9>  PE9       <0=> Secure <1=> Non-Secure
//   <o.10>  PE10       <0=> Secure <1=> Non-Secure
//   <o.11>  PE11       <0=> Secure <1=> Non-Secure
//   <o.12>  PE12       <0=> Secure <1=> Non-Secure
//   <o.13>  PE13       <0=> Secure <1=> Non-Secure
//   <o.14>  PE14       <0=> Secure <1=> Non-Secure
//   <o.15>  PE15       <0=> Secure <1=> Non-Secure
//   </h>
*/
#define SCU_INIT_IONSSET4_VAL      0x00000000


/*
// Bit 0..31
//   <h> PF
//   <o.0>  PF0       <0=> Secure <1=> Non-Secure
//   <o.1>  PF1       <0=> Secure <1=> Non-Secure
//   <o.2>  PF2       <0=> Secure <1=> Non-Secure
//   <o.3>  PF3       <0=> Secure <1=> Non-Secure
//   <o.4>  PF4       <0=> Secure <1=> Non-Secure
//   <o.5>  PF5       <0=> Secure <1=> Non-Secure
//   <o.6>  PF6       <0=> Secure <1=> Non-Secure
//   <o.7>  PF7       <0=> Secure <1=> Non-Secure
//   <o.8>  PF8       <0=> Secure <1=> Non-Secure
//   <o.9>  PF9       <0=> Secure <1=> Non-Secure
//   <o.10>  PF10       <0=> Secure <1=> Non-Secure
//   <o.11>  PF11       <0=> Secure <1=> Non-Secure
//   </h>
*/
#define SCU_INIT_IONSSET5_VAL      0x00000000


/*
// Bit 0..31
//   <h> PG
//   <o.2>  PG2       <0=> Secure <1=> Non-Secure
//   <o.3>  PG3       <0=> Secure <1=> Non-Secure
//   <o.4>  PG4       <0=> Secure <1=> Non-Secure
//   <o.9>  PG9       <0=> Secure <1=> Non-Secure
//   <o.10>  PG10       <0=> Secure <1=> Non-Secure
//   <o.11>  PG11       <0=> Secure <1=> Non-Secure
//   <o.12>  PG12       <0=> Secure <1=> Non-Secure
//   <o.13>  PG13       <0=> Secure <1=> Non-Secure
//   <o.14>  PG14       <0=> Secure <1=> Non-Secure
//   <o.15>  PG15       <0=> Secure <1=> Non-Secure
//   </h>
*/
#define SCU_INIT_IONSSET6_VAL      0x00000000

/*
// Bit 0..31
//   <h> PH
//   <o.4>  PH4       <0=> Secure <1=> Non-Secure
//   <o.5>  PH5       <0=> Secure <1=> Non-Secure
//   <o.6>  PH6       <0=> Secure <1=> Non-Secure
//   <o.7>  PH7       <0=> Secure <1=> Non-Secure
//   <o.8>  PH8       <0=> Secure <1=> Non-Secure
//   <o.9>  PH9       <0=> Secure <1=> Non-Secure
//   <o.10>  PH10       <0=> Secure <1=> Non-Secure
//   <o.11>  PH11       <0=> Secure <1=> Non-Secure
//   </h>
*/
#define SCU_INIT_IONSSET7_VAL      0x00000000

/*
// </h>
*/



/*
// <h>Assign GPIO Interrupt to Secure or Non-secure Vector
*/


/*
    Initialize GPIO ITNS (Interrupts 0..31)
*/

/*
// Bit 0..31
//   <o.0>  GPA         <0=> Secure <1=> Non-Secure
//   <o.1>  GPB         <0=> Secure <1=> Non-Secure
//   <o.2>  GPC         <0=> Secure <1=> Non-Secure
//   <o.3>  GPD         <0=> Secure <1=> Non-Secure
//   <o.4>  GPE         <0=> Secure <1=> Non-Secure
//   <o.5>  GPF         <0=> Secure <1=> Non-Secure
//   <o.6>  GPG         <0=> Secure <1=> Non-Secure
//   <o.7>  GPH         <0=> Secure <1=> Non-Secure
//   <o.8>  EINT0         <0=> Secure <1=> Non-Secure
//   <o.9>  EINT1         <0=> Secure <1=> Non-Secure
//   <o.10>  EINT2         <0=> Secure <1=> Non-Secure
//   <o.11>  EINT3         <0=> Secure <1=> Non-Secure
//   <o.12>  EINT4         <0=> Secure <1=> Non-Secure
//   <o.13>  EINT5         <0=> Secure <1=> Non-Secure
//   <o.14>  EINT6         <0=> Secure <1=> Non-Secure
//   <o.15>  EINT7         <0=> Secure <1=> Non-Secure
*/
#define SCU_INIT_IONSSET_VAL      0x0000
/*
// </h>
*/

#endif  /* PARTITION_M3351 */

/*** (C) COPYRIGHT 2020 Nuvoton Technology Corp. ***/
