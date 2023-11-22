//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Arc700/BootCodeEntry.s,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2014/01/08 12:42:56 $
// $Id: BootCodeEntry.s,v 1.3 2014/01/08 12:42:56 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#define __ASSEMBLY__


//-----------------------------------------------------------------------------
// Global Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Section Definitions
//-----------------------------------------------------------------------------
    .section .bootstrapldr, text


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
    .global _bootcodeentry


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : _bootcodeentry
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
    .align 4
_bootcodeentry:

    ; fpcore reloc
    mov     r3, 0xFC400000
    st      r3, [0xFFCFC084]
    st      r3, [0xFFCFC004]

    ; C part here, must initialize sp
    mov     sp, _estack
    bl      bootcodeldr_setup_ecc

    ; Wait for sram to ddr dgp completion
1:  ld.di   r0, [0xFFCD302C]
    and.f   r1, r0, 1                  ; dgp_done
    bz      1b

    ; Clear dp intrpt status
    mov     r0, 1
    st.di   r0, [0xFFCD3004]
    sync

    ; Disable dp after use
    mov     r0, 0x00010000
    mov     r1, 0xFFCE3038
    st.di   r0, [r1]
    mov     r0, 0x00016000
    st.di   r0, [r1]

    bl      bootcodeldr_main

    ; Disable arc cache
    sr      0x03, [0x11]
    sr      0x83, [0x48]

    j        0

end:
//=============================================================================
// $Log: BootCodeEntry.s,v $
// Revision 1.3  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.2  2013/08/08 16:44:21  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:08  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
