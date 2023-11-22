//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Public/NvConfig.h,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/05/13 13:54:23 $
// $Id: NvConfig.h,v 1.7 2014/05/13 13:54:23 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__NVCONFIG_H__)
#define __NVCONFIG_H__

#if defined(DEBUG)
_Inline void nvconfig_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "SysConfig.h"

//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define NV_FBX_CNT                      2
#define NV_CONFIG_SAVED_IND_A           0xEE1111EE
#define NV_CONFIG_SAVED_IND_B           0x11EEEE11

// Defines for PRODUCT_INFO_STRUCT
#define VENDOR_ID_LENGTH                8
#define PRODUCT_ID_LENGTH               16
#define SERIAL_NUMBER_LENGTH            12

#define MODEL_NAME_LENGTH               8
#define PART_NUMBER_LENGTH              16
#define DATE_CODE_LENGTH                8
#define BRD_SERIAL_LENGTH               12

// Jumper Setting
#define JUMPER_CNTLR_PTR                ((volatile unsigned long *)0xFFCE3434)
#define JUMPER_ERASE_ALL                0
#define WRITE_PROTECT                   2 // Jumper Pins are active low
#define JUMPER_REBUILD                  4 // Jumper Pins are active low
#define JUMPER_MASK                     6


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct DevConfigStruct
{
    unsigned long Filler[64];
} DEV_CONFIG_STRUCT;

typedef struct DmCfgStruct
{
    unsigned long SavedIndA;
    unsigned long UserSxnSz;
    unsigned long CntlSxnSz;
    unsigned long SegmentSz;
    unsigned long LbaSz;
    unsigned long UserSegmentSz;
    unsigned long FbxCnt;
    unsigned long DevCnt;
    unsigned long LbasPerSegment;
    unsigned long LbasPerUserSxn;
    unsigned long LbasPerPage;
    unsigned long UserSxnsPerPage;
    unsigned long CntlSxnsPerPage;
    unsigned long UserSxnsPerBlk;
    unsigned long CntlSxnsPerBlk;
    unsigned long SegPerUserSxn;
    unsigned long SegPerCntlSxn;
    unsigned long SegPerPage;
    unsigned long SegPerBlk;
    unsigned long SegPerSysBlk;
    unsigned long PagesPerBlk;
    unsigned long BlksPerFbx;
    unsigned long SegPerFbx;
    unsigned long PagesPerFbx;
    unsigned long UserBlksPerDev;
    unsigned long CntlBlksPerDev;
    unsigned long CntlBlksPerFbx;
    unsigned long UsableBlkCnt;
    unsigned long UsableSxnCnt;
    unsigned long UserSxnCnt;
    unsigned long CntlBlkCnt;
    unsigned long CntlSxnCnt;
    unsigned long Dir0SxnSz;
    unsigned long Dir0EntriesPerSxn;
    unsigned long Dir0SxnCnt;
    unsigned long SI0SxnSz;
    unsigned long SI0EntriesPerSxn;
    unsigned long SI0SxnCnt;
    unsigned long CntlSegPerFbx;
    unsigned long long LastLba;
    unsigned long Filler[22];
    unsigned long SavedIndB;
} DM_CFG_STRUCT;

typedef struct FlashConfigStruct
{
    unsigned long SavedIndA;
    unsigned long FlashId;
    unsigned long BlksPerDev;
    unsigned long BlksPerLun;
    unsigned long FlashBlkSz;
    unsigned long FlashPageSz;
    unsigned long Fller[57];
    unsigned long SavedIndB;
} FLASH_CONFIG_STRUCT;

