//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Bios/Sched.h,v $
// $Revision: 1.11 $
// $Author: rcantong $
// $Date: 2014/05/19 04:55:00 $
// $Id: Sched.h,v 1.11 2014/05/19 04:55:00 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__SCHED_H__)
#define __SCHED_H__

#if defined(DEBUG)
_Inline void sched_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define PCB_CNT                         256
#define TCB_CNT                         8192


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------

typedef struct DmFbxStruct DM_FBX_STRUCT;
typedef struct CntlCacheStruct CNTL_CACHE_STRUCT;
typedef struct CntlHdrStruct CNTL_HDR_STRUCT;
typedef struct PcbStruct PCB_STRUCT;
typedef void (*SCHED_FN)(PCB_STRUCT *);


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct PcbPoolStruct
{
    UTIL_SLL_STRUCT PcbList[2];
    unsigned long PcbCnt;
} PCB_POOL_STRUCT;

typedef struct PcbGenStruct
{
    unsigned long Word[9];
} PCB_GEN_STRUCT;

typedef struct PcbMrDmaStruct
{
    unsigned long Size;
    unsigned long HighAddr;
    unsigned long LowAddr;
    unsigned long DevAddr;
    unsigned long Ctrl1;
    unsigned long Ctrl2;
} PCB_MRDMA_STRUCT;

typedef struct PcbDmxEraseStruct
{
    BIT_STAT Stat;
    PBA_INT Pba;
} PCB_DMX_ERASE_STRUCT;

typedef struct PcbDmxWriteStruct
{
    BIT_STAT Stat;
    PBA_INT Pba;
    unsigned long BuffAddr;
    unsigned long SegCnt;
} PCB_DMX_WRITE_STRUCT;

typedef struct PcbDmxReadStruct
{
    BIT_STAT Stat;
    PBA_INT Pba;
    unsigned long BuffAddr;
    unsigned long SegCnt;
} PCB_DMX_READ_STRUCT;

typedef struct PcbDmxWriteCadStruct
{
    BIT_STAT Stat;
    PBA_INT Pba;
    unsigned long *BuffPtr;         // Gather address array
    unsigned long SegCnt;
} PCB_DMX_WRITE_CAD_STRUCT;

typedef struct PcbDmxWriteInPlaceStruct
{
    BIT_STAT Stat;
    PBA_INT Pba;
    unsigned long BuffAddr;
    unsigned short SegCnt;
    unsigned short SegIdx;
} PCB_DMX_WRITE_IN_PLACE_STRUCT;

typedef struct PcbDmxReadInPlaceStruct
{
    BIT_STAT Stat;
    PBA_INT Pba;
    unsigned long BuffAddr;
    unsigned short SegCnt;
    unsigned short SegIdx;
} PCB_DMX_READ_IN_PLACE_STRUCT;

typedef struct PcbDmxOpsStruct
{
    SCHED_FN OpFn;
} PCB_DMX_OPS_STRUCT;

typedef struct PcbDmInitStruct
{
    unsigned long DeployCtr;
} PCB_DM_INIT_STRUCT;

typedef struct PcbScreeningStruct
{
    PCB_DMX_WRITE_STRUCT Dmx;
    unsigned long ThoroughScanLoopCnt;
    unsigned long DevIdx;
    unsigned long BlkIdx;
    unsigned long SegIdx;
} PCB_SCREENING_STRUCT;

typedef struct PcbDirBuildStruct
{
    PCB_DMX_WRITE_STRUCT Dmx;
    unsigned long DevIdx;
    unsigned long Dir0SxnIdx;
} PCB_DIR_BUILD_STRUCT;

typedef struct PcbSiBuildStruct
{
    PCB_DMX_WRITE_STRUCT Dmx;
    unsigned long DevIdx;
    unsigned long Si0SxnIdx;
} PCB_SI_BUILD_STRUCT;

typedef struct PcbCntlFetchStruct
{
    PCB_DMX_READ_STRUCT Dmx;
    PBA_INT CurPba;
} PCB_CNTL_FETCH_STRUCT;

typedef struct PcbDirFetchStruct
{
    PCB_DMX_READ_STRUCT Dmx;
    unsigned long Dir0SxnIdx;
} PCB_DIR_FETCH_STRUCT;

