//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Led.c,v $
// $Revision: 1.4 $
// $Author: rcantong $
// $Date: 2014/04/30 15:28:42 $
// $Id: Led.c,v 1.4 2014/04/30 15:28:42 rcantong Exp $
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
#include "Interrupt.h"
#include "Iop.h"
#include "Led.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dccm_io")
LED_PARM_STRUCT LedParm;
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : led_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void led_init (void)
{
    ASSERT(BiosParm.CpuId == 0);

    LedParm.SeqPtr = led_sequencer;
    LedParm.Curr = 0;
    LedParm.Mask = 0;
    LedParm.PutCntLoc = 0;

    // Turn off all LEDs
    *LED_CNTLR_PTR &= ~LED_ALL_MASK;

    // Start the local arc timer 1
    _sr(3, 0x101);
    _sr(100 * 400000, 0x102); // 100ms based on arc frequency
    _sr(0, 0x100);

    return;
}


//-----------------------------------------------------------------------------
// Function    : led_sequencer
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void led_sequencer (void)
{
    unsigned long LedState;

    // Make sure other non-LED PIOs are not affected
    LedParm.Curr = LedParm.Curr ^ LedParm.Mask;

    LedState = *LED_CNTLR_PTR;
    LedState &= ~(LED_ORANGE_MASK | LED_YELLOW_MASK | LED_GREEN_MASK);

    *LED_CNTLR_PTR = LedState | LedParm.Curr;

    return;
}


//-----------------------------------------------------------------------------
// Function    : led_host_access
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void led_host_access (void)
{
    unsigned long LedState;

    // Make sure other non-LED PIOs are not affected
    LedState = *LED_CNTLR_PTR;
    LedState &= ~(LED_ORANGE_MASK | LED_YELLOW_MASK | LED_GREEN_MASK);

    // Drive is active
    if (MrDmaParm.PutCntLoc != LedParm.PutCntLoc)
    {
        LedParm.PutCntLoc = MrDmaParm.PutCntLoc;
        LedParm.Curr = LedParm.Curr ^ LedParm.Mask;
        *LED_CNTLR_PTR = LedState | LedParm.Curr;
    }

    // Drive is in idle state
    else
    {
        *LED_CNTLR_PTR = LedState | LED_GREEN_MASK;
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: Led.c,v $
// Revision 1.4  2014/04/30 15:28:42  rcantong
// 1. DEV: Generic use of ARC timer 1
// 1.1 Set timer 1 interval to 100ms
//
// Revision 1.3  2014/02/02 09:17:38  rcantong
// 1. DEV: Support LED as status indicator
// 1.1 Added control of LEDs to indicate the device status
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
