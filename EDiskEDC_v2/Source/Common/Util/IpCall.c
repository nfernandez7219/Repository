//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Util/IpCall.c,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2013/08/08 16:44:23 $
// $Id: IpCall.c,v 1.2 2013/08/08 16:44:23 rcantong Exp $
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
#include "SysConfig.h"

#include "Bios.h"
#include "Interrupt.h"
#include "IpCall.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".ipcall")
IPCALL_FIFO_STRUCT IpCallFifo[DM_CNT][2];
unsigned long FifoSharedIdx[DM_CNT][2];
#pragma BSS()

#pragma BSS(".dccm")
IPCALL_FIFO_CTRL_STRUCT MasterCtrl[DM_CNT][2];
IPCALL_FIFO_CTRL_STRUCT SlaveCtrl[2];
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : ipcall_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void ipcall_init (void)
{
    if (BiosParm.CpuId == EDC_BOOT_CORE)
    {
        util_init_pattern(IpCallFifo,
                          sizeof(IpCallFifo),
                          INIT_PATTERN_LO_VALUE);

        // Master fifo init
        MasterCtrl[0][0].Idx = 0;
        MasterCtrl[0][1].Idx = 0;
        MasterCtrl[1][0].Idx = 0;
        MasterCtrl[1][1].Idx = 0;
        MasterCtrl[2][0].Idx = 0;
        MasterCtrl[2][1].Idx = 0;

        // Ipcall fifo init
        FifoSharedIdx[0][1] = 0;
        FifoSharedIdx[1][0] = 0;
        FifoSharedIdx[1][1] = 0;
        FifoSharedIdx[2][0] = 0;
        FifoSharedIdx[2][1] = 0;
    }
    else
    {
        // Slave fifo init
        SlaveCtrl[0].Idx = 0;
        SlaveCtrl[1].Idx = 0;
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: IpCall.c,v $
// Revision 1.2  2013/08/08 16:44:23  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:08  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
