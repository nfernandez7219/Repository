//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Bios/EdcLocalCsr.h,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2013/08/08 16:42:06 $
// $Id: EdcLocalCsr.h,v 1.2 2013/08/08 16:42:06 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__EDC_LOCALCSR_H__)
#define __EDC_LOCALCSR_H__

#if defined(DEBUG)
_Inline void edc_localcsr_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define LC_CIR_PTR                      ((volatile unsigned long *)0xFFCFF000)
#define LC_BESR_PTR                     ((volatile unsigned long *)0xFFCFF004)
#define LC_BEAR_M0_PTR                  ((volatile unsigned long *)0xFFCFF008)
#define LC_BEAR_M1_PTR                  ((volatile unsigned long *)0xFFCFF00C)
#define LC_BEAR_M2_PTR                  ((volatile unsigned long *)0xFFCFF010)
#define LC_BEAR_M3_PTR                  ((volatile unsigned long *)0xFFCFF014)
#define LC_BEAR_M4_PTR                  ((volatile unsigned long *)0xFFCFF018)
#define LC_BEAR_M5_PTR                  ((volatile unsigned long *)0xFFCFF01C)
#define LC_BEAR_M6_PTR                  ((volatile unsigned long *)0xFFCFF020)
#define LC_BEAR_M7_PTR                  ((volatile unsigned long *)0xFFCFF024)
#define LC_MEIRM_PTR                    ((volatile unsigned long *)0xFFCFF028)
#define LC_ATEAR_M0_PTR                 ((volatile unsigned long *)0xFFCFF02C)
#define LC_ATEAR_M1_PTR                 ((volatile unsigned long *)0xFFCFF030)
#define LC_ATEAR_M2_PTR                 ((volatile unsigned long *)0xFFCFF034)
#define LC_ATEAR_M3_PTR                 ((volatile unsigned long *)0xFFCFF038)
#define LC_SLIO0_PTR                    ((volatile unsigned long *)0xFFCFF03C)
#define LC_SLIO1_PTR                    ((volatile unsigned long *)0xFFCFF040)
#define LC_SLIO2_PTR                    ((volatile unsigned long *)0xFFCFF044)
#define LC_SLIO3_PTR                    ((volatile unsigned long *)0xFFCFF048)
#define LC_SLIO4_PTR                    ((volatile unsigned long *)0xFFCFF04C)
#define LC_SLIO5_PTR                    ((volatile unsigned long *)0xFFCFF050)
#define LC_AKTOUT_PTR                   ((volatile unsigned long *)0xFFCFF054)
#define LC_REQTHRESH_PTR                ((volatile unsigned long *)0xFFCFF058)


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
// $Log: EdcLocalCsr.h,v $
// Revision 1.2  2013/08/08 16:42:06  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:01  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
