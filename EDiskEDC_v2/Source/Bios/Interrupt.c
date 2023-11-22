//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Interrupt.c,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/04/30 15:26:46 $
// $Id: Interrupt.c,v 1.5 2014/04/30 15:26:46 rcantong Exp $
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

#include "Bios.h"
#include "DmxRegs.h"
#include "Dmx.h"
#include "DmxCommon.h"
#include "DmxRecovery.h"
#include "Interrupt.h"
#include "Iop.h"


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
// Function    : int_arc_isr
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void IRQ_FN int_arc_isr (void)
{
    unsigned long CpuId;
    unsigned long IrqIdx;
    unsigned long IsrTableIdx;
    volatile unsigned long *IrqVecPtr;
    volatile unsigned long *IrqAckPtr;

    CpuId = BiosParm.CpuId;
    IrqIdx = _lr(0x40A); // REG_ICAUSE1
    IrqIdx = IrqIdx - 5; // EDC_LOWEST_DEFINABLE_IRQ

    IrqVecPtr = &IC_CPU0_IN5_IVR_PTR[(CpuId * 0x14) + IrqIdx];
    IrqAckPtr = &IC_CPU0_IN5_IIRR_PTR[(CpuId * 0x11) + IrqIdx];

    // Handle all pending irqs
    while (1)
    {
        IsrTableIdx = *IrqVecPtr;

        if (IsrTableIdx == 0)
        {
            break;
        }

        // Do handling here
        if (IsrTableIdx == 0x3c)
        {
            pecore_crit_int_handler();
        }
        else if (IsrTableIdx == 0x1c)
        {
            if ((*DMX_ISUM_PTR & DMX_CLEAR_DM_INTRPT) == DMX_CLEAR_DM_INTRPT)
            {
                msg_dmx_recovery_mstr_start(BIT_NULL_PTR);
            }
        }
        else
        {
            _brk();
        }

        // Acknowledge irq in iccore layer
        IsrTableIdx = *IrqAckPtr;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : int_arc_isr_normal
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void IRQ_FN int_arc_isr_normal (void)
{
    unsigned long IsrTableIdx;

    IsrTableIdx = *IC_CPU0_IN21_IVR_PTR;

    // Do handling here

    // Acknowledge irq in iccore layer
    IsrTableIdx = *IC_CPU0_IN21_IIRR_PTR;

    return;
}


//-----------------------------------------------------------------------------
// Function    : int_arc_isr_pg
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void IRQ_FN int_arc_isr_pg (void)
{
    unsigned long IsrTableIdx;

    IsrTableIdx = *IC_CPU0_IN13_IVR_PTR;

    // Do handling here

    // Acknowledge irq in iccore layer
    IsrTableIdx = *IC_CPU0_IN13_IIRR_PTR;

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: Interrupt.c,v $
// Revision 1.5  2014/04/30 15:26:46  rcantong
// 1. DEV: DMX recovery when critical interrupt occur
// 1.1 Added msg_dmx_recovery_mstr_start in dmx_crit_handler - JFaustino
//
// Revision 1.4  2014/03/03 12:54:12  rcantong
// 1. DEV: DMX critical interrupt handler
// 1.1 Added call to DMX_RECOVER - JFaustino
//
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
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