typedef struct PcbRclmStruct
{
    PCB_DMX_ERASE_STRUCT Dmx;
    unsigned long LaneIdx;
} PCB_RCLM_STRUCT;

typedef struct PcbUserReadStruct
{
    BIT_STAT Stat;
    PBA_INT Pba;
    unsigned long BuffAddr;
    unsigned short LbaOffset;
    unsigned short LbaCnt;
    unsigned long UserSxnIdx;
    unsigned long CmdSxnIdx;
    unsigned long DrvrCmdTag;
    CNTL_CACHE_STRUCT *Dir0CachePtr;
    PCB_STRUCT *NxtPcbPtr;
} PCB_USER_READ_STRUCT;

typedef struct PcbUserWriteStruct
{
    BIT_STAT Stat;
    PBA_INT Pba;
    unsigned long BuffAddr;
    unsigned long SxnCnt;
    unsigned long UserSxnIdx;
    unsigned long DrvrCmdTag;
    CNTL_CACHE_STRUCT *CntlCachePtr;
    PBA_INT OldPba;
    PCB_STRUCT *NxtPcbPtr;
} PCB_USER_WRITE_STRUCT;

typedef struct PcbCntlReadStruct
{
    PCB_DMX_READ_STRUCT Dmx;
    CNTL_CACHE_STRUCT *CntlCachePtr;
} PCB_CNTL_READ_STRUCT;

typedef struct PcbCdFlushStruct
{
    PCB_DMX_WRITE_CAD_STRUCT Dmx;
    CNTL_HDR_STRUCT *CntlHdrPtr;
    unsigned long *GatherPtr;       // Gather cd cache
    unsigned long ActiveFlag;
    unsigned long FlushPrio;
} PCB_CD_FLUSH_STRUCT;

typedef struct PcbCompactCdStruct
{
    unsigned long LaneIdx;
    unsigned long ActiveFlag;
    unsigned long Counter;
    PBA_INT CompactPba;
    CNTL_CACHE_STRUCT *CachePtr;
    unsigned long SxnIdx;
} PCB_COMPACT_CD_STRUCT;

typedef struct PcbCompactUdStruct
{
    PCB_DMX_READ_STRUCT Dmx;
    unsigned short LaneIdx;
    unsigned char ActiveFlag;
    unsigned char CmpctSxnCnt;
    PBA_INT CompactPba;
    unsigned long UserSxnIdx;
    CNTL_CACHE_STRUCT *CntlCachePtr;
    PCB_STRUCT *NxtPcbPtr;
} PCB_COMPACT_UD_STRUCT;

typedef struct PcbRemapCdStruct
{
    PCB_DMX_READ_STRUCT Dmx;
    CNTL_CACHE_STRUCT *Dir0CachePtr;
    CNTL_CACHE_STRUCT *Si0CachePtr;
    unsigned long SxnIdx;
    PBA_INT Pba;
} PCB_REMAP_CD_STRUCT;

typedef struct PcbRemapUdStruct
{
    PCB_DMX_READ_STRUCT Dmx;
    unsigned short ActiveFlag;
    unsigned short RemapSxnCnt;
    PBA_INT RemapPba;
    unsigned long UserSxnIdx;
    CNTL_CACHE_STRUCT *CntlCachePtr;
    PCB_STRUCT *NxtPcbPtr;
} PCB_REMAP_UD_STRUCT;

typedef struct PcbScrubUdStruct
{
    PCB_DMX_READ_STRUCT Dmx;
    PBA_INT UserPba;
    unsigned long Ongoing;
} PCB_SCRUB_UD_STRUCT;

typedef struct PcbMediaWriteInPlaceStruct
{
    PCB_DMX_WRITE_IN_PLACE_STRUCT Dmx;
    unsigned long IpCallFn;
} PCB_MEDIA_WRITE_IN_PLACE_STRUCT;

typedef struct PcbReportLunsStruct
{
    PCB_MRDMA_STRUCT MrDma;
    unsigned long Len;
} PCB_REPORT_LUNS_STRUCT;

