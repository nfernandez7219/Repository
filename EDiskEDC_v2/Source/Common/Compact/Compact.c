//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Compact/Compact.c,v $
// $Revision: 1.10 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:58 $
// $Id: Compact.c,v 1.10 2014/05/19 04:48:58 rcantong Exp $
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
#include "CntlDataCommon.h"
#include "CntlData.h"
#include "Compact.h"
#include "Dir.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "FreeList.h"
#include "Media.h"
#include "Reclaim.h"
#include "RemapUser.h"
#include "SxnInfo.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "CompactI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
COMPACT_LANE_STRUCT CompactCdLane[FBX_CNT][LANE_CNT];
L1CACHE_ALIGN(CompactCdLane);

COMPACT_LANE_STRUCT CompactUdLane[FBX_CNT][LANE_CNT];
L1CACHE_ALIGN(CompactUdLane);
#pragma BSS()

#pragma BSS(".dm_buffer")
unsigned char CompactUdBuff[FBX_CNT][LANE_CNT][FLASH_PAGE_SIZE];
L1CACHE_ALIGN(CompactUdBuff);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : compact_cd_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_cd_init_malloc (unsigned long FbxIdx)
{
    DM_FBX_STRUCT *FbxPtr;
    COMPACT_LANE_STRUCT *LanePtr;
    unsigned long LaneIdx;
    PCB_STRUCT *PcbPtr;

    FbxPtr = &DmFbx[FbxIdx];

    // Static malloc
    LanePtr = &CompactCdLane[FbxIdx][0];
    FbxPtr->CompactCd.LanePtr = LanePtr;

    util_init_pattern(CompactCdLane[FbxIdx],
                      sizeof(CompactCdLane[0]),
                      INIT_PATTERN_LO_VALUE);

    for (LaneIdx = 0;
         LaneIdx < LANE_CNT;
         LaneIdx++)
    {
        PcbPtr = &LanePtr->Pcb;
        PcbPtr->Word.FbxPtr = FbxPtr;
        PcbPtr->Info.CompactCd.LaneIdx = LaneIdx;
        PcbPtr->Info.CompactCd.ActiveFlag = OFF;
        PcbPtr->Info.CompactCd.Counter = 0;
        PcbPtr->Info.CompactCd.CompactPba = INVALID_MASK;
        PcbPtr->Info.CompactCd.CachePtr = BIT_NULL_PTR;
        PcbPtr->Info.CompactCd.SxnIdx = 0;
        LanePtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_cd_trigger_cmpct_prcs
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_cd_trigger_cmpct_prcs (DM_FBX_STRUCT *FbxPtr,
                                    unsigned long LaneIdx)
{
    PCB_STRUCT *PcbPtr;

    PcbPtr = &FbxPtr->CompactCd.LanePtr[LaneIdx].Pcb;

    if (PcbPtr->Info.CompactCd.ActiveFlag == OFF)
    {
        PcbPtr->Info.CompactCd.ActiveFlag = ON;
        PcbPtr->Info.CompactCd.Counter = 0;
        PcbPtr->Info.CompactCd.CompactPba = INVALID_MASK;
        compact_cd_start_compacting(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_ud_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_ud_init_malloc (unsigned long FbxIdx)
{
    DM_FBX_STRUCT *FbxPtr;
    COMPACT_LANE_STRUCT *LanePtr;
    unsigned long LaneIdx;

    FbxPtr = &DmFbx[FbxIdx];

    // Static malloc
    LanePtr = &CompactUdLane[FbxIdx][0];
    FbxPtr->CompactUd.LanePtr = LanePtr;

    util_init_pattern(CompactUdLane[FbxIdx],
                      sizeof(CompactUdLane[0]),
                      INIT_PATTERN_LO_VALUE);

    for (LaneIdx = 0;
         LaneIdx < LANE_CNT;
         LaneIdx++)
    {
        LanePtr->BuffAddr = (unsigned long)&CompactUdBuff[FbxIdx][LaneIdx][0];
        LanePtr->Pcb.Word.FbxPtr = FbxPtr;
        LanePtr->Pcb.Info.CompactUd.LaneIdx = LaneIdx;
        LanePtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_ud_trigger_cmpct_prcs
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_ud_trigger_cmpct_prcs (DM_FBX_STRUCT *FbxPtr,
                                    unsigned long LaneIdx)
{
    COMPACT_LANE_STRUCT *CmpctLanePtr;
    PCB_STRUCT *PcbPtr;

    CmpctLanePtr = &FbxPtr->CompactUd.LanePtr[LaneIdx];
    PcbPtr = &CmpctLanePtr->Pcb;

    if (PcbPtr->Info.CompactUd.ActiveFlag == OFF)
    {
        PcbPtr->Info.CompactUd.ActiveFlag = ON;

        PcbPtr->Info.DmxRead.BuffAddr
            = CmpctLanePtr->BuffAddr
            + (USER_SXN_SIZE * (USER_SXNS_PER_PAGE - 1));

        PcbPtr->Info.CompactUd.CmpctSxnCnt = 0;
        PcbPtr->Info.CompactUd.NxtPcbPtr = BIT_NULL_PTR;
        PcbPtr->Info.CompactUd.CompactPba = INVALID_MASK;

        compact_ud_prcs_get_cmpct_sxn(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_ud_trigger_idle_cmpct_prcs
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_ud_trigger_idle_cmpct_prcs (void)
{
    unsigned long LocalFbxIdx;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;

    LocalFbxIdx = 0;
    while (1)
    {
        FbxPtr = LocalFbxPtr[LocalFbxIdx];
        LocalFbxIdx++;
        if (FbxPtr == BIT_NULL_PTR)
        {
            break;
        }

        for (LaneIdx = 0;
             LaneIdx < LANE_CNT;
             LaneIdx++)
        {
            if (    RECLAIM_UD_CNT(FbxPtr->ReclaimUd, LaneIdx)
                 <= RECLAIM_COMPACT_THRESHOLD_1)
            {
                compact_ud_trigger_cmpct_prcs(FbxPtr,
                                              LaneIdx);
            }
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : compact_cd_dirty_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_cd_dirty_dir0 (PCB_STRUCT *PcbPtr)
{
    CNTL_CACHE_STRUCT *Dir0CachePtr;

    Dir0CachePtr = PcbPtr->Info.CompactCd.CachePtr;

    // Set cache to dirty
    COMPACT_CD_SET_DIRTY(Dir0CachePtr,
                         PcbPtr->Word.FbxPtr);

    // Unlock dir0 cache
    CNTLDATA_UNLOCK_WRITE(Dir0CachePtr);

    // Check if we do enough compact already
    PcbPtr->Info.CompactCd.Counter++;
    if (PcbPtr->Info.CompactCd.Counter < 1000)
    {
        // Try to evaluate next cntl section
        PcbPtr->Info.CompactCd.CompactPba += SEGMENTS_PER_CNTL_SXN;
        PcbPtr->Fn = compact_cd_start_compacting;
        SCHED_POST_PCB(PcbPtr);
    }
    else
    {
        // Turn off active flag
        PcbPtr->Info.CompactCd.ActiveFlag = OFF;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_cd_dirty_si0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_cd_dirty_si0 (PCB_STRUCT *PcbPtr)
{
    CNTL_CACHE_STRUCT *Si0CachePtr;

    Si0CachePtr = PcbPtr->Info.CompactCd.CachePtr;

    // Set cache to dirty
    COMPACT_CD_SET_DIRTY(Si0CachePtr,
                         PcbPtr->Word.FbxPtr);

    // Unlock si0 cache
    CNTLDATA_UNLOCK_WRITE(Si0CachePtr);

    // Check if we do enough compact already
    PcbPtr->Info.CompactCd.Counter++;
    if (PcbPtr->Info.CompactCd.Counter < 1000)
    {
        // Try to evaluate next section
        PcbPtr->Info.CompactCd.CompactPba += SEGMENTS_PER_CNTL_SXN;
        PcbPtr->Fn = compact_cd_start_compacting;
        SCHED_POST_PCB(PcbPtr);
    }
    else
    {
        // Turn off active flag
        PcbPtr->Info.CompactCd.ActiveFlag = OFF;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_cd_alloc_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_cd_alloc_dir0 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_COMPACT_CD_STRUCT *CompactCdPtr;
    BIT_STAT Stat;

    FbxPtr = PcbPtr->Word.FbxPtr;
    CompactCdPtr = (void *)&PcbPtr->Info;

    Stat = dir_alloc_compact(FbxPtr,
                             CompactCdPtr->SxnIdx,
                             &CompactCdPtr->CachePtr);

    if (Stat == CNTLDATA_INCACHE)
    {
        compact_cd_dirty_dir0(PcbPtr);
    }

    else if (Stat == CNTLDATA_INFLUSH)
    {
        CNTLDATA_UNLOCK_WRITE(CompactCdPtr->CachePtr);
        PcbPtr->Info.CompactCd.CompactPba += SEGMENTS_PER_CNTL_SXN;
        PcbPtr->Fn = compact_cd_start_compacting;
        SCHED_POST_PCB(PcbPtr);
    }

    else if (Stat == CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = compact_cd_dirty_dir0;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr, CompactCdPtr->CachePtr);
    }

    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        PcbPtr->Fn = compact_cd_alloc_dir0;
        CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr, FbxPtr->Dir);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_cd_alloc_si0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_cd_alloc_si0 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_COMPACT_CD_STRUCT *CompactCdPtr;
    BIT_STAT Stat;

    FbxPtr = PcbPtr->Word.FbxPtr;
    CompactCdPtr = (void *)&PcbPtr->Info;

    Stat = sxninfo_alloc_compact(FbxPtr,
                                 CompactCdPtr->SxnIdx,
                                 &CompactCdPtr->CachePtr);

    if (Stat == CNTLDATA_INCACHE)
    {
        compact_cd_dirty_si0(PcbPtr);
    }

    else if (Stat == CNTLDATA_INFLUSH)
    {
        CNTLDATA_UNLOCK_WRITE(CompactCdPtr->CachePtr);
        PcbPtr->Info.CompactCd.CompactPba += SEGMENTS_PER_CNTL_SXN;
        PcbPtr->Fn = compact_cd_start_compacting;
        SCHED_POST_PCB(PcbPtr);
    }

    else if (Stat == CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = compact_cd_dirty_si0;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr, CompactCdPtr->CachePtr);
    }

    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        PcbPtr->Fn = compact_cd_alloc_si0;
        CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr, FbxPtr->SxnInfo);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_cd_start_compacting
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_cd_start_compacting (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;
    PBA_INT CompactPba;
    IDENTITY_INT Identity;
    unsigned long Target;

    FbxPtr = PcbPtr->Word.FbxPtr;
    LaneIdx = PcbPtr->Info.CompactCd.LaneIdx;
    CompactPba = PcbPtr->Info.CompactCd.CompactPba;

    while (1)
    {
        // Make sure we have compact pba
        if ((CompactPba % SEGMENTS_PER_BLK) == 0)
        {
            // Get compact blk from blk record
            CompactPba = blkrecord_cd_get_compact_blk(FbxPtr,
                                                      LaneIdx);

            if (CompactPba == INVALID_MASK)
            {
                PcbPtr->Info.CompactCd.ActiveFlag = OFF;
                return;
            }

            if (freelist_cd_compare_cur_pba(FbxPtr,
                                            CompactPba) == SAME)
            {
                continue;
            }
        }

        // Get compact valid sxn within the compact blk
        CompactPba = blkrecord_cd_get_valid_sxn(FbxPtr,
                                                CompactPba);

        // If a valid sxn exists, we can proceed
        if (CompactPba != INVALID_MASK)
        {
            break;
        }
    }

    // Store the current section
    PcbPtr->Info.CompactCd.CompactPba = CompactPba;

    // Get the Identity of this Pba
    Identity = FbxPtr->CntlData.IdPtr[CompactPba];
    Target = Identity & TARGET_MASK;
    PcbPtr->Info.CompactCd.SxnIdx = SXNIDX(Identity);

    if (Target == TARGET_DIR0)
    {
        compact_cd_alloc_dir0(PcbPtr);
    }

    else if (Target == TARGET_SI0)
    {
        compact_cd_alloc_si0(PcbPtr);
    }

    else
    {
        // Try to evaluate next section
        PcbPtr->Info.CompactCd.CompactPba += SEGMENTS_PER_CNTL_SXN;
        PcbPtr->Fn = compact_cd_start_compacting;
        SCHED_POST_PCB(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_ud_prcs_get_cmpct_sxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_ud_prcs_get_cmpct_sxn (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_COMPACT_UD_STRUCT *CmpctInfoPtr;
    unsigned long LaneIdx;
    PBA_INT CompactPba;
    BIT_STAT Stat;

    FbxPtr = PcbPtr->Word.FbxPtr;
    CmpctInfoPtr = (void *)&PcbPtr->Info;
    LaneIdx = CmpctInfoPtr->LaneIdx;
    CompactPba = CmpctInfoPtr->CompactPba;

    while (1)
    {
        // Make sure we have compact pba
        if ((CompactPba % SEGMENTS_PER_BLK) == 0)
        {
            // Get compact blk from blk record
            CompactPba = blkrecord_ud_get_compact_blk(FbxPtr,
                                                      LaneIdx);

            if (CompactPba == INVALID_MASK)
            {
                PcbPtr->Fn = compact_ud_prcs_get_cmpct_sxn;
                SCHED_POST_PCB(PcbPtr);
                return;
            }

            if (freelist_ud_compare_cur_pba(FbxPtr,
                                            CompactPba) == SAME)
            {
                continue;
            }
        }

        // Get compact valid sxn within the compact blk
        CompactPba = blkrecord_ud_get_valid_sxn(FbxPtr,
                                                CompactPba);

        // If a valid sxn exists, we can proceed
        if (CompactPba != INVALID_MASK)
        {
            break;
        }
    }

    // Store the current section
    CmpctInfoPtr->CompactPba = CompactPba;

    // Si0 alloc read
    Stat = sxninfo_alloc_read(FbxPtr,
                              CompactPba,
                              &CmpctInfoPtr->CntlCachePtr);

    if (Stat <= CNTLDATA_INFLUSH)
    {
        compact_ud_read_userdata(PcbPtr);
    }
    else if (Stat == CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = compact_ud_read_userdata;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr, CmpctInfoPtr->CntlCachePtr);
    }
    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        
        blkrecord_ud_move_blk(FbxPtr,
                              CmpctInfoPtr->CompactPba);

        PcbPtr->Fn = compact_ud_prcs_get_cmpct_sxn;
        SCHED_POST_PCB(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_ud_read_userdata
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_ud_read_userdata (PCB_STRUCT *PcbPtr)
{
    PCB_COMPACT_UD_STRUCT *CmpctInfoPtr;
    volatile SI0_ENTRY_STRUCT *Si0EntryPtr;
    unsigned long Si0EntryIdx;
    unsigned long UserSxnIdx;
    DM_FBX_STRUCT *FbxPtr;

    CmpctInfoPtr = (void *)&PcbPtr->Info;
    Si0EntryPtr = (void *)CmpctInfoPtr->CntlCachePtr->DataAddr;
    Si0EntryIdx = CmpctInfoPtr->CompactPba / SEGMENTS_PER_USER_SXN;
    Si0EntryPtr += (Si0EntryIdx % SI0_ENTRIES_PER_SXN);

    UserSxnIdx = Si0EntryPtr->UserSxnIdx ^ SCRAMBLE_WORD(Si0EntryIdx);

    FbxPtr = PcbPtr->Word.FbxPtr;

    // Check if invalid sxn idx
    // in case the sxnidx is invalid get new pba for compact
    if (UserSxnIdx >= USABLE_SXN_CNT)
    {
        CmpctInfoPtr->CompactPba += SEGMENTS_PER_USER_SXN;
        PcbPtr->Fn = compact_ud_prcs_get_cmpct_sxn;
        SCHED_POST_PCB(PcbPtr);

        // Unlock si0 cache
        sxninfo_unlock_read(CmpctInfoPtr->CntlCachePtr);

        return;
    }

    PcbPtr->Info.CompactUd.UserSxnIdx = UserSxnIdx;

    // Unlock si0 cache
    sxninfo_unlock_read(CmpctInfoPtr->CntlCachePtr);

    if ((blkrecord_ud_chk_valid_sxn(FbxPtr,
                                    CmpctInfoPtr->CompactPba)) == OFF)
    {
        CmpctInfoPtr->CompactPba += SEGMENTS_PER_USER_SXN;
        PcbPtr->Fn = compact_ud_prcs_get_cmpct_sxn;
        SCHED_POST_PCB(PcbPtr);
        return;
    }

    // Check usersxn lock
    // if locked, ignore this usersxn, other process will update this userdata
    if (media_chk_n_lock_usersxn(FbxPtr, UserSxnIdx) == LOCKED)
    {
        CmpctInfoPtr->CompactPba += SEGMENTS_PER_USER_SXN;
        PcbPtr->Fn = compact_ud_prcs_get_cmpct_sxn;
        SCHED_POST_PCB(PcbPtr);

        return;
    }

    // Prepare dmx info for userdata read
    PcbPtr->Info.DmxRead.Pba = PcbPtr->Info.CompactUd.CompactPba;
    PcbPtr->Info.DmxRead.SegCnt = SEGMENTS_PER_USER_SXN;
    PcbPtr->Fn = compact_ud_read_userdata_done;
    dmx_ops_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_ud_read_userdata_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_ud_read_userdata_done (PCB_STRUCT *PcbPtr)
{
    BIT_STAT Stat;
    PCB_STRUCT *ChildPcbPtr;

    Stat = PcbPtr->Info.DmxRead.Stat;
    if (    (Stat != SUCCESSFUL)
         && (Stat != DMX_OPS_ECC_CORRECTABLE))
    {
        // Set Quarantine Bit
        blkrecord_ud_set_q_blk(PcbPtr->Word.FbxPtr,
                               PcbPtr->Info.DmxRead.Pba);

        // Ignore this section and then get new section
        media_unlock_usersxn(PcbPtr->Word.FbxPtr,
                             PcbPtr->Info.CompactUd.UserSxnIdx);

        PcbPtr->Info.CompactUd.CompactPba += SEGMENTS_PER_USER_SXN;
        compact_ud_prcs_get_cmpct_sxn(PcbPtr);

        return;
    }

    // Get pcb
    ChildPcbPtr = _sched_get_pcb();

    // Fill up child pcb for control update
    ChildPcbPtr->Word.FbxPtr = PcbPtr->Word.FbxPtr;
    ChildPcbPtr->Info.UserWrite.UserSxnIdx = PcbPtr->Info.CompactUd.UserSxnIdx;
    ChildPcbPtr->Info.UserWrite.OldPba = PcbPtr->Info.CompactUd.CompactPba;
    ChildPcbPtr->Info.UserWrite.NxtPcbPtr = PcbPtr->Info.CompactUd.NxtPcbPtr;
    PcbPtr->Info.CompactUd.NxtPcbPtr = ChildPcbPtr;

    PcbPtr->Info.CompactUd.CmpctSxnCnt++;
    if (PcbPtr->Info.CompactUd.CmpctSxnCnt < USER_SXNS_PER_PAGE)
    {
        PcbPtr->Info.DmxRead.BuffAddr -= USER_SXN_SIZE;
        PcbPtr->Info.CompactUd.CompactPba += SEGMENTS_PER_USER_SXN;
        compact_ud_prcs_get_cmpct_sxn(PcbPtr);
    }
    else
    {
        compact_ud_write_userdata(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_ud_write_userdata
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_ud_write_userdata (PCB_STRUCT *PcbPtr)
{
    PBA_INT Pba;

    if (SCHED_LOW_PCB_CNT())
    {
        PcbPtr->Fn = compact_ud_write_userdata;
        SCHED_POST_PCB(PcbPtr);
        return;
    }

    Pba = freelist_ud_cmpct_get_page(PcbPtr->Word.FbxPtr);
    if (Pba == INVALID_MASK)
    {
        PcbPtr->Fn = compact_ud_write_userdata;
        SCHED_POST_PCB(PcbPtr);
        return;
    }

    // Fill up dmx info for user write
    PcbPtr->Info.DmxWrite.Pba = Pba;
    PcbPtr->Info.DmxWrite.SegCnt = SEGMENTS_PER_PAGE;
    PcbPtr->Fn = compact_ud_write_userdata_done;

    #if defined(UD_VERIFY)
    dmx_ops_write_n_read(PcbPtr);
    #else
    dmx_ops_write(PcbPtr);
    #endif

    return;
}


//-----------------------------------------------------------------------------
// Function    : compact_ud_write_userdata_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void compact_ud_write_userdata_done (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT Pba;
    PCB_STRUCT *CurPcbPtr;
    unsigned long LaneIdx;
    unsigned long RclmThrshold;

    FbxPtr = PcbPtr->Word.FbxPtr;
    Pba = PcbPtr->Info.DmxWrite.Pba;

    // Check if flushing failed
    if (PcbPtr->Info.DmxWrite.Stat != SUCCESSFUL)
    {
        if (PcbPtr->Info.DmxWrite.Stat != DMX_OPS_CMD_RETRY)
        {
            // Set Quarantine Bit
            blkrecord_ud_set_q_blk(FbxPtr,
                                   Pba);
        }

        // Remove from freelist
        freelist_ud_chk_n_remove_cur_pba(FbxPtr,
                                         Pba);

        // Get new page again
        compact_ud_write_userdata(PcbPtr);

        return;
    }

    CurPcbPtr = PcbPtr->Info.CompactUd.NxtPcbPtr;

    // Loop page pba based on number of sxn
    do
    {
        // Save new user pba
        CurPcbPtr->Info.UserWrite.Pba = Pba;
        Pba += SEGMENTS_PER_USER_SXN;

        cntldata_write_alloc_si0(CurPcbPtr);

        CurPcbPtr = CurPcbPtr->Info.UserWrite.NxtPcbPtr;

    } while (CurPcbPtr != BIT_NULL_PTR);

    // Check freelist level
    LaneIdx = PcbPtr->Info.CompactUd.LaneIdx;

    RclmThrshold = RECLAIM_COMPACT_THRESHOLD_0;
    if (BIOS_IDLE_DRV())
    {
        RclmThrshold = RECLAIM_COMPACT_THRESHOLD_1;
    }

    if (RECLAIM_UD_CNT(FbxPtr->ReclaimUd, LaneIdx) <= RclmThrshold)
    {
        // Reset compact process parent info
        PcbPtr->Info.DmxRead.BuffAddr
            = FbxPtr->CompactUd.LanePtr[LaneIdx].BuffAddr
            + (USER_SXN_SIZE * (USER_SXNS_PER_PAGE - 1));

        PcbPtr->Info.CompactUd.CmpctSxnCnt = 0;
        PcbPtr->Info.CompactUd.NxtPcbPtr = BIT_NULL_PTR;
        PcbPtr->Info.CompactUd.CompactPba += SEGMENTS_PER_USER_SXN;

        PcbPtr->Fn = compact_ud_prcs_get_cmpct_sxn;
        SCHED_POST_PCB(PcbPtr);
    }
    else
    {
        // Turn off compact process flag
        PcbPtr->Info.CompactUd.ActiveFlag = OFF;
    }

    return;
}


//=============================================================================
// $Log: Compact.c,v $
// Revision 1.10  2014/05/19 04:48:58  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.9  2014/05/13 13:26:29  rcantong
// 1. BUGFIX: Compacting of already invalid sxn
// 1.1 Added re-checking of sxn valid during compacting - JParairo
//
// Revision 1.8  2014/04/30 15:40:09  rcantong
// 1. DEV: Prioritized flushing of compacting CD
// 1.1 Changed CD set dirty for compact process - MFenol
//
// Revision 1.7  2014/03/03 12:30:50  rcantong
// 1. BUGFIX: Handles unsync Dir and SI due to ungraceful shutdown
// 1.1 Removed compact_ud_verify_usersxn - MFenol
// 1.2 Skip compact when compact pba is not sync to dir - MFenol
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
