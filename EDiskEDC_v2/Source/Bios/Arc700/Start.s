//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Arc700/Start.s,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2013/11/11 08:20:49 $
// $Id: Start.s,v 1.3 2013/11/11 08:20:49 rcantong Exp $
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
    .section .start, text


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
    .global _start


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : _start
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_start:

    ; Disable all irqs until system ready
    flag    0

    ; Invalidate icache and dcache
    mov     r0, 1
    sr      r0, [0x10]
    sr      r0, [0x47]

    ; Enable icache
    mov_s   r0, 0x02
    sr      r0, [0x11]

    ; Enable dcache
    mov_s   r0, 0x82
    sr      r0, [0x48]

    ; Setup stack
    mov     sp, _estack

    ; Jump to main
    bl      main

    ; Jump to sched loop
    b       _sched_prcs_pcb


end:
//=============================================================================
// $Log: Start.s,v $
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:44:22  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:08  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
