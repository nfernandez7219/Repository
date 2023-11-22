//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Bios/Interrupt.h,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2013/08/08 16:42:06 $
// $Id: Interrupt.h,v 1.2 2013/08/08 16:42:06 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__INTERRUPT_H__)
#define __INTERRUPT_H__

#if defined(DEBUG)
_Inline void interrupt_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define IC_CIR_PTR                      ((volatile unsigned long *)0xFFCE4000)
#define IC_EX_IPR_PTR                   ((volatile unsigned long *)0xFFCE4004)
#define IC_EX_TTR_PTR                   ((volatile unsigned long *)0xFFCE4008)
#define IC_EX_PTR_PTR                   ((volatile unsigned long *)0xFFCE400C)
#define IC_EX_ISR_PTR                   ((volatile unsigned long *)0xFFCE4010)
#define IC_EX_IMR_PTR                   ((volatile unsigned long *)0xFFCE4014)

#define IC_SIR0_PTR                     ((volatile unsigned long *)0xFFCE4018)
#define IC_SIR1_PTR                     ((volatile unsigned long *)0xFFCE401C)
#define IC_SIR2_PTR                     ((volatile unsigned long *)0xFFCE4020)
#define IC_SIR3_PTR                     ((volatile unsigned long *)0xFFCE4024)

#define IC_PLR0_PTR                     ((volatile unsigned long *)0xFFCE4038)
#define IC_PLR1_PTR                     ((volatile unsigned long *)0xFFCE403C)
#define IC_PLR2_PTR                     ((volatile unsigned long *)0xFFCE4040)
#define IC_PLR3_PTR                     ((volatile unsigned long *)0xFFCE4044)
#define IC_PLR4_PTR                     ((volatile unsigned long *)0xFFCE4048)
#define IC_PLR5_PTR                     ((volatile unsigned long *)0xFFCE404C)
#define IC_PLR6_PTR                     ((volatile unsigned long *)0xFFCE4050)
#define IC_PLR7_PTR                     ((volatile unsigned long *)0xFFCE4054)

#define IC_IRR0_PTR                     ((volatile unsigned long *)0xFFCE4078)
#define IC_IRR1_PTR                     ((volatile unsigned long *)0xFFCE407C)
#define IC_IRR2_PTR                     ((volatile unsigned long *)0xFFCE4080)
#define IC_IRR3_PTR                     ((volatile unsigned long *)0xFFCE4084)
#define IC_IRR4_PTR                     ((volatile unsigned long *)0xFFCE4088)
#define IC_IRR5_PTR                     ((volatile unsigned long *)0xFFCE408C)
#define IC_IRR6_PTR                     ((volatile unsigned long *)0xFFCE4090)
#define IC_IRR7_PTR                     ((volatile unsigned long *)0xFFCE4094)

#define IC_CPU0_IN5_IVR_PTR             ((volatile unsigned long *)0xFFCE40B8)
#define IC_CPU0_IN6_IVR_PTR             ((volatile unsigned long *)0xFFCE40BC)
#define IC_CPU0_IN7_IVR_PTR             ((volatile unsigned long *)0xFFCE40C0)
#define IC_CPU0_IN8_IVR_PTR             ((volatile unsigned long *)0xFFCE40C4)
#define IC_CPU0_IN9_IVR_PTR             ((volatile unsigned long *)0xFFCE40C8)
#define IC_CPU0_IN10_IVR_PTR            ((volatile unsigned long *)0xFFCE40CC)
#define IC_CPU0_IN11_IVR_PTR            ((volatile unsigned long *)0xFFCE40D0)
#define IC_CPU0_IN12_IVR_PTR            ((volatile unsigned long *)0xFFCE40D4)
#define IC_CPU0_IN13_IVR_PTR            ((volatile unsigned long *)0xFFCE40D8)
#define IC_CPU0_IN14_IVR_PTR            ((volatile unsigned long *)0xFFCE40DC)
#define IC_CPU0_IN15_IVR_PTR            ((volatile unsigned long *)0xFFCE40E0)
#define IC_CPU0_IN16_IVR_PTR            ((volatile unsigned long *)0xFFCE40E4)
#define IC_CPU0_IN17_IVR_PTR            ((volatile unsigned long *)0xFFCE40E8)
#define IC_CPU0_IN18_IVR_PTR            ((volatile unsigned long *)0xFFCE40EC)
#define IC_CPU0_IN19_IVR_PTR            ((volatile unsigned long *)0xFFCE40F0)
#define IC_CPU0_IN20_IVR_PTR            ((volatile unsigned long *)0xFFCE40F4)
#define IC_CPU0_IN21_IVR_PTR            ((volatile unsigned long *)0xFFCE40F8)

