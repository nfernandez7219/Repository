//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Interface/Iop.h,v $
// $Revision: 1.6 $
// $Author: rcantong $
// $Date: 2014/04/30 15:05:13 $
// $Id: Iop.h,v 1.6 2014/04/30 15:05:13 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__IOP_H__)
#define __IOP_H__

#if defined(DEBUG)
_Inline void iop_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

// AWB
#define MRDMA_RX_DIR_N_AWB              0x04000000
#define MRDMA_TX_DIR_N_AWB              0x02000001

#define RING_DEPTH                      256
#define BUFFER_CNT                      192
#define BUFFER_SIZE                     16384
#define BUFFER_BASE_ADDR                0x60100000
#define TX_BUFFER_BASE_ADDR             (BUFFER_BASE_ADDR | MRDMA_TX_DIR_N_AWB)
#define RX_BUFFER_BASE_ADDR             (BUFFER_BASE_ADDR | MRDMA_RX_DIR_N_AWB)

#define CMD_FRAME_BULK_SIZE             4096
#define CMD_CNT_PER_BULK                (CMD_FRAME_BULK_SIZE / CMD_FRAME_SIZE)

#define MBX_SZ                          128
#define IOP_RING_READY                  0x88888888
#define MR_DMA_CONFIG                   0xFF64400A // 1KB Burst SZ
#define REG_PCSTCMD                     0x00100407

// For pe core isr
#define RING_ZERO_ATTN                  0x00010000
#define MEM_REQ_DONE                    0x00000002
#define EMP_PORT_ATTN                   0x00000004

// Hardcoded Min Threshold
#define TX_DATA_MIN_THRES_128           0x00000020
#define TX_DATA_MIN_THRES_256           0x00000040

#define CMD_FRAME_SIZE                  sizeof(GEN_CMD_FRM_STRUCT)
#define RSP_FRAME_SIZE                  sizeof(GEN_RSP_FRM_STRUCT)
#define CMD_FRAME_WORD_SIZE             (CMD_FRAME_SIZE / 4)
#define RSP_FRAME_WORD_SIZE             (RSP_FRAME_SIZE / 4)
#define DEV_CONFIG_SIZE                 sizeof(DEV_CONFIG_PARM_STRUCT)

// Command Opcodes
#define TASK_READ_BULK                  0x00000024
#define TASK_WRITE_BULK_PPAGE           0x00000025
#define TASK_WRITE_BULK_FPAGE           0x00000026
#define TASK_WRITE_BULK_FPAGE_PPAGE     0x00000027
#define TASK_WRITE_BULK_MB              0x00000028
#define TASK_WRITE_BULK_RW              0x00000029
#define TASK_INQUIRE_SYSTEM_READY       0x00000032

// Read modify write phase
#define RMW_READ_PHASE                  0x00000002
#define RMW_WRITE_PHASE                 0x00000004

#define UCODE_BASE_ADDR                 0xE0200000

// Max Payload Size Max Timer
#define MAX_PLD_SZ_MAX_TMR              600 // set to 1 minute

// Max Payload Size Flags
#define MAX_PLD_SZ_START_CHECK          2
#define MAX_PLD_SZ_STOP_CHECK           3

// MR Watchdog Related
#define MR_WD_TIME_LAPSE_DIFF           3


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------

typedef struct GenCmdFrmStruct GEN_CMD_FRM_STRUCT;
typedef struct GenRspFrmStruct GEN_RSP_FRAME_STRUCT;
typedef void (*IOP_PRCS_CMD_FN)(GEN_CMD_FRM_STRUCT *);
typedef void (*MRDMA_FN)(void *);


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct DevCacheConfigStruct
{
    union
    {
        struct _CacheConfigStruct
        {
            unsigned long ReadCacheDisable          : 1; // Byte 2 Bit 0
            unsigned long MultiplicationFactor      : 1; //        Bit 1
            unsigned long WriteCacheDisable         : 1; //        Bit 2
            unsigned long SizeEn                    : 1; //        Bit 3
            unsigned long Discontinuity             : 1; //        Bit 4
            unsigned long CacheAnalysisPermitted    : 1; //        Bit 5
            unsigned long AbortPrefetch             : 1; //        Bit 6
            unsigned long InitiatorControl          : 1; //        Bit 7
            unsigned long Rsv                       :24;
        } Cache;
        unsigned long Value;
    } Union;
} DEV_CACHE_CONFIG_STRUCT;

typedef struct DevJumperConfigStruct
{
    unsigned long EnableWriteProtect    : 1;
    unsigned long HwWriteProtect        : 1;
    unsigned long                       :30;
} DEV_JUMPER_CONFIG_STRUCT;

