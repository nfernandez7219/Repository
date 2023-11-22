//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/SxnInfo/SxnInfo.c,v $
// $Revision: 1.9 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:59 $
// $Id: SxnInfo.c,v 1.9 2014/05/19 04:48:59 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================


//-----------------------------------------------------------------------------
// Standard Library Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Includes
//-----------------------------------------------------------------------------
#include "BitDefs.h"
#include "Util.h"
#include "Sched.h"
#include "SysConfig.h"

#include "BlkRecord.h"
#include "CntlDataCommon.h"
#include "CntlData.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "Err.h"
#include "RemapCntl.h"
#include "SxnInfo.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "SxnInfoI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
SI0_CACHE_LOOKUP_STRUCT Si0CacheLookup[FBX_CNT][SI0_SXN_CNT + 8];
L1CACHE_ALIGN(Si0CacheLookup);

SI1_ENTRY_STRUCT Si1Entry[FBX_CNT][SI0_SXN_CNT];
L1CACHE_ALIGN(Si1Entry);

SQN_INT Si1Sqn[FBX_CNT][SI0_SXN_CNT];
L1CACHE_ALIGN(Si1Sqn);

CNTL_CACHE_STRUCT SiCacheBaseAddr[FBX_CNT][SI0_CACHE_LINE_CNT];
L1CACHE_ALIGN(SiCacheBaseAddr);
#pragma BSS()

