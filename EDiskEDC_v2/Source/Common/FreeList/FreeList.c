//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/FreeList/FreeList.c,v $
// $Revision: 1.8 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:59 $
// $Id: FreeList.c,v 1.8 2014/05/19 04:48:59 rcantong Exp $
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
#include "Defects.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "Err.h"
#include "FreeList.h"
#include "Reclaim.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "FreeListI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
FREELIST_ENTRY_STRUCT FreeListCdEntry[FBX_CNT][LANE_CNT][FREEBLKS_PER_CDLANE];
L1CACHE_ALIGN(FreeListCdEntry);

FREELIST_LANE_STRUCT FreeListCdLane[FBX_CNT][LANE_CNT];
L1CACHE_ALIGN(FreeListCdLane);

FREELIST_ENTRY_STRUCT FreeListUdEntry[FBX_CNT][LANE_CNT][FREEBLKS_PER_UDLANE];
L1CACHE_ALIGN(FreeListUdEntry);

FREELIST_LANE_STRUCT FreeListUdLane[FBX_CNT][LANE_CNT];
L1CACHE_ALIGN(FreeListUdLane);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : freelist_cd_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void freelist_cd_init_malloc (unsigned long FbxIdx)
{
    DM_FBX_STRUCT *FbxPtr;
    FREELIST_LANE_STRUCT *LanePtr;
    FREELIST_ENTRY_STRUCT *EntryPtr;
    unsigned long LaneIdx;

    // Setup fbx dccm
    FbxPtr = &DmFbx[FbxIdx];
    LanePtr = &FreeListCdLane[FbxIdx][0];
    EntryPtr = &FreeListCdEntry[FbxIdx][0][0];

    FbxPtr->FreeListCd.CurLaneIdx = 0;
    FbxPtr->FreeListCd.CmpctCurLaneIdx = 0;
    FbxPtr->FreeListCd.LanePtr = LanePtr;

    // Initialize lane members
    util_init_pattern(FreeListCdLane[FbxIdx],
                      sizeof(FreeListCdLane[0]),
                      INIT_PATTERN_LO_VALUE);

    // Initialize each entry base
    for (LaneIdx = 0;
         LaneIdx < LANE_CNT;
         LaneIdx++)
    {
        LanePtr->EntryPtr = EntryPtr;
        EntryPtr += FREEBLKS_PER_CDLANE;
        LanePtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : freelist_cd_init_lane
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void freelist_cd_init_lane (DM_FBX_STRUCT *FbxPtr,
                            unsigned long LaneIdx)
{
    FREELIST_LANE_STRUCT *LanePtr;

    LanePtr = &FbxPtr->FreeListCd.LanePtr[LaneIdx];

    // Get one new block
    LanePtr->CurPagePba = LanePtr->EntryPtr[0].BlkPba;
    LanePtr->CmpctCurPagePba = LanePtr->EntryPtr[1].BlkPba;
    LanePtr->FreeIdx = 2;
    ASSERT(LanePtr->RclmIdx > 0);
    ASSERT(LanePtr->RclmIdx != LanePtr->FreeIdx);

    return;
}


//-----------------------------------------------------------------------------
// Function    : freelist_cd_add_entry
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void freelist_cd_add_entry (DM_FBX_STRUCT *FbxPtr,
                            PBA_INT BlkPba)
{
    unsigned long LaneIdx;
    FREELIST_LANE_STRUCT *LanePtr;
    unsigned long RclmIdx;

    // Get lane where pba belong
    LaneIdx = GET_DEV_FROM_PBA(BlkPba);
    LanePtr = &FbxPtr->FreeListCd.LanePtr[LaneIdx];

    // Add new reclaimed pba
    RclmIdx = LanePtr->RclmIdx;
    LanePtr->EntryPtr[RclmIdx].BlkPba = BlkPba;

    // Increment reclaim idx
    LanePtr->RclmIdx = (RclmIdx + 1) % FREEBLKS_PER_CDLANE;
    ASSERT(LanePtr->RclmIdx != LanePtr->FreeIdx);

    // Increment counter
    ASSERT(LanePtr->EntryCnt < FREEBLKS_PER_CDLANE);
    LanePtr->EntryCnt++;
    FbxPtr->FreeListCd.TotEntryCnt++;

    return;
}


//-----------------------------------------------------------------------------
// Function    : freelist_cd_get_cmpct_page
// Description :
// Parameters  : FbxPtr
// Returns     : Pba
//-----------------------------------------------------------------------------
PBA_INT freelist_cd_get_cmpct_page (DM_FBX_STRUCT *FbxPtr)
{
    FREELIST_FBX_STRUCT *FreelistCdPtr;
    FREELIST_LANE_STRUCT *LanePtr;
    unsigned long LaneIdx;
    unsigned long AveEntryCnt;
    PBA_INT Pba;

    FreelistCdPtr = &FbxPtr->FreeListCd;

    LanePtr = FreelistCdPtr->LanePtr;
    LaneIdx = FreelistCdPtr->CmpctCurLaneIdx;
    AveEntryCnt = DIV_ROUND_UP(FreelistCdPtr->TotEntryCnt, LANE_CNT);

    // Search a lane with sufficient entries
    while (     (LanePtr[LaneIdx].EntryCnt < AveEntryCnt)
           ||   (LanePtr[LaneIdx].EntryCnt <= MIN_CD_CMPCT_ENTRY_CNT))
    {
        // Increment to next lane
        LaneIdx = (LaneIdx + 1) % LANE_CNT;
        if (LaneIdx == FreelistCdPtr->CmpctCurLaneIdx)
        {
            // All lanes are without sufficient entries
            return INVALID_MASK;
        }
    }

    // Get lane and pba
    LanePtr = &LanePtr[LaneIdx];

    Pba = LanePtr->CmpctCurPagePba;
    LanePtr->CmpctCurPagePba = Pba + SEGMENTS_PER_PAGE;

    // Check if we have given the last page of the block
    if ((LanePtr->CmpctCurPagePba % SEGMENTS_PER_BLK) == 0)
    {
        // Get new block entry for current lane
        freelist_cd_get_new_blk(FbxPtr,
                                LaneIdx,
                                &LanePtr->CmpctCurPagePba);
    }

    // Move to next fbx lane
    FreelistCdPtr->CmpctCurLaneIdx = (LaneIdx + 1) % LANE_CNT;

    return Pba;
}


//-----------------------------------------------------------------------------
// Function    : freelist_cd_get_page
// Description :
// Parameters  : FbxPtr
// Returns     : Pba
//-----------------------------------------------------------------------------
PBA_INT freelist_cd_get_page (DM_FBX_STRUCT *FbxPtr)
{
    FREELIST_FBX_STRUCT *FreelistCdPtr;
    FREELIST_LANE_STRUCT *LanePtr;
    unsigned long LaneIdx;
    PBA_INT Pba;

    FreelistCdPtr = &FbxPtr->FreeListCd;
    LanePtr = FreelistCdPtr->LanePtr;
    LaneIdx = FreelistCdPtr->CurLaneIdx;

    // Search a lane with sufficient entries
    while (LanePtr[LaneIdx].EntryCnt <= MIN_CD_ENTRY_CNT)
    {
        // Increment to next lane
        LaneIdx = (LaneIdx + 1) % LANE_CNT;
        if (LaneIdx == FreelistCdPtr->CurLaneIdx)
        {
            // All lanes are without sufficient entries
            return INVALID_MASK;
        }
    }

    // Get lane and pba
    LanePtr = &LanePtr[LaneIdx];

    Pba = LanePtr->CurPagePba;
    LanePtr->CurPagePba = Pba + SEGMENTS_PER_PAGE;

    // Check if we have given the last page of the block
    if ((LanePtr->CurPagePba % SEGMENTS_PER_BLK) == 0)
    {
        // Get new block entry for current lane
        freelist_cd_get_new_blk(FbxPtr,
                                LaneIdx,
                                &LanePtr->CurPagePba);
    }

    // Move to next fbx lane
    FreelistCdPtr->CurLaneIdx = (LaneIdx + 1) % LANE_CNT;

    return Pba;
}


//-----------------------------------------------------------------------------
// Function    : freelist_cd_check_level
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT freelist_cd_check_level (DM_FBX_STRUCT *FbxPtr,
                                  unsigned long LaneIdx)
{
    if (FbxPtr->FreeListCd.LanePtr[LaneIdx].EntryCnt >= FREELIST_CD_HI_THRES)
    {
        return FREELIST_HIGH;
    }

    return FREELIST_LOW;
}


//-----------------------------------------------------------------------------
// Function    : freelist_cd_compare_cur_pba
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT freelist_cd_compare_cur_pba (DM_FBX_STRUCT *FbxPtr,
                                      PBA_INT BlkPba)
{
    FREELIST_LANE_STRUCT *LanePtr;
    PBA_INT CurPba;
    unsigned long LaneIdx;

    LaneIdx = GET_DEV_FROM_PBA(BlkPba);

    // Get current active block that freelist is getting pages from
    LanePtr = &FbxPtr->FreeListCd.LanePtr[LaneIdx];

    // Convert to a block pba and see if they fall on the same block
    CurPba = DM_ALIGN_TO(SEGMENTS_PER_BLK, LanePtr->CurPagePba);

    if (BlkPba == CurPba)
    {
        return SAME;
    }

    // Convert to a block pba and see if they fall on the same block
    CurPba = DM_ALIGN_TO(SEGMENTS_PER_BLK, LanePtr->CmpctCurPagePba);

    if (BlkPba == CurPba)
    {
        return SAME;
    }

    return NOT_SAME;
}


//-----------------------------------------------------------------------------
// Function    : freelist_cd_chk_n_remove_cur_pba
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void freelist_cd_chk_n_remove_cur_pba (DM_FBX_STRUCT *FbxPtr,
                                       PBA_INT BlkPba)
{
    unsigned long LaneIdx;
    FREELIST_LANE_STRUCT *LanePtr;
    PBA_INT *CurPagePbaPtr;

    BlkPba = DM_ALIGN_TO(SEGMENTS_PER_BLK, BlkPba);
    LaneIdx = GET_DEV_FROM_PBA(BlkPba);

    // Get current active block that freelist is getting pages from
    LanePtr = &FbxPtr->FreeListCd.LanePtr[LaneIdx];

    if (BlkPba == DM_ALIGN_TO(SEGMENTS_PER_BLK, LanePtr->CurPagePba))
    {
        CurPagePbaPtr = &LanePtr->CurPagePba;
    }
    else if (BlkPba == DM_ALIGN_TO(SEGMENTS_PER_BLK, LanePtr->CmpctCurPagePba))
    {
        CurPagePbaPtr = &LanePtr->CmpctCurPagePba;
    }
    else
    {
        return;
    }

    freelist_cd_get_new_blk(FbxPtr,
                            LaneIdx,
                            CurPagePbaPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : freelist_ud_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void freelist_ud_init_malloc (unsigned long FbxIdx)
{
    DM_FBX_STRUCT *FbxPtr;
    FREELIST_LANE_STRUCT *LanePtr;
    FREELIST_ENTRY_STRUCT *EntryPtr;
    unsigned long LaneIdx;

    // Setup fbx dccm
    FbxPtr = &DmFbx[FbxIdx];
    LanePtr = &FreeListUdLane[FbxIdx][0];
    EntryPtr = &FreeListUdEntry[FbxIdx][0][0];

    FbxPtr->FreeListUd.CurLaneIdx = 0;
    FbxPtr->FreeListUd.CmpctCurLaneIdx = 0;
    FbxPtr->FreeListUd.LanePtr = LanePtr;

    // Initialize lane members
    util_init_pattern(FreeListUdLane[FbxIdx],
                      sizeof(FreeListUdLane[0]),
                      INIT_PATTERN_LO_VALUE);

    // Initialize each entry base
    for (LaneIdx = 0;
         LaneIdx < LANE_CNT;
         LaneIdx++)
    {
        LanePtr->EntryPtr = EntryPtr;
        EntryPtr += FREEBLKS_PER_UDLANE;
        LanePtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : freelist_ud_init_lane
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void freelist_ud_init_lane (DM_FBX_STRUCT *FbxPtr,
                            unsigned long LaneIdx)
{
    FREELIST_LANE_STRUCT *LanePtr;

    LanePtr = &FbxPtr->FreeListUd.LanePtr[LaneIdx];

    // Get one new block
    LanePtr->CurPagePba = LanePtr->EntryPtr[0].BlkPba;
    LanePtr->CmpctCurPagePba = LanePtr->EntryPtr[1].BlkPba;
    LanePtr->FreeIdx = 2;
    ASSERT(LanePtr->RclmIdx > 0);
    ASSERT(LanePtr->RclmIdx != LanePtr->FreeIdx);

    // Check if sufficient reclaim count for fast page programming
    LanePtr->FastPageFlag = OFF;
    if (RECLAIM_UD_CNT(FbxPtr->ReclaimUd, LaneIdx) >= RECLAIM_LPW_THRESHOLD_ON)
    {
        LanePtr->FastPageFlag = ON;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : freelist_ud_add_entry
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void freelist_ud_add_entry (DM_FBX_STRUCT *FbxPtr,
                            PBA_INT BlkPba)
{
    unsigned long LaneIdx;
    FREELIST_LANE_STRUCT *LanePtr;
    unsigned long RclmIdx;

    // Get lane where pba belong
    LaneIdx = GET_DEV_FROM_PBA(BlkPba);
    LanePtr = &FbxPtr->FreeListUd.LanePtr[LaneIdx];

    // Add new reclaimed pba
    RclmIdx = LanePtr->RclmIdx;
    LanePtr->EntryPtr[RclmIdx].BlkPba = BlkPba;

    // Increment reclaim idx
    LanePtr->RclmIdx = (RclmIdx + 1) % FREEBLKS_PER_UDLANE;
    ASSERT(LanePtr->RclmIdx != LanePtr->FreeIdx);

    // Increment counter
    ASSERT(LanePtr->EntryCnt < FREEBLKS_PER_UDLANE);
    LanePtr->EntryCnt++;
    FbxPtr->FreeListUd.TotEntryCnt++;

    return;
}


//-----------------------------------------------------------------------------
// Function    : freelist_ud_get_page
// Description :
// Parameters  : FbxPtr
// Returns     : Pba
//-----------------------------------------------------------------------------
PBA_INT freelist_ud_get_page (DM_FBX_STRUCT *FbxPtr)
{
    FREELIST_FBX_STRUCT *FreelistUdPtr;
    FREELIST_LANE_STRUCT *LanePtr;
    unsigned long LaneIdx;
    PBA_INT Pba;

    FreelistUdPtr = &FbxPtr->FreeListUd;

    LanePtr = FreelistUdPtr->LanePtr;
    LaneIdx = FreelistUdPtr->CurLaneIdx;

    // Search a lane with sufficient entries
    while (LanePtr[LaneIdx].EntryCnt <= MIN_UD_ENTRY_CNT)
    {
        // Increment to next lane
        LaneIdx = (LaneIdx + 1) % LANE_CNT;
        if (LaneIdx == FreelistUdPtr->CurLaneIdx)
        {
            // All lanes are without sufficient entries
            return INVALID_MASK;
        }
    }

    // Get lane and pba
    LanePtr = &LanePtr[LaneIdx];

    Pba = LanePtr->CurPagePba;

    if (LanePtr->FastPageFlag == ON)
    {
        Pba = Pba + SEGMENTS_PER_PAGE;
        LanePtr->CurPagePba = Pba + SEGMENTS_PER_PAGE;

        // Check if we have given the last fast page of the block
        if ((LanePtr->CurPagePba % SEGMENTS_PER_BLK) == LAST_FAST_PAGE)
        {
            // Get new block entry for current lane
            freelist_ud_get_new_blk(FbxPtr,
                                    LaneIdx,
                                    &LanePtr->CurPagePba);

            // Check if sufficient reclaim count for fast page programming
            if (RECLAIM_UD_CNT(FbxPtr->ReclaimUd, LaneIdx) <= RECLAIM_LPW_THRESHOLD_OFF)
            {
                LanePtr->FastPageFlag = OFF;
            }
        }
    }
    else
    {
        LanePtr->CurPagePba = Pba + SEGMENTS_PER_PAGE;

        // Check if we have given the last page of the block
        if ((LanePtr->CurPagePba % SEGMENTS_PER_BLK) == 0)
        {
            // Get new block entry for current lane
            freelist_ud_get_new_blk(FbxPtr,
                                    LaneIdx,
                                    &LanePtr->CurPagePba);

            if (RECLAIM_UD_CNT(FbxPtr->ReclaimUd, LaneIdx) >= RECLAIM_LPW_THRESHOLD_ON)
            {
                LanePtr->FastPageFlag = ON;
            }
        }
    }

    // Move to next fbx lane
    FreelistUdPtr->CurLaneIdx = (LaneIdx + 1) % LANE_CNT;

    return Pba;
}


//-----------------------------------------------------------------------------
// Function    : freelist_ud_cmpct_get_page
// Description :
// Parameters  : FbxPtr
// Returns     : Pba
//-----------------------------------------------------------------------------
PBA_INT freelist_ud_cmpct_get_page (DM_FBX_STRUCT *FbxPtr)
{
    FREELIST_FBX_STRUCT *FreelistUdPtr;
    FREELIST_LANE_STRUCT *LanePtr;
    unsigned long LaneIdx;
    unsigned long AveEntryCnt;
    PBA_INT Pba;

    FreelistUdPtr = &FbxPtr->FreeListUd;

    LanePtr = FreelistUdPtr->LanePtr;
    LaneIdx = FreelistUdPtr->CmpctCurLaneIdx;
    AveEntryCnt = DIV_ROUND_UP(FreelistUdPtr->TotEntryCnt, LANE_CNT);

    // Search a lane with sufficient entries
    while (   (LanePtr[LaneIdx].EntryCnt < AveEntryCnt)
           || (LanePtr[LaneIdx].EntryCnt <= MIN_UD_CMPCT_ENTRY_CNT))
    {
        // Increment to next lane
        LaneIdx = (LaneIdx + 1) % LANE_CNT;
        if (LaneIdx == FreelistUdPtr->CmpctCurLaneIdx)
        {
            // All lanes are without sufficient entries
            return INVALID_MASK;
        }
    }

    // Get lane and pba
    LanePtr = &LanePtr[LaneIdx];

    Pba = LanePtr->CmpctCurPagePba;
    LanePtr->CmpctCurPagePba = Pba + SEGMENTS_PER_PAGE;

    // Check if we have given the last page of the block
    if ((LanePtr->CmpctCurPagePba % SEGMENTS_PER_BLK) == 0)
    {
        // Get new block entry for current lane
        freelist_ud_get_new_blk(FbxPtr,
                                LaneIdx,
                                &LanePtr->CmpctCurPagePba);
    }

    // Move to next fbx lane
    FreelistUdPtr->CmpctCurLaneIdx = (LaneIdx + 1) % LANE_CNT;

    return Pba;
}


//-----------------------------------------------------------------------------
// Function    : freelist_ud_check_level
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT freelist_ud_check_level (DM_FBX_STRUCT *FbxPtr,
                                  unsigned long LaneIdx)
{
    if (FbxPtr->FreeListUd.LanePtr[LaneIdx].EntryCnt >= FREELIST_UD_HI_THRES)
    {
        return FREELIST_HIGH;
    }

    return FREELIST_LOW;
}


//-----------------------------------------------------------------------------
// Function    : freelist_ud_compare_cur_pba
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT freelist_ud_compare_cur_pba (DM_FBX_STRUCT *FbxPtr,
                                      PBA_INT BlkPba)
{
    FREELIST_LANE_STRUCT *LanePtr;
    PBA_INT CurPba;
    unsigned long LaneIdx;

    LaneIdx = GET_DEV_FROM_PBA(BlkPba);

    // Get current active block that freelist is getting pages from
    LanePtr = &FbxPtr->FreeListUd.LanePtr[LaneIdx];

    // Convert to a block pba and see if they fall on the same block
    CurPba = DM_ALIGN_TO(SEGMENTS_PER_BLK, LanePtr->CurPagePba);

    if (BlkPba == CurPba)
    {
        return SAME;
    }

    // Convert to a block pba and see if they fall on the same block
    CurPba = DM_ALIGN_TO(SEGMENTS_PER_BLK, LanePtr->CmpctCurPagePba);

    if (BlkPba == CurPba)
    {
        return SAME;
    }

    return NOT_SAME;
}


//-----------------------------------------------------------------------------
// Function    : freelist_ud_chk_n_remove_cur_pba
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void freelist_ud_chk_n_remove_cur_pba (DM_FBX_STRUCT *FbxPtr,
                                       PBA_INT BlkPba)
{
    unsigned long LaneIdx;
    FREELIST_LANE_STRUCT *LanePtr;
    PBA_INT *CurPagePbaPtr;

    BlkPba = DM_ALIGN_TO(SEGMENTS_PER_BLK, BlkPba);
    LaneIdx = GET_DEV_FROM_PBA(BlkPba);

    // Get current active block that freelist is getting pages from
    LanePtr = &FbxPtr->FreeListUd.LanePtr[LaneIdx];

    if (BlkPba == DM_ALIGN_TO(SEGMENTS_PER_BLK, LanePtr->CurPagePba))
    {
        CurPagePbaPtr = &LanePtr->CurPagePba;
    }
    else if (BlkPba == DM_ALIGN_TO(SEGMENTS_PER_BLK, LanePtr->CmpctCurPagePba))
    {
        CurPagePbaPtr = &LanePtr->CmpctCurPagePba;
    }
    else
    {
        return;
    }

    freelist_ud_get_new_blk(FbxPtr,
                            LaneIdx,
                            CurPagePbaPtr);

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : freelist_cd_get_new_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void freelist_cd_get_new_blk (DM_FBX_STRUCT *FbxPtr,
                              unsigned long LaneIdx,
                               PBA_INT *CurPagePbaPtr)
{
    FREELIST_LANE_STRUCT *LanePtr;
    unsigned long FreeIdx;

    // Move to the next block entry for the lane and setup cur page pba
    LanePtr = &FbxPtr->FreeListCd.LanePtr[LaneIdx];
    FreeIdx = LanePtr->FreeIdx;
    *CurPagePbaPtr = LanePtr->EntryPtr[FreeIdx].BlkPba;

    LanePtr->FreeIdx = (FreeIdx + 1) % FREEBLKS_PER_CDLANE;
    ASSERT(FreeIdx != LanePtr->RclmIdx);

    // Decrement counters
    ASSERT(LanePtr->EntryCnt > 0);
    LanePtr->EntryCnt--;
    FbxPtr->FreeListCd.TotEntryCnt--;

    if (LanePtr->EntryCnt <= FREELIST_CD_LO_THRES)
    {
        // Trigger reclaim
        reclaim_cd_trigger_reclaim_lane(FbxPtr,
                                        LaneIdx);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : freelist_ud_get_new_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void freelist_ud_get_new_blk (DM_FBX_STRUCT *FbxPtr,
                              unsigned long LaneIdx,
                              PBA_INT *CurPagePbaPtr)
{
    FREELIST_LANE_STRUCT *LanePtr;
    unsigned long FreeIdx;
    ASSERT(LaneIdx < DEV_CNT);
    // Move to the next block entry for the lane and setup cur page pba
    LanePtr = &FbxPtr->FreeListUd.LanePtr[LaneIdx];
    FreeIdx = LanePtr->FreeIdx;
    *CurPagePbaPtr = LanePtr->EntryPtr[FreeIdx].BlkPba;

    LanePtr->FreeIdx = (FreeIdx + 1) % FREEBLKS_PER_UDLANE;
    ASSERT(FreeIdx != LanePtr->RclmIdx);

    // Decrement counters
    ASSERT(LanePtr->EntryCnt > 0);
    LanePtr->EntryCnt--;
    FbxPtr->FreeListUd.TotEntryCnt--;

    if (LanePtr->EntryCnt <= FREELIST_UD_LO_THRES)
    {
        // Trigger reclaim
        reclaim_ud_trigger_reclaim_lane(FbxPtr,
                                        LaneIdx);
    }

    return;
}


//=============================================================================
// $Log: FreeList.c,v $
// Revision 1.8  2014/05/19 04:48:59  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.7  2014/04/30 14:01:15  rcantong
// 1. DEV: Support freelist for compact
// 1.1 Added freelist threshold for compact page - JAbad
//
// Revision 1.6  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:57  rcantong
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
// Revision 1.2  2013/08/08 16:44:24  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:17  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================