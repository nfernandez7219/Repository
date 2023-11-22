//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/RemapUser/RemapUser.h,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2014/05/19 05:09:21 $
// $Id: RemapUser.h,v 1.3 2014/05/19 05:09:21 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__REMAPUSER_H__)
#define __REMAPUSER_H__

#if defined(DEBUG)
_Inline void remapuser_h (void) { return; }
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

void remap_ud_init_malloc (unsigned long FbxIdx);

void remap_ud_add_to_remap_list (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT BlkPba);

#endif
//=============================================================================
// $Log: RemapUser.h,v $
// Revision 1.3  2014/05/19 05:09:21  rcantong
// 1. DEV: Cleanup
// 1.1 Removed unused user remap per section functions
//
// Revision 1.2  2014/02/02 10:00:32  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.1  2013/07/03 19:34:13  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
