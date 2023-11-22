//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Ipi.c,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2013/08/08 16:44:22 $
// $Id: Ipi.c,v 1.2 2013/08/08 16:44:22 rcantong Exp $
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


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : arc_service_ipirq
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void IRQ_FN arc_service_ipirq (void)
{
    unsigned long CpuId;
    unsigned long IntAckReg;

    CpuId = BiosParm.CpuId;
    IntAckReg = IC_CPU0_IN9_IVR_PTR[CpuId * 0x14];

    IC_IPIR0_PTR[CpuId] = 0;
    ipcall_service_calls();

    IntAckReg = IC_CPU0_IN9_IIRR_PTR[CpuId * 0x11];

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: Ipi.c,v $
// Revision 1.2  2013/08/08 16:44:22  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:09  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
