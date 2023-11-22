//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Reclaim/Reclaim.h,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/04/30 14:20:46 $
// $Id: Reclaim.h,v 1.5 2014/04/30 14:20:46 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__RECLAIM_H__)
#define __RECLAIM_H__

#if defined(DEBUG)
_Inline void reclaim_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define RECLAIM_COMPACT_THRESHOLD_0     8
#define RECLAIM_COMPACT_THRESHOLD_1     64
#define RECLAIM_LPW_THRESHOLD_OFF       8
#define RECLAIM_LPW_THRESHOLD_ON        32


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct ReclaimLaneStruct
{
    PCB_STRUCT PcbStruct;
    unsigned long RclmBlkCnt;
} RECLAIM_LANE_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define RECLAIM_UD_INC_RCLM_BLK(ReclaimUd, LaneIdx) \
    (ReclaimUd.LanePtr[LaneIdx].RclmBlkCnt++)

#define RECLAIM_UD_DEC_RCLM_BLK(ReclaimUd, LaneIdx) \
    (ReclaimUd.LanePtr[LaneIdx].RclmBlkCnt--)

#define RECLAIM_UD_CNT_LOW(ReclaimUd, LaneIdx) \
    (ReclaimUd.LanePtr[LaneIdx].RclmBlkCnt <= RECLAIM_COMPACT_THRESHOLD_0)

#define RECLAIM_UD_CNT(ReclaimUd, LaneIdx) \
    (ReclaimUd.LanePtr[LaneIdx].RclmBlkCnt)


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

// Control Data
void reclaim_cd_init_malloc (unsigned long FbxIdx);

void reclaim_cd_init_reclaim_fbx (PCB_STRUCT *PcbPtr);

void reclaim_cd_trigger_reclaim_lane (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx);

// User Data
void reclaim_ud_init_malloc (unsigned long FbxIdx);

void reclaim_ud_init_reclaim_fbx (PCB_STRUCT *PcbPtr);

void reclaim_ud_init_trigger_reclaim (PCB_STRUCT *PcbPtr);

void reclaim_ud_trigger_reclaim_lane (DM_FBX_STRUCT *FbxPtr,
                                      unsigned long LaneIdx);

void reclaim_ud_exec_reclaim_lane (DM_FBX_STRUCT *FbxPtr,
                                   unsigned long LaneIdx);

void reclaim_ud_wakeup_reclaim_lane (DM_FBX_STRUCT *FbxPtr,
                                     unsigned long LaneIdx);


#endif
//=============================================================================
// $Log: Reclaim.h,v $
// Revision 1.5  2014/04/30 14:20:46  rcantong
// 1. DEV: Support counter for reclaimable blocks
// 1.1 Added API to monitor and count reclaim blocks - MFenol
// 2. BUGFIX: Lack of freelist during power on
// 2.1 Triggered reclaim at end of dm init - MFenol
//
// Revision 1.4  2013/12/05 13:06:35  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:08  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:13  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