typedef struct ProductInfoStruct
{
    unsigned long SavedIndA;                         // R/O Saved/Curr
    unsigned char VendorId[VENDOR_ID_LENGTH];        // R/W Saved/Curr
    unsigned char ProductId[PRODUCT_ID_LENGTH];      // R/W Saved/Curr
    unsigned char PartNumber[PART_NUMBER_LENGTH];    // R/W Saved/Curr
    unsigned char DateCode[DATE_CODE_LENGTH];        // R/W Saved/Curr
    unsigned char SerialNumber[SERIAL_NUMBER_LENGTH];// R/W Saved/Curr
    unsigned char CtlBrdSerNo[BRD_SERIAL_LENGTH];    // R/W Saved/Curr
    unsigned char PwrModSerNo[BRD_SERIAL_LENGTH];    // R/W Saved/Curr
    unsigned char PGBrdSerNo[BRD_SERIAL_LENGTH];     // R/W Saved/Curr
    unsigned char ModelName[MODEL_NAME_LENGTH];      // R/W Saved/Curr
    unsigned long Filler[36];
    unsigned long SavedIndB;                         // R/O Saved/Curr
} PRODUCT_INFO_STRUCT;

typedef struct FwInfoStruct
{
    unsigned long SavedIndA;
    unsigned long FwVer;
    unsigned long FwDate;
    unsigned long FwMonth;
    unsigned long FwYear;
    unsigned long Filler[26];
    unsigned long SaveIndB;
} FW_INFO_STRUCT;

typedef struct DiskHealthStruct
{
    unsigned long SavedIndA;
    unsigned long EraseCnt;
    unsigned long long WriteStat;
    unsigned long long ReadStat;
    unsigned long RefTemp;
    unsigned long CurrentTemp;
    unsigned long Filler[23];
    unsigned long SavedIndB;
} DISK_HEALTH_STRUCT;

// Page: Caching (0x08)
typedef struct ScsiCachingPageStruct                 // PageCode 0x08
{
    unsigned char PageCode                      :6;  // Byte 0 Bits 0-5
    unsigned char SubPageFormat                 :1;  //        Bit 6
    unsigned char ParmsSaveable                 :1;  //        Bit 7
    unsigned char PageLength;                        // Byte 1
    unsigned char ReadCacheDisable              :1;  // Byte 2 Bit 0
    unsigned char MultiplicationFactor          :1;  //        Bit 1
    unsigned char WriteCacheEnable              :1;  //        Bit 2
    unsigned char SizeEn                        :1;  //        Bit 3
    unsigned char Discontinuity                 :1;  //        Bit 4
    unsigned char CacheAnalysisPermitted        :1;  //        Bit 5
    unsigned char AbortPrefetch                 :1;  //        Bit 6
    unsigned char InitiatorControl              :1;  //        Bit 7
    unsigned char WriteRetentionPriority        :4;  // Byte 3 Bits 0-3
    unsigned char DemandReadRetentionPriority   :4;  //        Bits 4-7
    unsigned char DisablePrefetchXferLengthMsb;      // Byte 4
    unsigned char DisablePrefetchXferLengthLsb;      // Byte 5
    unsigned char MinPrefetchMsb;                    // Byte 6
    unsigned char MinPrefetchLsb;                    // Byte 7
    unsigned char MaxPrefetchMsb;                    // Byte 8
    unsigned char MaxPrefetchLsb;                    // Byte 9
    unsigned char MaxPrefetchCeilingMsb;             // Byte 10
    unsigned char MaxPrefetchCeilingLsb;             // Byte 11
    unsigned char NVCacheDis                    :1;  // Byte 12 Bits 0
    unsigned char Reserved12Bit2                :2;  // Byte 12 Bits 1-2
    unsigned char VendorSpecific                :2;  //         Bits 3-4
    unsigned char DisableReadAhead              :1;  //         Bit 5
    unsigned char LogBlkCacheSegmentSize        :1;  //         Bit 6
    unsigned char ForceSeqWrite                 :1;  //         Bit 7
    unsigned char NbrOfCacheSegments;                // Byte 13
    unsigned char CacheSegmentSizeMsb;               // Byte 14
    unsigned char CacheSegmentSizeLsb;               // Byte 15
    unsigned char Reserved16;                        // Byte 16
    unsigned char Obsolete17;                        // Byte 17
    unsigned char Obsolete18;                        // Byte 18
    unsigned char Obsolete19;                        // Byte 19
} SCSI_CACHING_PAGE_STRUCT;

