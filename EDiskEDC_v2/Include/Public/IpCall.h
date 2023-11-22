//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Public/IpCall.h,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2013/08/08 16:42:06 $
// $Id: IpCall.h,v 1.2 2013/08/08 16:42:06 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__IPCALL_H__)
#define __IPCALL_H__

#if defined(DEBUG)
_Inline void ipcall_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define FIFO_CNT                        256


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------

typedef void (*IPCALL_FN)(unsigned long *);


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct IpCallFifoCtrlStruct
{
    unsigned long Idx; // both put and get at the same time
} IPCALL_FIFO_CTRL_STRUCT;

typedef struct IpCallStruct
{
    IPCALL_FN Fn;
    unsigned long Arg[15];
} IPCALL_STRUCT;

typedef struct IpCallFifoStruct
{
    IPCALL_STRUCT Entry[FIFO_CNT];
} IPCALL_FIFO_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

#pragma BSS(".ipcall")
extern IPCALL_FIFO_STRUCT IpCallFifo[DM_CNT][2];
extern unsigned long FifoSharedIdx[DM_CNT][2];
#pragma BSS()

#pragma BSS(".dccm")
extern IPCALL_FIFO_CTRL_STRUCT MasterCtrl[DM_CNT][2];
extern IPCALL_FIFO_CTRL_STRUCT SlaveCtrl[2];
#pragma BSS()


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void ipcall_init (void);


//-----------------------------------------------------------------------------
// Function    : ipcall_add_slave_entry
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline IPCALL_STRUCT *ipcall_add_slave_entry (unsigned long DmId)
{
    unsigned long PutIdx;
    IPCALL_STRUCT *EntryPtr;

    ASSERT(DmId < 3);
    PutIdx = MasterCtrl[DmId][0].Idx;
    EntryPtr = &IpCallFifo[DmId][0].Entry[PutIdx];
    MasterCtrl[DmId][0].Idx = (PutIdx + 1) & (FIFO_CNT - 1);

    return EntryPtr;
}


//-----------------------------------------------------------------------------
// Function    : ipcall_add_master_entry
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline IPCALL_STRUCT *ipcall_add_master_entry (void)
{
    unsigned long DmId;
    unsigned long PutIdx;
    IPCALL_STRUCT *EntryPtr;

    DmId = BiosParm.DmId;
    PutIdx = SlaveCtrl[1].Idx;
    EntryPtr = &IpCallFifo[DmId][1].Entry[PutIdx];
    SlaveCtrl[1].Idx = (PutIdx + 1) & (FIFO_CNT - 1);

    return EntryPtr;
}


//-----------------------------------------------------------------------------
// Function    : ipcall_slaves_exec_calls
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void ipcall_slaves_exec_calls (void)
{
    FifoSharedIdx[0][0] = MasterCtrl[0][0].Idx;
    *IC_IPIR1_PTR = 1;

    FifoSharedIdx[1][0] = MasterCtrl[1][0].Idx;
    *IC_IPIR2_PTR = 1;

    FifoSharedIdx[2][0] = MasterCtrl[2][0].Idx;
    *IC_IPIR3_PTR = 1;

    return;
}


//-----------------------------------------------------------------------------
// Function    : ipcall_slave_exec_calls
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void ipcall_slave_exec_calls (unsigned long DmId)
{
    ASSERT(DmId < 3);
    FifoSharedIdx[DmId][0] = MasterCtrl[DmId][0].Idx;
    IC_IPIR1_PTR[DmId] = 1;
    return;
}


//-----------------------------------------------------------------------------
// Function    : ipcall_master_exec_calls
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void ipcall_master_exec_calls (void)
{
    unsigned long DmId;

    DmId = BiosParm.DmId;
    FifoSharedIdx[DmId][1] = SlaveCtrl[1].Idx;
    *IC_IPIR0_PTR = 0x100 << (DmId << 3);

    return;
}


//-----------------------------------------------------------------------------
// Function    : ipcall_slave_process_calls
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void ipcall_slave_process_calls (unsigned long DmId)
{
    IPCALL_STRUCT *IpCallBasePtr;
    unsigned long PutIdx;
    unsigned long GetIdx;
    IPCALL_STRUCT *IpCallPtr;

    IpCallBasePtr = IpCallFifo[DmId][0].Entry;
    PutIdx = FifoSharedIdx[DmId][0];

    GetIdx = SlaveCtrl[0].Idx;

    while (GetIdx != PutIdx)
    {
        IpCallPtr = &IpCallBasePtr[GetIdx];
        IpCallPtr->Fn(&IpCallPtr->Arg[0]);
        GetIdx = (GetIdx + 1) & (FIFO_CNT - 1);
    }

    SlaveCtrl[0].Idx = GetIdx;

    return;
}


//-----------------------------------------------------------------------------
// Function    : ipcall_master_process_calls
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void ipcall_master_process_calls (unsigned long DmId)
{
    IPCALL_STRUCT *IpCallBasePtr;
    unsigned long PutIdx;
    unsigned long GetIdx;
    IPCALL_STRUCT *IpCallPtr;

    IpCallBasePtr = IpCallFifo[DmId][1].Entry;
    PutIdx = FifoSharedIdx[DmId][1];

    GetIdx = MasterCtrl[DmId][1].Idx;

    while (GetIdx != PutIdx)
    {
        IpCallPtr = &IpCallBasePtr[GetIdx];
        IpCallPtr->Fn(&IpCallPtr->Arg[0]);
        GetIdx = (GetIdx + 1) & (FIFO_CNT - 1);
    }

    MasterCtrl[DmId][1].Idx = GetIdx;

    return;
}


//-----------------------------------------------------------------------------
// Function    : ipcall_service_calls
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void ipcall_service_calls (void)
{
    unsigned long DmId;

    DmId = BiosParm.DmId;

    // Slaves
    if (DmId <= DM_CNT)
    {
        ipcall_slave_process_calls(DmId);
        return;
    }

    // Master
    ipcall_master_process_calls(0);
    ipcall_master_process_calls(1);
    ipcall_master_process_calls(2);

    return;
}


#endif
//=============================================================================
// $Log: IpCall.h,v $
// Revision 1.2  2013/08/08 16:42:06  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:00  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