#pragma BSS(".dm_buffer")
CNTL_BUFF_STRUCT SiBuffBaseAddr[FBX_CNT][SI0_CACHE_LINE_CNT];
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : sxninfo_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_init_malloc (unsigned long FbxIdx)
{
    unsigned long CacheCnt;
    CNTL_CACHE_STRUCT *CacheBasePtr;
    CNTL_BUFF_STRUCT *BuffBasePtr;
    SXNINFO_FBX_STRUCT *SxnInfoPtr;

    CacheCnt = SI0_CACHE_LINE_CNT;
    CacheBasePtr = &SiCacheBaseAddr[FbxIdx][0];
    BuffBasePtr = &SiBuffBaseAddr[FbxIdx][0];
    SxnInfoPtr = &DmFbx[FbxIdx].SxnInfo;

    util_dll_init(&SxnInfoPtr->CleanList);
    util_sll_init(&SxnInfoPtr->RqstCacheWaitQ);

    SxnInfoPtr->CacheBasePtr = CacheBasePtr;
    SxnInfoPtr->Si0CacheLookupPtr = &Si0CacheLookup[FbxIdx][0];
    SxnInfoPtr->Si1EntryPtr = &Si1Entry[FbxIdx][0];
    SxnInfoPtr->Si1SqnPtr = &Si1Sqn[FbxIdx][0];

    util_init_pattern(Si0CacheLookup[FbxIdx],
                      sizeof(Si0CacheLookup[0]),
                      INIT_PATTERN_LO_VALUE);

    util_init_pattern(Si1Entry[FbxIdx],
                      sizeof(Si1Entry[0]),
                      INIT_PATTERN_HI_VALUE);

    util_init_pattern(Si1Sqn[FbxIdx],
                      sizeof(Si1Sqn[0]),
                      INIT_PATTERN_LO_VALUE);

    while (CacheCnt > 0)
    {
        util_dll_insert_at_tail(&CacheBasePtr->Link,
                                &SxnInfoPtr->CleanList);

        CacheBasePtr->Identity = TARGET_SI0 | SI0_SXN_CNT;
        CacheBasePtr->DataAddr = (unsigned long)BuffBasePtr;
        CacheBasePtr->FbxIdx = FbxIdx;
        CacheBasePtr->LockCnt = 0;
        CacheBasePtr->DirtyCnt = 0;
        CacheBasePtr->State = CNTLDATA_INCACHE;
        util_sll_init(&CacheBasePtr->StateWaitQ);

        CacheBasePtr++;
        BuffBasePtr++;
        CacheCnt--;
    }

    SxnInfoPtr->Si0CacheLookupPtr[SI0_SXN_CNT].Si0CachePtr = BIT_NULL_PTR;

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_build_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_build_fbx (PCB_STRUCT *PcbPtr)
{
    unsigned long FbxIdx;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long DevIdx;
    PCB_STRUCT *ChildPcbPtr;
    unsigned long BuffAddr;

    FbxIdx = PcbPtr->Word.FbxIdx;
    FbxPtr = &DmFbx[FbxIdx];
    PcbPtr->Info.DmInit.DeployCtr = DEV_CNT;

    // Deploy child pcbs
    for (DevIdx = 0;
         DevIdx < DEV_CNT;
         DevIdx++)
    {
        BuffAddr = GET_DEV_BUFF_ADDR(FbxIdx, DevIdx);

        // Fill si0 entries with unmap pba
        dm_fill_random_pattern((void *)BuffAddr,
                               FLASH_PAGE_SIZE);

        ChildPcbPtr = _sched_get_pcb();
        ChildPcbPtr->Fn = sxninfo_build_dev_stg1;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;
        ChildPcbPtr->Info.DmxWrite.SegCnt = SEGMENTS_PER_PAGE;
        ChildPcbPtr->Info.DmxWrite.BuffAddr = BuffAddr;
        ChildPcbPtr->Info.SiBuild.DevIdx = DevIdx;
        ChildPcbPtr->Info.SiBuild.Si0SxnIdx = DevIdx;
        SCHED_POST_PCB(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_init_si1_entry
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_init_si1_entry (DM_FBX_STRUCT *FbxPtr,
                             IDENTITY_INT Identity,
                             SQN_INT Sqn,
                             PBA_INT Pba)
{
    unsigned long Si0SxnIdx;

    Si0SxnIdx = SXNIDX(Identity);
    ASSERT(Si0SxnIdx < SI0_SXN_CNT);
    ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

    #if defined(CD_MIRROR)
    if (Sqn > FbxPtr->SxnInfo.Si1SqnPtr[Si0SxnIdx])
    {
        FbxPtr->SxnInfo.Si1SqnPtr[Si0SxnIdx] = Sqn;
        FbxPtr->SxnInfo.Si1EntryPtr[Si0SxnIdx].Si0Pba0 = Pba;

        // Invalidate the Pba of mirror sxn
        FbxPtr->SxnInfo.Si1EntryPtr[Si0SxnIdx].Si0Pba1 = INVALID_MASK;
    }

    else if (Sqn == FbxPtr->SxnInfo.Si1SqnPtr[Si0SxnIdx])
    {
        FbxPtr->SxnInfo.Si1EntryPtr[Si0SxnIdx].Si0Pba1 = Pba;
    }

    #else
    if (Sqn > FbxPtr->SxnInfo.Si1SqnPtr[Si0SxnIdx])
    {
        FbxPtr->SxnInfo.Si1SqnPtr[Si0SxnIdx] = Sqn;
        FbxPtr->SxnInfo.Si1EntryPtr[Si0SxnIdx].Si0Pba0 = Pba;
    }
    #endif

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_scrub_si1
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_scrub_si1 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long EntryIdx;
    PBA_INT Si0Pba;
    IDENTITY_INT *IdPtr;

    FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];
    IdPtr = FbxPtr->CntlData.IdPtr;

    for (EntryIdx = 0;
         EntryIdx < SI0_SXN_CNT;
         EntryIdx++)
    {
        Si0Pba = FbxPtr->SxnInfo.Si1EntryPtr[EntryIdx].Si0Pba0;
        if ((Si0Pba & INVALID_MASK) == 0)
        {
            ASSERT(IdPtr[Si0Pba] == TARGET_UNKNOWN);
            IdPtr[Si0Pba] = TARGET_SI0 | EntryIdx;

            blkrecord_cd_set_valid_sxn(FbxPtr,
                                       Si0Pba);
        }
        else
        {
            // Missing Si1Entry is not yet supported
            err_gross();
        }

        #if defined(CD_MIRROR)
        // Mirror Pba
        Si0Pba = FbxPtr->SxnInfo.Si1EntryPtr[EntryIdx].Si0Pba1;
        if ((Si0Pba & INVALID_MASK) == 0)
        {
            ASSERT(IdPtr[Si0Pba] == TARGET_UNKNOWN);
            IdPtr[Si0Pba] = TARGET_SI0 | EntryIdx;

            blkrecord_cd_set_valid_sxn(FbxPtr,
                                       Si0Pba);
        }
        #endif
    }

    SCHED_POST_PCB(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_alloc_read
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT sxninfo_alloc_read (DM_FBX_STRUCT *FbxPtr,
                             PBA_INT Pba,
                             CNTL_CACHE_STRUCT **Si0CachePtr2Ptr)
{
    unsigned long Si0EntryIdx;
    unsigned long Si0SxnIdx;
    CNTL_CACHE_STRUCT *Si0CachePtr;
    unsigned long OldSxnIdx;

    Si0EntryIdx = Pba / SEGMENTS_PER_USER_SXN;
    Si0SxnIdx = Si0EntryIdx / SI0_ENTRIES_PER_SXN;
    Si0CachePtr = SXNINFO_GET_CACHE_FR_LOOKUP(FbxPtr->SxnInfo, Si0SxnIdx);

    if (Si0CachePtr == BIT_NULL_PTR)
    {
        // Get cacheline
        Si0CachePtr = util_dll_peek_head_entry(&FbxPtr->SxnInfo.CleanList);

        if (Si0CachePtr == BIT_NULL_PTR)
        {
            return CNTLDATA_NOCACHE;
        }

        ASSERT(Si0CachePtr->DirtyCnt == 0);
        ASSERT(Si0CachePtr->LockCnt == 0);
        ASSERT(Si0CachePtr->State == CNTLDATA_INCACHE);

        // Deassociate from old owner
        OldSxnIdx = SXNIDX(Si0CachePtr->Identity);
        FbxPtr->SxnInfo.Si0CacheLookupPtr[OldSxnIdx].Si0CachePtr = BIT_NULL_PTR;

        // Associate to new owner
        Si0CachePtr->Identity = TARGET_SI0 | Si0SxnIdx;
        FbxPtr->SxnInfo.Si0CacheLookupPtr[Si0SxnIdx].Si0CachePtr = Si0CachePtr;

        // Fetch
        sxninfo_post_si0_fetching(Si0CachePtr);
    }

    // Assign si cache
    *Si0CachePtr2Ptr = Si0CachePtr;

    // Read lock
    if (    (Si0CachePtr->LockCnt == 0)
         && (Si0CachePtr->DirtyCnt == 0))
    {
        // First owner and not dirty, need to remove from clean list
        util_dll_remove_from_middle(&Si0CachePtr->Link);
    }

    Si0CachePtr->LockCnt += READ_LOCK_VALUE;

    return Si0CachePtr->State;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_alloc_write
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT sxninfo_alloc_write (DM_FBX_STRUCT *FbxPtr,
                              PBA_INT Pba,
                              CNTL_CACHE_STRUCT **Si0CachePtr2Ptr)
{
    unsigned long Si0EntryIdx;
    unsigned long Si0SxnIdx;
    CNTL_CACHE_STRUCT *Si0CachePtr;
    unsigned long OldSxnIdx;

    Si0EntryIdx = Pba / SEGMENTS_PER_USER_SXN;
    Si0SxnIdx = Si0EntryIdx / SI0_ENTRIES_PER_SXN;
    Si0CachePtr = SXNINFO_GET_CACHE_FR_LOOKUP(FbxPtr->SxnInfo, Si0SxnIdx);

    if (Si0CachePtr == BIT_NULL_PTR)
    {
        // Get cacheline
        Si0CachePtr = util_dll_peek_head_entry(&FbxPtr->SxnInfo.CleanList);

        if (Si0CachePtr == BIT_NULL_PTR)
        {
            return CNTLDATA_NOCACHE;
        }

        ASSERT(Si0CachePtr->DirtyCnt == 0);
        ASSERT(Si0CachePtr->LockCnt == 0);
        ASSERT(Si0CachePtr->State == CNTLDATA_INCACHE);

        // Deassociate from old owner
        OldSxnIdx = SXNIDX(Si0CachePtr->Identity);
        FbxPtr->SxnInfo.Si0CacheLookupPtr[OldSxnIdx].Si0CachePtr = BIT_NULL_PTR;

        // Associate to new owner
        Si0CachePtr->Identity = TARGET_SI0 | Si0SxnIdx;
        FbxPtr->SxnInfo.Si0CacheLookupPtr[Si0SxnIdx].Si0CachePtr = Si0CachePtr;

        // Fetch
        sxninfo_post_si0_fetching(Si0CachePtr);
    }

    // Assign si cache
    *Si0CachePtr2Ptr = Si0CachePtr;

    // Write lock
    if (    (Si0CachePtr->LockCnt == 0)
         && (Si0CachePtr->DirtyCnt == 0))
    {
        // First owner and not dirty, need to remove from clean list
        util_dll_remove_from_middle(&Si0CachePtr->Link);
    }

    Si0CachePtr->LockCnt += WRITE_LOCK_VALUE;

    return Si0CachePtr->State;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_alloc_compact
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT sxninfo_alloc_compact (DM_FBX_STRUCT *FbxPtr,
                                unsigned long Si0SxnIdx,
                                CNTL_CACHE_STRUCT **Si0CachePtr2Ptr)
{
    CNTL_CACHE_STRUCT *Si0CachePtr;
    unsigned long OldSxnIdx;

    Si0CachePtr = SXNINFO_GET_CACHE_FR_LOOKUP(FbxPtr->SxnInfo,
                                              Si0SxnIdx);

    if (Si0CachePtr == BIT_NULL_PTR)
    {
        // Get cacheline
        Si0CachePtr = util_dll_peek_head_entry(&FbxPtr->SxnInfo.CleanList);

        if (Si0CachePtr == BIT_NULL_PTR)
        {
            return CNTLDATA_NOCACHE;
        }

        ASSERT(Si0CachePtr->DirtyCnt == 0);
        ASSERT(Si0CachePtr->LockCnt == 0);
        ASSERT(Si0CachePtr->State == CNTLDATA_INCACHE);

        // Deassociate from old owner
        OldSxnIdx = SXNIDX(Si0CachePtr->Identity);
        FbxPtr->SxnInfo.Si0CacheLookupPtr[OldSxnIdx].Si0CachePtr = BIT_NULL_PTR;

        // Associate to new owner
        Si0CachePtr->Identity = TARGET_SI0 | Si0SxnIdx;
        FbxPtr->SxnInfo.Si0CacheLookupPtr[Si0SxnIdx].Si0CachePtr = Si0CachePtr;

        // Fetch
        sxninfo_post_si0_fetching(Si0CachePtr);
    }

    // Assign si cache
    *Si0CachePtr2Ptr = Si0CachePtr;

    // Write lock
    if (    (Si0CachePtr->LockCnt == 0)
         && (Si0CachePtr->DirtyCnt == 0))
    {
        // First owner and not dirty, need to remove from clean list
        util_dll_remove_from_middle(&Si0CachePtr->Link);
    }

    Si0CachePtr->LockCnt += WRITE_LOCK_VALUE;

    return Si0CachePtr->State;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_unlock_read
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_unlock_read (CNTL_CACHE_STRUCT *Si0CachePtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_STRUCT *WakePcbPtr;

    ASSERT(Si0CachePtr->LockCnt > 0);
    Si0CachePtr->LockCnt -= READ_LOCK_VALUE;

    if (    (Si0CachePtr->LockCnt == 0)
         && (Si0CachePtr->DirtyCnt == 0))
    {
        FbxPtr = &DmFbx[Si0CachePtr->FbxIdx];

        // Put to clean list
        SXNINFO_PUT_TO_CLEAN_LIST(Si0CachePtr, FbxPtr->SxnInfo);

        // Wake sleeping pcb
        WakePcbPtr = CNTLDATA_GET_HEAD_RQST_WAITQ(FbxPtr->SxnInfo);

        if (WakePcbPtr != BIT_NULL_PTR)
        {
            WakePcbPtr->Fn(WakePcbPtr);
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_post_si0_fetching
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_post_si0_fetching (CNTL_CACHE_STRUCT *Si0CachePtr)
{
    PCB_STRUCT *PcbPtr;

    PcbPtr = _sched_get_pcb();

    Si0CachePtr->State = CNTLDATA_INFETCH;
    PcbPtr->Info.CntlRead.CntlCachePtr = Si0CachePtr;
    PcbPtr->Word.FbxPtr = &DmFbx[Si0CachePtr->FbxIdx];
    sxninfo_fetch_si0(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : sxninfo_build_dev_stg1
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_build_dev_stg1 (PCB_STRUCT *PcbPtr)
{
    unsigned long Si0SxnIdx;
    unsigned long EntryIdx;
    volatile CNTL_HDR_STRUCT *CntlHdrPtr;
    volatile unsigned long *BuffAddrPtr;
    unsigned long Idx;

    // Setup si0 sections header
    Si0SxnIdx = PcbPtr->Info.SiBuild.Si0SxnIdx;
    CntlHdrPtr = (void *)PcbPtr->Info.SiBuild.Dmx.BuffAddr;
    BuffAddrPtr = (void *)(PcbPtr->Info.SiBuild.Dmx.BuffAddr + CNTL_SXN_SIZE);

    CntlHdrPtr->Signature = CNTL_HDR_SIGNATURE;
    CntlHdrPtr->Sqn = 1;

    // Fill control header
    for (EntryIdx = 0;
         EntryIdx < (SEGMENTS_PER_PAGE - 1);
         EntryIdx++)
    {
        for (Idx = 0;
             Idx < SI0_ENTRIES_PER_SXN;
             Idx++)
        {
            *BuffAddrPtr = INVALID_MASK
                ^ SCRAMBLE_WORD((Si0SxnIdx * SI0_ENTRIES_PER_SXN) + Idx);
        }

        if (Si0SxnIdx < SI0_SXN_CNT)
        {
            CntlHdrPtr->Identity[EntryIdx] = Si0SxnIdx | TARGET_SI0;
            Si0SxnIdx += DEV_CNT;
        }
        else
        {
            CntlHdrPtr->Identity[EntryIdx] = GET_RANDOM_PATTERN();
        }
    }

    // Null indicator
    CntlHdrPtr->Identity[EntryIdx] = GET_RANDOM_PATTERN();

    // Set the next Si0SxnIdx to populate
    PcbPtr->Info.SiBuild.Si0SxnIdx = Si0SxnIdx;

    sxninfo_build_dev_stg2(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_build_dev_stg2
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_build_dev_stg2 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT *CurPbaPtr;
    PBA_INT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    CurPbaPtr = &FbxPtr->DevCurPbaPtr[PcbPtr->Info.SiBuild.DevIdx];

    while (1)
    {
        Pba = *CurPbaPtr;
        ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

        // Increment pba by page
        *CurPbaPtr = dmx_incr_pba_by_devpage(Pba);

        // Check if blk pba is good
        if (blkrecord_cd_check_bad_blk(FbxPtr, Pba) == GOOD_BLK)
        {
            break;
        }

        // Bad blk, go to next blk
        *CurPbaPtr += SEGMENTS_PER_SYSBLK;
        *CurPbaPtr &= ~(SEGMENTS_PER_BLK - 1);
    }

    PcbPtr->Fn = sxninfo_build_dev_stg3;
    PcbPtr->Info.DmxWrite.Pba = Pba;
    dmx_ops_write_n_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_build_dev_stg3
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_build_dev_stg3 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT *CurPbaPtr;

    if (PcbPtr->Info.DmxWrite.Stat == SUCCESSFUL)
    {
        #if defined(CD_MIRROR)
        // Flush the duplicate
        sxninfo_build_dev_stg4(PcbPtr);

        #else
        if (PcbPtr->Info.SiBuild.Si0SxnIdx < SI0_SXN_CNT)
        {
            // Go to next section
            sxninfo_build_dev_stg1(PcbPtr);
        }

        else
        {
            dm_notify_completion(PcbPtr);

            // Return the pcb
            _sched_return_pcb(PcbPtr);
        }
        #endif
    }
    else
    {
        if (PcbPtr->Info.DmxWrite.Stat != DMX_OPS_CMD_RETRY)
        {
            FbxPtr = PcbPtr->Word.FbxPtr;

            // Set quarantine blk
            blkrecord_cd_set_q_blk(FbxPtr,
                                   PcbPtr->Info.DmxWrite.Pba);

            // Do not write to succeeding pages of a defective blk
            CurPbaPtr = &FbxPtr->DevCurPbaPtr[PcbPtr->Info.SiBuild.DevIdx];
            *CurPbaPtr += SEGMENTS_PER_SYSBLK;
            *CurPbaPtr &= ~(SEGMENTS_PER_BLK - 1);
        }

        // Get next page
        sxninfo_build_dev_stg2(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_build_dev_stg4
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_build_dev_stg4 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT *CurPbaPtr;
    PBA_INT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    CurPbaPtr = &FbxPtr->DevCurPbaPtr[PcbPtr->Info.SiBuild.DevIdx];

    while (1)
    {
        Pba = *CurPbaPtr;
        ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

        // Increment pba by page
        *CurPbaPtr = dmx_incr_pba_by_devpage(Pba);

        // Check if blk pba is good
        if (blkrecord_cd_check_bad_blk(FbxPtr, Pba) == GOOD_BLK)
        {
            break;
        }

        // Bad blk, go to next blk
        *CurPbaPtr += SEGMENTS_PER_SYSBLK;
        *CurPbaPtr &= ~(SEGMENTS_PER_BLK - 1);
    }

    PcbPtr->Fn = sxninfo_build_dev_stg5;
    PcbPtr->Info.DmxWrite.Pba = Pba;
    dmx_ops_write_n_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_build_dev_stg5
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_build_dev_stg5 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT *CurPbaPtr;

    if (PcbPtr->Info.DmxWrite.Stat == SUCCESSFUL)
    {
        if (PcbPtr->Info.SiBuild.Si0SxnIdx < SI0_SXN_CNT)
        {
            // Go to next section
            sxninfo_build_dev_stg1(PcbPtr);
        }

        else
        {
            dm_notify_completion(PcbPtr);

            // Return the pcb
            _sched_return_pcb(PcbPtr);
        }
    }
    else
    {
        if (PcbPtr->Info.DmxWrite.Stat != DMX_OPS_CMD_RETRY)
        {
            FbxPtr = PcbPtr->Word.FbxPtr;

            // Set quarantine blk
            blkrecord_cd_set_q_blk(FbxPtr,
                                   PcbPtr->Info.DmxWrite.Pba);

            // Do not write to succeeding pages of a defective blk
            CurPbaPtr = &FbxPtr->DevCurPbaPtr[PcbPtr->Info.SiBuild.DevIdx];
            *CurPbaPtr += SEGMENTS_PER_SYSBLK;
            *CurPbaPtr &= ~(SEGMENTS_PER_BLK - 1);
        }

        // Get next page
        sxninfo_build_dev_stg4(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_fetch_si0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_fetch_si0 (PCB_STRUCT *PcbPtr)
{
    CNTL_CACHE_STRUCT *Si0CachePtr;
    unsigned long Si0SxnIdx;
    DM_FBX_STRUCT *FbxPtr;

    Si0CachePtr = PcbPtr->Info.CntlRead.CntlCachePtr;
    Si0SxnIdx = SXNIDX(Si0CachePtr->Identity);
    FbxPtr = PcbPtr->Word.FbxPtr;

    PcbPtr->Info.DmxRead.Pba = FbxPtr->SxnInfo.Si1EntryPtr[Si0SxnIdx].Si0Pba0;
    PcbPtr->Info.DmxRead.BuffAddr = Si0CachePtr->DataAddr;
    PcbPtr->Info.DmxRead.SegCnt = 1;
    PcbPtr->Fn = sxninfo_fetch_si0_done;
    dmx_ops_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_fetch_si0_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_fetch_si0_done (PCB_STRUCT *PcbPtr)
{
    BIT_STAT DmxStat;
    CNTL_CACHE_STRUCT *Si0CachePtr;
    PCB_STRUCT *WakePcbPtr;
    IDENTITY_INT Identity;

    DmxStat = PcbPtr->Info.DmxRead.Stat;
    Si0CachePtr = PcbPtr->Info.CntlRead.CntlCachePtr;

    if (DmxStat == DMX_OPS_ECC_UNCORRECTABLE)
    {
        #if defined(CD_MIRROR)
        sxninfo_fetch_si0_mirror(PcbPtr);
        #else
        err_gross();
        #endif

        return;
    }

    else if (DmxStat == DMX_OPS_ECC_CORRECTABLE)
    {
        // Remap this si section by simply scheduling it to be written
        CNTLDATA_SET_DIRTY(Si0CachePtr,
                           PcbPtr->Word.FbxPtr);

        // but! mark the page where section is located so no future allocations
        // will be made to it. Eventually, all valid sections in the page will
        // be invalidated making this a fully invalid and bad page.

        // Remap entire block
        remap_cntl_trigger_remap(PcbPtr->Word.FbxPtr,
                                 PcbPtr->Info.DmxRead.Pba);
    }

    // Turn on IncacheFlag
    Si0CachePtr->State = CNTLDATA_INCACHE;
    Identity = Si0CachePtr->Identity;

    // Wake up all
    while (1)
    {
        WakePcbPtr = util_sll_get_head_entry(&Si0CachePtr->StateWaitQ);

        if (WakePcbPtr == BIT_NULL_PTR)
        {
            break;
        }

        WakePcbPtr->Fn(WakePcbPtr);

        if (Identity != Si0CachePtr->Identity)
        {
            break;
        }
    }

    // Return the pcb
    _sched_return_pcb(PcbPtr);

    return;
}


#if defined(CD_MIRROR)
//-----------------------------------------------------------------------------
// Function    : sxninfo_fetch_si0_mirror
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_fetch_si0_mirror (PCB_STRUCT *PcbPtr)
{
    CNTL_CACHE_STRUCT *Si0CachePtr;
    unsigned long Si0SxnIdx;
    DM_FBX_STRUCT *FbxPtr;

    Si0CachePtr = PcbPtr->Info.CntlRead.CntlCachePtr;
    Si0SxnIdx = SXNIDX(Si0CachePtr->Identity);
    FbxPtr = PcbPtr->Word.FbxPtr;

    PcbPtr->Info.DmxRead.Pba = FbxPtr->SxnInfo.Si1EntryPtr[Si0SxnIdx].Si0Pba1;
    PcbPtr->Info.DmxRead.BuffAddr = Si0CachePtr->DataAddr;
    PcbPtr->Info.DmxRead.SegCnt = 1;
    PcbPtr->Fn = sxninfo_fetch_si0_mirror_done;
    dmx_ops_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : sxninfo_fetch_si0_mirror_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sxninfo_fetch_si0_mirror_done (PCB_STRUCT *PcbPtr)
{
    BIT_STAT DmxStat;
    CNTL_CACHE_STRUCT *Si0CachePtr;
    PCB_STRUCT *WakePcbPtr;
    IDENTITY_INT Identity;

    DmxStat = PcbPtr->Info.DmxRead.Stat;
    Si0CachePtr = PcbPtr->Info.CntlRead.CntlCachePtr;

    if (DmxStat == DMX_OPS_ECC_UNCORRECTABLE)
    {
        // Both original and mirror sxns are uncorrectable.
        err_gross();
    }

    else if (DmxStat == DMX_OPS_ECC_CORRECTABLE)
    {
        // but! mark the page where section is located so no future allocations
        // will be made to it. Eventually, all valid sections in the page will
        // be invalidated making this a fully invalid and bad page.

        // Remap entire block
        remap_cntl_trigger_remap(PcbPtr->Word.FbxPtr,
                                 PcbPtr->Info.DmxRead.Pba);
    }

    // Set dirty in order to Remap or Mirror the sxn through flushing.
    CNTLDATA_SET_DIRTY(Si0CachePtr,
                       PcbPtr->Word.FbxPtr);

    // Turn on IncacheFlag
    Si0CachePtr->State = CNTLDATA_INCACHE;
    Identity = Si0CachePtr->Identity;

    // Wake up all
    while (1)
    {
        WakePcbPtr = util_sll_get_head_entry(&Si0CachePtr->StateWaitQ);

        if (WakePcbPtr == BIT_NULL_PTR)
        {
            break;
        }

        WakePcbPtr->Fn(WakePcbPtr);

        if (Identity != Si0CachePtr->Identity)
        {
            break;
        }
    }

    // Return the pcb
    _sched_return_pcb(PcbPtr);

    return;
}
#endif


//=============================================================================
// $Log: SxnInfo.c,v $
// Revision 1.9  2014/05/19 04:48:59  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.8  2014/04/30 13:56:45  rcantong
// 1. DEV: Support CD mirroring
// 1.1 Added process to utilize the CD copy - JAbad
//
// Revision 1.7  2014/03/03 12:55:45  rcantong
// 1. DEV: FID hang handler
// 1.1 Added dmx stat checking for FID_HANG_TIMEOUT - JFaustino
//
// Revision 1.6  2014/02/02 10:00:32  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:58  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.4  2013/12/05 13:06:35  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:50  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:44:25  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:18  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
