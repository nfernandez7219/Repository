//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/CdFlushMgr/CdFlushMgr.c,v $
// $Revision: 1.9 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:58 $
// $Id: CdFlushMgr.c,v 1.9 2014/05/19 04:48:58 rcantong Exp $
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

#include "Bios.h"
#include "BlkRecord.h"
#include "CdFlushMgr.h"
#include "CntlData.h"
#include "CntlDataCommon.h"
#include "Dir.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "FreeList.h"
#include "Scrub.h"
#include "SxnInfo.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "CdFlushMgrI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
CD_FLUSH_WORKER_STRUCT CdFlushWorker[FBX_CNT][FLUSH_WORKER_CNT];
L1CACHE_ALIGN(CdFlushWorker);

unsigned char CdHdrBuffer[FBX_CNT][FLUSH_WORKER_CNT][CNTL_SXN_SIZE];
L1CACHE_ALIGN(CdHdrBuffer);

// This is supposed to be (CNTL_SXNS_PER_PAGE - 1), but we want
// to play nicely with the DMX so we pad one more ulong to make it happen.
unsigned long BufferGatherAddr[FBX_CNT][FLUSH_WORKER_CNT][CNTL_SXNS_PER_PAGE];
L1CACHE_ALIGN(BufferGatherAddr);