typedef struct PcbReadCapacityStruct
{
    PCB_MRDMA_STRUCT MrDma;
    unsigned long Len;
    unsigned long LongLba;
} PCB_READ_CAPACITY_STRUCT;

typedef struct ModeSelFlagStruct
{
    unsigned char LongLba               : 1;
    unsigned char BlockDescData         : 1;
    unsigned char SavePages             : 1;
    unsigned char MultipleDmaOnGoing    : 1;
    unsigned char LongHeader            : 1;
    unsigned char CachingData           : 1;
    unsigned char Reserved              : 2;
} MODE_SEL_FLAG_STRUCT;

typedef struct PcbModeSelectStruct
{
    PCB_MRDMA_STRUCT MrDma;
    unsigned short ParmListLen;
    unsigned char BlkDescLength;
    unsigned char Phase;
    //unsigned char PageFormat;
    //unsigned long DataSz;
    unsigned char DataIdx; //used for data transfer
    unsigned char ParmIdx; //used for parameter validation
    MODE_SEL_FLAG_STRUCT ModeSelFlag;
} PCB_MODE_SELECT_STRUCT;

typedef struct PcbModeSenseStruct
{
    PCB_MRDMA_STRUCT MrDma;
    unsigned char LongHeader;       // Indicates whether mode6 or mode10
    unsigned char DisableBlkDesc;
    unsigned char PageCntlFld;      // Mode page location
    unsigned char PageCode;         // Mode page type to be sent
    unsigned char SubPageCode;      // Mode page format
    unsigned long AllocLen;
} PCB_MODE_SENSE_STRUCT;

typedef struct PcbInquiryStruct
{
    PCB_MRDMA_STRUCT MrDma;
    unsigned long EnabVitalProdData; // Send vital product data flag
    unsigned long PageCode;          // Classify type of pagecode to be sent
    unsigned long AllocLen;          // Allocated len by app
} PCB_INQUIRY_STRUCT;

typedef struct PcbRqstSenseStruct
{
    PCB_MRDMA_STRUCT MrDma;
    unsigned char Desc;             // Descriptor Format
    unsigned long AllocLen;         // Allocated len by app
} PCB_RQST_SENSE_STRUCT;

typedef struct PcbDataCtlStruct
{
    BLK_INT StartLba;               // Starting LBA of the requested range
    BLK_INT LimitLba;               // LBA beyond requested range
    BLK_INT NxtSentLba;             // LBA issued to DMX/IOC
    BLK_INT NxtDoneLba;             // LBA confirmed by DMX/IOC
} PCB_DATA_CTL_STRUCT;

typedef struct PcbSyncStruct
{
    PCB_DATA_CTL_STRUCT DataCtl;
    unsigned char ImmediateStatusFlg;
    unsigned char CacheDrainIssuedFlg;
    unsigned char FlushCtlBlkIssuedFlg;
} PCB_SYNC_STRUCT;

typedef struct PcbUcodeStruct
{
    unsigned long DevAddr;
    unsigned long CurrSize;
    unsigned long FwSize;
    unsigned long CheckSum;
    unsigned long CurrCheckSum;
    unsigned long Profile;
    unsigned long Status;
} PCB_UCODE_STRUCT;

// BIT Specific
typedef struct PcbBitCmdStruct
{
    PCB_MRDMA_STRUCT MrDma;
    unsigned char Mode;
    unsigned char Phase;
    unsigned short Data;
    unsigned long Len;
} PCB_BIT_CMD_STRUCT;

