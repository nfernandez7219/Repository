//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Reclaim/Reclaim.c,v $
// $Revision: 1.8 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:59 $
// $Id: Reclaim.c,v 1.8 2014/05/19 04:48:59 rcantong Exp $
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

#include "BlkInfo.h"
#include "BlkRecord.h"
#include "Compact.h"
#include "Defects.h"
#include "Disturb.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "Err.h"
#include "FreeList.h"
#include "Reclaim.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "ReclaimI.h"
#include "BlkRecordI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
RECLAIM_LANE_STRUCT ReclaimCdLane[FBX_CNT][LANE_CNT];
L1CACHE_ALIGN(ReclaimCdLane);

RECLAIM_LANE_STRUCT ReclaimUdLane[FBX_CNT][LANE_CNT];
L1CACHE_ALIGN(ReclaimUdLane);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : reclaim_cd_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_cd_init_malloc (unsigned long FbxIdx)
{
    DM_FBX_STRUCT *FbxPtr;
    RECLAIM_LANE_STRUCT *LanePtr;
    unsigned long LaneIdx;

    FbxPtr = &DmFbx[FbxIdx];

    FbxPtr->ReclaimCd.ReclaimFlag = OFF;
    LanePtr = &ReclaimCdLane[FbxIdx][0];
    FbxPtr->ReclaimCd.LanePtr = LanePtr;

    util_init_pattern(LanePtr,
                      sizeof(ReclaimCdLane[0]),
                      INIT_PATTERN_LO_VALUE);

    for (LaneIdx = 0;
         LaneIdx < LANE_CNT;
         LaneIdx++)
    {
        LanePtr->PcbStruct.Word.FbxPtr = FbxPtr;
        LanePtr->PcbStruct.Info.Rclm.LaneIdx =  LaneIdx;
        LanePtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_cd_init_reclaim_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_cd_init_reclaim_fbx (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;
    PCB_STRUCT *ChildPcbPtr;

    FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];
    PcbPtr->Info.DmInit.DeployCtr = LANE_CNT;

    // Deploy child pcbs
    for (LaneIdx = 0;
         LaneIdx < LANE_CNT;
         LaneIdx++)
    {
        ChildPcbPtr = _sched_get_pcb();
        ChildPcbPtr->Fn = reclaim_cd_init_reclaim_lane;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;
        ChildPcbPtr->Info.Rclm.LaneIdx = LaneIdx;
        SCHED_POST_PCB(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_cd_trigger_reclaim_lane
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_cd_trigger_reclaim_lane (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx)
{
    PCB_STRUCT *PcbPtr;
    RECLAIM_FBX_STRUCT *RclmPtr;
    unsigned long RclmFlag;
    unsigned long LaneMask;

    RclmPtr = &FbxPtr->ReclaimCd;

    // Determine what lane will commence reclaim
    // and set its reclaim status if we haven't done so
    RclmFlag = RclmPtr->ReclaimFlag;
    LaneMask = 1 << LaneIdx;

    // Check if reclaim for this lane is currently active
    if ((RclmFlag & LaneMask) != 0)
    {
        return;
    }

    // Trigger Reclaim process on the specific lane now
    PcbPtr = &RclmPtr->LanePtr[LaneIdx].PcbStruct;
    PcbPtr->Fn = reclaim_cd_reclaim_lane;
    SCHED_POST_PCB(PcbPtr);

    RclmPtr->ReclaimFlag =  RclmFlag | LaneMask;

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_ud_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_ud_init_malloc (unsigned long FbxIdx)
{
    DM_FBX_STRUCT *FbxPtr;
    RECLAIM_LANE_STRUCT *LanePtr;
    unsigned long LaneIdx;

    FbxPtr = &DmFbx[FbxIdx];

    FbxPtr->ReclaimUd.ReclaimFlag = OFF;
    LanePtr = &ReclaimUdLane[FbxIdx][0];
    FbxPtr->ReclaimUd.LanePtr = LanePtr;

    util_init_pattern(LanePtr,
                      sizeof(ReclaimUdLane[0]),
                      INIT_PATTERN_LO_VALUE);

    for (LaneIdx = 0;
         LaneIdx < LANE_CNT;
         LaneIdx++)
    {
        LanePtr->PcbStruct.Word.FbxPtr = FbxPtr;
        LanePtr->PcbStruct.Info.Rclm.LaneIdx =  LaneIdx;
        LanePtr->RclmBlkCnt = 0;
        LanePtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_ud_init_reclaim_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_ud_init_reclaim_fbx (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;
    PCB_STRUCT *ChildPcbPtr;

    FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];
    PcbPtr->Info.DmInit.DeployCtr = LANE_CNT;

    // Deploy child pcbs
    for (LaneIdx = 0;
         LaneIdx < LANE_CNT;
         LaneIdx++)
    {
        ChildPcbPtr = _sched_get_pcb();
        ChildPcbPtr->Fn = reclaim_ud_init_reclaim_lane;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;
        ChildPcbPtr->Info.Rclm.LaneIdx = LaneIdx;
        SCHED_POST_PCB(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_ud_init_trigger_reclaim
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_ud_init_trigger_reclaim (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;

    FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];

    for (LaneIdx = 0;
         LaneIdx < LANE_CNT;
         LaneIdx++)
    {
        if (RECLAIM_UD_CNT_LOW(FbxPtr->ReclaimUd, LaneIdx))
        {
            reclaim_ud_trigger_reclaim_lane(FbxPtr,
                                            LaneIdx);
        }
    }

    SCHED_POST_PCB(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_ud_trigger_reclaim_lane
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_ud_trigger_reclaim_lane (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx)
{
    PCB_STRUCT *PcbPtr;
    unsigned long RclmFlag;
    unsigned long LaneMask;

    ASSERT(LaneIdx < DEV_CNT);
    RclmFlag = FbxPtr->ReclaimUd.ReclaimFlag;
    LaneMask = 1 << LaneIdx;

    // Check if reclaim for this lane is currently active
    if ((RclmFlag & LaneMask) != 0)
    {
        return;
    }

    // Trigger Reclaim
    PcbPtr = &FbxPtr->ReclaimUd.LanePtr[LaneIdx].PcbStruct;
    PcbPtr->Fn = reclaim_ud_reclaim_lane;
    SCHED_POST_PCB(PcbPtr);

    FbxPtr->ReclaimUd.ReclaimFlag =  RclmFlag | LaneMask;

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_ud_wakeup_reclaim_lane
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_ud_wakeup_reclaim_lane (DM_FBX_STRUCT *FbxPtr,
                                     unsigned long LaneIdx)
{
    ASSERT((FbxPtr->ReclaimUd.ReclaimFlag & (1 << LaneIdx)) != 0);
    reclaim_ud_reclaim_lane(&FbxPtr->ReclaimUd.LanePtr[LaneIdx].PcbStruct);

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : reclaim_cd_init_reclaim_lane
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_cd_init_reclaim_lane (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;
    PBA_INT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    LaneIdx = PcbPtr->Info.Rclm.LaneIdx;

    Pba = blkrecord_cd_get_reclaim_blk(FbxPtr,
                                       LaneIdx);

    // Check if there are any blocks to be reclaimed
    if ((Pba & INVALID_MASK) == 0)
    {
        ASSERT(Pba >= SEGMENTS_PER_SYSBLK);
        ASSERT(blkrecord_ud_check_bad_blk(FbxPtr,Pba) == GOOD_BLK);

        PcbPtr->Fn = reclaim_cd_init_reclaim_lane_cb;
        PcbPtr->Info.DmxErase.Pba = Pba;
        dmx_ops_erase(PcbPtr);

    }
    else
    {
        // Initialize freelist parameters for this lane
        freelist_cd_init_lane(FbxPtr,
                              LaneIdx);

        dm_notify_completion(PcbPtr);
        _sched_return_pcb(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_cd_init_reclaim_lane_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_cd_init_reclaim_lane_cb (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;
    PBA_INT Pba;
    BIT_STAT ListLevel;

    FbxPtr = PcbPtr->Word.FbxPtr;
    Pba = PcbPtr->Info.DmxErase.Pba;

    // Check stat
    if (PcbPtr->Info.DmxErase.Stat == SUCCESSFUL)
    {
        // Increment EraseCnt of BlkInfo
        blkinfo_update_entry (FbxPtr,
                              Pba);

        freelist_cd_add_entry(FbxPtr,
                              Pba);
    }

    else
    {
        defects_add_defect_entry(FbxPtr,
                                 Pba,
                                 PcbPtr->Info.DmxErase.Stat);
    }

    blkrecord_cd_put_to_temp_q(FbxPtr,
                               Pba);

    LaneIdx = PcbPtr->Info.Rclm.LaneIdx;

    ListLevel = freelist_cd_check_level(FbxPtr,
                                        LaneIdx);

    // Check if have enough freelist
    if (ListLevel == FREELIST_LOW)
    {
        reclaim_cd_init_reclaim_lane(PcbPtr);
    }

    else
    {
        // Initialize freelist parameters for this lane
        freelist_cd_init_lane(FbxPtr,
                              LaneIdx);

        dm_notify_completion(PcbPtr);
        _sched_return_pcb(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_cd_reclaim_lane
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_cd_reclaim_lane (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;
    PBA_INT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    LaneIdx = PcbPtr->Info.Rclm.LaneIdx;

    while (1)
    {
        // Get a new block, preferrably fully invalid blocks
        Pba = blkrecord_cd_get_reclaim_blk(FbxPtr,
                                           LaneIdx);

        if (Pba == INVALID_MASK)
        {
            // Schedule pcb to be woken up on a reclaim block available event
            // and trigger compacting
            PcbPtr->Fn = reclaim_cd_reclaim_lane;
            BLKRECORD_CD_SLEEP_PCB(FbxPtr,
                                   PcbPtr,
                                   LaneIdx);

            compact_cd_trigger_cmpct_prcs(FbxPtr,
                                          LaneIdx);
            return;
        }

        if (blkrecord_cd_check_q_blk(FbxPtr, Pba) == GOOD_BLK)
        {
            break;
        }

        // temporary: add to defects
        defects_add_defect_entry(FbxPtr,
                                 Pba,
                                 DMX_OPS_ECC_CORRECTABLE);

        blkrecord_cd_put_to_temp_q(FbxPtr,
                                   Pba);
    }

    ASSERT(blkrecord_cd_check_bad_blk(FbxPtr,Pba) == GOOD_BLK);
    ASSERT(Pba >= SEGMENTS_PER_SYSBLK);

    // If we got a good block, we proceed to erase so we can give a
    // pre-erased block to the freelist lane that requested the reclaim
    PcbPtr->Fn = reclaim_cd_reclaim_lane_cb;
    PcbPtr->Info.DmxErase.Pba = Pba;
    dmx_ops_erase(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_cd_reclaim_lane_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_cd_reclaim_lane_cb (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT Pba;
    unsigned long LaneIdx;
    BIT_STAT ListLevel;

    FbxPtr = PcbPtr->Word.FbxPtr;
    Pba = PcbPtr->Info.DmxErase.Pba;

    // Check stat
    if (PcbPtr->Info.DmxErase.Stat == SUCCESSFUL)
    {
        // Increment EraseCnt of BlkInfo
        // blkinfo_update_entry (FbxPtr,
                              // Pba);

        freelist_cd_add_entry(FbxPtr,
                              Pba);
    }
    else
    {
        defects_add_defect_entry(FbxPtr,
                                 Pba,
                                 PcbPtr->Info.DmxErase.Stat);
    }

    blkrecord_cd_put_to_temp_q(FbxPtr,
                               Pba);

    LaneIdx = PcbPtr->Info.Rclm.LaneIdx;

    ListLevel = freelist_cd_check_level(FbxPtr,
                                        LaneIdx);

    // If we already have enough freelist, reflect that we have finished
    // reclaim for this lane
    if (ListLevel != FREELIST_LOW)
    {
        FbxPtr->ReclaimCd.ReclaimFlag &= ~(1 << LaneIdx);
    }
    else
    {
        // If we still haven't got enough freelist in this lane...
        reclaim_cd_reclaim_lane(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_ud_init_reclaim_lane
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_ud_init_reclaim_lane (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;
    PBA_INT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    LaneIdx = PcbPtr->Info.Rclm.LaneIdx;

    Pba = blkrecord_ud_get_reclaim_blk(FbxPtr,
                                       LaneIdx);

    // Check if there are any blocks to be reclaimed
    if ((Pba & INVALID_MASK) == 0)
    {
        ASSERT(Pba >= SEGMENTS_PER_SYSBLK);

        PcbPtr->Fn = reclaim_ud_init_reclaim_lane_cb;
        PcbPtr->Info.DmxErase.Pba = Pba;
        dmx_ops_erase(PcbPtr);
    }
    else
    {
        // Initialize freelist parameters for this lane
        freelist_ud_init_lane(FbxPtr,
                              LaneIdx);

        dm_notify_completion(PcbPtr);
        _sched_return_pcb(PcbPtr);
    }


    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_ud_reclaim_init_lane_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_ud_init_reclaim_lane_cb (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;
    PBA_INT Pba;
    BIT_STAT ListLevel;

    FbxPtr = PcbPtr->Word.FbxPtr;
    Pba = PcbPtr->Info.DmxErase.Pba;

    // Check stat
    if (PcbPtr->Info.DmxErase.Stat == SUCCESSFUL)
    {
        // Increment EraseCnt of BlkInfo
        blkinfo_update_entry (FbxPtr,
                              Pba);

        freelist_ud_add_entry(FbxPtr,
                              Pba);
    }

    else
    {
        defects_add_defect_entry(FbxPtr,
                                 Pba,
                                 PcbPtr->Info.DmxErase.Stat);
    }

    DISTURB_RESET_READ_CNT(FbxPtr, (Pba / SEGMENTS_PER_BLK));

    // move to freelist queue or quarantine queue
    blkrecord_ud_put_to_temp_q(FbxPtr,
                               Pba);

    LaneIdx = PcbPtr->Info.Rclm.LaneIdx;

    ListLevel = freelist_ud_check_level(FbxPtr,
                                        LaneIdx);

    // Check if have enough freelist
    if (ListLevel == FREELIST_LOW)
    {
        reclaim_ud_init_reclaim_lane(PcbPtr);
    }

    else
    {
        // Initialize freelist parameters for this lane
        freelist_ud_init_lane(FbxPtr,
                              LaneIdx);

        dm_notify_completion(PcbPtr);
        _sched_return_pcb(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_ud_reclaim_lane
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_ud_reclaim_lane (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;
    PBA_INT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    LaneIdx = PcbPtr->Info.Rclm.LaneIdx;
    ASSERT(LaneIdx < DEV_CNT);
    while (1)
    {
        // Get a new block, preferrably fully invalid blocks
        Pba = blkrecord_ud_get_reclaim_blk(FbxPtr,
                                           LaneIdx);

        if (RECLAIM_UD_CNT_LOW(FbxPtr->ReclaimUd, LaneIdx))
        {
            compact_ud_trigger_cmpct_prcs(FbxPtr,
                                          LaneIdx);
        }

        if (Pba == INVALID_MASK)
        {
            // Schedule pcb to be woken up on a reclaim block available event
            // and trigger compacting
            PcbPtr->Fn = reclaim_ud_reclaim_lane;
            BLKRECORD_UD_SLEEP_PCB(FbxPtr,
                                   PcbPtr,
                                   LaneIdx);

            return;
        }

        if (blkrecord_ud_check_q_blk(FbxPtr, Pba) == GOOD_BLK)
        {
            break;
        }

        // temporary: add to defects
        defects_add_defect_entry(FbxPtr,
                                 Pba,
                                 DMX_OPS_ECC_CORRECTABLE);

        // move to quarantine queue
        blkrecord_ud_put_to_temp_q(FbxPtr,
                                   Pba);
    }

    ASSERT(blkrecord_ud_check_bad_blk(FbxPtr,Pba) == GOOD_BLK);
    ASSERT(Pba >= SEGMENTS_PER_SYSBLK);

    // If we got a good block, we proceed to erase so we can give a 
    // pre-erased block to the freelist lane that requested the reclaim 
    PcbPtr->Fn = reclaim_ud_reclaim_lane_cb;
    PcbPtr->Info.DmxErase.Pba = Pba;
    dmx_ops_erase(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : reclaim_ud_reclaim_lane_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void reclaim_ud_reclaim_lane_cb (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT Pba;
    unsigned long LaneIdx;
    BIT_STAT ListLevel;

    FbxPtr = PcbPtr->Word.FbxPtr;
    Pba = PcbPtr->Info.DmxErase.Pba;

    // Check stat
    if (PcbPtr->Info.DmxErase.Stat == SUCCESSFUL)
    {
        // Increment EraseCnt of BlkInfo
        blkinfo_update_entry (FbxPtr,
                              Pba);

        freelist_ud_add_entry(FbxPtr,
                              Pba);
    }
    else
    {
        defects_add_defect_entry(FbxPtr,
                                 Pba,
                                 PcbPtr->Info.DmxErase.Stat);
    }

    DISTURB_RESET_READ_CNT(FbxPtr, (Pba / SEGMENTS_PER_BLK));

    // move to freelist queue or quarantine queue
    blkrecord_ud_put_to_temp_q(FbxPtr,
                               Pba);

    LaneIdx = PcbPtr->Info.Rclm.LaneIdx;

    ListLevel = freelist_ud_check_level(FbxPtr,
                                        LaneIdx);

    // If we already have enough freelist, reflect that we have finished
    // reclaim for this lane
    if (ListLevel != FREELIST_LOW)
    {
        FbxPtr->ReclaimUd.ReclaimFlag &= ~(1 << LaneIdx);
    }
    else
    {
        // If we still haven't got enough freelist in this lane...
        reclaim_ud_reclaim_lane(PcbPtr);
    }

    return;
}


//=============================================================================
// $Log: Reclaim.c,v $
// Revision 1.8  2014/05/19 04:48:59  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.7  2014/04/30 14:20:46  rcantong
// 1. DEV: Support counter for reclaimable blocks
// 1.1 Added API to monitor and count reclaim blocks - MFenol
// 2. BUGFIX: Lack of freelist during power on
// 2.1 Triggered reclaim at end of dm init - MFenol
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
// Revision 1.2  2013/08/08 16:44:24  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:17  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================