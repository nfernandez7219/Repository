//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/BlkRecord/BlkRecordI.h,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:58 $
// $Id: BlkRecordI.h,v 1.7 2014/05/19 04:48:58 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__BLKRECORDI_H__)
#define __BLKRECORDI_H__

#if defined(DEBUG)
_Inline void blkrecordi_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Local Constant Macros
//-----------------------------------------------------------------------------

#define BR_QUE_CNT                      64
#define CD_QUE_DIV                      (CNTL_SXNS_PER_BLK / BR_QUE_CNT)
#define UD_QUE_DIV                      (USER_SXNS_PER_BLK / BR_QUE_CNT)

#define CD_FULLVQ                       (BR_QUE_CNT)
#define CD_RCLMQ                        (BR_QUE_CNT + 1)
#define CD_TEMPQ                        (BR_QUE_CNT + 2)
#define CD_QUE_CNT                      (BR_QUE_CNT + 3)

#define UD_FULLVQ                       (BR_QUE_CNT)
#define UD_RCLMQ                        (BR_QUE_CNT + 1)
#define UD_TEMPQ                        (BR_QUE_CNT + 2)
#define UD_RMPQ                         (BR_QUE_CNT + 3)
#define UD_QUE_CNT                      (BR_QUE_CNT + 4)

#define REMAP_MSK                       0x1
#define QUARANTINE_MSK                  0x2
#define BAD_MSK                         0x8


//-----------------------------------------------------------------------------
// Local Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Structure Definitions
//-----------------------------------------------------------------------------

typedef struct BlkRecordQueStruct
{
    UTIL_DLL_STRUCT Ctrl;
} BLKRECORD_QUE_STRUCT;

typedef struct BlkRecordCdLaneStruct
{
    BLKRECORD_QUE_STRUCT Que[CD_QUE_CNT];
    PCB_STRUCT *WaitRclmPcbPtr;
} BLKRECORD_CD_LANE_STRUCT;

typedef struct BlkRecordCdEntryStruct
{
    UTIL_DLL_ENTRY_STRUCT Link;
    PBA_INT BlkPba;
    unsigned short ValidCnt;
    unsigned char QueIdx;
    unsigned char BlkStat; // [0] Quarantine Bit, [1] Remap Bit, [2] Bad Bit
    unsigned long ValidBmp[CNTL_SXNS_PER_BLK / BITS_PER_WORD];
} BLKRECORD_CD_ENTRY_STRUCT;

typedef struct BlkRecordUdLaneStruct
{
    BLKRECORD_QUE_STRUCT Que[UD_QUE_CNT];
    PCB_STRUCT *WaitRclmPcbPtr;
} BLKRECORD_UD_LANE_STRUCT;

typedef struct BlkRecordUdEntryStruct
{
    UTIL_DLL_ENTRY_STRUCT Link;
    PBA_INT BlkPba;
    unsigned short ValidCnt;
    unsigned char QueIdx;
    unsigned char BlkStat; // [0] Quarantine Bit, [1] Remap Bit, [2] Bad Bit
    unsigned long ValidBmp[USER_SXNS_PER_BLK / BITS_PER_WORD];
} BLKRECORD_UD_ENTRY_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------


#endif
//=============================================================================
// $Log: BlkRecordI.h,v $
// Revision 1.7  2014/05/19 04:48:58  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
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
