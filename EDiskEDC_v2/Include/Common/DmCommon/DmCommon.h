//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/DmCommon/DmCommon.h,v $
// $Revision: 1.9 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:59 $
// $Id: DmCommon.h,v 1.9 2014/05/19 04:48:59 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__DMCOMMON_H__)
#define __DMCOMMON_H__

#if defined(DEBUG)
_Inline void dmcommon_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define INVALID_MASK                    0x80000000

// Will be used for CD Scrambling
#define KEY1_OFFSET                     0x32464532
#define KEY2_OFFSET                     0x88234FEC
#define KEY3_OFFSET                     0x2CFE7835
#define KEY4_OFFSET                     0x9EFCB821


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------

typedef struct Dir0CacheLookupStruct DIR0_CACHE_LOOKUP_STRUCT;
typedef struct Dir1EntryStruct DIR1_ENTRY_STRUCT;
typedef struct Si0CacheLookupStruct SI0_CACHE_LOOKUP_STRUCT;
typedef struct Si1EntryStruct SI1_ENTRY_STRUCT;
typedef struct UserSxnBmpStruct USER_SXN_BMP_STRUCT;
typedef struct BlkRecordCdEntryStruct BLKRECORD_CD_ENTRY_STRUCT;
typedef struct BlkRecordUdEntryStruct BLKRECORD_UD_ENTRY_STRUCT;
typedef struct BlkRecordCdLaneStruct BLKRECORD_CD_LANE_STRUCT;
typedef struct BlkRecordUdLaneStruct BLKRECORD_UD_LANE_STRUCT;
typedef struct CntlCacheStruct CNTL_CACHE_STRUCT;
typedef struct FreeListLaneStruct FREELIST_LANE_STRUCT;
typedef struct ReclaimLaneStruct RECLAIM_LANE_STRUCT;
typedef struct CompactLaneStruct COMPACT_LANE_STRUCT;
typedef struct CdFlushWorkerStruct CD_FLUSH_WORKER_STRUCT;
typedef struct BlkInfoParmStruct BLKINFO_FBX_PARM_STRUCT;
typedef struct FbxLogInfoStruct NV_FBX_LOG_INFO_STRUCT;


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct DmParmStruct
{
    unsigned long DevCurPba[FBX_CNT][DEV_CNT];
    PCB_STRUCT *MasterPcbPtr;
    unsigned long Filler[7];
} DM_PARM_STRUCT;

typedef struct DirFbxStruct
{
    CNTL_CACHE_STRUCT *CacheBasePtr;
    UTIL_DLL_STRUCT CleanList;
    UTIL_SLL_STRUCT RqstCacheWaitQ;
    DIR0_CACHE_LOOKUP_STRUCT *Dir0CacheLookupPtr;
    DIR1_ENTRY_STRUCT *Dir1EntryPtr;
    SQN_INT *Dir1SqnPtr;
} DIR_FBX_STRUCT;

typedef struct BlkInfoFbxStruct
{
    CNTL_CACHE_STRUCT *BlkInfoCachePtr;
    BLKINFO_FBX_PARM_STRUCT *BlkInfoParmPtr;
    volatile unsigned long *EraseCntPtr;
} BLKINFO_FBX_STRUCT;

typedef struct SxnInfoFbxStruct
{
    CNTL_CACHE_STRUCT *CacheBasePtr;
    UTIL_DLL_STRUCT CleanList;
    UTIL_SLL_STRUCT RqstCacheWaitQ;
    SI0_CACHE_LOOKUP_STRUCT *Si0CacheLookupPtr;
    SI1_ENTRY_STRUCT *Si1EntryPtr;
    SQN_INT *Si1SqnPtr;
} SXNINFO_FBX_STRUCT;

typedef struct CntlDataFbxStruct
{
    UTIL_DLL_STRUCT DirtyList;
    UTIL_DLL_STRUCT FlushList;
    IDENTITY_INT *IdPtr;
} CNTLDATA_FBX_STRUCT;

typedef struct BlkRecordCdFbxStruct
{
    BLKRECORD_CD_ENTRY_STRUCT *EntryPtr;
    BLKRECORD_CD_LANE_STRUCT *LanePtr;
} BLKRECORD_CD_FBX_STRUCT;

typedef struct BlkRecordUdFbxStruct
{
    BLKRECORD_UD_ENTRY_STRUCT *EntryPtr;
    BLKRECORD_UD_LANE_STRUCT *LanePtr;
    UTIL_DLL_STRUCT RemapList;
} BLKRECORD_UD_FBX_STRUCT;

typedef struct FreeListFbxStruct
{
    unsigned short CurLaneIdx;
    unsigned short CmpctCurLaneIdx;
    unsigned long TotEntryCnt;
    FREELIST_LANE_STRUCT *LanePtr;
} FREELIST_FBX_STRUCT;

typedef struct ReclaimFbxStruct
{
    unsigned long ReclaimFlag;
    RECLAIM_LANE_STRUCT *LanePtr;
} RECLAIM_FBX_STRUCT;

typedef struct RemapUdFbxStruct
{
    PCB_STRUCT *RemapUdPcbPtr;
    unsigned long BuffAddr;
} REMAP_UD_FBX_STRUCT;