typedef struct DeviceStatusStruct
{
    unsigned long Format : 8;
    unsigned long Rsv    :24;
} DEVICE_STATUS_STRUCT;

typedef struct HostBaseAddStruct
{
    unsigned long RingLow;
    unsigned long RingHigh;
    unsigned long DataLow;
    unsigned long DataHigh;
} HOST_BASE_ADD_STRUCT;

typedef struct ExtLbaInfoStruct
{
    unsigned long IsEnabled;    // Indicates if ext lba is enabled
    unsigned long Percentage;
    unsigned long IsFreshBuild; // Indicates if drive came from rebuilding
} EXT_LBA_INFO_STRUCT;

typedef struct DevConfigParmStruct
{
    // Dev lba related
    unsigned long long          DevLastLba;
    unsigned long               DevLbaSize;
    unsigned long               DevSxnSize;
    unsigned long               FbxCount;

    DEVICE_STATUS_STRUCT        DevFormatStatus;
    unsigned long               DevSanitStat;
    unsigned long               DevProtCacheStatus;

    DEV_CACHE_CONFIG_STRUCT     DevCacheStatus;
    DEV_JUMPER_CONFIG_STRUCT    DevJumpConfig;

    unsigned long               PowerOnCondition;

    EXT_LBA_INFO_STRUCT         ExtLbaInfo;
    HOST_BASE_ADD_STRUCT        Host;
    unsigned long               DetectPhase;
    unsigned long               LastRspSqnIdx;
    unsigned long               LastCmdSqnIdx;
    unsigned long               ReadRetryCount;
    unsigned long               RWSystemReady;
    unsigned long long          LbaPattern;
    unsigned long               LastRspDataSqnIdx;
    unsigned long               CurrRspGetIdx;
    unsigned long               LastMbCmdIdx;

// Note: Add new members above this comment,
//       then subtract number of words used from the Reserved
//       (Driver and FW must always have the same struct alignment)

    unsigned long               Reserved[35];
    unsigned long               Signature;
} DEV_CONFIG_PARM_STRUCT;

//----------------
// Command Frames
//----------------
typedef struct ProfileStruct
{
    unsigned long Rsvd      : 6;
    unsigned long DmIdx     : 2;
    unsigned long BuffIdx   :16;
    unsigned long OpCode    : 8;
} PROFILE_STRUCT;

typedef struct GenCmdFrmStruct
{
    unsigned long Word[16];
} GEN_CMD_FRM_STRUCT;

typedef struct UserCmdFrmStruct
{
    unsigned long Profile;
    unsigned long HostAddrHigh;
    unsigned long HostAddrLow;
    unsigned long BuffAddr;
    unsigned long CntNSize;
    unsigned long SxnOfsts;
    unsigned long LbaCnts;
    unsigned long SxnIdx[8];
    unsigned long FbxIdxs;
} USER_CMD_FRM_STRUCT;

typedef struct ScsiCmdFrameStruct
{
    unsigned long Profile;
    unsigned long ByteSz;
    unsigned long HostAddrHi;
    unsigned long HostAddrLo;
    unsigned long SenseBuffLen;
    unsigned long SenseBuffAddrHi;
    unsigned long SenseBuffAddrLo;
    unsigned char Cdb[32];
    unsigned long Filler;
} SCSI_CMD_FRAME_STRUCT;

typedef struct GenRspFrmStruct
{
    unsigned long Word[8];
} GEN_RSP_FRM_STRUCT;

typedef struct ReadCmdInfoStruct
{
    unsigned short SectionStat;
    unsigned short DmaSize;
    unsigned long ErrorBmp[2];
} READ_CMD_INFO_STRUCT;

typedef struct WriteCmdInfoStruct
{
    unsigned short Phase;
    unsigned short IoDeployCnt;
    unsigned short FbxIdx;
    unsigned short ValidSxnCnt;
    unsigned long UsrSxn[4];
    unsigned long LbaCnts;
    unsigned long LbaOffsets;
} WRITE_CMD_INFO_STRUCT;

typedef union CmdInfoUnion
{
    READ_CMD_INFO_STRUCT RdCmdInfo;
    WRITE_CMD_INFO_STRUCT WrCmdInfo;
} CMD_INFO_UNION;

typedef struct IopCmdInfoStruct
{
    unsigned short InUse;
    unsigned short DmxDeployCnt;
    unsigned long Profile;
    unsigned long Filler[6];
    CMD_INFO_UNION CmdInfo;
} IOP_CMD_INFO_STRUCT;

