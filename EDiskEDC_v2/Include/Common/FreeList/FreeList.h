//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/FreeList/FreeList.h,v $
// $Revision: 1.8 $
// $Author: rcantong $
// $Date: 2014/05/19 05:02:10 $
// $Id: FreeList.h,v 1.8 2014/05/19 05:02:10 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__FREELIST_H__)
#define __FREELIST_H__

#if defined(DEBUG)
_Inline void freelist_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define FREELIST_HIGH                   0xA5A5
#define FREELIST_LOW                    0xDED0


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

// Control Data
void freelist_cd_init_malloc (unsigned long FbxIdx);

void freelist_cd_init_lane (DM_FBX_STRUCT *FbxPtr,
                            unsigned long LaneIdx);

void freelist_cd_add_entry (DM_FBX_STRUCT *FbxPtr,
                            PBA_INT BlkPba);

PBA_INT freelist_cd_get_page (DM_FBX_STRUCT *FbxPtr);

PBA_INT freelist_cd_get_cmpct_page (DM_FBX_STRUCT *FbxPtr);

BIT_STAT freelist_cd_check_level (DM_FBX_STRUCT *FbxPtr,
                                  unsigned long LaneIdx);

BIT_STAT freelist_cd_compare_cur_pba (DM_FBX_STRUCT *FbxPtr,
                                      PBA_INT BlkPba);

void freelist_cd_chk_n_remove_cur_pba (DM_FBX_STRUCT *FbxPtr,
                                       PBA_INT BlkPba);

// User Data
void freelist_ud_init_malloc (unsigned long FbxIdx);

void freelist_ud_init_lane (DM_FBX_STRUCT *FbxPtr,
                            unsigned long LaneIdx);

void freelist_ud_add_entry (DM_FBX_STRUCT *FbxPtr,
                            PBA_INT BlkPba);

PBA_INT freelist_ud_get_page (DM_FBX_STRUCT *FbxPtr);

PBA_INT freelist_ud_cmpct_get_page (DM_FBX_STRUCT *FbxPtr);

BIT_STAT freelist_ud_check_level (DM_FBX_STRUCT *FbxPtr,
                                  unsigned long LaneIdx);

BIT_STAT freelist_ud_compare_cur_pba (DM_FBX_STRUCT *FbxPtr,
                                      PBA_INT BlkPba);

void freelist_ud_chk_n_remove_cur_pba (DM_FBX_STRUCT *FbxPtr,
                                       PBA_INT BlkPba);


#endif
//=============================================================================
// $Log: FreeList.h,v $
// Revision 1.8  2014/05/19 05:02:10  rcantong
// 1. DEV: Support control remap
// 1.1 Added control remapping functions - BBantigue
//
// Revision 1.7  2014/04/30 14:01:15  rcantong
// 1. DEV: Support freelist for compact
// 1.1 Added freelist threshold for compact page - JAbad
//
// Revision 1.6  2014/02/02 10:00:32  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:57  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.4  2013/12/05 13:06:34  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:08  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:04  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
