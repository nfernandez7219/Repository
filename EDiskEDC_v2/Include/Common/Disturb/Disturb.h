//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Disturb/Disturb.h,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2014/02/02 10:00:31 $
// $Id: Disturb.h,v 1.2 2014/02/02 10:00:31 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__DISTURB_H__)
#define __DISTURB_H__

#if defined(DEBUG)
_Inline void disturb_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define DISTURB_READ_THRESH             1000000


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define DISTURB_INC_READ_CNT(FbxPtr, BlkIdx) \
(++(FbxPtr->DisturbMgr.DisturbReadCntPtr[BlkIdx]))

#define DISTURB_RESET_READ_CNT(FbxPtr, BlkIdx) \
(FbxPtr->DisturbMgr.DisturbReadCntPtr[BlkIdx] = 0)

#define DISTURB_CHECK_READ_THRESH(FbxPtr, BlkIdx) \
((FbxPtr->DisturbMgr.DisturbReadCntPtr[BlkIdx] > DISTURB_READ_THRESH) ? 1 : 0)


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
extern unsigned long DisturbReadCntPerBlk[FBX_CNT][BLKS_PER_FBX];
#pragma BSS()


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void disturb_init_malloc (unsigned long FbxIdx);


#endif
//=============================================================================
// $Log: Disturb.h,v $
// Revision 1.2  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.1  2013/07/03 19:34:03  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
