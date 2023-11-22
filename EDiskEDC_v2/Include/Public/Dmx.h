//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Public/Dmx.h,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/04/30 15:14:17 $
// $Id: Dmx.h,v 1.7 2014/04/30 15:14:17 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__DMX_H__)
#define __DMX_H__

#if defined(DEBUG)
_Inline void dmx_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define DMX_OPS_ERASE_ERROR             1
#define DMX_OPS_ECC_CORRECTABLE         2
#define DMX_OPS_ECC_UNCORRECTABLE       3
#define DMX_OPS_ERASED                  4
#define DMX_OPS_PROGRAM_ERROR           5
#define DMX_OPS_CMD_RETRY               6


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define CALC_PBA(DevIdx, BlkIdx, SegIdx) \
    (((BlkIdx) * SEGMENTS_PER_SYSBLK) + \
    ((DevIdx) * SEGMENTS_PER_BLK) + \
    (SegIdx))

#define GET_DEV_FROM_PBA(Pba) \
    ((Pba / SEGMENTS_PER_BLK) % DEV_CNT)


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void dmx_lite_init (void);

void dmx_init (void);

void dmx_init_enable_dmx_que (void);

void dmx_ops_init_malloc (unsigned long FbxIdx);

void dmx_ops_erase (PCB_STRUCT *PcbPtr);

void dmx_ops_write (PCB_STRUCT *PcbPtr);

void dmx_ops_read (PCB_STRUCT *PcbPtr);

void dmx_ops_write_user (PCB_STRUCT *PcbPtr);

void dmx_ops_read_user (PCB_STRUCT *PcbPtr);

void dmx_ops_write_cad (PCB_STRUCT *PcbPtr);

void dmx_ops_write_n_read (PCB_STRUCT *PcbPtr);

void dmx_ops_write_in_place (PCB_STRUCT *PcbPtr);

void dmx_ops_read_in_place (PCB_STRUCT *PcbPtr);

PBA_INT dmx_incr_pba_by_devpage (PBA_INT Pba);


#endif
//=============================================================================
// $Log: Dmx.h,v $
// Revision 1.7  2014/04/30 15:14:17  rcantong
// 1. DEV: Handling of HW error that needed retry
// 1.1. Added DMX_OPS_CMD_RETRY stat for dmx ops - JFaustino
//
// Revision 1.6  2014/03/03 12:47:11  rcantong
// 1. DEV: FID hang handler
// 1.1 Added dmx ops error define for FID_HANG_TIMEOUT - JFaustino
//
// Revision 1.5  2014/02/02 08:52:34  rcantong
// 1. DEV: Support 256Gb Toshiba MLC
// 1.1 Added configuration for 256Gb Toshiba MLC
//
// Revision 1.4  2013/12/05 13:06:33  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:06  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:00  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
