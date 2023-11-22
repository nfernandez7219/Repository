//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Defects/Defects.h,v $
// $Revision: 1.6 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:58 $
// $Id: Defects.h,v 1.6 2014/05/19 04:48:58 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__DEFECTS_H__)
#define __DEFECTS_H__

#if defined(DEBUG)
_Inline void defects_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define DEFECTS_MFG_DEFECT_ERR          0x10
#define DEFECTS_SCREENING_ERR           0x11
#define DEFECTS_RESERVED_BLK            0x12
#define DEFECTS_BLOCK                   0xB16B00B5

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

void defects_init_malloc (unsigned long FbxIdx);

void defects_init_fbx (PCB_STRUCT *PcbPtr);

void defects_build_fbx (PCB_STRUCT *PcbPtr);

void defects_erase_fbx (PCB_STRUCT *PcbPtr);

void defects_erase_all_fbx (PCB_STRUCT *PcbPtr);

void defects_scan_mfg_fbx (PCB_STRUCT *PcbPtr);

void defects_thorough_scan_erase_fbx (PCB_STRUCT *PcbPtr);

void defects_thorough_scan_write_fbx (PCB_STRUCT *PcbPtr);

void defects_thorough_scan_read_fbx (PCB_STRUCT *PcbPtr);

void defects_add_defect_entry (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba,
                               unsigned long ErrorType);


#endif
//=============================================================================
// $Log: Defects.h,v $
// Revision 1.6  2014/05/19 04:48:58  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.5  2014/04/30 13:53:35  rcantong
// 1. DEV: Support defects screening
// 1.1 Added process for thorough scanning - BBantigue
// 2. BUGFIX: Enhanced scrambler pattern
// 2.1 Used tiny encryption algo to generate scrambler pattern
//
// Revision 1.4  2014/02/02 09:31:33  rcantong
// 1. DEV: Support more defects info and runtime flushing
// 1.1 Added more defects info and runtime flushing
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
