//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Dm/Dm.c,v $
// $Revision: 1.6 $
// $Author: rcantong $
// $Date: 2014/05/13 13:19:51 $
// $Id: Dm.c,v 1.6 2014/05/13 13:19:51 rcantong Exp $
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

#include "Bios.h"
#include "Dm.h"
#include "DmCommon.h"
#include "Interrupt.h"
#include "IpCall.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

DM_CONFIG_STRUCT DmConfig;
L1CACHE_ALIGN(DmConfig);

DM_PARM_STRUCT DmParm;
L1CACHE_ALIGN(DmParm);

DM_FLAG_PARM_STRUCT DmFlagParm;
L1CACHE_ALIGN(DmFlagParm);

#pragma BSS(".dccm_dm")
DM_FBX_STRUCT DmFbx[FBX_CNT];

// This will help the cd flush manager to check all the
// fbx which belongs to the local CPU
DM_FBX_STRUCT *LocalFbxPtr[FBX_CNT];
unsigned long CdFlushTmrCnt;
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : dm_fill_random_pattern
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_fill_random_pattern (void *TgtPtr,
                             unsigned long ByteLength)
{
    volatile unsigned long *MemPtr;
    unsigned long MaxIdx;
    unsigned long Idx;
    unsigned long Key[4];

    MemPtr = TgtPtr;
    MaxIdx = ByteLength / 8;

    for (Idx = 0;
         Idx < MaxIdx;
         Idx++)
    {
        Key[0] = KEY1_OFFSET + GET_RANDOM_PATTERN();
        Key[1] = KEY2_OFFSET + GET_RANDOM_PATTERN();
        Key[2] = KEY3_OFFSET + GET_RANDOM_PATTERN();
        Key[3] = KEY4_OFFSET + GET_RANDOM_PATTERN();

        dm_scramble_data(MemPtr,
                         Key,
                         INVALID_MASK);

        MemPtr += 2;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_fill_scramble_pattern
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_fill_scramble_pattern (void *TgtPtr,
                               unsigned long ByteLength)
{
    volatile unsigned long *MemPtr;
    unsigned long MaxIdx;
    unsigned long Idx;
    unsigned long Key[4];

    MemPtr = TgtPtr;
    MaxIdx = ByteLength / 8;

    Key[0] = KEY1_OFFSET;
    Key[1] = KEY2_OFFSET;
    Key[2] = KEY3_OFFSET;
    Key[3] = KEY4_OFFSET;

    for (Idx = 0;
         Idx < MaxIdx;
         Idx++)
    {
        MemPtr[0] = 0;
        MemPtr[1] = 0;

        dm_scramble_data(MemPtr,
                         Key,
                         0);

        Key[0]++;
        Key[1]++;
        Key[2]++;
        Key[3]++;

        MemPtr += 2;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_scramble_data
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_scramble_data (volatile unsigned long *DataPtr,
                       unsigned long *KeyPtr,
                       unsigned long Mask)
{
    unsigned long D0 = DataPtr[0];
    unsigned long D1 = DataPtr[1];
    unsigned long Sum = 0;
    unsigned long Delta = 0x9E3779B9; // a key schedule constant
    unsigned long Loop = 32;

    while (Loop-- > 0)
    {
        // basic cycle start
        Sum += Delta;
        D0 += ((D1 << 4) + KeyPtr[0]) ^ (D1 + Sum) ^ ((D1 >> 5) + KeyPtr[1]);
        D1 += ((D0 << 4) + KeyPtr[2]) ^ (D0 + Sum) ^ ((D0 >> 5) + KeyPtr[3]);
    }

    DataPtr[0] = D0 | Mask;
    DataPtr[1] = D1 | Mask;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_notify_completion
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_notify_completion (PCB_STRUCT *PcbPtr)
{
    PCB_STRUCT *ParentPcbPtr;

    ParentPcbPtr = PcbPtr->Word.FbxPtr->ParentPcbPtr;

    ParentPcbPtr->Info.DmInit.DeployCtr--;

    if (ParentPcbPtr->Info.DmInit.DeployCtr == 0)
    {
        SCHED_POST_PCB(ParentPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_master_send_cmd
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_master_send_cmd (SCHED_FN Fn)
{
    unsigned long FbxIdx;
    unsigned long DmId;
    IPCALL_STRUCT *IpCallPtr;

    DmParm.MasterPcbPtr->Word.FbxIdx = FBX_CNT;

    for (FbxIdx = 0;
         FbxIdx < FBX_CNT;
         FbxIdx++)
    {
        DmId = FbxToDmId[FbxIdx];
        IpCallPtr = ipcall_add_slave_entry(DmId);
        IpCallPtr->Fn = dm_slave_recv_cmd;
        IpCallPtr->Arg[0] = (unsigned long)Fn;
        IpCallPtr->Arg[1] = FbxIdx;
        ipcall_slave_exec_calls(DmId);
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : dm_slave_recv_cmd
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_slave_recv_cmd (unsigned long *PayloadPtr)
{
    PCB_STRUCT *PcbPtr;
    SCHED_FN Fn;

    PcbPtr = _sched_get_pcb();
    Fn = (SCHED_FN)PayloadPtr[0];
    PcbPtr->Word.FbxIdx = PayloadPtr[1];

    DmFbx[PcbPtr->Word.FbxIdx].ParentPcbPtr = PcbPtr;
    PcbPtr->Fn = dm_slave_send_rsp;
    Fn(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_slave_send_rsp
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_slave_send_rsp (PCB_STRUCT *PcbPtr)
{
    IPCALL_STRUCT *IpCallPtr;

    IpCallPtr = ipcall_add_master_entry();
    IpCallPtr->Fn = dm_master_recv_rsp;
    IpCallPtr->Arg[0] = PcbPtr->Word.FbxIdx;
    ipcall_master_exec_calls();

    _sched_return_pcb(PcbPtr);
}


//-----------------------------------------------------------------------------
// Function    : dm_master_recv_rsp
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_master_recv_rsp (unsigned long *PayloadPtr)
{
    DmParm.MasterPcbPtr->Word.FbxIdx--;

    if (DmParm.MasterPcbPtr->Word.FbxIdx == 0)
    {
        SCHED_POST_PCB(DmParm.MasterPcbPtr);
    }

    return;
}


//=============================================================================
// $Log: Dm.c,v $
// Revision 1.6  2014/05/13 13:19:51  rcantong
// 1. DEV: Support background scrubber
// 1.1 Added background scrubber process - PPestano
//
// Revision 1.5  2014/04/30 15:28:17  rcantong
// 1. BUGFIX: Enhanced scrambler pattern
// 1.1 Used tiny encryption algo to generate scrambler pattern
//
// Revision 1.4  2013/12/05 13:06:34  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:44:23  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:16  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
