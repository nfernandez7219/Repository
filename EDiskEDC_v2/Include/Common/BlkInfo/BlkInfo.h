//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/BlkInfo/BlkInfo.h,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2014/04/30 13:34:46 $
// $Id: BlkInfo.h,v 1.2 2014/04/30 13:34:46 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__BLKINFO_H__)
#define __BLKINFO_H__

#if defined(DEBUG)
_Inline void blkinfo_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------


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

void blkinfo_init_malloc (unsigned long FbxIdx);

void blkinfo_init_fbx (PCB_STRUCT *PcbPtr);

void blkinfo_fetch_fbx (PCB_STRUCT *PcbPtr);

void blkinfo_fetch_blkinfo (PCB_STRUCT *PcbPtr);

void blkinfo_build_fbx (PCB_STRUCT *PcbPtr);

void blkinfo_update_entry (DM_FBX_STRUCT *FbxPtr,
                           unsigned long BlkIdx);

unsigned long blkinfo_get_min_erase_cnt (DM_FBX_STRUCT *FbxPtr,
                                         unsigned long BlkIdx);

unsigned long blkinfo_get_max_erase_cnt (DM_FBX_STRUCT *FbxPtr,
                                         unsigned long BlkIdx);


#endif
//=============================================================================
// $Log: BlkInfo.h,v $
// Revision 1.2  2014/04/30 13:34:46  rcantong
// 1. DEV: Support BlkInfo control data to monitor erase count
// 1.1 Added process for BlkInfo - BBantigue
//
// Revision 1.1  2013/07/03 19:34:02  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
