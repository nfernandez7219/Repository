//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/BlkRecord/BlkRecord.h,v $
// $Revision: 1.8 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:58 $
// $Id: BlkRecord.h,v 1.8 2014/05/19 04:48:58 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__BLKRECORD_H__)
#define __BLKRECORD_H__

#if defined(DEBUG)
_Inline void blkrecord_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define LANE_CNT                        (DEV_CNT)
#define GOOD_BLK                        0
#define REMAP_BLK                       1
#define QUARANTINE_BLK                  2
#define BAD_BLK                         3


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define BLKRECORD_CD_SLEEP_PCB(FbxPtr, PcbPtr, LaneIdx) \
    (FbxPtr->BlkRecordCd.LanePtr[LaneIdx].WaitRclmPcbPtr = PcbPtr)

#define BLKRECORD_UD_SLEEP_PCB(FbxPtr, PcbPtr, LaneIdx) \
    (FbxPtr->BlkRecordUd.LanePtr[LaneIdx].WaitRclmPcbPtr = PcbPtr)

#define BLKRECORD_UD_PUT_TO_REMAP_Q(FbxPtr,EntryPtr)       \
{                                                          \
    EntryPtr->QueIdx = UD_RMPQ;                            \
    util_dll_move_to_tail(&EntryPtr->Link,                 \
                          &FbxPtr->BlkRecordUd.RemapList); \
}


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

// Control data
void blkrecord_cd_init_malloc (unsigned long FbxIdx);

void blkrecord_cd_init_entries (PCB_STRUCT *PcbPtr);

void blkrecord_cd_set_valid_blk (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba);

void blkrecord_cd_set_valid_sxn (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba);

void blkrecord_cd_set_invalid_sxn (DM_FBX_STRUCT *FbxPtr,
                                   PBA_INT Pba);

void blkrecord_cd_set_invalid_page (DM_FBX_STRUCT *FbxPtr,
                                    PBA_INT Pba);

PBA_INT blkrecord_cd_get_reclaim_blk (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx);

PBA_INT blkrecord_cd_get_compact_blk (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx);

PBA_INT blkrecord_cd_get_valid_sxn (DM_FBX_STRUCT *FbxPtr,
                                    PBA_INT StartPba);

void blkrecord_cd_set_q_blk (DM_FBX_STRUCT *FbxPtr,
                             PBA_INT Pba);

void blkrecord_cd_unset_q_blk (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba);

unsigned long blkrecord_cd_check_q_blk (DM_FBX_STRUCT *FbxPtr,
                                        PBA_INT Pba);

void blkrecord_cd_set_bad_blk (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba);

BIT_STAT blkrecord_cd_check_bad_blk (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba);

void blkrecord_cd_set_remap_blk (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba);

void blkrecord_cd_unset_remap_blk (DM_FBX_STRUCT *FbxPtr,
                                   PBA_INT Pba);

BIT_STAT blkrecord_cd_check_remap_blk (DM_FBX_STRUCT *FbxPtr,
                                       PBA_INT Pba);

void blkrecord_cd_put_to_temp_q (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba);

void blkrecord_cd_move_blk (DM_FBX_STRUCT *FbxPtr,
                            PBA_INT Pba);

// User data
void blkrecord_ud_init_malloc (unsigned long FbxIdx);

void blkrecord_ud_init_entries (PCB_STRUCT *PcbPtr);

void blkrecord_ud_set_valid_blk (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba);

void blkrecord_ud_set_valid_sxn (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba);

BIT_STAT blkrecord_ud_chk_valid_sxn (DM_FBX_STRUCT *FbxPtr,
                                     PBA_INT Pba);

void blkrecord_ud_set_invalid_sxn (DM_FBX_STRUCT *FbxPtr,
                                   PBA_INT Pba);

void blkrecord_ud_set_invalid_page (DM_FBX_STRUCT *FbxPtr,
                                    PBA_INT Pba);

PBA_INT blkrecord_ud_get_reclaim_blk (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx);

PBA_INT blkrecord_ud_get_compact_blk (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx);

PBA_INT blkrecord_ud_get_valid_sxn (DM_FBX_STRUCT *FbxPtr,
                                    PBA_INT StartPba);

void blkrecord_ud_set_q_blk (DM_FBX_STRUCT *FbxPtr,
                             PBA_INT Pba);

void blkrecord_ud_unset_q_blk (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba);

unsigned long blkrecord_ud_check_q_blk (DM_FBX_STRUCT *FbxPtr,
                                        PBA_INT Pba);

void blkrecord_ud_set_bad_blk (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba);

BIT_STAT blkrecord_ud_check_bad_blk (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba);

PBA_INT blkrecord_ud_get_rmp_blk (DM_FBX_STRUCT *FbxPtr);

void blkrecord_ud_set_remap_blk (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba);

void blkrecord_ud_put_to_temp_q (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT Pba);

void blkrecord_ud_move_blk (DM_FBX_STRUCT *FbxPtr,
                            PBA_INT Pba);

#endif
//=============================================================================
// $Log: BlkRecord.h,v $
// Revision 1.8  2014/05/19 04:48:58  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.7  2014/05/13 13:26:29  rcantong
// 1. BUGFIX: Compacting of already invalid sxn
// 1.1 Added re-checking of sxn valid during compacting - JParairo
//
// Revision 1.6  2014/04/30 13:38:54  rcantong
// 1. DEV: Support compact queues limited to 64 only
// 1.1 Update BR_QUE_CNT to 64 - JParairo
// 1.2 Added QueIdx in BlkRecordUdEntryStruct - JParairo
// 2. DEV: Support user flash block remapping
// 2.1 Added UD_RMPQ - PPestano
// 2.2 Added API for user flash block remapping - PPestano
//
// Revision 1.5  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
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
