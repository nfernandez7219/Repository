//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Dm/DmBuild.c,v $
// $Revision: 1.6 $
// $Author: rcantong $
// $Date: 2014/05/19 05:12:37 $
// $Id: DmBuild.c,v 1.6 2014/05/19 05:12:37 rcantong Exp $
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

#include "BaseBlk.h"
#include "BlkInfo.h"
#include "Dm.h"
#include "DmCommon.h"
#include "Defects.h"
#include "Dir.h"
#include "EdcFwVersion.h"
#include "Led.h"
#include "SxnInfo.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "DmBuildI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : dm_scanning
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_scanning (PCB_STRUCT *PcbPtr)
{
    DmFlagParm.SystemStat = SYSTEM_BUILD;

    LED_START_SCAN_DEFECTS();
    dm_master_send_cmd(defects_scan_mfg_fbx);
    PcbPtr->Fn = dm_build_flush_scan_mfg_defects;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_build
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_build (PCB_STRUCT *PcbPtr)
{
    DmFlagParm.SystemStat = SYSTEM_BUILD;
    dm_build_dir(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : dm_build_flush_scan_mfg_defects
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_build_flush_scan_mfg_defects (PCB_STRUCT *PcbPtr)
{
    DmFlagParm.BuildFlag = BUILD_SCAN_MFG_DONE;
    dm_master_send_cmd(defects_build_fbx);
    PcbPtr->Fn = dm_build_thorough_scan_fbx;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_build_thorough_scan_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_build_thorough_scan_fbx (PCB_STRUCT *PcbPtr)
{
    PcbPtr->Info.Screening.ThoroughScanLoopCnt = 0;
    dm_master_send_cmd(defects_thorough_scan_erase_fbx);
    PcbPtr->Fn = dm_build_thorough_scan_write_fbx;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_build_thorough_scan_write_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_build_thorough_scan_write_fbx (PCB_STRUCT *PcbPtr)
{
    dm_master_send_cmd(defects_thorough_scan_write_fbx);
    PcbPtr->Fn = dm_build_thorough_scan_read_fbx;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_build_thorough_scan_read_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_build_thorough_scan_read_fbx (PCB_STRUCT *PcbPtr)
{
    dm_master_send_cmd(defects_thorough_scan_read_fbx);
    PcbPtr->Fn = dm_build_thorough_scan_erase_fbx;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_build_thorough_scan_erase_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_build_thorough_scan_erase_fbx (PCB_STRUCT *PcbPtr)
{
    dm_master_send_cmd(defects_thorough_scan_erase_fbx);

    PcbPtr->Info.Screening.ThoroughScanLoopCnt++;
    if (PcbPtr->Info.Screening.ThoroughScanLoopCnt < 1)
    {
        // loop again
        PcbPtr->Fn = dm_build_thorough_scan_write_fbx;
    }
    else
    {
        PcbPtr->Fn = dm_build_flush_thorough_defects;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_build_flush_thorough_defects
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_build_flush_thorough_defects (PCB_STRUCT *PcbPtr)
{

    DmFlagParm.BuildFlag = BUILD_THOROUGH_DONE;
    dm_master_send_cmd(defects_build_fbx);
    PcbPtr->Fn = dm_build_dir;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_build_dir
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_build_dir (PCB_STRUCT *PcbPtr)
{
    LED_STOP_SEQUENCE();
    LED_START_BUILDING();
    
    DmFlagParm.SystemStat = SYSTEM_BUILD; 
    dm_master_send_cmd(dir_build_fbx);
    PcbPtr->Fn = dm_build_sxninfo;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_build_sxninfo
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_build_sxninfo (PCB_STRUCT *PcbPtr)
{
    dm_master_send_cmd(sxninfo_build_fbx);
    PcbPtr->Fn = dm_build_defects;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_build_defects
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_build_defects (PCB_STRUCT *PcbPtr)
{
    DmFlagParm.BuildFlag = BUILD_DONE;
    dm_master_send_cmd(defects_build_fbx);
    PcbPtr->Fn = dm_build_done;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_build_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_build_done (PCB_STRUCT *PcbPtr)
{
    _sched_return_pcb(PcbPtr);

    LED_STOP_SEQUENCE();

    // Building done indicator
    *LED_CNTLR_PTR = LED_GREEN_MASK
                   | LED_ORANGE_MASK;

    _brk();

    return;
}


//=============================================================================
// $Log: DmBuild.c,v $
// Revision 1.6  2014/05/19 05:12:37  rcantong
// 1. DEV: LED indicator
// 1.1 Added build done LED indicator - MManzo
//
// Revision 1.5  2014/04/30 13:53:35  rcantong
// 1. DEV: Support defects screening
// 1.1 Added process for thorough scanning - BBantigue
// 2. BUGFIX: Enhanced scrambler pattern
// 2.1 Used tiny encryption algo to generate scrambler pattern
//
// Revision 1.4  2014/02/02 09:26:36  rcantong
// 1. DEV: Support LED as status indicator
// 1.1 Added control of LEDs to indicate the device status
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