// Page: Info Exception (0x1C)
typedef struct ScsiInfoExceptionCtlPageStruct        // PageCode 0x1C
{
    unsigned char PageCode                      :6;  // Byte 0 Bits 0-5
    unsigned char SubPageFormat                 :1;  //        Bit 6
    unsigned char ParmsSaveable                 :1;  //        Bit 7
    unsigned char PageLength;                        // Byte 1
    unsigned char LogErrors                     :1;  // Byte 2 Bit 0 LOG ERR
    unsigned char EBackErr                      :1;  //        Bit 1 EBackErr
    unsigned char Test                          :1;  //        Bit 2 TEST
    unsigned char DisableExceptionCtl           :1;  //        Bit 3 DEXCPT
    unsigned char EnableWarning                 :1;  //        Bit 4 EWASC
    unsigned char EnableBkgrndFunction          :1;  //        Bit 5 EBF
    unsigned char Reserved2Bit6                 :1;  //        Bit 6
    unsigned char Performance                   :1;  //        Bit 7 PERF
    unsigned char MethodRptInfoException        :4;  // Byte 3 Bits 0-3 MRIE
    unsigned char Reserved3Bit7                 :4;  //        Bits 4-7
    unsigned char IntervalTimerByte0;                // Byte 4
    unsigned char IntervalTimerByte1;                // Byte 5
    unsigned char IntervalTimerByte2;                // Byte 6
    unsigned char IntervalTimerByte3;                // Byte 7
    unsigned char ReportCountByte0;                  // Byte 8
    unsigned char ReportCountByte1;                  // Byte 9
    unsigned char ReportCountByte2;                  // Byte 10
    unsigned char ReportCountByte3;                  // Byte 11
} SCSI_INFO_EXCEPTION_CTL_PAGE_STRUCT;

// Mode Pages
typedef struct ScsiProtModePageStruct
{
    SCSI_CACHING_PAGE_STRUCT Caching;                       // 0x08
    SCSI_INFO_EXCEPTION_CTL_PAGE_STRUCT InfoExceptionCtl;   // 0x1C
} SCSI_PROT_MODE_PAGE_STRUCT;

typedef struct ProtModePageStruct
{
    unsigned long SavedIndA;
    SCSI_PROT_MODE_PAGE_STRUCT Scsi;
    unsigned long SavedIndB;
} PROT_MODE_PAGE_STRUCT;

// IOC specific
typedef struct PcieConfigPageStruct
{
    unsigned long SavedIndA;
    //to be filled up
    unsigned long PhyStat;                          //0xfff28000
    unsigned long LinkStat;                         //0xfff28800
    unsigned long CurrentLinkSpeed;                 //0xfff21070
    unsigned long TargetLinkSpeed;                  //0xfff21090
    unsigned long MaxPayloadSz;                     //0xfff21068
    unsigned long MaxReadRqstSz;
    unsigned long RdCmpletionBoundary;              //0xfff21070
    unsigned long LinkWidth;                        //0xfff21070
    unsigned long Filler[118];
    unsigned long SavedIndB;
} PCIE_CONFIG_PAGE_STRUCT;

// Channel Mode Page
typedef struct ChnlModeStruct
{
    unsigned long SavedIndA;

    unsigned long Revision;
    unsigned long LBlkSize;

    BLK_INT LbaLast;            //contains last:: <=MAX
    BLK_INT MaxUserSegments;
    PROT_MODE_PAGE_STRUCT ProtMode;
    unsigned long Filler[110];
    unsigned long SavedIndB;
} CHNL_MODE_PAGE_STRUCT;

typedef struct JumperConfigStruct
{
    unsigned long SavedIndA;
    unsigned long OperatingMode;
    unsigned long InterfaceMode;
    unsigned long IntfcBusId;
    unsigned long DisabWriteProtect;
    unsigned long DisabFullDiag;
    unsigned long FpromBootUpBypassDiag;
    unsigned long ForceRebuild;
    unsigned long PowerGuard;
    unsigned long RemovableEDisk;
    unsigned long Filler[21];
    unsigned long SavedIndB;
} JUMPER_CONFIG_STRUCT;

