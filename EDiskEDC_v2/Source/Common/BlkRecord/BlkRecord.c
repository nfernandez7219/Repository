//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/BlkRecord/BlkRecord.c,v $
// $Revision: 1.10 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:58 $
// $Id: BlkRecord.c,v 1.10 2014/05/19 04:48:58 rcantong Exp $
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
#include "DmCommon.h"
#include "Dmx.h"
#include "Freelist.h"
#include "Reclaim.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "BlkRecordI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
BLKRECORD_CD_ENTRY_STRUCT BlkRecordCdEntry[FBX_CNT][CNTL_BLKS_PER_FBX];
L1CACHE_ALIGN(BlkRecordCdEntry);

BLKRECORD_CD_LANE_STRUCT BlkRecordCdLane[FBX_CNT][LANE_CNT];
L1CACHE_ALIGN(BlkRecordCdLane);

BLKRECORD_UD_ENTRY_STRUCT BlkRecordUdEntry[FBX_CNT][BLKS_PER_FBX];
L1CACHE_ALIGN(BlkRecordUdEntry);

BLKRECORD_UD_LANE_STRUCT BlkRecordUdLane[FBX_CNT][LANE_CNT];
L1CACHE_ALIGN(BlkRecordUdLane);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_cd_init_malloc (unsigned long FbxIdx)
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;
    BLKRECORD_CD_LANE_STRUCT *LanePtr;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;
    unsigned long QueIdx;
    unsigned long BlkIdx;
    unsigned long *ValidBmpPtr;
    unsigned long WordIdx;

    // Static malloc
    EntryPtr = &BlkRecordCdEntry[FbxIdx][0];
    LanePtr = &BlkRecordCdLane[FbxIdx][0];

    // Setup fbx dccm of cd blkrecord
    FbxPtr = &DmFbx[FbxIdx];
    FbxPtr->BlkRecordCd.EntryPtr = EntryPtr;
    FbxPtr->BlkRecordCd.LanePtr = LanePtr;

    // Initialize lanes
    for (LaneIdx = 0;
         LaneIdx < LANE_CNT;
         LaneIdx++)
    {
        for (QueIdx = 0;
             QueIdx < CD_QUE_CNT;
             QueIdx++)
        {
            util_dll_init(&LanePtr->Que[QueIdx].Ctrl);
        }

        LanePtr->WaitRclmPcbPtr = BIT_NULL_PTR;
        LanePtr++;
    }

    // Initialize cntl data entries then put to temp que
    LanePtr = FbxPtr->BlkRecordCd.LanePtr;
    for (BlkIdx = 0;
         BlkIdx < CNTL_BLKS_PER_FBX;
         BlkIdx++)
    {
        EntryPtr->ValidCnt = 0;
        EntryPtr->QueIdx = 0;
        EntryPtr->BlkStat = 0;
        EntryPtr->BlkPba = BlkIdx * SEGMENTS_PER_BLK;
        LaneIdx = BlkIdx % LANE_CNT;
        util_dll_insert_at_tail(&EntryPtr->Link,
                                &LanePtr[LaneIdx].Que[CD_TEMPQ].Ctrl);

        ValidBmpPtr = &EntryPtr->ValidBmp[0];
        for (WordIdx = 0;
             WordIdx < (CNTL_SXNS_PER_BLK / BITS_PER_WORD);
             WordIdx++)
        {
            *ValidBmpPtr = 0;
            ValidBmpPtr++;
        }

        EntryPtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_init_entries
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_cd_init_entries (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    BLKRECORD_CD_LANE_STRUCT *LanePtr;
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;
    unsigned long SysBlkIdx;
    unsigned long *ValidBmpPtr;
    unsigned long ValidCnt;
    unsigned long WordIdx;
    unsigned long ValidBmp;
    BLKRECORD_QUE_STRUCT *CdQuePtr;

    FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];
    LanePtr = FbxPtr->BlkRecordCd.LanePtr;
    EntryPtr = FbxPtr->BlkRecordCd.EntryPtr;

    for (SysBlkIdx = 0;
         SysBlkIdx < CNTL_BLKS_PER_FBX;
         SysBlkIdx++)
    {
        ValidBmpPtr = &EntryPtr->ValidBmp[0];
        ValidCnt = 0;

        for (WordIdx = 0;
             WordIdx < (CNTL_SXNS_PER_BLK / BITS_PER_WORD);
             WordIdx++)
        {
            ValidBmp = *ValidBmpPtr;
            ValidBmpPtr++;

            util_count_bits(ValidBmp,
                            ValidCnt);
        }

        EntryPtr->ValidCnt = ValidCnt;

        // Check for fully invalid
        if (ValidCnt == 0)
        {
            // Put to reclaim queue
            EntryPtr->QueIdx = CD_RCLMQ;
            if ((EntryPtr->BlkStat & BAD_MSK) == BAD_MSK)
            {
                EntryPtr->QueIdx = CD_TEMPQ;
            }

            CdQuePtr = &LanePtr[SysBlkIdx % LANE_CNT].Que[EntryPtr->QueIdx];

            util_dll_move_to_tail(&EntryPtr->Link,
                                  &CdQuePtr->Ctrl);
        }

        else
        {
            // Put to respective compact queue
            ASSERT(EntryPtr->QueIdx == (ValidCnt / CD_QUE_DIV));
        }

        EntryPtr++;
    }

    blkrecord_ud_init_entries(PcbPtr);

    SCHED_POST_PCB(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_set_valid_sxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_cd_set_valid_sxn (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba)
{
    unsigned long SysBlkIdx;
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;
    unsigned long BlkSxnIdx;
    unsigned long WordIdx;
    unsigned long BitIdx;
    unsigned long *ValidBmpPtr;
    unsigned long ValidCnt;
    unsigned long QueIdx;
    BLKRECORD_CD_LANE_STRUCT *LanePtr;

    SysBlkIdx = Pba / SEGMENTS_PER_BLK;
    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[SysBlkIdx];

    BlkSxnIdx = (Pba % SEGMENTS_PER_BLK) / SEGMENTS_PER_CNTL_SXN;
    WordIdx = BlkSxnIdx / BITS_PER_WORD;
    BitIdx = BlkSxnIdx % BITS_PER_WORD;
    ValidBmpPtr = &EntryPtr->ValidBmp[WordIdx];

    // Check if sxn already invalidated
    ASSERT((*ValidBmpPtr & (1 << BitIdx)) == 0);

    *ValidBmpPtr |= (1 << BitIdx);

    EntryPtr->ValidCnt++;
    ASSERT(EntryPtr->ValidCnt <= CNTL_SXNS_PER_BLK);

    ValidCnt = EntryPtr->ValidCnt;

    QueIdx = ValidCnt / CD_QUE_DIV;
    LanePtr = &FbxPtr->BlkRecordCd.LanePtr[SysBlkIdx % LANE_CNT];

    // Check if ready to be moved
    if (EntryPtr->QueIdx != QueIdx)
    {
        // Move to new QueIdx
        EntryPtr->QueIdx = QueIdx;
        util_dll_move_to_tail(&EntryPtr->Link,
                              &LanePtr->Que[QueIdx].Ctrl);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_set_invalid_sxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_cd_set_invalid_sxn (DM_FBX_STRUCT *FbxPtr,
                                   PBA_INT Pba)
{
    unsigned long SysBlkIdx;
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;
    unsigned long BlkSxnIdx;
    unsigned long WordIdx;
    unsigned long BitIdx;
    unsigned long ValidCnt;
    unsigned long QueIdx;
    unsigned long *ValidBmpPtr;
    BLKRECORD_CD_LANE_STRUCT *LanePtr;
    BLKRECORD_QUE_STRUCT *CdQuePtr;
    PCB_STRUCT *PcbPtr;

    SysBlkIdx = Pba / SEGMENTS_PER_BLK;
    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[SysBlkIdx];

    BlkSxnIdx = (Pba % SEGMENTS_PER_BLK) / SEGMENTS_PER_CNTL_SXN;
    WordIdx = BlkSxnIdx / BITS_PER_WORD;
    BitIdx = BlkSxnIdx % BITS_PER_WORD;
    ValidBmpPtr = &EntryPtr->ValidBmp[WordIdx];

    // Check if sxn already invalidated
    ASSERT((*ValidBmpPtr & (1 << BitIdx)) != 0);

    // Proceed to invalidation
    *ValidBmpPtr &= (~(1 << BitIdx));
    ASSERT(EntryPtr->ValidCnt > 0);
    EntryPtr->ValidCnt--;

    ValidCnt = EntryPtr->ValidCnt;
    LanePtr = &FbxPtr->BlkRecordCd.LanePtr[SysBlkIdx % LANE_CNT];

    // Check if already reclaimable
    if (ValidCnt == 0)
    {
        ASSERT(EntryPtr->QueIdx != CD_RCLMQ);
        ASSERT((EntryPtr->BlkStat & BAD_MSK) == 0);

        // In case the block is came from remap process need to unmask
        EntryPtr->BlkStat &= ~REMAP_MSK;

        // Put to reclaim queue
        EntryPtr->QueIdx = CD_RCLMQ;
        CdQuePtr = &LanePtr->Que[CD_RCLMQ];

        util_dll_move_to_tail(&EntryPtr->Link,
                              &CdQuePtr->Ctrl);

        PcbPtr = LanePtr->WaitRclmPcbPtr;

        // Check for pending Reclaim
        if (PcbPtr != BIT_NULL_PTR)
        {
            SCHED_POST_PCB(PcbPtr);
            LanePtr->WaitRclmPcbPtr = BIT_NULL_PTR;
        }
    }

    else
    {
        // Put to specific queue
        QueIdx = ValidCnt / CD_QUE_DIV;

        // Check if ready to be moved
        if (QueIdx != EntryPtr->QueIdx)
        {
            // Move to new QueIdx
            EntryPtr->QueIdx = QueIdx;
            CdQuePtr = &LanePtr->Que[QueIdx];

            util_dll_move_to_tail(&EntryPtr->Link,
                                  &CdQuePtr->Ctrl);
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_get_reclaim_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
PBA_INT blkrecord_cd_get_reclaim_blk (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx)
{
    BLKRECORD_CD_LANE_STRUCT *LanePtr;
    BLKRECORD_QUE_STRUCT *ReclaimQuePtr;
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;

    LanePtr = &FbxPtr->BlkRecordCd.LanePtr[LaneIdx];
    ReclaimQuePtr = &LanePtr->Que[CD_RCLMQ];

    while (1)
    {
        EntryPtr = util_dll_peek_head_entry(&ReclaimQuePtr->Ctrl);

        if (EntryPtr == BIT_NULL_PTR)
        {
            return INVALID_MASK;
        }

        if (freelist_cd_compare_cur_pba(FbxPtr, EntryPtr->BlkPba) == NOT_SAME)
        {
            break;
        }

        EntryPtr->QueIdx = CD_TEMPQ;
        util_dll_move_to_tail(&EntryPtr->Link,
                              &LanePtr->Que[CD_TEMPQ].Ctrl);
    }

    return EntryPtr->BlkPba;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_get_compact_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
PBA_INT blkrecord_cd_get_compact_blk (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx)
{
    BLKRECORD_CD_LANE_STRUCT *LanePtr;
    BLKRECORD_QUE_STRUCT *CompactQuePtr;
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;

    LanePtr = &FbxPtr->BlkRecordCd.LanePtr[LaneIdx];
    CompactQuePtr = &LanePtr->Que[0];

    // Search compact queue
    while (1)
    {
        EntryPtr = util_dll_peek_head_entry(&CompactQuePtr->Ctrl);
        if (EntryPtr != BIT_NULL_PTR)
        {
            break;
        }
        CompactQuePtr++;
    }

    if (EntryPtr->ValidCnt >= (CNTL_SXNS_PER_BLK - PAGES_PER_BLK))
    {
        return INVALID_MASK;
    }

    // Move to temp que
    EntryPtr->QueIdx = CD_TEMPQ;
    util_dll_move_to_tail(&EntryPtr->Link,
                          &LanePtr->Que[CD_TEMPQ].Ctrl);

    return EntryPtr->BlkPba;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_get_valid_sxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
PBA_INT blkrecord_cd_get_valid_sxn (DM_FBX_STRUCT *FbxPtr,
                                    PBA_INT StartPba)
{
    unsigned long SysBlkIdx;
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;
    unsigned long BlkSxnIdx;
    unsigned long WordIdx;
    unsigned long BitIdx;
    unsigned long *ValidBmpPtr;
    unsigned long ValidBmpWord;

    // Pba must belong to the flash regions assigned for control data
    ASSERT(StartPba < CNTL_SEGMENTS_PER_FBX);

    SysBlkIdx = StartPba / SEGMENTS_PER_BLK;
    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[SysBlkIdx];

    BlkSxnIdx = (StartPba % SEGMENTS_PER_BLK) / SEGMENTS_PER_CNTL_SXN;
    WordIdx = BlkSxnIdx / BITS_PER_WORD;
    BitIdx = BlkSxnIdx % BITS_PER_WORD;

    ValidBmpPtr = &EntryPtr->ValidBmp[WordIdx];
    ValidBmpWord = (*ValidBmpPtr >> BitIdx) << BitIdx;

    // Search word with valid bit
    while (ValidBmpWord == 0)
    {
        // Move to next word and check if still in bounds in the current block
        WordIdx++;
        if (WordIdx >= (CNTL_SXNS_PER_BLK / BITS_PER_WORD))
        {
            return INVALID_MASK;
        }

        ValidBmpPtr++;
        ValidBmpWord = *ValidBmpPtr;
    }

    util_clz(ValidBmpWord,
             BitIdx);

    return (   (StartPba & ~(SEGMENTS_PER_BLK - 1))
             + (WordIdx * BITS_PER_WORD * SEGMENTS_PER_CNTL_SXN)
             + (BitIdx * SEGMENTS_PER_CNTL_SXN));
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_set_q_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_cd_set_q_blk (DM_FBX_STRUCT *FbxPtr,
                             PBA_INT Pba)
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;

    // Pba must belong to the flash regions assigned for control data
    ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    EntryPtr->BlkStat |= QUARANTINE_MSK;

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_unset_q_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_cd_unset_q_blk (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba)
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;

    // Pba must belong to the flash regions assigned for control data
    ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    EntryPtr->BlkStat &= ~QUARANTINE_MSK;

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_check_q_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long blkrecord_cd_check_q_blk (DM_FBX_STRUCT *FbxPtr,
                                        PBA_INT Pba)
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;

    // Pba must belong to the flash regions assigned for control data
    ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[Pba / SEGMENTS_PER_BLK];

    if ((EntryPtr->BlkStat & QUARANTINE_MSK) == QUARANTINE_MSK)
    {
        return QUARANTINE_BLK;
    }
    else
    {
        return GOOD_BLK;
    }
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_set_bad_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_cd_set_bad_blk (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba)
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;

    // Pba must belong to the flash regions assigned for control data
    ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    EntryPtr->BlkStat |= BAD_MSK;

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_check_bad_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT blkrecord_cd_check_bad_blk (DM_FBX_STRUCT *FbxPtr,
                                     PBA_INT Pba)
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;

    // Pba must belong to the flash regions assigned for control data
    ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[Pba / SEGMENTS_PER_BLK];

    if ((EntryPtr->BlkStat & BAD_MSK) == BAD_MSK)
    {
        return BAD_BLK;
    }
    else
    {
        return GOOD_BLK;
    }
}



//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_set_remap_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_cd_set_remap_blk (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba)
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;

    // Pba must belong to the flash regions assigned for control data
    ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    EntryPtr->BlkStat |= REMAP_MSK;

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_unset_remap_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_cd_unset_remap_blk (DM_FBX_STRUCT *FbxPtr,
                                   PBA_INT Pba)
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;

    // Pba must belong to the flash regions assigned for control data
    ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    EntryPtr->BlkStat &= ~REMAP_MSK;

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_check_remap_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT blkrecord_cd_check_remap_blk (DM_FBX_STRUCT *FbxPtr,
                                       PBA_INT Pba)
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;

    // Pba must belong to the flash regions assigned for control data
    ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[Pba / SEGMENTS_PER_BLK];

    if ((EntryPtr->BlkStat & REMAP_MSK) == REMAP_MSK)
    {
        return REMAP_BLK;
    }
    else
    {
        return GOOD_BLK;
    }
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_put_to_temp_q
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_cd_put_to_temp_q (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba)
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;
    BLKRECORD_CD_LANE_STRUCT *LanePtr;

    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    LanePtr = &FbxPtr->BlkRecordCd.LanePtr[GET_DEV_FROM_PBA(Pba)];

    // Move to temp que
    EntryPtr->QueIdx = CD_TEMPQ;
    util_dll_move_to_tail(&EntryPtr->Link,
                          &LanePtr->Que[CD_TEMPQ].Ctrl);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_cd_move_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_cd_move_blk (DM_FBX_STRUCT *FbxPtr,
                            PBA_INT Pba)
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;
    unsigned long LaneIdx;
    BLKRECORD_CD_LANE_STRUCT *LanePtr;
    unsigned long ValidCnt;
    BLKRECORD_QUE_STRUCT *CdQuePtr;
    unsigned long QueIdx;
    PCB_STRUCT *PcbPtr;

    EntryPtr = &FbxPtr->BlkRecordCd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    LaneIdx = GET_DEV_FROM_PBA(Pba);
    LanePtr = &FbxPtr->BlkRecordCd.LanePtr[LaneIdx];

    ValidCnt = EntryPtr->ValidCnt;

    if (ValidCnt == 0)
    {
        ASSERT((EntryPtr->BlkStat & BAD_MSK) == 0);

        // In case the block is came from remap process need to unmask
        EntryPtr->BlkStat &= ~REMAP_MSK;

        // Put to reclaim queue
        EntryPtr->QueIdx = CD_RCLMQ;
        CdQuePtr = &LanePtr->Que[CD_RCLMQ];

        util_dll_move_to_tail(&EntryPtr->Link,
                              &CdQuePtr->Ctrl);

        PcbPtr = LanePtr->WaitRclmPcbPtr;

        // Check for pending Reclaim
        if (PcbPtr != BIT_NULL_PTR)
        {
            SCHED_POST_PCB(PcbPtr);
            LanePtr->WaitRclmPcbPtr = BIT_NULL_PTR;
        }
    }
    else
    {
        QueIdx = ValidCnt / UD_QUE_DIV;

        // Check if ready to be moved
        if (EntryPtr->QueIdx != QueIdx)
        {
            // Move to new QueIdx
            EntryPtr->QueIdx = QueIdx;
            util_dll_move_to_tail(&EntryPtr->Link,
                                  &LanePtr->Que[QueIdx].Ctrl);
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_ud_init_malloc (unsigned long FbxIdx)
{
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;
    BLKRECORD_UD_LANE_STRUCT *LanePtr;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long LaneIdx;
    unsigned long QueIdx;
    unsigned long BlkIdx;
    unsigned long *ValidBmpPtr;
    unsigned long WordIdx;

    // Static malloc
    EntryPtr = &BlkRecordUdEntry[FbxIdx][0];
    LanePtr = &BlkRecordUdLane[FbxIdx][0];

    // Setup fbx dccm of ud blkrecord
    FbxPtr = &DmFbx[FbxIdx];
    FbxPtr->BlkRecordUd.EntryPtr = EntryPtr;
    FbxPtr->BlkRecordUd.LanePtr = LanePtr;

    // Initialize list
    util_dll_init(&FbxPtr->BlkRecordUd.RemapList);

    // Initialize lanes
    for (LaneIdx = 0;
         LaneIdx < LANE_CNT;
         LaneIdx++)
    {
        for (QueIdx = 0;
             QueIdx < UD_QUE_CNT;
             QueIdx++)
        {
            util_dll_init(&LanePtr->Que[QueIdx].Ctrl);
        }

        LanePtr->WaitRclmPcbPtr = BIT_NULL_PTR;
        LanePtr++;
    }

    // Initialize user data entries then put to temp que
    LanePtr = FbxPtr->BlkRecordUd.LanePtr;
    for (BlkIdx = 0;
         BlkIdx < BLKS_PER_FBX;
         BlkIdx++)
    {
        EntryPtr->ValidCnt = 0;
        EntryPtr->QueIdx = 0;
        EntryPtr->BlkStat = 0;
        EntryPtr->BlkPba = BlkIdx * SEGMENTS_PER_BLK;
        LaneIdx = BlkIdx % LANE_CNT;
        util_dll_insert_at_tail(&EntryPtr->Link,
                                &LanePtr[LaneIdx].Que[UD_TEMPQ].Ctrl);

        ValidBmpPtr = &EntryPtr->ValidBmp[0];
        for (WordIdx = 0;
             WordIdx < (USER_SXNS_PER_BLK / BITS_PER_WORD);
             WordIdx++)
        {
            *ValidBmpPtr = 0;
            ValidBmpPtr++;
        }

        EntryPtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_init_entries
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_ud_init_entries (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    BLKRECORD_UD_LANE_STRUCT *LanePtr;
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;
    unsigned long SysBlkIdx;
    unsigned long *ValidBmpPtr;
    unsigned long ValidCnt;
    unsigned long WordIdx;
    unsigned long ValidBmp;
    unsigned long LaneIdx;
    BLKRECORD_QUE_STRUCT *UdQuePtr;

    FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];
    LanePtr = FbxPtr->BlkRecordUd.LanePtr;
    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[CNTL_BLKS_PER_FBX];

    for (SysBlkIdx = CNTL_BLKS_PER_FBX;
         SysBlkIdx < BLKS_PER_FBX;
         SysBlkIdx++)
    {
        ValidBmpPtr = &EntryPtr->ValidBmp[0];
        ValidCnt = 0;

        for (WordIdx = 0;
             WordIdx < (USER_SXNS_PER_BLK / BITS_PER_WORD);
             WordIdx++)
        {
            ValidBmp = *ValidBmpPtr;
            ValidBmpPtr++;

            util_count_bits(ValidBmp,
                            ValidCnt);
        }

        EntryPtr->ValidCnt = ValidCnt;
        LaneIdx = SysBlkIdx % LANE_CNT;

        // Check for fully invalid
        if (ValidCnt == 0)
        {
            if ((EntryPtr->BlkStat & BAD_MSK) == BAD_MSK)
            {
                EntryPtr->QueIdx = UD_TEMPQ;
            }
            else
            {
                // Put to reclaim queue
                EntryPtr->QueIdx = UD_RCLMQ;
                RECLAIM_UD_INC_RCLM_BLK(FbxPtr->ReclaimUd, LaneIdx);
            }

            ASSERT(EntryPtr->QueIdx != UD_RMPQ);
            UdQuePtr = &LanePtr[LaneIdx].Que[EntryPtr->QueIdx];

            util_dll_move_to_tail(&EntryPtr->Link,
                                  &UdQuePtr->Ctrl);
        }

        else
        {
            // Put to respective compact queue
            ASSERT(EntryPtr->QueIdx == (ValidCnt / UD_QUE_DIV));
        }

        EntryPtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_set_valid_sxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_ud_set_valid_sxn (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba)
{
    unsigned long SysBlkIdx;
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;
    unsigned long BlkSxnIdx;
    unsigned long WordIdx;
    unsigned long BitIdx;
    unsigned long *ValidBmpPtr;
    unsigned long ValidCnt;
    unsigned long QueIdx;
    unsigned long LaneIdx;
    BLKRECORD_UD_LANE_STRUCT *LanePtr;

    SysBlkIdx = Pba / SEGMENTS_PER_BLK;
    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[SysBlkIdx];

    BlkSxnIdx = (Pba % SEGMENTS_PER_BLK) / SEGMENTS_PER_USER_SXN;
    WordIdx = BlkSxnIdx / BITS_PER_WORD;
    BitIdx = BlkSxnIdx % BITS_PER_WORD;
    ValidBmpPtr = &EntryPtr->ValidBmp[WordIdx];

    ASSERT((*ValidBmpPtr & (1 << BitIdx)) == 0);

    *ValidBmpPtr |= (1 << BitIdx);
    EntryPtr->ValidCnt++;
    ASSERT(EntryPtr->ValidCnt <= USER_SXNS_PER_BLK);

    ValidCnt = EntryPtr->ValidCnt;

    QueIdx = ValidCnt / UD_QUE_DIV;
    LaneIdx = SysBlkIdx % LANE_CNT;
    LanePtr = &FbxPtr->BlkRecordUd.LanePtr[LaneIdx];

    ASSERT(EntryPtr->QueIdx != UD_RMPQ);
    // Check if ready to be moved
    if (EntryPtr->QueIdx != QueIdx)
    {
        if(EntryPtr->QueIdx == UD_RCLMQ)
        {
            RECLAIM_UD_DEC_RCLM_BLK(FbxPtr->ReclaimUd, LaneIdx);
        }
        // Move to new QueIdx
        EntryPtr->QueIdx = QueIdx;
        util_dll_move_to_tail(&EntryPtr->Link,
                              &LanePtr->Que[QueIdx].Ctrl);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_chk_valid_sxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT blkrecord_ud_chk_valid_sxn (DM_FBX_STRUCT *FbxPtr,
                                     PBA_INT Pba)
{
    unsigned long SysBlkIdx;
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;
    unsigned long BlkSxnIdx;
    unsigned long WordIdx;
    unsigned long BitIdx;
    unsigned long *ValidBmpPtr;

    SysBlkIdx = Pba / SEGMENTS_PER_BLK;
    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[SysBlkIdx];

    BlkSxnIdx = (Pba % SEGMENTS_PER_BLK) / SEGMENTS_PER_USER_SXN;
    WordIdx = BlkSxnIdx / BITS_PER_WORD;
    BitIdx = BlkSxnIdx % BITS_PER_WORD;
    ValidBmpPtr = &EntryPtr->ValidBmp[WordIdx];

    return ((*ValidBmpPtr >> BitIdx) & 1);
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_set_invalid_sxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_ud_set_invalid_sxn (DM_FBX_STRUCT *FbxPtr,
                                   PBA_INT Pba)
{
    unsigned long SysBlkIdx;
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;
    unsigned long BlkSxnIdx;
    unsigned long WordIdx;
    unsigned long BitIdx;
    unsigned long ValidCnt;
    unsigned long QueIdx;
    unsigned long *ValidBmpPtr;
    unsigned long LaneIdx;
    BLKRECORD_UD_LANE_STRUCT *LanePtr;
    BLKRECORD_QUE_STRUCT *UdQuePtr;
    PCB_STRUCT *PcbPtr;

    SysBlkIdx = Pba / SEGMENTS_PER_BLK;
    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[SysBlkIdx];

    BlkSxnIdx = (Pba % SEGMENTS_PER_BLK) / SEGMENTS_PER_USER_SXN;
    WordIdx = BlkSxnIdx / BITS_PER_WORD;
    BitIdx = BlkSxnIdx % BITS_PER_WORD;
    ValidBmpPtr = &EntryPtr->ValidBmp[WordIdx];

    // Check if sxn already invalidated
    ASSERT((*ValidBmpPtr & (1 << BitIdx)) != 0);

    // Proceed to invalidation
    *ValidBmpPtr &= (~(1 << BitIdx));
    ASSERT(EntryPtr->ValidCnt > 0);
    EntryPtr->ValidCnt--;

    // Check ValidCnt
    ValidCnt = EntryPtr->ValidCnt;
    LaneIdx = SysBlkIdx % LANE_CNT;
    LanePtr = &FbxPtr->BlkRecordUd.LanePtr[LaneIdx];

    // Check if already reclaimable
    if (ValidCnt == 0)
    {
        ASSERT(EntryPtr->QueIdx != UD_RCLMQ);
        ASSERT((EntryPtr->BlkStat & BAD_MSK) == 0);

        // In case the block is came from remap process need to unmask
        EntryPtr->BlkStat &= ~REMAP_MSK;

        // Put to reclaim queue
        EntryPtr->QueIdx = UD_RCLMQ;
        UdQuePtr = &LanePtr->Que[UD_RCLMQ];

        RECLAIM_UD_INC_RCLM_BLK(FbxPtr->ReclaimUd, LaneIdx);
        util_dll_move_to_tail(&EntryPtr->Link,
                              &UdQuePtr->Ctrl);

        PcbPtr = LanePtr->WaitRclmPcbPtr;

        // Check for pending Reclaim
        if (PcbPtr != BIT_NULL_PTR)
        {
            LanePtr->WaitRclmPcbPtr = BIT_NULL_PTR;
            SCHED_POST_PCB(PcbPtr);
        }
    }

    else
    {
        // Put to specific queue
        QueIdx = ValidCnt / UD_QUE_DIV;

        // Check if ready to be moved
        if (    (EntryPtr->QueIdx != QueIdx)
            && (EntryPtr->QueIdx != UD_RMPQ))
        {
            ASSERT(EntryPtr->QueIdx != UD_RCLMQ);
            // Move to new QueIdx
            EntryPtr->QueIdx = QueIdx;
            UdQuePtr = &LanePtr->Que[QueIdx];

            util_dll_move_to_tail(&EntryPtr->Link,
                                  &UdQuePtr->Ctrl);
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_get_reclaim_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
PBA_INT blkrecord_ud_get_reclaim_blk (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx)
{
    BLKRECORD_UD_LANE_STRUCT *LanePtr;
    BLKRECORD_QUE_STRUCT *ReclaimQuePtr;
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;

    LanePtr = &FbxPtr->BlkRecordUd.LanePtr[LaneIdx];
    ReclaimQuePtr = &LanePtr->Que[UD_RCLMQ];

    while (1)
    {
        EntryPtr = util_dll_peek_head_entry(&ReclaimQuePtr->Ctrl);

        if (EntryPtr == BIT_NULL_PTR)
        {
            return INVALID_MASK;
        }

        RECLAIM_UD_DEC_RCLM_BLK(FbxPtr->ReclaimUd, LaneIdx);
        if (freelist_ud_compare_cur_pba(FbxPtr, EntryPtr->BlkPba) == NOT_SAME)
        {
            break;
        }

        EntryPtr->QueIdx = UD_TEMPQ;
        util_dll_move_to_tail(&EntryPtr->Link,
                              &LanePtr->Que[UD_TEMPQ].Ctrl);
    }

    return EntryPtr->BlkPba;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_get_compact_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
PBA_INT blkrecord_ud_get_compact_blk (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx)
{
    BLKRECORD_UD_LANE_STRUCT *LanePtr;
    BLKRECORD_QUE_STRUCT *CompactQuePtr;
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;

    LanePtr = &FbxPtr->BlkRecordUd.LanePtr[LaneIdx];
    CompactQuePtr = &LanePtr->Que[0];

    // Search compact queue
    while (1)
    {
        EntryPtr = util_dll_peek_head_entry(&CompactQuePtr->Ctrl);
        if (EntryPtr != BIT_NULL_PTR)
        {
            break;
        }
        CompactQuePtr++;
    }

    if (EntryPtr->ValidCnt >= USER_SXNS_PER_BLK)
    {
        return INVALID_MASK;
    }

    // Move to temp que
    EntryPtr->QueIdx = UD_TEMPQ;
    util_dll_move_to_tail(&EntryPtr->Link,
                          &LanePtr->Que[UD_TEMPQ].Ctrl);

    return EntryPtr->BlkPba;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_get_valid_sxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
PBA_INT blkrecord_ud_get_valid_sxn (DM_FBX_STRUCT *FbxPtr,
                                    PBA_INT StartPba)
{
    unsigned long SysBlkIdx;
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;
    unsigned long BlkSxnIdx;
    unsigned long WordIdx;
    unsigned long BitIdx;
    unsigned long *ValidBmpPtr;
    unsigned long ValidBmpWord;

    // Pba must belong to the flash regions assigned for user data
    ASSERT(StartPba < SEGMENTS_PER_FBX);

    SysBlkIdx = StartPba / SEGMENTS_PER_BLK;
    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[SysBlkIdx];

    BlkSxnIdx = (StartPba % SEGMENTS_PER_BLK) / SEGMENTS_PER_USER_SXN;
    WordIdx = BlkSxnIdx / BITS_PER_WORD;
    BitIdx = BlkSxnIdx % BITS_PER_WORD;

    ValidBmpPtr = &EntryPtr->ValidBmp[WordIdx];
    ValidBmpWord = (*ValidBmpPtr >> BitIdx) << BitIdx;

    // Search word with valid bit
    while (ValidBmpWord == 0)
    {
        // Move to next word and check if still in bounds in the current block
        WordIdx++;
        if (WordIdx >= (USER_SXNS_PER_BLK / BITS_PER_WORD))
        {
            return INVALID_MASK;
        }

        ValidBmpPtr++;
        ValidBmpWord = *ValidBmpPtr;
    }

    util_clz(ValidBmpWord,
             BitIdx);

    return (   (StartPba & ~(SEGMENTS_PER_BLK - 1))
             + (WordIdx * BITS_PER_WORD * SEGMENTS_PER_USER_SXN)
             + (BitIdx * SEGMENTS_PER_USER_SXN));
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_set_q_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_ud_set_q_blk (DM_FBX_STRUCT *FbxPtr,
                             PBA_INT Pba)
{
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;

    ASSERT(Pba < SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    EntryPtr->BlkStat |= QUARANTINE_MSK;

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_unset_q_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_ud_unset_q_blk (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba)
{
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;

    ASSERT(Pba < SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    EntryPtr->BlkStat &= ~QUARANTINE_MSK;

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_check_q_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long blkrecord_ud_check_q_blk (DM_FBX_STRUCT *FbxPtr,
                                        PBA_INT Pba)
{
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;

    ASSERT(Pba < SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[Pba / SEGMENTS_PER_BLK];

    if ((EntryPtr->BlkStat & QUARANTINE_MSK) == QUARANTINE_MSK)
    {
        return QUARANTINE_BLK;
    }
    else
    {
        return GOOD_BLK;
    }
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_set_bad_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_ud_set_bad_blk (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba)
{
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;

    ASSERT(Pba < SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    EntryPtr->BlkStat |= BAD_MSK;

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_check_bad_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT blkrecord_ud_check_bad_blk (DM_FBX_STRUCT *FbxPtr,
                                     PBA_INT Pba)
{
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;

    ASSERT(Pba < SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[Pba / SEGMENTS_PER_BLK];

    if ((EntryPtr->BlkStat & BAD_MSK) == BAD_MSK)
    {
        return BAD_BLK;
    }
    else
    {
        return GOOD_BLK;
    }
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_get_rmp_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
PBA_INT blkrecord_ud_get_rmp_blk (DM_FBX_STRUCT *FbxPtr)
{
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;
    unsigned long SysBlkIdx;
    BLKRECORD_UD_LANE_STRUCT *LanePtr;
    unsigned long ValidCnt;
    unsigned long QueIdx;
    BLKRECORD_QUE_STRUCT *UdQuePtr;

    EntryPtr = util_dll_peek_head_entry(&FbxPtr->BlkRecordUd.RemapList);

    if (EntryPtr == BIT_NULL_PTR)
    {
        return INVALID_MASK;
    }

    ASSERT((EntryPtr->BlkStat & REMAP_MSK) == REMAP_MSK);

    SysBlkIdx = EntryPtr->BlkPba / SEGMENTS_PER_BLK;
    LanePtr = &FbxPtr->BlkRecordUd.LanePtr[SysBlkIdx % LANE_CNT];

    // Check valid cnt
    // Put to specific queue
    ValidCnt = EntryPtr->ValidCnt;
    ASSERT(ValidCnt > 0)
    QueIdx = ValidCnt / UD_QUE_DIV;

    // Move to new QueIdx
    EntryPtr->QueIdx = QueIdx;
    UdQuePtr = &LanePtr->Que[QueIdx];

    util_dll_move_to_tail(&EntryPtr->Link,
                          &UdQuePtr->Ctrl);

    return EntryPtr->BlkPba;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_set_remap_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_ud_set_remap_blk (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba)
{
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;

    ASSERT(Pba < SEGMENTS_PER_FBX);

    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[Pba / SEGMENTS_PER_BLK];

    if ((EntryPtr->BlkStat & REMAP_MSK) == REMAP_MSK)
    {
        return;
    }

    EntryPtr->BlkStat |= REMAP_MSK;
    BLKRECORD_UD_PUT_TO_REMAP_Q(FbxPtr,EntryPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_put_to_temp_q
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_ud_put_to_temp_q (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba)
{
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;
    BLKRECORD_UD_LANE_STRUCT *LanePtr;

    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    LanePtr = &FbxPtr->BlkRecordUd.LanePtr[GET_DEV_FROM_PBA(Pba)];

    // Move to temp que
    EntryPtr->QueIdx = UD_TEMPQ;
    util_dll_move_to_tail(&EntryPtr->Link,
                          &LanePtr->Que[UD_TEMPQ].Ctrl);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkrecord_ud_move_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkrecord_ud_move_blk (DM_FBX_STRUCT *FbxPtr,
                            PBA_INT Pba)
{
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;
    unsigned long LaneIdx;
    BLKRECORD_UD_LANE_STRUCT *LanePtr;
    unsigned long ValidCnt;
    BLKRECORD_QUE_STRUCT *UdQuePtr;
    unsigned long QueIdx;
    PCB_STRUCT *PcbPtr;

    EntryPtr = &FbxPtr->BlkRecordUd.EntryPtr[Pba / SEGMENTS_PER_BLK];
    LaneIdx = GET_DEV_FROM_PBA(Pba);
    LanePtr = &FbxPtr->BlkRecordUd.LanePtr[LaneIdx];

    ValidCnt = EntryPtr->ValidCnt;

    if (ValidCnt == 0)
    {
        ASSERT((EntryPtr->BlkStat & BAD_MSK) == 0);

        // In case the block is came from remap process need to unmask
        EntryPtr->BlkStat &= ~REMAP_MSK;

        // Put to reclaim queue
        EntryPtr->QueIdx = UD_RCLMQ;
        UdQuePtr = &LanePtr->Que[UD_RCLMQ];

        RECLAIM_UD_INC_RCLM_BLK(FbxPtr->ReclaimUd, LaneIdx);
        util_dll_move_to_tail(&EntryPtr->Link,
                              &UdQuePtr->Ctrl);

        PcbPtr = LanePtr->WaitRclmPcbPtr;

        // Check for pending Reclaim
        if (PcbPtr != BIT_NULL_PTR)
        {
            LanePtr->WaitRclmPcbPtr = BIT_NULL_PTR;
            SCHED_POST_PCB(PcbPtr);
        }
    }
    else
    {
        QueIdx = ValidCnt / UD_QUE_DIV;

        // Check if ready to be moved
        if (EntryPtr->QueIdx != QueIdx)
        {
            if(EntryPtr->QueIdx == UD_RCLMQ)
            {
                RECLAIM_UD_DEC_RCLM_BLK(FbxPtr->ReclaimUd, LaneIdx);
            }
            // Move to new QueIdx
            EntryPtr->QueIdx = QueIdx;
            util_dll_move_to_tail(&EntryPtr->Link,
                                  &LanePtr->Que[QueIdx].Ctrl);
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: BlkRecord.c,v $
// Revision 1.10  2014/05/19 04:48:58  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.9  2014/05/13 13:26:29  rcantong
// 1. BUGFIX: Compacting of already invalid sxn
// 1.1 Added re-checking of sxn valid during compacting - JParairo
//
// Revision 1.8  2014/04/30 13:38:54  rcantong
// 1. DEV: Support compact queues limited to 64 only
// 1.1 Update BR_QUE_CNT to 64 - JParairo
// 1.2 Added QueIdx in BlkRecordUdEntryStruct - JParairo
// 2. DEV: Support user flash block remapping
// 2.1 Added UD_RMPQ - PPestano
// 2.2 Added API for user flash block remapping - PPestano
//
// Revision 1.7  2014/03/03 13:03:04  rcantong
// 1. BUGFIX: No compact blocks candidate available
// 1.1 Retun invalid if no compact blk candidate - MFenol
//
// Revision 1.6  2014/02/02 10:00:30  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.4  2013/12/05 13:06:33  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
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