#define IC_CPU1_IN5_IVR_PTR             ((volatile unsigned long *)0xFFCE4108)
#define IC_CPU1_IN6_IVR_PTR             ((volatile unsigned long *)0xFFCE410C)
#define IC_CPU1_IN7_IVR_PTR             ((volatile unsigned long *)0xFFCE4110)
#define IC_CPU1_IN8_IVR_PTR             ((volatile unsigned long *)0xFFCE4114)
#define IC_CPU1_IN9_IVR_PTR             ((volatile unsigned long *)0xFFCE4118)
#define IC_CPU1_IN10_IVR_PTR            ((volatile unsigned long *)0xFFCE411C)
#define IC_CPU1_IN11_IVR_PTR            ((volatile unsigned long *)0xFFCE4120)
#define IC_CPU1_IN12_IVR_PTR            ((volatile unsigned long *)0xFFCE4124)
#define IC_CPU1_IN13_IVR_PTR            ((volatile unsigned long *)0xFFCE4128)
#define IC_CPU1_IN14_IVR_PTR            ((volatile unsigned long *)0xFFCE412C)
#define IC_CPU1_IN15_IVR_PTR            ((volatile unsigned long *)0xFFCE4130)
#define IC_CPU1_IN16_IVR_PTR            ((volatile unsigned long *)0xFFCE4134)
#define IC_CPU1_IN17_IVR_PTR            ((volatile unsigned long *)0xFFCE4138)
#define IC_CPU1_IN18_IVR_PTR            ((volatile unsigned long *)0xFFCE413C)
#define IC_CPU1_IN19_IVR_PTR            ((volatile unsigned long *)0xFFCE4140)
#define IC_CPU1_IN20_IVR_PTR            ((volatile unsigned long *)0xFFCE4144)
#define IC_CPU1_IN21_IVR_PTR            ((volatile unsigned long *)0xFFCE4148)

#define IC_CPU2_IN5_IVR_PTR             ((volatile unsigned long *)0xFFCE4158)
#define IC_CPU2_IN6_IVR_PTR             ((volatile unsigned long *)0xFFCE415C)
#define IC_CPU2_IN7_IVR_PTR             ((volatile unsigned long *)0xFFCE4160)
#define IC_CPU2_IN8_IVR_PTR             ((volatile unsigned long *)0xFFCE4164)
#define IC_CPU2_IN9_IVR_PTR             ((volatile unsigned long *)0xFFCE4168)
#define IC_CPU2_IN10_IVR_PTR            ((volatile unsigned long *)0xFFCE416C)
#define IC_CPU2_IN11_IVR_PTR            ((volatile unsigned long *)0xFFCE4170)
#define IC_CPU2_IN12_IVR_PTR            ((volatile unsigned long *)0xFFCE4174)
#define IC_CPU2_IN13_IVR_PTR            ((volatile unsigned long *)0xFFCE4178)
#define IC_CPU2_IN14_IVR_PTR            ((volatile unsigned long *)0xFFCE417C)
#define IC_CPU2_IN15_IVR_PTR            ((volatile unsigned long *)0xFFCE4180)
#define IC_CPU2_IN16_IVR_PTR            ((volatile unsigned long *)0xFFCE4184)
#define IC_CPU2_IN17_IVR_PTR            ((volatile unsigned long *)0xFFCE4188)
#define IC_CPU2_IN18_IVR_PTR            ((volatile unsigned long *)0xFFCE418C)
#define IC_CPU2_IN19_IVR_PTR            ((volatile unsigned long *)0xFFCE4190)
#define IC_CPU2_IN20_IVR_PTR            ((volatile unsigned long *)0xFFCE4194)
#define IC_CPU2_IN21_IVR_PTR            ((volatile unsigned long *)0xFFCE4198)

#define IC_CPU3_IN5_IVR_PTR             ((volatile unsigned long *)0xFFCE41A8)
#define IC_CPU3_IN6_IVR_PTR             ((volatile unsigned long *)0xFFCE41AC)
#define IC_CPU3_IN7_IVR_PTR             ((volatile unsigned long *)0xFFCE41B0)
#define IC_CPU3_IN8_IVR_PTR             ((volatile unsigned long *)0xFFCE41B4)
#define IC_CPU3_IN9_IVR_PTR             ((volatile unsigned long *)0xFFCE41B8)
#define IC_CPU3_IN10_IVR_PTR            ((volatile unsigned long *)0xFFCE41BC)
#define IC_CPU3_IN11_IVR_PTR            ((volatile unsigned long *)0xFFCE41C0)
#define IC_CPU3_IN12_IVR_PTR            ((volatile unsigned long *)0xFFCE41C4)
#define IC_CPU3_IN13_IVR_PTR            ((volatile unsigned long *)0xFFCE41C8)
#define IC_CPU3_IN14_IVR_PTR            ((volatile unsigned long *)0xFFCE41CC)
#define IC_CPU3_IN15_IVR_PTR            ((volatile unsigned long *)0xFFCE41D0)
#define IC_CPU3_IN16_IVR_PTR            ((volatile unsigned long *)0xFFCE41D4)
#define IC_CPU3_IN17_IVR_PTR            ((volatile unsigned long *)0xFFCE41D8)
#define IC_CPU3_IN18_IVR_PTR            ((volatile unsigned long *)0xFFCE41DC)
#define IC_CPU3_IN19_IVR_PTR            ((volatile unsigned long *)0xFFCE41E0)
#define IC_CPU3_IN20_IVR_PTR            ((volatile unsigned long *)0xFFCE41E4)
#define IC_CPU3_IN21_IVR_PTR            ((volatile unsigned long *)0xFFCE41E8)

