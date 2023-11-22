//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Dm/DmBuildI.h,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2014/04/30 13:53:35 $
// $Id: DmBuildI.h,v 1.3 2014/04/30 13:53:35 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__DMBUILDI_H__)
#define __DMBUILDI_H__

#if defined(DEBUG)
_Inline void dmbuildi_h (void) { return; }
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

void dm_build_flush_scan_mfg_defects (PCB_STRUCT *PcbPtr);

void dm_build_thorough_scan_write_fbx (PCB_STRUCT *PcbPtr);

void dm_build_thorough_scan_read_fbx (PCB_STRUCT *PcbPtr);

void dm_build_thorough_scan_erase_fbx (PCB_STRUCT *PcbPtr);

void dm_build_blkinfo (PCB_STRUCT *PcbPtr);

void dm_build_dir (PCB_STRUCT *PcbPtr);

void dm_build_sxninfo (PCB_STRUCT *PcbPtr);

void dm_build_defects (PCB_STRUCT *PcbPtr);

void dm_build_done (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: DmBuildI.h,v $
// Revision 1.3  2014/04/30 13:53:35  rcantong
// 1. DEV: Support defects screening
// 1.1 Added process for thorough scanning - BBantigue
// 2. BUGFIX: Enhanced scrambler pattern
// 2.1 Used tiny encryption algo to generate scrambler pattern
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
