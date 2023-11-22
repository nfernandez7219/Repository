//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Media/Media.h,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/02/02 10:00:32 $
// $Id: Media.h,v 1.5 2014/02/02 10:00:32 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__MEDIA_H__)
#define __MEDIA_H__

#if defined(DEBUG)
_Inline void media_h (void) { return; }
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

void media_init_malloc (unsigned long FbxIdx);

BIT_STAT media_chk_usersxn_lock (DM_FBX_STRUCT *FbxPtr,
                                 unsigned long UserSxnIdx);

BIT_STAT media_chk_n_lock_usersxn (DM_FBX_STRUCT *FbxPtr,
                                   unsigned long UserSxnIdx);

void media_unlock_usersxn (DM_FBX_STRUCT *FbxPtr,
                           unsigned long UserSxnIdx);

void media_prcs_read_bulk (unsigned long *PayloadPtr);

void media_prcs_write_bulk (unsigned long *PayloadPtr);

void media_prcs_rmw_read_bulk (unsigned long *PayloadPtr);

void media_prcs_rmw_write_bulk (unsigned long *PayloadPtr);

void media_write_in_place (unsigned long *PayloadPtr);


#endif
//=============================================================================
// $Log: Media.h,v $
// Revision 1.5  2014/02/02 10:00:32  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.4  2014/01/08 12:42:57  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
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