#define IC_VIRR_PTR                     ((volatile unsigned long *)0xFFCE4208)
#define IC_VMCR_PTR                     ((volatile unsigned long *)0xFFCE4210)
#define IC_IPIR0_PTR                    ((volatile unsigned long *)0xFFCE4214)
#define IC_IPIR1_PTR                    ((volatile unsigned long *)0xFFCE4218)
#define IC_IPIR2_PTR                    ((volatile unsigned long *)0xFFCE421C)
#define IC_IPIR3_PTR                    ((volatile unsigned long *)0xFFCE4220)
#define IC_IPICR_PTR                    ((volatile unsigned long *)0xFFCE4224)
#define IC_TESTMODE_PTR                 ((volatile unsigned long *)0xFFCE4228)

#define IC_CPU0_IN5_IIRR_PTR            ((volatile unsigned long *)0xFFCE422C)
#define IC_CPU0_IN6_IIRR_PTR            ((volatile unsigned long *)0xFFCE4230)
#define IC_CPU0_IN7_IIRR_PTR            ((volatile unsigned long *)0xFFCE4234)
#define IC_CPU0_IN8_IIRR_PTR            ((volatile unsigned long *)0xFFCE4238)
#define IC_CPU0_IN9_IIRR_PTR            ((volatile unsigned long *)0xFFCE423C)
#define IC_CPU0_IN10_IIRR_PTR           ((volatile unsigned long *)0xFFCE4240)
#define IC_CPU0_IN11_IIRR_PTR           ((volatile unsigned long *)0xFFCE4244)
#define IC_CPU0_IN12_IIRR_PTR           ((volatile unsigned long *)0xFFCE4248)
#define IC_CPU0_IN13_IIRR_PTR           ((volatile unsigned long *)0xFFCE424C)
#define IC_CPU0_IN14_IIRR_PTR           ((volatile unsigned long *)0xFFCE4250)
#define IC_CPU0_IN15_IIRR_PTR           ((volatile unsigned long *)0xFFCE4254)
#define IC_CPU0_IN16_IIRR_PTR           ((volatile unsigned long *)0xFFCE4258)
#define IC_CPU0_IN17_IIRR_PTR           ((volatile unsigned long *)0xFFCE425C)
#define IC_CPU0_IN18_IIRR_PTR           ((volatile unsigned long *)0xFFCE4260)
#define IC_CPU0_IN19_IIRR_PTR           ((volatile unsigned long *)0xFFCE4264)
#define IC_CPU0_IN20_IIRR_PTR           ((volatile unsigned long *)0xFFCE4268)
#define IC_CPU0_IN21_IIRR_PTR           ((volatile unsigned long *)0xFFCE426C)

#define IC_CPU1_IN5_IIRR_PTR            ((volatile unsigned long *)0xFFCE4270)
#define IC_CPU1_IN6_IIRR_PTR            ((volatile unsigned long *)0xFFCE4274)
#define IC_CPU1_IN7_IIRR_PTR            ((volatile unsigned long *)0xFFCE4278)
#define IC_CPU1_IN8_IIRR_PTR            ((volatile unsigned long *)0xFFCE427C)
#define IC_CPU1_IN9_IIRR_PTR            ((volatile unsigned long *)0xFFCE4280)
#define IC_CPU1_IN10_IIRR_PTR           ((volatile unsigned long *)0xFFCE4284)
#define IC_CPU1_IN11_IIRR_PTR           ((volatile unsigned long *)0xFFCE4288)
#define IC_CPU1_IN12_IIRR_PTR           ((volatile unsigned long *)0xFFCE428C)
#define IC_CPU1_IN13_IIRR_PTR           ((volatile unsigned long *)0xFFCE4290)
#define IC_CPU1_IN14_IIRR_PTR           ((volatile unsigned long *)0xFFCE4294)
#define IC_CPU1_IN15_IIRR_PTR           ((volatile unsigned long *)0xFFCE4298)
#define IC_CPU1_IN16_IIRR_PTR           ((volatile unsigned long *)0xFFCE429C)
#define IC_CPU1_IN17_IIRR_PTR           ((volatile unsigned long *)0xFFCE42A0)
#define IC_CPU1_IN18_IIRR_PTR           ((volatile unsigned long *)0xFFCE42A4)
#define IC_CPU1_IN19_IIRR_PTR           ((volatile unsigned long *)0xFFCE42A8)
#define IC_CPU1_IN20_IIRR_PTR           ((volatile unsigned long *)0xFFCE42AC)
#define IC_CPU1_IN21_IIRR_PTR           ((volatile unsigned long *)0xFFCE42B0)

