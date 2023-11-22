//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Arc700/ArcTimer.c,v $
// $Revision: 1.6 $
// $Author: rcantong $
// $Date: 2014/05/13 13:43:41 $
// $Id: ArcTimer.c,v 1.6 2014/05/13 13:43:41 rcantong Exp $
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
#include "CdFlushMgr.h"
#include "Compact.h"
#include "DmxCommon.h"
#include "DmxRecovery.h"
#include "Iop.h"
#include "Led.h"


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
// Function    : arc_timer0_int_hdl
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void IRQ_FN arc_timer0_int_hdl (void)
{
    if (BiosParm.CpuId == 0)
    {
        iop_main();
        MrTmrCntlFnPtr();
        _sr(3, 0x22);
    }
    else
    {
        TmrCntlFnPtr();
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : arc_timer1_int_hdl
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long Cnt1s = 0;
void IRQ_FN arc_timer1_int_hdl (void)
{
    if (BiosParm.CpuId == EDC_BOOT_CORE)
    {
        dmx_recovery_refresh();

        ASSERT(LedParm.SeqPtr != BIT_NULL_PTR);
        LedParm.SeqPtr();

        if (iop_read_max_payload_size_flag() == MAX_PLD_SZ_START_CHECK)
        {
            Cnt1s++;
            if (Cnt1s == MAX_PLD_SZ_MAX_TMR) // 1 min
            {
                iop_check_max_payload_size_done();
            }
        }
    }

    else
    {
        BIOS_INC_IDLE_TMR();
        if (BIOS_IDLE_TM_REACH())
        {
            // Trigger
            compact_ud_trigger_idle_cmpct_prcs();
        }

        arc_timer1_cdflushmgr();
    }

    _sr(3, 0x101);

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: ArcTimer.c,v $
// Revision 1.6  2014/05/13 13:43:41  rcantong
// 1. BUGFIX: Fid hang prevention by performing DMX reset
// 1.1 Added DMX refresher set every 5 min - JFaustino
//
// Revision 1.5  2014/04/30 15:21:59  rcantong
// 1. DEV: Support MRLAR Hang detection
// 1.1 Added MrTmrCntlFnPtr - JFaustino
// 2. DEV: Detection of max payload for Dell server
// 2.1 Added timing process in getting max payload size - ROrcullo
//
// Revision 1.4  2014/03/03 12:50:17  rcantong
// 1. DEV: FID hang handler
// 1.1 Added call to fid hang watchdog timer - JFaustino
//
// Revision 1.3  2014/02/02 09:12:19  rcantong
// 1. DEV: Support LED as status indicator
// 1.1 Added control of LEDs to indicate the device status
//
// Revision 1.2  2013/08/08 16:44:21  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:08  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
