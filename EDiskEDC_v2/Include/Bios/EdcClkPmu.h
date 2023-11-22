//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Bios/EdcClkPmu.h,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2013/11/11 08:20:47 $
// $Id: EdcClkPmu.h,v 1.3 2013/11/11 08:20:47 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__EDC_CLKPMU_H__)
#define __EDC_CLKPMU_H__

#if defined(DEBUG)
_Inline void edc_clkpmu_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define CR_CIR_PTR                      ((volatile unsigned long *)0xFFCE3000)
#define CR_POR_PTR                      ((volatile unsigned long *)0xFFCE3004)
#define CR_P0CR_PTR                     ((volatile unsigned long *)0xFFCE3008)
#define CR_P1CR_PTR                     ((volatile unsigned long *)0xFFCE300C)
#define CR_P2CR_PTR                     ((volatile unsigned long *)0xFFCE3010)
#define CR_P3CR_PTR                     ((volatile unsigned long *)0xFFCE3014)
#define CR_OSCSR_PTR                    ((volatile unsigned long *)0xFFCE3018)
#define CR_PCSR_PTR                     ((volatile unsigned long *)0xFFCE301C)
#define CR_LOCK_PTR                     ((volatile unsigned long *)0xFFCE3020)
#define CR_ISC_PTR                      ((volatile unsigned long *)0xFFCE3024)
#define CR_CGDLY_PTR                    ((volatile unsigned long *)0xFFCE3028)
#define CR_SSCCFGSEL_PTR                ((volatile unsigned long *)0xFFCE302C)
#define CR_PMU0_CPU_PTR                 ((volatile unsigned long *)0xFFCE3030)
#define CR_PMU1_AR_DP0_PTR              ((volatile unsigned long *)0xFFCE3034)
#define CR_PMU1_AR_DP1_PTR              ((volatile unsigned long *)0xFFCE3038)
#define CR_PMU1_AR_DP2_PTR              ((volatile unsigned long *)0xFFCE303C)
#define CR_PMU1_AR_DP3_PTR              ((volatile unsigned long *)0xFFCE3040)
#define CR_PMU1_AR_DP4_PTR              ((volatile unsigned long *)0xFFCE3044)
#define CR_PMU2_SD_PTR                  ((volatile unsigned long *)0xFFCE3048)
#define CR_PMU3_PE0_PTR                 ((volatile unsigned long *)0xFFCE304C)
#define CR_PMU3_PE1_PTR                 ((volatile unsigned long *)0xFFCE3050)
#define CR_PMU4_PER0_PTR                ((volatile unsigned long *)0xFFCE3054)
#define CR_PMU4_PER1_PTR                ((volatile unsigned long *)0xFFCE3058)
#define CR_PMU5_SA0_PTR                 ((volatile unsigned long *)0xFFCE305C)
#define CR_PMU5_SA1_PTR                 ((volatile unsigned long *)0xFFCE3060)
#define CR_PMU6_SYS_PTR                 ((volatile unsigned long *)0xFFCE3064)
#define CR_PMU7_DM0_PTR                 ((volatile unsigned long *)0xFFCE3068)
#define CR_PMU7_DM1_PTR                 ((volatile unsigned long *)0xFFCE306C)
#define CR_PMU7_DM2_PTR                 ((volatile unsigned long *)0xFFCE3070)
#define CR_PMU7_DM3_PTR                 ((volatile unsigned long *)0xFFCE3074)
#define CR_PMU7_DM4_PTR                 ((volatile unsigned long *)0xFFCE3078)
#define CR_PMU7_DM5_PTR                 ((volatile unsigned long *)0xFFCE307C)
#define CR_PMU7_DM6_PTR                 ((volatile unsigned long *)0xFFCE3080)
#define CR_PMU1_AR_DP_STAT_PTR          ((volatile unsigned long *)0xFFCE3084)
#define CR_PMU3_PE_STAT_PTR             ((volatile unsigned long *)0xFFCE3088)
#define CR_PMU4_PER_STAT_PTR            ((volatile unsigned long *)0xFFCE308C)
#define CR_PMU5_SA_STAT_PTR             ((volatile unsigned long *)0xFFCE3090)
#define CR_PMU7_DM_STAT_PTR             ((volatile unsigned long *)0xFFCE3094)
#define CR_PHYTESTCLKEN_PTR             ((volatile unsigned long *)0xFFCE3098)


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
// $Log: EdcClkPmu.h,v $
// Revision 1.3  2013/11/11 08:20:47  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:06  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:01  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
