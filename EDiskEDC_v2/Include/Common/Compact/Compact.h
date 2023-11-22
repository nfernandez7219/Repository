//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Compact/Compact.h,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/04/30 15:40:09 $
// $Id: Compact.h,v 1.5 2014/04/30 15:40:09 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__COMPACT_H__)
#define __COMPACT_H__

#if defined(DEBUG)
_Inline void compact_h (void) { return; }
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

#define COMPACT_CD_SET_DIRTY(CntlCachePtr, FbxPtr)              \
{                                                               \
    CntlCachePtr->DirtyCnt++;                                   \
    if (CntlCachePtr->DirtyCnt == 1)                            \
    {                                                           \
        util_dll_insert_at_head(&CntlCachePtr->Link,            \
                                &FbxPtr->CntlData.FlushList);   \
    }                                                           \
                                                                \
    else if (CntlCachePtr->DirtyCnt > 1)                        \
    {                                                           \
        util_dll_remove_from_middle(&CntlCachePtr->Link);       \
        util_dll_insert_at_head(&CntlCachePtr->Link,            \
                                &FbxPtr->CntlData.FlushList);   \
    }                                                           \
}


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void compact_cd_init_malloc (unsigned long FbxIdx);

void compact_cd_trigger_cmpct_prcs (DM_FBX_STRUCT *FbxPtr,
                                    unsigned long LaneIdx);

void compact_ud_init_malloc (unsigned long FbxIdx);

void compact_ud_trigger_cmpct_prcs (DM_FBX_STRUCT *FbxPtr,
                                    unsigned long LaneIdx);

void compact_ud_trigger_idle_cmpct_prcs (void);


#endif
//=============================================================================
// $Log: Compact.h,v $
// Revision 1.5  2014/04/30 15:40:09  rcantong
// 1. DEV: Prioritized flushing of compacting CD
// 1.1 Changed CD set dirty for compact process - MFenol
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