typedef struct MrDmaParmStruct
{
    unsigned long UsedEngine;
    unsigned long PutCntLoc;
    unsigned long RspCntLoc;
    unsigned long RcvCmdMax;
    unsigned long RspCntInf;
    unsigned long HostHighBase;
    unsigned long HostCmdFrmBase;
    unsigned long HostRspFrmBase;
    unsigned long HostDataBase;
    unsigned long DevCmdFrmBase;
    unsigned long DevRspFrmBase;
    GEN_CMD_FRM_STRUCT *DevCmdFrmPtr;
    GEN_RSP_FRM_STRUCT *DevRspFrmPtr;
    UTIL_SLL_STRUCT MrDmaQue;
    unsigned long MaxPayloadSize;
    unsigned long FourByteShiftCnt;
} MRDMA_PARM_STRUCT;

typedef struct FwHeaderStruct
{
    unsigned long FwSize;
    unsigned long Checksum;
    unsigned long BlockCnt;
    unsigned long HeaderStart;
} FW_HEADER_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define MR_POST_TO_WATCHDOG(Channel) MrChWdTimer[Channel] = _MrRawTmrClk


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

#pragma BSS(".dccm_io")
extern MRDMA_PARM_STRUCT MrDmaParm;

// MR Watchdog
extern unsigned long _MrRawTmrClk;
extern unsigned long MrRecoverFlag;
extern unsigned long MrChWdTimer[4];
extern void (*MrTmrCntlFnPtr)(void);
#pragma BSS()

#pragma BSS(".sram")
extern IOP_CMD_INFO_STRUCT IopCmdInfo[BUFFER_CNT];
#pragma BSS()


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

PCB_STRUCT *iop_post_mrlar (void);

void iop_activate_mrlar (void);

void iop_main (void);

void iop_prepare_read_rsp (PCB_STRUCT *PcbPtr);

void iop_process_data_rmw (unsigned long BuffIdx);

void iop_prepare_write_rsp (unsigned long Profile);

void iop_msg_read_error_reply (unsigned long *PayloadPtr);

void iop_msg_rmw_unmapped_reply (unsigned long *PayloadPtr);

void pecore_crit_int_handler(void);

// Local Function Prototypes

void iop_init_parm (void);

void iop_detect_max_payload_size (void);

void iop_init_max_payload_size (PCB_STRUCT *PcbPtr);

void iop_check_max_payload_size_done (void);

unsigned long iop_read_max_payload_size_flag (void);

void iop_check_bar (PCB_STRUCT *PcbPtr);

void iop_init_internal_regs (void);

void iop_init_config_regs (void);

void iop_init_bm (void);

void iop_init_enable_host_cfg (void);

void iop_init_ring (void);

void iop_ucode_init (void);

void iop_init (void);

void iop_serv_mrlar (void);

void iop_prcs_cmd_read_bulk (GEN_CMD_FRM_STRUCT *FramePtr);

void iop_prcs_cmd_write_bulk (GEN_CMD_FRM_STRUCT *FramePtr);

void iop_prcs_cmd_rmw_bulk (GEN_CMD_FRM_STRUCT *FramePtr);

void iop_prcs_cmd_check_system_stat (GEN_CMD_FRM_STRUCT *FramePtr);

void iop_prcs_cmd_shutdown (GEN_CMD_FRM_STRUCT *FramePtr);

void iop_prcs_cmd_ucode (GEN_CMD_FRM_STRUCT *FramePtr);

void iop_prcs_non_user_cmd (GEN_CMD_FRM_STRUCT *FramePtr);

void iop_prcs_cmd_err_gross (GEN_CMD_FRM_STRUCT *FramePtr);

BIT_STAT iop_check_sys_ready (void);

void iop_process_cmd (PCB_STRUCT *PcbPtr);

void iop_process_data_write (PCB_STRUCT *PcbPtr);

void iop_process_data_rmw_read (unsigned long BuffIdx);

void iop_process_data_rmw_write (PCB_STRUCT *PcbPtr);

void iop_non_usr_rsp_phase (PCB_STRUCT *PcbPtr);

void iop_prepare_rsp (GEN_RSP_FRAME_STRUCT *RspFrmPtr);

void iop_process_rsp (PCB_STRUCT *PcbPtr);

void mr_recovery_watchdog (void);

void mr_recover_init (void);

void mr_recovery_start_recover (void);


#endif
//=============================================================================
// $Log: Iop.h,v $
// Revision 1.6  2014/04/30 15:05:13  rcantong
// 1. DEV: Detection of max payload for Dell server
// 1.1 Added timing process in getting max payload size - ROrcullo
// 2. DEV: Support MRLAR hang detection
// 2.1 Added process for detecting MRLAR hang - JFaustino
//
// Revision 1.5  2014/02/02 08:49:05  rcantong
// 1. DEV: Support mode select and bit specific config commands
// 1.1 Added handling of mode select and bit specific config commands
//
// Revision 1.4  2014/01/08 12:42:57  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:08  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:04  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
