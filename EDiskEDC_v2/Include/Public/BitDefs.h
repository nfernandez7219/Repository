//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Public/BitDefs.h,v $
// $Revision: 1.6 $
// $Author: rcantong $
// $Date: 2014/05/19 05:45:12 $
// $Id: BitDefs.h,v 1.6 2014/05/19 05:45:12 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__BITDEFS_H__)
#define __BITDEFS_H__

#if defined(DEBUG)
_Inline void bitdefs_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define BIT_NULL_PTR                    (void *)0

#define SUCCESSFUL                      0
#define NOT_SUCCESSFUL                  1

#define PASSED                          0
#define FAILED                          1

#define OFF                             0
#define ON                              1

#define UNLOCKED                        0
#define LOCKED                          1

#define SAME                            0
#define NOT_SAME                        1

#define DCACHE_LINE_SIZE                32
#define BITS_PER_WORD                   32

#define IRQ_FN                          _CC(_NEVER_RETURNS)

#define BYTE_SHIFT                      8
#define TWO_BYTES_SHIFT                 16
#define THREE_BYTES_SHIFT               24

// Protocol related status
#define PROT_CMD_VALID                  SUCCESSFUL
#define PROT_CMD_INVALID                0x5000
#define PROT_CMD_DONE                   0x5001
#define PROT_CMD_BUSY                   0x5002
#define PROT_CMD_ILLEGAL                0x5003
#define PROT_CMD_INVALID_CDB_FIELD      0x5004
#define PROT_CMD_INVALID_LUN            0x5005
#define PROT_CMD_LBA_OUT_OF_RANGE       0x5006
#define PROT_CMD_PUA                    0x5007
#define PROT_CMD_FORMAT_IN_PROGRESS     0x5008
#define PROT_CMD_RESERVATIONS_CONFLICT  0x5009
#define PROT_CMD_OVERLAPPED_CMDS        0x500A
#define PROT_CMD_FORMAT_REQUIRED        0x500B
#define PROT_CMD_UNIT_BECOMING_READY    0x500C
#define PROT_CMD_IDLE                   0x500D
#define PROT_CMD_INCORRECT_BLK_LEN      0x500E
#define PROT_CMD_DATA_PHASE_ERROR       0x500F
#define PROT_CMD_DATA_LEN_MISMATCH      0x5010
#define PROT_CMD_WR_PROTECT_VIOLATION   0x5011
#define PROT_EXEC_SUCCESSFUL            SUCCESSFUL
#define PROT_EXEC_BUSY                  0x5101
#define PROT_EXEC_CONT                  0x5102
#define PROT_EXEC_DATA_PHASE_ERROR      SUCCESSFUL
#define PROT_EXEC_INVALID_PARM_FIELD    0x5104
#define PROT_MODES_VALID                0x5FFF
#define PROT_MODES_PROCESSED            0x5FFF
#define PROT_RSV_OUT_VALID              0x6000
#define PROT_RSV_RSRCS_VALID            0x6001
#define PROT_CMD_SELFTEST_IN_PROGRESS   0x6002
#define PROT_CMD_HW_ERR                 0x6003
#define PROT_CMD_START_STOP_IN_PROGRESS 0x6004
#define PROT_CMD_SEQ_ERROR              0x6005

// System Related Status
#define SYSTEM_NOT_READY                0x0C03

// IOC Related Status
#define IOC_DATA_DIR_ERR                0x3001
#define IOC_DATA_LEN_MISMATCH           0x3007

#define LED_CNTLR_PTR                   ((volatile unsigned long *)0xFFCE342C)
#define LED_RED_MASK                    (1 << 28)


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------

typedef void *BIT_HDL;
typedef unsigned long BIT_STAT;
typedef unsigned long PBA_INT;
typedef unsigned long IDENTITY_INT;
typedef unsigned long long BLK_INT;
typedef unsigned long long SQN_INT;


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define REG_WRITE(Addr, Data) (*((volatile unsigned long *)(Addr)) = Data)
#define REG_READ(Addr) (*(volatile unsigned long *)(Addr))

#if defined(DEBUG)
#define ASSERT(Cond)                       \
{                                          \
    if (!(Cond))                           \
    {                                      \
        while (1)                          \
        {                                  \
            *LED_CNTLR_PTR = LED_RED_MASK; \
            _brk();                        \
        }                                  \
    }                                      \
}
#else
#define ASSERT(Cond)
#endif

#define FW_INIT_CODE(Fn) pragma ALLOC_TEXT(".text_init", Fn)
#define FW_PERF_CODE(Fn) pragma ALLOC_TEXT(".text_perf", Fn)
#define L1CACHE_ALIGN(Obj) pragma ALIGN_TO(32, Obj)


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------


#endif
//=============================================================================
// $Log: BitDefs.h,v $
// Revision 1.6  2014/05/19 05:45:12  rcantong
// 1. DEV: LED indicator
// 1.1 Added lit red LED when ASSERT error occur
//
// Revision 1.5  2014/04/30 15:12:02  rcantong
// 1. DEV: Support constants for comparing result
// 1.1 Added SAME and NOT_SAME defines
//
// Revision 1.4  2013/12/05 13:06:33  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
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
