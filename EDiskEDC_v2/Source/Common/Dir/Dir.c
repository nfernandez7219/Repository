//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Dir/Dir.c,v $
// $Revision: 1.9 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:58 $
// $Id: Dir.c,v 1.9 2014/05/19 04:48:58 rcantong Exp $
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
#include "Dir.h"
#include "Dm.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "Err.h"
#include "RemapCntl.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "DirI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
DIR0_CACHE_LOOKUP_STRUCT Dir0CacheLookup[FBX_CNT][DIR0_SXN_CNT + 8];
L1CACHE_ALIGN(Dir0CacheLookup);

DIR1_ENTRY_STRUCT Dir1Entry[FBX_CNT][DIR0_SXN_CNT];
L1CACHE_ALIGN(Dir1Entry);

SQN_INT Dir1Sqn[FBX_CNT][DIR0_SXN_CNT];
L1CACHE_ALIGN(Dir1Sqn);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : dir_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_init_malloc (unsigned long FbxIdx)
{
    unsigned long UnusedSdram;
    unsigned long FreeMem;
    unsigned long CacheMemSz;
    unsigned long CacheCnt;
    CNTL_CACHE_STRUCT *CacheBasePtr;
    CNTL_BUFF_STRUCT *BuffBasePtr;
    DIR_FBX_STRUCT *DirPtr;

    // Get free sdram memory
    UnusedSdram = (unsigned long)_unused_sdram;
    FreeMem = (SDRAM_SIZE - UnusedSdram) / FBX_CNT;

    // Get cache count that can accomodate by free mem
    CacheMemSz = sizeof(CNTL_CACHE_STRUCT) + CNTL_SXN_SIZE;
    CacheCnt = FreeMem / CacheMemSz;

    // Get cache address and buffer address
    CacheBasePtr = (void *)(UnusedSdram + (FbxIdx * CacheCnt * CacheMemSz));
    BuffBasePtr = (void *)(CacheBasePtr + CacheCnt);

    DirPtr = &DmFbx[FbxIdx].Dir;

    util_dll_init(&DirPtr->CleanList);
    util_sll_init(&DirPtr->RqstCacheWaitQ);

    DirPtr->CacheBasePtr = CacheBasePtr;
    DirPtr->Dir0CacheLookupPtr = &Dir0CacheLookup[FbxIdx][0];
    DirPtr->Dir1EntryPtr = &Dir1Entry[FbxIdx][0];
    DirPtr->Dir1SqnPtr = &Dir1Sqn[FbxIdx][0];

    util_init_pattern(Dir0CacheLookup[FbxIdx],
                      sizeof(Dir0CacheLookup[0]),
                      INIT_PATTERN_LO_VALUE);

    util_init_pattern(Dir1Entry[FbxIdx],
                      sizeof(Dir1Entry[0]),
                      INIT_PATTERN_HI_VALUE);

    util_init_pattern(Dir1Sqn[FbxIdx],
                      sizeof(Dir1Sqn[0]),
                      INIT_PATTERN_LO_VALUE);

    while (CacheCnt > 0)
    {
        util_dll_insert_at_tail(&CacheBasePtr->Link,
                                &DirPtr->CleanList);

        CacheBasePtr->Identity = TARGET_DIR0 | DIR0_SXN_CNT;
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

    DirPtr->Dir0CacheLookupPtr[DIR0_SXN_CNT].Dir0CachePtr = BIT_NULL_PTR;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_build_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_build_fbx (PCB_STRUCT *PcbPtr)
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
        FbxPtr->DevCurPbaPtr[DevIdx] = CALC_PBA(DevIdx, 1, 0);
        BuffAddr = GET_DEV_BUFF_ADDR(FbxIdx, DevIdx);

        ChildPcbPtr = _sched_get_pcb();
        ChildPcbPtr->Fn = dir_build_dev_stg1;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;
        ChildPcbPtr->Info.DmxWrite.SegCnt = SEGMENTS_PER_PAGE;
        ChildPcbPtr->Info.DmxWrite.BuffAddr = BuffAddr;
        ChildPcbPtr->Info.DirBuild.DevIdx = DevIdx;
        ChildPcbPtr->Info.DirBuild.Dir0SxnIdx = DevIdx;
        SCHED_POST_PCB(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_fetch_dir0_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_fetch_dir0_fbx (PCB_STRUCT *PcbPtr)
{
    unsigned long FbxIdx;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long DevIdx;
    PCB_STRUCT *ChildPcbPtr;
    unsigned long BuffAddr;

    FbxIdx = PcbPtr->Word.FbxIdx;
    DmFlagParm.FbxStat[FbxIdx] = FBX_SCRUB;

    FbxPtr = &DmFbx[FbxIdx];
    PcbPtr->Info.DmInit.DeployCtr = DEV_CNT;

    // Deploy child pcbs
    for (DevIdx = 0;
         DevIdx < DEV_CNT;
         DevIdx++)
    {
        BuffAddr = GET_DEV_BUFF_ADDR(FbxIdx, DevIdx);

        ChildPcbPtr = _sched_get_pcb();
        ChildPcbPtr->Fn = dir_fetch_dir0_dev_stg1;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;
        ChildPcbPtr->Info.DmxRead.SegCnt = 1;
        ChildPcbPtr->Info.DmxRead.BuffAddr = BuffAddr;
        ChildPcbPtr->Info.DirFetch.Dir0SxnIdx = DevIdx;
        SCHED_POST_PCB(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_init_dir1_entry
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_init_dir1_entry (DM_FBX_STRUCT *FbxPtr,
                          IDENTITY_INT Identity,
                          SQN_INT Sqn,
                          PBA_INT Pba)
{
    unsigned long Dir0SxnIdx;

    Dir0SxnIdx = SXNIDX(Identity);
    ASSERT(Dir0SxnIdx < DIR0_SXN_CNT);
    ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

    #if defined(CD_MIRROR)
    if (Sqn > FbxPtr->Dir.Dir1SqnPtr[Dir0SxnIdx])
    {
        FbxPtr->Dir.Dir1SqnPtr[Dir0SxnIdx] = Sqn;
        FbxPtr->Dir.Dir1EntryPtr[Dir0SxnIdx].Dir0Pba0 = Pba;

        // Invalidate the Pba of mirror sxn
        FbxPtr->Dir.Dir1EntryPtr[Dir0SxnIdx].Dir0Pba1 = INVALID_MASK;
    }

    else if (Sqn == FbxPtr->Dir.Dir1SqnPtr[Dir0SxnIdx])
    {
        FbxPtr->Dir.Dir1EntryPtr[Dir0SxnIdx].Dir0Pba1 = Pba;
    }

    #else
    if (Sqn > FbxPtr->Dir.Dir1SqnPtr[Dir0SxnIdx])
    {
        FbxPtr->Dir.Dir1SqnPtr[Dir0SxnIdx] = Sqn;
        FbxPtr->Dir.Dir1EntryPtr[Dir0SxnIdx].Dir0Pba0 = Pba;
    }
    #endif

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_scrub_dir1
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_scrub_dir1 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long EntryIdx;
    PBA_INT Dir0Pba;
    IDENTITY_INT *IdPtr;

    FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];
    IdPtr = FbxPtr->CntlData.IdPtr;

    for (EntryIdx = 0;
         EntryIdx < DIR0_SXN_CNT;
         EntryIdx++)
    {
        Dir0Pba = FbxPtr->Dir.Dir1EntryPtr[EntryIdx].Dir0Pba0;
        if ((Dir0Pba & INVALID_MASK) == 0)
        {
            ASSERT(IdPtr[Dir0Pba] == TARGET_UNKNOWN);
            IdPtr[Dir0Pba] = TARGET_DIR0 | EntryIdx;

            blkrecord_cd_set_valid_sxn(FbxPtr,
                                       Dir0Pba);
        }
        else
        {
            // Missing Dir1Entry is not yet supported
            err_gross();
        }

        #if defined(CD_MIRROR)
        // Mirror Pba
        Dir0Pba = FbxPtr->Dir.Dir1EntryPtr[EntryIdx].Dir0Pba1;
        if ((Dir0Pba & INVALID_MASK) == 0)
        {
            ASSERT(IdPtr[Dir0Pba] == TARGET_UNKNOWN);
            IdPtr[Dir0Pba] = TARGET_DIR0 | EntryIdx;

            blkrecord_cd_set_valid_sxn(FbxPtr,
                                       Dir0Pba);
        }
        #endif
    }

    SCHED_POST_PCB(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_alloc_read
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT dir_alloc_read (DM_FBX_STRUCT *FbxPtr,
                         unsigned long UserSxnIdx,
                         CNTL_CACHE_STRUCT **Dir0CachePtr2Ptr)
{
    unsigned long Dir0SxnIdx;
    CNTL_CACHE_STRUCT *Dir0CachePtr;
    unsigned long OldSxnIdx;

    Dir0SxnIdx = UserSxnIdx / DIR0_ENTRIES_PER_SXN;
    Dir0CachePtr = DIR_GET_CACHE_FR_LOOKUP(FbxPtr->Dir, Dir0SxnIdx);

    if (Dir0CachePtr == BIT_NULL_PTR)
    {
        // Get cacheline
        Dir0CachePtr = util_dll_peek_head_entry(&FbxPtr->Dir.CleanList);

        if (Dir0CachePtr == BIT_NULL_PTR)
        {
            return CNTLDATA_NOCACHE;
        }

        ASSERT(Dir0CachePtr->DirtyCnt == 0);
        ASSERT(Dir0CachePtr->LockCnt == 0);
        ASSERT(Dir0CachePtr->State == CNTLDATA_INCACHE);

        // Deassociate from old owner
        OldSxnIdx = SXNIDX(Dir0CachePtr->Identity);
        FbxPtr->Dir.Dir0CacheLookupPtr[OldSxnIdx].Dir0CachePtr = BIT_NULL_PTR;

        // Associate to new owner
        Dir0CachePtr->Identity = TARGET_DIR0 | Dir0SxnIdx;
        FbxPtr->Dir.Dir0CacheLookupPtr[Dir0SxnIdx].Dir0CachePtr = Dir0CachePtr;

        // Fetch
        dir_post_dir0_fetching(Dir0CachePtr);
    }

    // Assign dir cache
    *Dir0CachePtr2Ptr = Dir0CachePtr;

    // Read lock
    if (    (Dir0CachePtr->LockCnt == 0)
         && (Dir0CachePtr->DirtyCnt == 0))
    {
        // First owner and not dirty, need to remove from clean list
        util_dll_remove_from_middle(&Dir0CachePtr->Link);
    }

    Dir0CachePtr->LockCnt += READ_LOCK_VALUE;

    return Dir0CachePtr->State;
}


//-----------------------------------------------------------------------------
// Function    : dir_alloc_write
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT dir_alloc_write (DM_FBX_STRUCT *FbxPtr,
                          unsigned long UserSxnIdx,
                          CNTL_CACHE_STRUCT **Dir0CachePtr2Ptr)
{
    unsigned long Dir0SxnIdx;
    CNTL_CACHE_STRUCT *Dir0CachePtr;
    unsigned long OldSxnIdx;

    Dir0SxnIdx = UserSxnIdx / DIR0_ENTRIES_PER_SXN;
    Dir0CachePtr = DIR_GET_CACHE_FR_LOOKUP(FbxPtr->Dir, Dir0SxnIdx);

    if (Dir0CachePtr == BIT_NULL_PTR)
    {
        // Get cacheline
        Dir0CachePtr = util_dll_peek_head_entry(&FbxPtr->Dir.CleanList);

        if (Dir0CachePtr == BIT_NULL_PTR)
        {
            return CNTLDATA_NOCACHE;
        }

        ASSERT(Dir0CachePtr->DirtyCnt == 0);
        ASSERT(Dir0CachePtr->LockCnt == 0);
        ASSERT(Dir0CachePtr->State == CNTLDATA_INCACHE);

        // Deassociate from old owner
        OldSxnIdx = SXNIDX(Dir0CachePtr->Identity);
        FbxPtr->Dir.Dir0CacheLookupPtr[OldSxnIdx].Dir0CachePtr = BIT_NULL_PTR;

        // Associate to new owner
        Dir0CachePtr->Identity = TARGET_DIR0 | Dir0SxnIdx;
        FbxPtr->Dir.Dir0CacheLookupPtr[Dir0SxnIdx].Dir0CachePtr = Dir0CachePtr;

        // Fetch
        dir_post_dir0_fetching(Dir0CachePtr);
    }

    // Assign dir cache
    *Dir0CachePtr2Ptr = Dir0CachePtr;

    // Write lock
    if (    (Dir0CachePtr->LockCnt == 0)
         && (Dir0CachePtr->DirtyCnt == 0))
    {
        // First owner and not dirty, need to remove from clean list
        util_dll_remove_from_middle(&Dir0CachePtr->Link);
    }

    Dir0CachePtr->LockCnt += WRITE_LOCK_VALUE;

    return Dir0CachePtr->State;
}


//-----------------------------------------------------------------------------
// Function    : dir_alloc_compact
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT dir_alloc_compact (DM_FBX_STRUCT *FbxPtr,
                            unsigned long Dir0SxnIdx,
                            CNTL_CACHE_STRUCT **Dir0CachePtr2Ptr)
{
    unsigned long OldSxnIdx;
    CNTL_CACHE_STRUCT *Dir0CachePtr;

    Dir0CachePtr = DIR_GET_CACHE_FR_LOOKUP(FbxPtr->Dir,
                                           Dir0SxnIdx);

    if (Dir0CachePtr == BIT_NULL_PTR)
    {
        // Get cacheline
        Dir0CachePtr = util_dll_peek_head_entry(&FbxPtr->Dir.CleanList);

        if (Dir0CachePtr == BIT_NULL_PTR)
        {
            return CNTLDATA_NOCACHE;
        }

        ASSERT(Dir0CachePtr->DirtyCnt == 0);
        ASSERT(Dir0CachePtr->LockCnt == 0);
        ASSERT(Dir0CachePtr->State == CNTLDATA_INCACHE);

        // Deassociate from old owner
        OldSxnIdx = SXNIDX(Dir0CachePtr->Identity);
        FbxPtr->Dir.Dir0CacheLookupPtr[OldSxnIdx].Dir0CachePtr = BIT_NULL_PTR;

        // Associate to new owner
        Dir0CachePtr->Identity = TARGET_DIR0 | Dir0SxnIdx;
        FbxPtr->Dir.Dir0CacheLookupPtr[Dir0SxnIdx].Dir0CachePtr = Dir0CachePtr;

        // fetch dir0
        dir_post_dir0_fetching(Dir0CachePtr);
    }

    *Dir0CachePtr2Ptr = Dir0CachePtr;

    // Write lock
    if (    (Dir0CachePtr->LockCnt == 0)
         && (Dir0CachePtr->DirtyCnt == 0))
    {
        // First owner and not dirty, need to remove from clean list
        util_dll_remove_from_middle(&Dir0CachePtr->Link);
    }

    Dir0CachePtr->LockCnt += WRITE_LOCK_VALUE;

    return Dir0CachePtr->State;
}


//-----------------------------------------------------------------------------
// Function    : dir_unlock_read
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_unlock_read (CNTL_CACHE_STRUCT *Dir0CachePtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_STRUCT *WakePcbPtr;

    ASSERT(Dir0CachePtr->LockCnt > 0);
    Dir0CachePtr->LockCnt -= READ_LOCK_VALUE;

    if (    (Dir0CachePtr->LockCnt == 0)
         && (Dir0CachePtr->DirtyCnt == 0))
    {
        FbxPtr = &DmFbx[Dir0CachePtr->FbxIdx];

        // Put to clean list
        DIR_PUT_TO_CLEAN_LIST(Dir0CachePtr, FbxPtr->Dir);

        // Wake sleeping pcb
        WakePcbPtr = CNTLDATA_GET_HEAD_RQST_WAITQ(FbxPtr->Dir);

        if (WakePcbPtr != BIT_NULL_PTR)
        {
            WakePcbPtr->Fn(WakePcbPtr);
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_post_dir0_fetching
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_post_dir0_fetching (CNTL_CACHE_STRUCT *Dir0CachePtr)
{
    PCB_STRUCT *PcbPtr;

    PcbPtr = _sched_get_pcb();

    Dir0CachePtr->State = CNTLDATA_INFETCH;
    PcbPtr->Info.CntlRead.CntlCachePtr = Dir0CachePtr;
    PcbPtr->Word.FbxPtr = &DmFbx[Dir0CachePtr->FbxIdx];
    dir_fetch_dir0(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : dir_build_dev_stg1
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_build_dev_stg1 (PCB_STRUCT *PcbPtr)
{
    unsigned long Dir0SxnIdx;
    unsigned long EntryIdx;
    volatile CNTL_HDR_STRUCT *CntlHdrPtr;
    volatile unsigned long *BuffAddrPtr;
    unsigned long Idx;

    // Setup dir0 sections header
    Dir0SxnIdx = PcbPtr->Info.DirBuild.Dir0SxnIdx;
    CntlHdrPtr = (void *)PcbPtr->Info.DirBuild.Dmx.BuffAddr;
    BuffAddrPtr = (void *)(PcbPtr->Info.DirBuild.Dmx.BuffAddr + CNTL_SXN_SIZE);

    CntlHdrPtr->Signature = CNTL_HDR_SIGNATURE;
    CntlHdrPtr->Sqn = 1;

    // Fill control header
    for (EntryIdx = 0;
         EntryIdx < (SEGMENTS_PER_PAGE - 1);
         EntryIdx++)
    {
        for (Idx = 0;
             Idx < DIR0_ENTRIES_PER_SXN;
             Idx++)
        {
            *BuffAddrPtr = INVALID_MASK
                ^ SCRAMBLE_WORD((Dir0SxnIdx * DIR0_ENTRIES_PER_SXN) + Idx);
            BuffAddrPtr++;
        }

        if (Dir0SxnIdx < DIR0_SXN_CNT)
        {
            CntlHdrPtr->Identity[EntryIdx] = Dir0SxnIdx | TARGET_DIR0;
            Dir0SxnIdx += DEV_CNT;
        }
        else
        {
            CntlHdrPtr->Identity[EntryIdx] = GET_RANDOM_PATTERN();
        }
    }

    // Null indicator
    CntlHdrPtr->Identity[EntryIdx] = GET_RANDOM_PATTERN();

    // Set the next Dir0SxnIdx to populate
    PcbPtr->Info.DirBuild.Dir0SxnIdx = Dir0SxnIdx;

    dir_build_dev_stg2(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_build_dev_stg2
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_build_dev_stg2 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT *CurPbaPtr;
    PBA_INT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    CurPbaPtr = &FbxPtr->DevCurPbaPtr[PcbPtr->Info.DirBuild.DevIdx];

    while (1)
    {
        Pba = *CurPbaPtr;
        ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

        // Increment pba by page
        *CurPbaPtr = dmx_incr_pba_by_devpage(Pba);

        // Check if pba is good
        if (blkrecord_cd_check_bad_blk(FbxPtr, Pba) == GOOD_BLK)
        {
            break;
        }

        // Bad blk, go to next blk
        *CurPbaPtr += SEGMENTS_PER_SYSBLK;
        *CurPbaPtr &= ~(SEGMENTS_PER_BLK - 1);
    }

    PcbPtr->Fn = dir_build_dev_stg3;
    PcbPtr->Info.DmxWrite.Pba = Pba;
    dmx_ops_write_n_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_build_dev_stg3
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_build_dev_stg3 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT *CurPbaPtr;

    if (PcbPtr->Info.DmxWrite.Stat == SUCCESSFUL)
    {
        #if defined(CD_MIRROR)
        // Flush the duplicate
        dir_build_dev_stg4(PcbPtr);

        #else
        if (PcbPtr->Info.DirBuild.Dir0SxnIdx < DIR0_SXN_CNT)
        {
            // Go to next section
            dir_build_dev_stg1(PcbPtr);
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
            CurPbaPtr = &FbxPtr->DevCurPbaPtr[PcbPtr->Info.DirBuild.DevIdx];
            *CurPbaPtr += SEGMENTS_PER_SYSBLK;
            *CurPbaPtr &= ~(SEGMENTS_PER_BLK - 1);
        }

        // Get next page
        dir_build_dev_stg2(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_build_dev_stg4
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_build_dev_stg4 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT *CurPbaPtr;
    PBA_INT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    CurPbaPtr = &FbxPtr->DevCurPbaPtr[PcbPtr->Info.DirBuild.DevIdx];

    while (1)
    {
        Pba = *CurPbaPtr;
        ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

        // Increment pba by page
        *CurPbaPtr = dmx_incr_pba_by_devpage(Pba);

        // Check if pba is good
        if (blkrecord_cd_check_bad_blk(FbxPtr, Pba) == GOOD_BLK)
        {
            break;
        }

        // Bad blk, go to next blk
        *CurPbaPtr += SEGMENTS_PER_SYSBLK;
        *CurPbaPtr &= ~(SEGMENTS_PER_BLK - 1);
    }

    PcbPtr->Fn = dir_build_dev_stg5;
    PcbPtr->Info.DmxWrite.Pba = Pba;
    dmx_ops_write_n_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_build_dev_stg5
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_build_dev_stg5 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT *CurPbaPtr;

    if (PcbPtr->Info.DmxWrite.Stat == SUCCESSFUL)
    {
        if (PcbPtr->Info.DirBuild.Dir0SxnIdx < DIR0_SXN_CNT)
        {
            // Go to next section
            dir_build_dev_stg1(PcbPtr);
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
            // Set quarantine blk
            blkrecord_cd_set_q_blk(PcbPtr->Word.FbxPtr,
                                   PcbPtr->Info.DmxWrite.Pba);

            // Do not write to succeeding pages of a defective blk
            FbxPtr = PcbPtr->Word.FbxPtr;
            CurPbaPtr = &FbxPtr->DevCurPbaPtr[PcbPtr->Info.DirBuild.DevIdx];
            *CurPbaPtr += SEGMENTS_PER_SYSBLK;
            *CurPbaPtr &= ~(SEGMENTS_PER_BLK - 1);
        }

        // Get next page
        dir_build_dev_stg4(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_fetch_dir0_dev_stg1
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_fetch_dir0_dev_stg1 (PCB_STRUCT *PcbPtr)
{
    unsigned long Dir1EntryIdx;
    DIR1_ENTRY_STRUCT *Dir1EntryPtr;

    Dir1EntryIdx = PcbPtr->Info.DirFetch.Dir0SxnIdx;
    Dir1EntryPtr = PcbPtr->Word.FbxPtr->Dir.Dir1EntryPtr;
    Dir1EntryPtr += Dir1EntryIdx;

    PcbPtr->Fn = dir_fetch_dir0_dev_stg2;
    PcbPtr->Info.DmxRead.Pba = Dir1EntryPtr->Dir0Pba0;
    dmx_ops_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_fetch_dir0_dev_stg2
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_fetch_dir0_dev_stg2 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long Dir0SxnIdx;
    volatile DIR0_ENTRY_STRUCT *Dir0EntryPtr;
    unsigned long Dir0EntryIdx;
    unsigned long UserPba;

    if (    (PcbPtr->Info.DmxRead.Stat == SUCCESSFUL)
         || (PcbPtr->Info.DmxRead.Stat == DMX_OPS_ECC_CORRECTABLE))
    {
        FbxPtr = PcbPtr->Word.FbxPtr;
        Dir0SxnIdx = PcbPtr->Info.DirFetch.Dir0SxnIdx;
        Dir0EntryPtr = (void *)PcbPtr->Info.DmxRead.BuffAddr;

        // Scan the content
        for (Dir0EntryIdx = 0;
             Dir0EntryIdx < DIR0_ENTRIES_PER_SXN;
             Dir0EntryIdx++)
        {
            UserPba = Dir0EntryPtr->UserPba
                ^ SCRAMBLE_WORD((Dir0SxnIdx * DIR0_ENTRIES_PER_SXN)
                    + Dir0EntryIdx);

            Dir0EntryPtr++;

            if ((UserPba & INVALID_MASK) == 0)
            {
                ASSERT(UserPba < SEGMENTS_PER_FBX);
                blkrecord_ud_set_valid_sxn(FbxPtr,
                                           UserPba);
            }
        }

        PcbPtr->Info.DirFetch.Dir0SxnIdx += DEV_CNT;
        if (PcbPtr->Info.DirFetch.Dir0SxnIdx < DIR0_SXN_CNT)
        {
            // Read next dir0 section
            dir_fetch_dir0_dev_stg1(PcbPtr);
        }
        else
        {
            // Done
            dm_notify_completion(PcbPtr);
            _sched_return_pcb(PcbPtr);
        }
    }

    else
    {
        #if defined(CD_MIRROR)
        dir_fetch_dir0_dev_stg1_mirror(PcbPtr);
        #else
        err_gross();
        #endif
    }

    return;
}


#if defined(CD_MIRROR)
//-----------------------------------------------------------------------------
// Function    : dir_fetch_dir0_dev_stg1_mirror
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_fetch_dir0_dev_stg1_mirror (PCB_STRUCT *PcbPtr)
{
    unsigned long Dir1EntryIdx;
    DIR1_ENTRY_STRUCT *Dir1EntryPtr;

    Dir1EntryIdx = PcbPtr->Info.DirFetch.Dir0SxnIdx;
    Dir1EntryPtr = PcbPtr->Word.FbxPtr->Dir.Dir1EntryPtr;
    Dir1EntryPtr += Dir1EntryIdx;

    PcbPtr->Fn = dir_fetch_dir0_dev_stg2_mirror;
    PcbPtr->Info.DmxRead.Pba = Dir1EntryPtr->Dir0Pba1;
    dmx_ops_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_fetch_dir0_dev_stg2_mirror
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_fetch_dir0_dev_stg2_mirror (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long Dir0SxnIdx;
    volatile DIR0_ENTRY_STRUCT *Dir0EntryPtr;
    unsigned long Dir0EntryIdx;
    unsigned long UserPba;

    if (    (PcbPtr->Info.DmxRead.Stat == SUCCESSFUL)
         || (PcbPtr->Info.DmxRead.Stat == DMX_OPS_ECC_CORRECTABLE))
    {
        FbxPtr = PcbPtr->Word.FbxPtr;
        Dir0SxnIdx = PcbPtr->Info.DirFetch.Dir0SxnIdx;
        Dir0EntryPtr = (void *)PcbPtr->Info.DmxRead.BuffAddr;

        // Scan the content
        for (Dir0EntryIdx = 0;
             Dir0EntryIdx < DIR0_ENTRIES_PER_SXN;
             Dir0EntryIdx++)
        {
            UserPba = Dir0EntryPtr->UserPba
                ^ SCRAMBLE_WORD((Dir0SxnIdx * DIR0_ENTRIES_PER_SXN)
                    + Dir0EntryIdx);

            Dir0EntryPtr++;

            if ((UserPba & INVALID_MASK) == 0)
            {
                ASSERT(UserPba < SEGMENTS_PER_FBX);
                blkrecord_ud_set_valid_sxn(FbxPtr,
                                           UserPba);
            }
        }

        PcbPtr->Info.DirFetch.Dir0SxnIdx += DEV_CNT;
        if (PcbPtr->Info.DirFetch.Dir0SxnIdx < DIR0_SXN_CNT)
        {
            // Read next dir0 section
            dir_fetch_dir0_dev_stg1(PcbPtr);
        }
        else
        {
            // Done
            dm_notify_completion(PcbPtr);
            _sched_return_pcb(PcbPtr);
        }
    }

    else
    {
        // Both original and mirror dir0sxns are uncorrectable.
        err_gross();
    }

    return;
}
#endif


//-----------------------------------------------------------------------------
// Function    : dir_fetch_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_fetch_dir0 (PCB_STRUCT *PcbPtr)
{
    CNTL_CACHE_STRUCT *Dir0CachePtr;
    unsigned long Dir0SxnIdx;
    DM_FBX_STRUCT *FbxPtr;

    Dir0CachePtr = PcbPtr->Info.CntlRead.CntlCachePtr;
    Dir0SxnIdx = SXNIDX(Dir0CachePtr->Identity);
    FbxPtr = PcbPtr->Word.FbxPtr;

    PcbPtr->Info.DmxRead.Pba = FbxPtr->Dir.Dir1EntryPtr[Dir0SxnIdx].Dir0Pba0;
    PcbPtr->Info.DmxRead.BuffAddr = Dir0CachePtr->DataAddr;
    PcbPtr->Info.DmxRead.SegCnt = 1;
    PcbPtr->Fn = dir_fetch_dir0_done;
    dmx_ops_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_fetch_dir0_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_fetch_dir0_done (PCB_STRUCT *PcbPtr)
{
    BIT_STAT DmxStat;
    CNTL_CACHE_STRUCT *Dir0CachePtr;
    PCB_STRUCT *WakePcbPtr;
    IDENTITY_INT Identity;

    DmxStat = PcbPtr->Info.DmxRead.Stat;
    Dir0CachePtr = PcbPtr->Info.CntlRead.CntlCachePtr;

    if (DmxStat == DMX_OPS_ECC_UNCORRECTABLE)
    {
        #if defined(CD_MIRROR)
        dir_fetch_dir0_mirror(PcbPtr);
        #else
        err_gross();
        #endif

        return;
    }

    else if (DmxStat == DMX_OPS_ECC_CORRECTABLE)
    {
        // Remap this dir section by simply scheduling it to be written
        CNTLDATA_SET_DIRTY(Dir0CachePtr,
                           PcbPtr->Word.FbxPtr);

        // but! mark the page where section is located so no future allocations
        // will be made to it. Eventually, all valid sections in the page will
        // be invalidated making this a fully invalid and bad page.

        // Remap entire block
        remap_cntl_trigger_remap(PcbPtr->Word.FbxPtr,
                                 PcbPtr->Info.DmxRead.Pba);
    }

    // Turn on IncacheFlag
    Dir0CachePtr->State = CNTLDATA_INCACHE;
    Identity = Dir0CachePtr->Identity;

    // Wake up all
    while (1)
    {
        WakePcbPtr = util_sll_get_head_entry(&Dir0CachePtr->StateWaitQ);

        if (WakePcbPtr == BIT_NULL_PTR)
        {
            break;
        }

        WakePcbPtr->Fn(WakePcbPtr);

        if (Identity != Dir0CachePtr->Identity)
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
// Function    : dir_fetch_dir0_mirror
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_fetch_dir0_mirror (PCB_STRUCT *PcbPtr)
{
    CNTL_CACHE_STRUCT *Dir0CachePtr;
    unsigned long Dir0SxnIdx;
    DM_FBX_STRUCT *FbxPtr;

    Dir0CachePtr = PcbPtr->Info.CntlRead.CntlCachePtr;
    Dir0SxnIdx = SXNIDX(Dir0CachePtr->Identity);
    FbxPtr = PcbPtr->Word.FbxPtr;

    PcbPtr->Info.DmxRead.Pba = FbxPtr->Dir.Dir1EntryPtr[Dir0SxnIdx].Dir0Pba1;
    PcbPtr->Info.DmxRead.BuffAddr = Dir0CachePtr->DataAddr;
    PcbPtr->Info.DmxRead.SegCnt = 1;
    PcbPtr->Fn = dir_fetch_dir0_mirror_done;
    dmx_ops_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : dir_fetch_dir0_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dir_fetch_dir0_mirror_done (PCB_STRUCT *PcbPtr)
{
    BIT_STAT DmxStat;
    CNTL_CACHE_STRUCT *Dir0CachePtr;
    PCB_STRUCT *WakePcbPtr;
    IDENTITY_INT Identity;

    DmxStat = PcbPtr->Info.DmxRead.Stat;
    Dir0CachePtr = PcbPtr->Info.CntlRead.CntlCachePtr;

    if (DmxStat == DMX_OPS_ECC_UNCORRECTABLE)
    {
        // Both original and mirror Dir0Sxns are uncorrectable.
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
    CNTLDATA_SET_DIRTY(Dir0CachePtr,
                       PcbPtr->Word.FbxPtr);

    // Turn on IncacheFlag
    Dir0CachePtr->State = CNTLDATA_INCACHE;
    Identity = Dir0CachePtr->Identity;

    // Wake up all
    while (1)
    {
        WakePcbPtr = util_sll_get_head_entry(&Dir0CachePtr->StateWaitQ);

        if (WakePcbPtr == BIT_NULL_PTR)
        {
            break;
        }

        WakePcbPtr->Fn(WakePcbPtr);

        if (Identity != Dir0CachePtr->Identity)
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
// $Log: Dir.c,v $
// Revision 1.9  2014/05/19 04:48:58  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.8  2014/04/30 13:56:45  rcantong
// 1. DEV: Support CD mirroring
// 1.1 Added process to utilize the CD copy - JAbad
//
// Revision 1.7  2014/03/03 12:55:44  rcantong
// 1. DEV: FID hang handler
// 1.1 Added dmx stat checking for FID_HANG_TIMEOUT - JFaustino
//
// Revision 1.6  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.4  2013/12/05 13:06:34  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:44:23  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:16  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