typedef struct CompactFbxStruct
{
    COMPACT_LANE_STRUCT *LanePtr;
} COMPACT_FBX_STRUCT;

typedef struct CdFlushMgrFbxStruct
{
    CD_FLUSH_WORKER_STRUCT *WorkerPtr;
} CD_FLUSH_MGR_FBX_STRUCT;

typedef struct DisturbFbxStruct
{
    unsigned long *DisturbReadCntPtr;
} DISTURB_FBX_STRUCT;

typedef struct ScrubUdFbxStruct
{
    PCB_STRUCT *PcbPtr;
} SCRUB_UD_FBX_STRUCT;

typedef struct NvFbxStruct
{
    NV_FBX_LOG_INFO_STRUCT *LogInfoPtr;
} NV_FBX_STRUCT;

// Reside in dccm, maintain power of two size
typedef struct DmFbxStruct
{
    unsigned long FbxIdx;
    unsigned long MfgDefectsCnt;
    unsigned long TotalDefectsCnt;
    CNTL_CACHE_STRUCT *BaseBlkCachePtr;
    CNTL_CACHE_STRUCT *DefectsCachePtr;
    unsigned long *UserSxnBmpPtr;
    DIR_FBX_STRUCT Dir;
    BLKINFO_FBX_STRUCT BlkInfo;
    SXNINFO_FBX_STRUCT SxnInfo;
    CNTLDATA_FBX_STRUCT CntlData;
    BLKRECORD_CD_FBX_STRUCT BlkRecordCd;
    BLKRECORD_UD_FBX_STRUCT BlkRecordUd;
    FREELIST_FBX_STRUCT FreeListCd;
    FREELIST_FBX_STRUCT FreeListUd;
    RECLAIM_FBX_STRUCT ReclaimCd;
    RECLAIM_FBX_STRUCT ReclaimUd;
    REMAP_UD_FBX_STRUCT RemapUd;
    COMPACT_FBX_STRUCT CompactCd;
    COMPACT_FBX_STRUCT CompactUd;
    CD_FLUSH_MGR_FBX_STRUCT CdFlushMgr;
    DISTURB_FBX_STRUCT DisturbMgr;
    SCRUB_UD_FBX_STRUCT ScrubUd;
    NV_FBX_STRUCT NvConfig;
    PCB_STRUCT *ParentPcbPtr;
    unsigned long *DevCurPbaPtr;
    unsigned char FmToFid[FMEM_CNT];
    SQN_INT Sqn;
    unsigned long Filler[5];
} DM_FBX_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define GET_DEV_BUFF_ADDR(FbxIdx, DevIdx) \
    (0xE0100000 + (((FbxIdx) * DEV_CNT) + (DevIdx)) * FLASH_PAGE_SIZE)

#define GET_RANDOM_PATTERN() \
    (_lr(0x100) | (_lr(0x21) << 16) | INVALID_MASK)

#define DM_ALIGN_TO(AlignVal, ObjVar) ((ObjVar) & ~((AlignVal) - 1))

#define SCRAMBLE_WORD(Offset) (ScrambleBase[(Offset) % 0x100000])


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

extern DM_PARM_STRUCT DmParm;

#pragma BSS(".dm_global")
extern unsigned long ScrambleBase[0x100000];
#pragma BSS()

#pragma BSS(".dccm_dm")
extern DM_FBX_STRUCT DmFbx[FBX_CNT];
extern DM_FBX_STRUCT *LocalFbxPtr[FBX_CNT];
extern unsigned long CdFlushTmrCnt;
#pragma BSS()


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void dm_scanning (PCB_STRUCT *PcbPtr);

void dm_build (PCB_STRUCT *PcbPtr);

void dm_build_thorough_scan_fbx (PCB_STRUCT *PcbPtr);

void dm_build_flush_thorough_defects (PCB_STRUCT *PcbPtr);

void dm_fill_random_pattern (void *TgtPtr,
                             unsigned long ByteLength);

void dm_fill_scramble_pattern (void *TgtPtr,
                               unsigned long ByteLength);

void dm_scramble_data (volatile unsigned long *DataPtr,
                       unsigned long *KeyPtr,
                       unsigned long Mask);

void dm_notify_completion (PCB_STRUCT *PcbPtr);

void dm_master_send_cmd (SCHED_FN Fn);

void dm_slave_recv_cmd (unsigned long *PayloadPtr);

void dm_slave_send_rsp (PCB_STRUCT *PcbPtr);

void dm_master_recv_rsp (unsigned long *PayloadPtr);


#endif
//=============================================================================
// $Log: DmCommon.h,v $
// Revision 1.9  2014/05/19 04:48:59  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.8  2014/05/13 13:19:51  rcantong
// 1. DEV: Support background scrubber
// 1.1 Added background scrubber process - PPestano
//
// Revision 1.7  2014/04/30 13:53:35  rcantong
// 1. DEV: Support defects screening
// 1.1 Added process for thorough scanning - BBantigue
// 2. BUGFIX: Enhanced scrambler pattern
// 2.1 Used tiny encryption algo to generate scrambler pattern
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
