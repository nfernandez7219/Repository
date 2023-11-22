//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Bios/Bios.h,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/02/02 09:57:26 $
// $Id: Bios.h,v 1.5 2014/02/02 09:57:26 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__BIOS_H__)
#define __BIOS_H__

#if defined(DEBUG)
_Inline void bios_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define EDC_BOOT_CORE                   0
#define MAINFW_A_BLK_PBA                ((unsigned long)(0 * SEGMENTS_PER_BLK))
#define MAINFW_B_BLK_PBA                ((unsigned long)(1 * SEGMENTS_PER_BLK))
#define MAX_FW_SIZE                     0x80000 // 512KB

#define BIOS_IDLE_TIME                  12000 // "1 = 5mS"  1 min


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct BiosParmStruct
{
    unsigned long CpuId;
    unsigned long DmId;
    unsigned long IdleTmrCnt;
    unsigned long Filler;
} BIOS_PARM_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define BIOS_RESET_IDLE_TMR()           (BiosParm.IdleTmrCnt = 0)

#define BIOS_INC_IDLE_TMR()             (BiosParm.IdleTmrCnt++)

#define BIOS_IDLE_TM_REACH()            (BiosParm.IdleTmrCnt == BIOS_IDLE_TIME)

#define BIOS_IDLE_DRV()                 (BiosParm.IdleTmrCnt >= BIOS_IDLE_TIME)


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

extern unsigned long _vector[];
extern unsigned long _bootstrap_addr[];
extern unsigned long _bootstrap_size[];
extern unsigned long _firmware_size[];
extern unsigned long _unused_sdram[];

#pragma BSS(".dccm")
extern BIOS_PARM_STRUCT BiosParm;
#pragma BSS()


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void bios_main (void);

void crcore_init (void);

void bios_enable_cores (void);

void bios_jumper_init (void);

void bios_flush_dc (void);

void bios_flush_dc_lines (unsigned long Addr,
                          unsigned long ByteSize);

void bios_invalidate_dc_lines (unsigned long Addr,
                               unsigned long ByteSize);

void main (void);


//-----------------------------------------------------------------------------
// Function    : bios_flush_dc_line
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Asm void bios_flush_dc_line (void *Addr)
{
%reg Addr
    sr      Addr, [0x4C] //REG_DC_FLDL
    nop
    nop
    nop
    nop
}


//-----------------------------------------------------------------------------
// Function    : bios_invalidate_dc_line
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Asm void bios_invalidate_dc_line (void *Addr)
{
%reg Addr
    sr      Addr, [0x4A] //REG_DC_IVDL
    nop
    nop
    nop
    nop
}


#endif
//=============================================================================
// $Log: Bios.h,v $
// Revision 1.5  2014/02/02 09:57:26  rcantong
// 1. DEV: Standalone for SLC boards
// 1.1 Changed MAX_FW_SIZE from 1MB to 512KB
//
// Revision 1.4  2014/01/08 12:42:55  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
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
