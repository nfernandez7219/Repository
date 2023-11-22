//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Sched.c,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2014/04/30 15:24:56 $
// $Id: Sched.c,v 1.3 2014/04/30 15:24:56 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================


//-----------------------------------------------------------------------------
// Standard Library Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Includes
//-----------------------------------------------------------------------------
#include "Bios.h"
#include "BitDefs.h"
#include "Util.h"
#include "Sched.h"
#include "SysConfig.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------
#pragma BSS(".dccm")
PCB_POOL_STRUCT PcbPool;
UTIL_SLL_STRUCT RunQueue;
PCB_STRUCT PcbBase[PCB_CNT];
#pragma BSS()

PCB_STRUCT TcbBase[CPU_CNT][TCB_CNT];
L1CACHE_ALIGN(TcbBase);


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : sched_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sched_init (void)
{
    unsigned long PcbIdx;
    PCB_STRUCT *PcbPtr;

    // Initialize state controllers
    util_sll_init(&PcbPool.PcbList[0]);
    util_sll_init(&PcbPool.PcbList[1]);
    util_sll_init(&RunQueue);

    // Initialize pcb
    util_init_pattern(PcbBase,
                      sizeof(PcbBase),
                      INIT_PATTERN_LO_VALUE);

    util_init_pattern(TcbBase[BiosParm.CpuId],
                      sizeof(TcbBase[0]),
                      INIT_PATTERN_LO_VALUE);

    // Get pcb base
    PcbPtr = &PcbBase[0];

    // Add each pcb to pcb pool
    for (PcbIdx = 0;
         PcbIdx < PCB_CNT;
         PcbIdx++)
    {
        util_sll_insert_at_tail(&PcbPtr->Link,
                                &PcbPool.PcbList[1]);

        PcbPtr++;
    }

    // Get tcb base
    PcbPtr = &TcbBase[BiosParm.CpuId][0];

    // Add each tcb to tcb pool
    for (PcbIdx = 0;
         PcbIdx < TCB_CNT;
         PcbIdx++)
    {
        util_sll_insert_at_tail(&PcbPtr->Link,
                                &PcbPool.PcbList[0]);

        PcbPtr++;
    }

    PcbPool.PcbCnt = PCB_CNT + TCB_CNT;

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: Sched.c,v $
// Revision 1.3  2014/04/30 15:24:56  rcantong
// 1. BUGFIX: Insufficient PCB
// 1.1 Added PCB counter and monitoring of low PCB count - MFenol
//
// Revision 1.2  2013/08/08 16:44:22  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:09  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