typedef union PcbInfoUnion
{
    PCB_GEN_STRUCT Gen;
    PCB_MRDMA_STRUCT MrDma;
    PCB_DMX_ERASE_STRUCT DmxErase;
    PCB_DMX_WRITE_STRUCT DmxWrite;
    PCB_DMX_READ_STRUCT DmxRead;
    PCB_DMX_WRITE_CAD_STRUCT DmxWriteCad;
    PCB_DMX_WRITE_IN_PLACE_STRUCT DmxWriteInPlace;
    PCB_DMX_READ_IN_PLACE_STRUCT DmxReadInPlace;
    PCB_DMX_OPS_STRUCT DmxOps;
    PCB_DM_INIT_STRUCT DmInit;
    PCB_SCREENING_STRUCT Screening;
    PCB_DIR_BUILD_STRUCT DirBuild;
    PCB_SI_BUILD_STRUCT SiBuild;
    PCB_CNTL_FETCH_STRUCT CntlFetch;
    PCB_DIR_FETCH_STRUCT DirFetch;
    PCB_RCLM_STRUCT Rclm;
    PCB_USER_READ_STRUCT UserRead;
    PCB_USER_WRITE_STRUCT UserWrite;
    PCB_CNTL_READ_STRUCT CntlRead;
    PCB_CD_FLUSH_STRUCT CntlWrite;
    PCB_COMPACT_CD_STRUCT CompactCd;
    PCB_COMPACT_UD_STRUCT CompactUd;
    PCB_REMAP_CD_STRUCT RemapCd;
    PCB_REMAP_UD_STRUCT RemapUd;
    PCB_SCRUB_UD_STRUCT ScrubUd;
    PCB_MEDIA_WRITE_IN_PLACE_STRUCT MediaWriteInPlace;
    PCB_REPORT_LUNS_STRUCT ReportLuns;
    PCB_READ_CAPACITY_STRUCT ReadCapacity;
    PCB_MODE_SELECT_STRUCT ModeSelect;
    PCB_MODE_SENSE_STRUCT ModeSense;
    PCB_INQUIRY_STRUCT Inquiry;
    PCB_SYNC_STRUCT Sync;
    PCB_UCODE_STRUCT Ucode;
    PCB_BIT_CMD_STRUCT BitCmd;
    PCB_RQST_SENSE_STRUCT RqstSense;
} PCB_INFO_UNION;

typedef union PcbWordUnion
{
    unsigned long FbxIdx;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long OpCd;
    BIT_HDL FrameHdl;
} PCB_WORD_UNION;

typedef struct PcbStruct
{
    UTIL_SLL_ENTRY_STRUCT Link;
    SCHED_FN Fn;
    PCB_WORD_UNION Word;
    PCB_INFO_UNION Info;
} PCB_STRUCT;

// To allocate dcache align Pcb
typedef struct PcbAlignStruct
{
    PCB_STRUCT Pcb;
    unsigned long Filler[4];
} PCB_ALIGN_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define SCHED_POST_PCB(PcbPtr) \
    (util_sll_insert_at_tail(&(PcbPtr)->Link, &RunQueue))

#define SCHED_LOW_PCB_CNT() (PcbPool.PcbCnt <= 2048)


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

#pragma BSS(".dccm")
extern PCB_POOL_STRUCT PcbPool;
extern UTIL_SLL_STRUCT RunQueue;
#pragma BSS()


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void sched_init (void);

PCB_STRUCT *_sched_get_pcb (void);

void _sched_return_pcb (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: Sched.h,v $
// Revision 1.11  2014/05/19 04:55:00  rcantong
// 1. DEV: Support control remap
// 1.1 Added Pcb for control remap process - BBantigue
// 2. BUGFIX: Update scrub from LA-based to PA-based scrubber
// 2.1 Update Pcb for Scrub to be Pba based - PPestano
//
// Revision 1.10  2014/05/13 13:19:51  rcantong
// 1. DEV: Support background scrubber
// 1.1 Added background scrubber process - PPestano
//
// Revision 1.9  2014/04/30 13:31:04  rcantong
// 1. DEV: Support defects screening
// 1.1 Added PCB info for defects screening process - BBantigue
// 2. BUGFIX: Insufficient PCB
// 2.1 Added PCB counter and monitoring of low PCB count - MFenol
//
// Revision 1.8  2014/03/03 12:30:56  rcantong
// 1. BUGFIX: Handles unsync Dir and SI due to ungraceful shutdown
// 1.1 Removed compact_ud_verify_usersxn - MFenol
// 1.2 Skip compact when compact pba is not sync to dir - MFenol
//
// Revision 1.7  2014/02/06 14:37:39  rcantong
// 1. DEV: Support request sense
// 1.1 Added request sense handling
//
// Revision 1.6  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.4  2013/12/05 13:06:35  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:47  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:06  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:01  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
