//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Interface/IopMrlar.c,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2014/04/30 15:11:03 $
// $Id: IopMrlar.c,v 1.2 2014/04/30 15:11:03 rcantong Exp $
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
#include "Err.h"
#include "Iop.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dccm_io")
unsigned long MrRecoverFlag;
unsigned long MrChWdTimer[4];
unsigned long TmrTick;
unsigned long _MrRawTmrClk;
void (*MrTmrCntlFnPtr)(void);
#pragma BSS()

volatile unsigned long MrTotalResets;


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : mr_recover_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void mr_recover_init (void)
{
    // This must run only in Core 0

    MrTotalResets = 0;
    TmrTick = 0;
    _MrRawTmrClk = 0;
    MrRecoverFlag = 0;

    // Timers per MR Engine Channel
    MrChWdTimer[0] = 0;
    MrChWdTimer[1] = 0;
    MrChWdTimer[2] = 0;
    MrChWdTimer[3] = 0;

    MrTmrCntlFnPtr = &mr_recovery_watchdog;

    return;
}


//-----------------------------------------------------------------------------
// Function    : mr_recovery_watchdog
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void mr_recovery_watchdog (void)
{
    unsigned long InUsedEngine;
    unsigned long Idx;

    if (    (TmrTick != 100)
         || MrRecoverFlag)
    {
        TmrTick++;
        return;
    }

    TmrTick = 0;
    _MrRawTmrClk++;

    InUsedEngine = MrDmaParm.UsedEngine;

    for (Idx = 0;
         Idx < 4;
         Idx++)
    {
        if (InUsedEngine & (1 << Idx))
        {
            if ((_MrRawTmrClk - MrChWdTimer[Idx]) >= MR_WD_TIME_LAPSE_DIFF)
            {
                mr_recovery_start_recover();
            }
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : mr_recovery_start_recover
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void mr_recovery_start_recover (void)
{
    // No handler yet!
    err_gross();

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: IopMrlar.c,v $
// Revision 1.2  2014/04/30 15:11:03  rcantong
// 1. DEV: Support MRLAR hang detection
// 1.1 Added process for detecting MRLAR hang - JFaustino
//
// Revision 1.1  2013/07/15 17:54:17  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
