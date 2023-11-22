//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Compact/CompactI.h,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/04/30 15:40:09 $
// $Id: CompactI.h,v 1.7 2014/04/30 15:40:09 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__COMPACTI_H__)
#define __COMPACTI_H__

#if defined(DEBUG)
_Inline void compacti_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Local Constant Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Structure Definitions
//-----------------------------------------------------------------------------

typedef struct CompactLaneStruct
{
    PCB_STRUCT Pcb;
    unsigned long BuffAddr;
    unsigned long Padding[3];
} COMPACT_LANE_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

// Control Data
void compact_cd_dirty_dir0 (PCB_STRUCT *PcbPtr);

void compact_cd_dirty_si0 (PCB_STRUCT *PcbPtr);

void compact_cd_alloc_dir0 (PCB_STRUCT *PcbPtr);

void compact_cd_alloc_si0 (PCB_STRUCT *PcbPtr);

void compact_cd_start_compacting (PCB_STRUCT *PcbPtr);


// User Data
void compact_ud_prcs_get_cmpct_sxn (PCB_STRUCT *PcbPtr);

void compact_ud_read_userdata (PCB_STRUCT *PcbPtr);

void compact_ud_read_userdata_done (PCB_STRUCT *PcbPtr);

void compact_ud_write_userdata (PCB_STRUCT *PcbPtr);

void compact_ud_write_userdata_done (PCB_STRUCT *PcbPtr);

void compact_ud_write_alloc_dir0 (PCB_STRUCT *PcbPtr);

void compact_ud_write_update_dir0 (PCB_STRUCT *PcbPtr);

void compact_ud_write_alloc_si0 (PCB_STRUCT *PcbPtr);

void compact_ud_write_update_si0 (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: CompactI.h,v $
// Revision 1.7  2014/04/30 15:40:09  rcantong
// 1. DEV: Prioritized flushing of compacting CD
// 1.1 Changed CD set dirty for compact process - MFenol
//
// Revision 1.6  2014/03/03 12:30:55  rcantong
// 1. BUGFIX: Handles unsync Dir and SI due to ungraceful shutdown
// 1.1 Removed compact_ud_verify_usersxn - MFenol
// 1.2 Skip compact when compact pba is not sync to dir - MFenol
//
// Revision 1.5  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.4  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:07  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:02  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