typedef struct FbxLogInfoStruct
{
    unsigned long EraseCnt;
    unsigned long UserRetryUncorrCnt;
    unsigned long UserUncorrCnt;
    unsigned long UserCorrCnt;
    unsigned long CntlRetryUncorrCnt;
    unsigned long CntlUncorrCnt;
    unsigned long CntlCorrCnt;
    unsigned long DmxResetCnt;
    unsigned long RefreshCnt;
    unsigned long Filler[12];
} FBX_LOG_INFO_STRUCT;

typedef struct NvLogInfoStruct
{
    FBX_LOG_INFO_STRUCT FbxLogInfo[FBX_CNT];
    unsigned long Filler[2];
} NV_LOG_INFO_STRUCT;

typedef struct ReportInfoStruct
{
    unsigned long Filler[128];
} REPORT_INFO_STRUCT;

typedef struct SysConfigStruct
{
    DEV_CONFIG_STRUCT                   DevCfg;             // 256bytes
    PRODUCT_INFO_STRUCT                 ProdInfo;           // 256bytes
    FLASH_CONFIG_STRUCT                 FlashCfg;           // 256bytes
    DM_CFG_STRUCT                       DmCfg;              // 256bytes
    FW_INFO_STRUCT                      FwInfo;             // 128bytes
    DISK_HEALTH_STRUCT                  DiskHealth;         // 128bytes
    CHNL_MODE_PAGE_STRUCT               ChnlMode;           // 512bytes
    PCIE_CONFIG_PAGE_STRUCT             IocCfg;             // 512bytes
    JUMPER_CONFIG_STRUCT                JmprCfg;            // 128bytes
    NV_LOG_INFO_STRUCT                  LogInfo;            // 512bytes
    REPORT_INFO_STRUCT                  ReportInfo;         // 512bytes
    unsigned long                       Filler[3231];
    unsigned long                       Sqn;
    // ChannelLogPage       - 768bytes                      // 0x05 - 0xA
    // ProtectInfo          - 256bytes
    // PowerManagement      - 256bytes
    // DiskHealthAttribute  - 256bytes
    // PG                   - 256bytes
    // SpecialFeature       - 256bytes
} SYSCONFIG_STRUCT;

typedef struct NvConfigStruct
{
    unsigned long ConfigBuff[NV_FBX_CNT];
    unsigned long SysConfigStat[NV_FBX_CNT];
    unsigned long EraseFlag;
    unsigned long FlushingFlag;
    unsigned long DirtyFlag;
    unsigned long Filler[1];
} NVCONFIG_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

#pragma BSS(".usram")
extern volatile SYSCONFIG_STRUCT SysConfigSaved;
extern volatile SYSCONFIG_STRUCT SysConfigCurr;
extern volatile NVCONFIG_STRUCT NvConfigParm;
#pragma BSS()


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void nv_lite_preinit (void);
void nv_init (void);
void nv_init_malloc (unsigned long FbxIdx);
void nv_flush_config (void);
void nv_sync_loginfo (void);
void nv_sync_diskhealth (void);


#endif
//=============================================================================
// $Log: NvConfig.h,v $
// Revision 1.7  2014/05/13 13:54:23  rcantong
// 1. DEV: Able to rebuild by using hardware jumper
// 1.1 Added detection of jumper config to set rebuild flag - MManzo
//
// Revision 1.6  2014/04/30 15:17:16  rcantong
// 1. DEV: Synchronized size of string in Inquiry and SysConfig
// 1.1 Changed values of string length used in product info
// 2. DEV: Support board serial number in product info
// 2.1 Added CtlBrdSerNo, PwrModSerNo and PGBrdSerNo - JParairo
// 3. DEV: Support disk health and log info
// 3.1 Added DiskHealth and LogInfo structure - JParairo
//
// Revision 1.5  2014/02/06 14:27:27  rcantong
// 1. DEV: Support MLC 16KB page size
// 1.1 Changed SysConfig size from 8KB to 16KB
//
// Revision 1.4  2014/02/02 08:59:50  rcantong
// 1. DEV: Support mode select and bit specific config commands
// 1.1 Added handling of mode select and bit specific config commands
//
// Revision 1.3  2014/01/08 12:42:57  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.2  2013/08/08 16:42:06  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:00  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
