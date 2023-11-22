//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/CntlData/CntlData.h,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/02/02 10:00:31 $
// $Id: CntlData.h,v 1.5 2014/02/02 10:00:31 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__CNTLDATA_H__)
#define __CNTLDATA_H__

#if defined(DEBUG)
_Inline void cntldata_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

// Do not modify the arrangment because of media alloc optimization
#define CNTLDATA_INCACHE                0
#define CNTLDATA_INFLUSH                1
#define CNTLDATA_INFETCH                2
#define CNTLDATA_NOCACHE                3


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define CNTLDATA_PUT_TO_DIRTY_LIST(CntlCachePtr, CntlData) \
    util_dll_insert_at_tail(&CntlCachePtr->Link, &CntlData.DirtyList)

#define CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr, CntlCachePtr) \
    util_sll_insert_at_tail(&PcbPtr->Link, &CntlCachePtr->StateWaitQ);

#define CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr, CntlData) \
    (util_sll_insert_at_tail(&PcbPtr->Link, &CntlData.RqstCacheWaitQ))

#define CNTLDATA_GET_HEAD_RQST_WAITQ(CntlData) \
    (util_sll_get_head_entry(&CntlData.RqstCacheWaitQ))

#define CNTLDATA_UNLOCK_WRITE(CachePtr)            \
do {                                               \
    ASSERT(CachePtr->LockCnt >= WRITE_LOCK_VALUE); \
    CachePtr->LockCnt -= WRITE_LOCK_VALUE;         \
} while (0)

#define CNTLDATA_SET_DIRTY(CachePtr, FbxPtr)          \
do {                                                  \
    CachePtr->DirtyCnt++;                             \
    if (CachePtr->DirtyCnt == 1)                      \
    {                                                 \
        CNTLDATA_PUT_TO_DIRTY_LIST(CachePtr,          \
                                   FbxPtr->CntlData); \
    }                                                 \
} while (0)


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void cntldata_init_malloc (unsigned long FbxIdx);

void cntldata_fetch_fbx (PCB_STRUCT *PcbPtr);

void cntldata_write_alloc_dir0 (PCB_STRUCT *PcbPtr);

void cntldata_write_alloc_si0 (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: CntlData.h,v $
// Revision 1.5  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.4  2013/12/05 13:06:33  rcantong
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
// Revision 1.1  2013/07/03 19:34:02  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