// We temporarily store references to our cntl data sections here
// so we don't have to modify the dirty list before doing flushes, especially
// if we skip through many locked entries
unsigned long GatherArray[FBX_CNT][FLUSH_WORKER_CNT][CNTL_SXNS_PER_PAGE];
L1CACHE_ALIGN(GatherArray);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : cdflushmgr_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cdflushmgr_init_malloc (unsigned long FbxIdx)
{
    DM_FBX_STRUCT *FbxPtr;
    volatile CNTL_HDR_STRUCT *CdHdrBufPtr;
    PCB_STRUCT *PcbPtr;
    PCB_CD_FLUSH_STRUCT *CntlWritePtr;
    CD_FLUSH_WORKER_STRUCT *WorkerPtr;
    unsigned long WorkerIdx;

    FbxPtr = &DmFbx[FbxIdx];

    WorkerPtr = &CdFlushWorker[FbxIdx][0];
    FbxPtr->CdFlushMgr.WorkerPtr = WorkerPtr;

    CdHdrBufPtr = (void *)&CdHdrBuffer[FbxIdx][0][0];
    dm_fill_random_pattern((void *)CdHdrBufPtr,
                           CNTL_SXN_SIZE * FLUSH_WORKER_CNT);

    for (WorkerIdx = 0;
         WorkerIdx < FLUSH_WORKER_CNT;
         WorkerIdx++)
    {
        CdHdrBufPtr = (void *)&CdHdrBuffer[FbxIdx][WorkerIdx][0];
        CdHdrBufPtr->Signature = CNTL_HDR_SIGNATURE;

        // Initialize the worker
        PcbPtr = &WorkerPtr[WorkerIdx].Pcb;
        PcbPtr->Word.FbxPtr = FbxPtr;

        PcbPtr->Info.DmxWriteCad.BuffPtr
                = &BufferGatherAddr[FbxIdx][WorkerIdx][0];

        PcbPtr->Info.DmxWriteCad.BuffPtr[0]
                = (unsigned long)CdHdrBufPtr;

        // Initialize the gather address parameters for dmx_write_cad()
        CntlWritePtr = (void *)&PcbPtr->Info;
        CntlWritePtr->ActiveFlag = OFF;
        CntlWritePtr->CntlHdrPtr = (void *)(CdHdrBufPtr);
        CntlWritePtr->GatherPtr = &GatherArray[FbxIdx][WorkerIdx][0];

        if (WorkerIdx < (FLUSH_WORKER_CNT / 2))
        {
            CntlWritePtr->FlushPrio = HIGH_PRIO;
        }
        else
        {
            CntlWritePtr->FlushPrio = LOW_PRIO;
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : cdflushmgr_start_mgr_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cdflushmgr_start_mgr_fbx (PCB_STRUCT *PcbPtr)
{
    // The last fbx who will initialize in this domain will be the one to
    // turn on the arc timer 1. we assume FBX_CNT_PER_DOMAIN == 2.
    if (PcbPtr->Word.FbxIdx < 3)
    {
        _sr(3, 0x101);

        // 1ms interrupts based from ARC 400MHz
        _sr(5 * 400000, 0x102);

        // Start the local timer 1 now
        _sr(0, 0x100);

        // Count for 500ms cd flushing
        CdFlushTmrCnt = 0;
    }

    SCHED_POST_PCB(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : arc_timer1_cdflushmgr
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void arc_timer1_cdflushmgr (void)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_STRUCT *PcbPtr;
    unsigned long LocalFbxIdx = 0;
    unsigned long WorkerIdx;

    #if defined(UD_SCRUB)
    while (1)
    {
        FbxPtr = LocalFbxPtr[LocalFbxIdx++];
        if (FbxPtr == BIT_NULL_PTR)
        {
            break;
        }

        scrub_start_scrubbing(FbxPtr);
    }

    LocalFbxIdx = 0;
    #endif

    CdFlushTmrCnt++;
    if (CdFlushTmrCnt < 100)
    {
        return;
    }

    CdFlushTmrCnt = 0;

    while (1)
    {
        FbxPtr = LocalFbxPtr[LocalFbxIdx++];
        if (FbxPtr == BIT_NULL_PTR)
        {
            break;
        }

        // Move all entries from dirty list to flush list
        util_dll_move_all_entries(&FbxPtr->CntlData.DirtyList,
                                  &FbxPtr->CntlData.FlushList);

        for (WorkerIdx = 0;
             WorkerIdx < FLUSH_WORKER_CNT;
             WorkerIdx++)
        {
            PcbPtr = &FbxPtr->CdFlushMgr.WorkerPtr[WorkerIdx].Pcb;

            if (PcbPtr->Info.CntlWrite.ActiveFlag == OFF)
            {
                // This is the first call to flush, so we don't
                // require a full page.
                cdflushmgr_start_flush_ops(PcbPtr,
                                           1);
            }
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : cdflushmgr_start_flush_ops
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cdflushmgr_start_flush_ops (PCB_STRUCT *PcbPtr,
                                 unsigned long MinCntlSxnCnt)
{
    DM_FBX_STRUCT *FbxPtr;
    UTIL_DLL_STRUCT *FlushListPtr;
    CNTL_CACHE_STRUCT *CntlCachePtr;
    unsigned long *GatherPtr;
    unsigned long SegCnt;
    unsigned long *BuffPtr;
    volatile CNTL_HDR_STRUCT *CntlHdrPtr;
    unsigned long Idx;

    FbxPtr = PcbPtr->Word.FbxPtr;
    FlushListPtr = &FbxPtr->CntlData.FlushList;

    CntlCachePtr = util_dll_peek_head_entry(FlushListPtr);

    // Preparation of gather list prior to DMX operation
    // traverse our dirty list without modifying it.
    // Just record all the unlocked control caches that we can flush.
    GatherPtr = PcbPtr->Info.CntlWrite.GatherPtr;
    SegCnt = 0;

    while (CntlCachePtr != BIT_NULL_PTR)
    {
        ASSERT(CntlCachePtr->DirtyCnt > 0);

        // Record the cache ptr
        GatherPtr[SegCnt] = (unsigned long)CntlCachePtr;

        SegCnt++;

        if (SegCnt == (CNTL_SXNS_PER_PAGE_LIMIT - 1))
        {
            break;
        }

        // Move to next cache pointer in dirty list
        CntlCachePtr = util_dll_peek_next(&CntlCachePtr->Link,
                                          FlushListPtr);
    }

    if (SegCnt >= MinCntlSxnCnt)
    {
        BuffPtr = &PcbPtr->Info.DmxWriteCad.BuffPtr[1];
        CntlHdrPtr = (void *)PcbPtr->Info.CntlWrite.CntlHdrPtr;

        FbxPtr->Sqn++;
        CntlHdrPtr->Sqn = FbxPtr->Sqn;

        for (Idx = 0;
             Idx < SegCnt;
             Idx++)
        {
            CntlCachePtr = (CNTL_CACHE_STRUCT *)GatherPtr[Idx];

            // Record the gather address itself
            BuffPtr[Idx] = (unsigned long)CntlCachePtr->DataAddr;

            // Also make sure incoming write requests don't use it while we're
            // flushing. They'll just sleep on the queue. Reads on the
            // other hand are allowed.
            CntlCachePtr->State = CNTLDATA_INFLUSH;

            // Update the control header
            CntlHdrPtr->Identity[Idx] = CntlCachePtr->Identity;

            util_dll_remove_from_middle(&CntlCachePtr->Link);
        }

        CntlHdrPtr->Identity[Idx] = 0;

        // Control data segments + cntl hdr segment
        PcbPtr->Info.DmxWriteCad.SegCnt = SegCnt + 1;

        PcbPtr->Info.CntlWrite.ActiveFlag = ON;

        // Gather list is complete...
        cdflushmgr_flush_page(PcbPtr);
    }
    else
    {
        PcbPtr->Info.CntlWrite.ActiveFlag = OFF;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : cdflushmgr_flush_page
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cdflushmgr_flush_page (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    if (PcbPtr->Info.CntlWrite.FlushPrio == HIGH_PRIO)
    {
        Pba = freelist_cd_get_cmpct_page(FbxPtr);
    }
    else
    {
        Pba = freelist_cd_get_page(FbxPtr);
    }

    if (PcbPtr->Info.DmxWriteCad.Pba == INVALID_MASK)
    {
        PcbPtr->Fn = cdflushmgr_flush_page;
        SCHED_POST_PCB(PcbPtr);
        return;
    }

    // Prepare dmx operation parameters and do actual transfer
    PcbPtr->Info.DmxWriteCad.Pba = Pba;
    PcbPtr->Fn = cdflushmgr_flush_page_cb;
    dmx_ops_write_cad(PcbPtr);

    return;
}


#if defined(CD_MIRROR)
//-----------------------------------------------------------------------------
// Function    : cdflushmgr_flush_page_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cdflushmgr_flush_page_cb (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT Pba;
    unsigned long *GatherPtr;
    unsigned long SegCnt;
    unsigned long Idx;
    CNTL_CACHE_STRUCT *CachePtr;
    unsigned long Target;
    unsigned long SxnIdx;
    PBA_INT OldPba;
    IDENTITY_INT *CdSiPtr;

    FbxPtr = PcbPtr->Word.FbxPtr;
    Pba = PcbPtr->Info.DmxWriteCad.Pba;

    // We wrote to a defective page. Mark it accordingly and repeat the
    // flushing again.
    if (PcbPtr->Info.DmxWriteCad.Stat != SUCCESSFUL)
    {
        if (PcbPtr->Info.DmxWriteCad.Stat != DMX_OPS_CMD_RETRY)
        {
            // Set Quarantine Bit
            blkrecord_cd_set_q_blk(FbxPtr,
                                   Pba);
        }

        freelist_cd_chk_n_remove_cur_pba(FbxPtr,
                                         Pba);

        cdflushmgr_flush_page(PcbPtr);

        return;
    }

    GatherPtr = PcbPtr->Info.CntlWrite.GatherPtr;
    CdSiPtr = FbxPtr->CntlData.IdPtr;

    Pba++;
    SegCnt = PcbPtr->Info.DmxWriteCad.SegCnt - 1;

    for (Idx = 0;
         Idx < SegCnt;
         Idx++)
    {
        CachePtr = (CNTL_CACHE_STRUCT *)*GatherPtr;
        GatherPtr++;

        Target = CachePtr->Identity & TARGET_MASK;
        SxnIdx = CachePtr->Identity & IDENTITY_SXNIDX_MASK;

        CdSiPtr[Pba] = CachePtr->Identity;

        if (Target == TARGET_DIR0)
        {
            // validate new pba section
            blkrecord_cd_set_valid_sxn(FbxPtr,
                                       Pba);

            OldPba = FbxPtr->Dir.Dir1EntryPtr[SxnIdx].Dir0Pba0;
            // Invalidate old pba section
            blkrecord_cd_set_invalid_sxn(FbxPtr,
                                         OldPba);

            // Update parent
            FbxPtr->Dir.Dir1EntryPtr[SxnIdx].Dir0Pba0 = Pba;            
        }

        else if (Target == TARGET_SI0)
        {
            // validate new pba section
            blkrecord_cd_set_valid_sxn(FbxPtr,
                                       Pba);

            OldPba = FbxPtr->SxnInfo.Si1EntryPtr[SxnIdx].Si0Pba0;
            // Invalidate old pba section
            blkrecord_cd_set_invalid_sxn(FbxPtr,
                                         OldPba);

            // Update parent
            FbxPtr->SxnInfo.Si1EntryPtr[SxnIdx].Si0Pba0 = Pba;
        }

        else
        {
            _brk();
        }

        // Move to next Segment in page
        Pba++;
    }

    cdflushmgr_flush_mirror_page(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : cdflushmgr_flush_mirror_page
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cdflushmgr_flush_mirror_page (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    if (PcbPtr->Info.CntlWrite.FlushPrio == HIGH_PRIO)
    {
        Pba = freelist_cd_get_cmpct_page(FbxPtr);
    }
    else
    {
        Pba = freelist_cd_get_page(FbxPtr);
    }

    if (Pba == INVALID_MASK)
    {
        PcbPtr->Fn = cdflushmgr_flush_mirror_page;
        SCHED_POST_PCB(PcbPtr);
        return;
    }

    // Prepare dmx operation parameters and do actual transfer
    PcbPtr->Info.DmxWriteCad.Pba = Pba;
    PcbPtr->Fn = cdflushmgr_flush_mirror_page_cb;
    dmx_ops_write_cad(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : cdflushmgr_flush_mirror_page_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cdflushmgr_flush_mirror_page_cb (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT Pba;
    unsigned long *GatherPtr;
    unsigned long SegCnt;
    unsigned long Idx;
    CNTL_CACHE_STRUCT *CachePtr;
    PCB_STRUCT *WakePcbPtr;
    unsigned long Target;
    unsigned long SxnIdx;
    PBA_INT OldPba;
    IDENTITY_INT *CdSiPtr;
    IDENTITY_INT Identity;

    FbxPtr = PcbPtr->Word.FbxPtr;
    Pba = PcbPtr->Info.DmxWriteCad.Pba;

    // We wrote to a defective page. Mark it accordingly and repeat the
    // flushing again.
    if (PcbPtr->Info.DmxWriteCad.Stat != SUCCESSFUL)
    {
        if (PcbPtr->Info.DmxWriteCad.Stat != DMX_OPS_CMD_RETRY)
        {
            // Set Quarantine Bit
            blkrecord_cd_set_q_blk(FbxPtr,
                                   Pba);
        }

        freelist_cd_chk_n_remove_cur_pba(FbxPtr,
                                         Pba);

        cdflushmgr_flush_mirror_page(PcbPtr);

        return;
    }

    GatherPtr = PcbPtr->Info.CntlWrite.GatherPtr;
    CdSiPtr = FbxPtr->CntlData.IdPtr;

    Pba++;
    SegCnt = PcbPtr->Info.DmxWriteCad.SegCnt - 1;

    for (Idx = 0;
         Idx < SegCnt;
         Idx++)
    {
        CachePtr = (CNTL_CACHE_STRUCT *)*GatherPtr;
        GatherPtr++;

        // Cntl data is now clean
        ASSERT(CachePtr->DirtyCnt != 0);
        CachePtr->DirtyCnt = 0;

        // If there are processes that need to be woken up, wake them now.
        CachePtr->State = CNTLDATA_INCACHE;

        Target = CachePtr->Identity & TARGET_MASK;
        SxnIdx = CachePtr->Identity & IDENTITY_SXNIDX_MASK;

        CdSiPtr[Pba] = CachePtr->Identity;

        if (Target == TARGET_DIR0)
        {
            OldPba = FbxPtr->Dir.Dir1EntryPtr[SxnIdx].Dir0Pba1;

            // validate new pba section
            blkrecord_cd_set_valid_sxn(FbxPtr,
                                       Pba);

            // Check if there is no mirror Pba,
            // This is possible during init when one sxn is uncorrectable.
            if (OldPba != INVALID_MASK)
            {
                // Invalidate section
                blkrecord_cd_set_invalid_sxn(FbxPtr,
                                             OldPba);
            }

            // Update parent
            FbxPtr->Dir.Dir1EntryPtr[SxnIdx].Dir0Pba1 = Pba;

            if (CachePtr->LockCnt == 0)
            {
                DIR_PUT_TO_CLEAN_LIST(CachePtr, FbxPtr->Dir);

                // Wake the dir request wait q
                WakePcbPtr = CNTLDATA_GET_HEAD_RQST_WAITQ(FbxPtr->Dir);
                if (WakePcbPtr != BIT_NULL_PTR)
                {
                    WakePcbPtr->Fn(WakePcbPtr);
                }
            }

            else
            {
                Identity = CachePtr->Identity;
                while (1)
                {
                    WakePcbPtr = util_sll_get_head_entry(&CachePtr->StateWaitQ);
                    if (WakePcbPtr == BIT_NULL_PTR)
                    {
                        break;
                    }

                    WakePcbPtr->Fn(WakePcbPtr);

                    if (Identity != CachePtr->Identity)
                    {
                        break;
                    }
                }
            }
        }

        else if (Target == TARGET_SI0)
        {
            OldPba = FbxPtr->SxnInfo.Si1EntryPtr[SxnIdx].Si0Pba1;

            // validate new pba section
            blkrecord_cd_set_valid_sxn(FbxPtr,
                                       Pba);

            // Check if there is no mirror Pba,
            // This is possible during init when one sxn is uncorrectable.
            if (OldPba != INVALID_MASK)
            {
                blkrecord_cd_set_invalid_sxn(FbxPtr,
                                             OldPba);
            }

            // Update parent
            FbxPtr->SxnInfo.Si1EntryPtr[SxnIdx].Si0Pba1 = Pba;

            if (CachePtr->LockCnt == 0)
            {
                SXNINFO_PUT_TO_CLEAN_LIST(CachePtr, FbxPtr->SxnInfo);

                // Wake the si request wait q
                WakePcbPtr = CNTLDATA_GET_HEAD_RQST_WAITQ(FbxPtr->SxnInfo);
                if (WakePcbPtr != BIT_NULL_PTR)
                {
                    WakePcbPtr->Fn(WakePcbPtr);
                }
            }

            else
            {
                Identity = CachePtr->Identity;
                while (1)
                {
                    WakePcbPtr = util_sll_get_head_entry(&CachePtr->StateWaitQ);
                    if (WakePcbPtr == BIT_NULL_PTR)
                    {
                        break;
                    }

                    WakePcbPtr->Fn(WakePcbPtr);

                    if (Identity != CachePtr->Identity)
                    {
                        break;
                    }
                }
            }
        }

        else
        {
            _brk();
        }

        // Move to next Segment in page
        Pba++;
    }

    // Try to continue flushing.
    // but since this is a continuation, we now require a full page.
    cdflushmgr_start_flush_ops(PcbPtr,
                               CNTL_SXNS_PER_PAGE_LIMIT - 1);

    return;
}


#else
//-----------------------------------------------------------------------------
// Function    : cdflushmgr_flush_page_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cdflushmgr_flush_page_cb (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT Pba;
    unsigned long *GatherPtr;
    unsigned long SegCnt;
    unsigned long Idx;
    CNTL_CACHE_STRUCT *CachePtr;
    PCB_STRUCT *WakePcbPtr;
    unsigned long Target;
    unsigned long SxnIdx;
    PBA_INT OldPba;
    IDENTITY_INT *CdSiPtr;
    IDENTITY_INT Identity;

    FbxPtr = PcbPtr->Word.FbxPtr;
    Pba = PcbPtr->Info.DmxWriteCad.Pba;

    // We wrote to a defective page. Mark it accordingly and repeat the
    // flushing again.
    if (PcbPtr->Info.DmxWriteCad.Stat != SUCCESSFUL)
    {
        if (PcbPtr->Info.DmxWriteCad.Stat != DMX_OPS_CMD_RETRY)
        {
            // Set Quarantine Bit
            blkrecord_cd_set_q_blk(FbxPtr,
                                   Pba);
        }

        freelist_cd_chk_n_remove_cur_pba(FbxPtr,
                                         Pba);

        cdflushmgr_flush_page(PcbPtr);

        return;
    }

    GatherPtr = PcbPtr->Info.CntlWrite.GatherPtr;
    CdSiPtr = FbxPtr->CntlData.IdPtr;

    Pba++;
    SegCnt = PcbPtr->Info.DmxWriteCad.SegCnt - 1;

    for (Idx = 0;
         Idx < SegCnt;
         Idx++)
    {
        CachePtr = (CNTL_CACHE_STRUCT *)*GatherPtr;
        GatherPtr++;

        // Cntl data is now clean
        ASSERT(CachePtr->DirtyCnt != 0);
        CachePtr->DirtyCnt = 0;

        // If there are processes that need to be woken up, wake them now.
        CachePtr->State = CNTLDATA_INCACHE;

        Target = CachePtr->Identity & TARGET_MASK;
        SxnIdx = CachePtr->Identity & IDENTITY_SXNIDX_MASK;

        CdSiPtr[Pba] = CachePtr->Identity;

        if (Target == TARGET_DIR0)
        {
            OldPba = FbxPtr->Dir.Dir1EntryPtr[SxnIdx].Dir0Pba0;

            // validate new pba section
            blkrecord_cd_set_valid_sxn(FbxPtr,
                                       Pba);

            // Invalidate section
            blkrecord_cd_set_invalid_sxn(FbxPtr,
                                         OldPba);

            // Update parent
            FbxPtr->Dir.Dir1EntryPtr[SxnIdx].Dir0Pba0 = Pba;

            if (CachePtr->LockCnt == 0)
            {
                DIR_PUT_TO_CLEAN_LIST(CachePtr, FbxPtr->Dir);

                // Wake the dir request wait q
                WakePcbPtr = CNTLDATA_GET_HEAD_RQST_WAITQ(FbxPtr->Dir);
                if (WakePcbPtr != BIT_NULL_PTR)
                {
                    WakePcbPtr->Fn(WakePcbPtr);
                }
            }

            else
            {
                Identity = CachePtr->Identity;
                while (1)
                {
                    WakePcbPtr = util_sll_get_head_entry(&CachePtr->StateWaitQ);
                    if (WakePcbPtr == BIT_NULL_PTR)
                    {
                        break;
                    }

                    WakePcbPtr->Fn(WakePcbPtr);

                    if (Identity != CachePtr->Identity)
                    {
                        break;
                    }
                }
            }
        }

        else if (Target == TARGET_SI0)
        {
            OldPba = FbxPtr->SxnInfo.Si1EntryPtr[SxnIdx].Si0Pba0;

            // validate new pba section
            blkrecord_cd_set_valid_sxn(FbxPtr,
                                       Pba);

            // Invalidate section
            blkrecord_cd_set_invalid_sxn(FbxPtr,
                                         OldPba);

            // Update parent
            FbxPtr->SxnInfo.Si1EntryPtr[SxnIdx].Si0Pba0 = Pba;

            if (CachePtr->LockCnt == 0)
            {
                SXNINFO_PUT_TO_CLEAN_LIST(CachePtr, FbxPtr->SxnInfo);

                // Wake the si request wait q
                WakePcbPtr = CNTLDATA_GET_HEAD_RQST_WAITQ(FbxPtr->SxnInfo);
                if (WakePcbPtr != BIT_NULL_PTR)
                {
                    WakePcbPtr->Fn(WakePcbPtr);
                }
            }

            else
            {
                Identity = CachePtr->Identity;
                while (1)
                {
                    WakePcbPtr = util_sll_get_head_entry(&CachePtr->StateWaitQ);
                    if (WakePcbPtr == BIT_NULL_PTR)
                    {
                        break;
                    }

                    WakePcbPtr->Fn(WakePcbPtr);

                    if (Identity != CachePtr->Identity)
                    {
                        break;
                    }
                }
            }
        }

        else
        {
            _brk();
        }

        // Move to next Segment in page
        Pba++;
    }

    // Try to continue flushing.
    // but since this is a continuation, we now require a full page.
    cdflushmgr_start_flush_ops(PcbPtr,
                               CNTL_SXNS_PER_PAGE_LIMIT - 1);

    return;
}
#endif


//=============================================================================
// $Log: CdFlushMgr.c,v $
// Revision 1.9  2014/05/19 04:48:58  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.8  2014/05/13 13:19:51  rcantong
// 1. DEV: Support background scrubber
// 1.1 Added background scrubber process - PPestano
//
// Revision 1.7  2014/04/30 13:48:54  rcantong
// 1. DEV: Prioritized flushing of compacting CD
// 1.1 Added process for flushing of compacting CD - MFenol
// 2. DEV: Support CD mirroring
// 2.1 Added API for flush mirror page - JAbad
//
// Revision 1.6  2014/03/03 13:04:40  rcantong
// 1. BUGFIX: Waking CD cache with different owner already
// 1.1 Stop waking Pcb when owner changes
//
// Revision 1.5  2014/02/02 10:00:30  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.4  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.3  2013/12/05 13:06:33  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.2  2013/08/08 16:44:22  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:10  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