#define IC_CPU2_IN5_IIRR_PTR            ((volatile unsigned long *)0xFFCE42B4)
#define IC_CPU2_IN6_IIRR_PTR            ((volatile unsigned long *)0xFFCE42B8)
#define IC_CPU2_IN7_IIRR_PTR            ((volatile unsigned long *)0xFFCE42BC)
#define IC_CPU2_IN8_IIRR_PTR            ((volatile unsigned long *)0xFFCE42C0)
#define IC_CPU2_IN9_IIRR_PTR            ((volatile unsigned long *)0xFFCE42C4)
#define IC_CPU2_IN10_IIRR_PTR           ((volatile unsigned long *)0xFFCE42C8)
#define IC_CPU2_IN11_IIRR_PTR           ((volatile unsigned long *)0xFFCE42CC)
#define IC_CPU2_IN12_IIRR_PTR           ((volatile unsigned long *)0xFFCE42D0)
#define IC_CPU2_IN13_IIRR_PTR           ((volatile unsigned long *)0xFFCE42D4)
#define IC_CPU2_IN14_IIRR_PTR           ((volatile unsigned long *)0xFFCE42D8)
#define IC_CPU2_IN15_IIRR_PTR           ((volatile unsigned long *)0xFFCE42DC)
#define IC_CPU2_IN16_IIRR_PTR           ((volatile unsigned long *)0xFFCE42E0)
#define IC_CPU2_IN17_IIRR_PTR           ((volatile unsigned long *)0xFFCE42E4)
#define IC_CPU2_IN18_IIRR_PTR           ((volatile unsigned long *)0xFFCE42E8)
#define IC_CPU2_IN19_IIRR_PTR           ((volatile unsigned long *)0xFFCE42EC)
#define IC_CPU2_IN20_IIRR_PTR           ((volatile unsigned long *)0xFFCE42F0)
#define IC_CPU2_IN21_IIRR_PTR           ((volatile unsigned long *)0xFFCE42F4)

#define IC_CPU3_IN5_IIRR_PTR            ((volatile unsigned long *)0xFFCE42F8)
#define IC_CPU3_IN6_IIRR_PTR            ((volatile unsigned long *)0xFFCE42FC)
#define IC_CPU3_IN7_IIRR_PTR            ((volatile unsigned long *)0xFFCE4300)
#define IC_CPU3_IN8_IIRR_PTR            ((volatile unsigned long *)0xFFCE4304)
#define IC_CPU3_IN9_IIRR_PTR            ((volatile unsigned long *)0xFFCE4308)
#define IC_CPU3_IN10_IIRR_PTR           ((volatile unsigned long *)0xFFCE430C)
#define IC_CPU3_IN11_IIRR_PTR           ((volatile unsigned long *)0xFFCE4310)
#define IC_CPU3_IN12_IIRR_PTR           ((volatile unsigned long *)0xFFCE4314)
#define IC_CPU3_IN13_IIRR_PTR           ((volatile unsigned long *)0xFFCE4318)
#define IC_CPU3_IN14_IIRR_PTR           ((volatile unsigned long *)0xFFCE431C)
#define IC_CPU3_IN15_IIRR_PTR           ((volatile unsigned long *)0xFFCE4320)
#define IC_CPU3_IN16_IIRR_PTR           ((volatile unsigned long *)0xFFCE4324)
#define IC_CPU3_IN17_IIRR_PTR           ((volatile unsigned long *)0xFFCE4328)
#define IC_CPU3_IN18_IIRR_PTR           ((volatile unsigned long *)0xFFCE432C)
#define IC_CPU3_IN19_IIRR_PTR           ((volatile unsigned long *)0xFFCE4330)
#define IC_CPU3_IN20_IIRR_PTR           ((volatile unsigned long *)0xFFCE4334)
#define IC_CPU3_IN21_IIRR_PTR           ((volatile unsigned long *)0xFFCE4338)


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------


#endif
//=============================================================================
// $Log: Interrupt.h,v $
// Revision 1.2  2013/08/08 16:42:06  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:01  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
