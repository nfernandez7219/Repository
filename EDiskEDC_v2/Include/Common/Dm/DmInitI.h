//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Dm/DmInitI.h,v $
// $Revision: 1.6 $
// $Author: rcantong $
// $Date: 2014/05/19 05:21:06 $
// $Id: DmInitI.h,v 1.6 2014/05/19 05:21:06 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__DMINITI_H__)
#define __DMINITI_H__

#if defined(DEBUG)
_Inline void dminiti_h (void) { return; }
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


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void dm_init_blkinfo (PCB_STRUCT *PcbPtr);

void dm_init_check_blkinfo (PCB_STRUCT *PcbPtr);

void dm_init_defects (PCB_STRUCT *PcbPtr);

void dm_init_check_defects (PCB_STRUCT *PcbPtr);

void dm_init_fetch_cntlhdr (PCB_STRUCT *PcbPtr);

void dm_init_fetch_blkinfo (PCB_STRUCT *PcbPtr);

void dm_init_scrub_dir1 (PCB_STRUCT *PcbPtr);

void dm_init_scrub_si1 (PCB_STRUCT *PcbPtr);

void dm_init_fetch_dir0 (PCB_STRUCT *PcbPtr);

void dm_init_scrub_vi (PCB_STRUCT *PcbPtr);

void dm_init_cd_reclaim (PCB_STRUCT *PcbPtr);

void dm_init_ud_reclaim (PCB_STRUCT *PcbPtr);

void dm_init_cdflushmgr (PCB_STRUCT *PcbPtr);

void dm_init_ud_trigger_reclaim (PCB_STRUCT *PcbPtr);

void dm_init_done (PCB_STRUCT *PcbPtr);

void dm_init_malloc_fbx (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: DmInitI.h,v $
// Revision 1.6  2014/05/19 05:21:06  rcantong
// 1. BUGFIX: Support jumper rebuild for production board
// 1.1 Scanning without erase if production board - MManzo
//
// Revision 1.5  2014/04/30 13:53:35  rcantong
// 1. DEV: Support defects screening
// 1.1 Added process for thorough scanning - BBantigue
// 2. BUGFIX: Enhanced scrambler pattern
// 2.1 Used tiny encryption algo to generate scrambler pattern
//
// Revision 1.4  2013/12/05 13:06:34  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:07  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:03  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
