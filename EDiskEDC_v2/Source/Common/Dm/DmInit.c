//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Dm/DmInit.c,v $
// $Revision: 1.9 $
// $Author: rcantong $
// $Date: 2014/05/19 05:21:06 $
// $Id: DmInit.c,v 1.9 2014/05/19 05:21:06 rcantong Exp $
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
#include "Bios.h"
#include "BlkInfo.h"
#include "BlkRecord.h"
#include "CdFlushMgr.h"
#include "CntlDataCommon.h"
#include "CntlData.h"
#include "Compact.h"
#include "Defects.h"
#include "Dir.h"
#include "Disturb.h"
#include "Dm.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "EdcFwVersion.h"
#include "Err.h"
#include "FreeList.h"
#include "Led.h"
#include "Media.h"
#include "Reclaim.h"
#include "RemapUser.h"
#include "Scrub.h"
#include "SxnInfo.h"
#include "NvConfig.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "DmInitI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
unsigned long ScrambleBase[0x100000];
L1CACHE_ALIGN(ScrambleBase);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : dm_init_config
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_config (void)
{
    // Fill scramble pattern
    dm_fill_scramble_pattern(ScrambleBase,
                             0x400000);

    // Initialize dm config
    DmConfig.FlashPageSize = FLASH_PAGE_SIZE;
    DmConfig.FlashBlkSize = FLASH_BLK_SIZE;
    DmConfig.Dir0SxnCnt = DIR0_SXN_CNT;
    DmConfig.Si0SxnCnt = SI0_SXN_CNT;
    DmConfig.SegmentsPerFbx = SEGMENTS_PER_FBX;
    DmConfig.BlksPerFbx = BLKS_PER_FBX;
    DmConfig.CntlSegmentsPerFbx = CNTL_SEGMENTS_PER_FBX;
    DmConfig.CntlBlksPerFbx = CNTL_BLKS_PER_FBX;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_domain
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_domain (void)
{
    unsigned long MapIdx;
    unsigned long FbxIdx;

    FbxIdx = BiosParm.DmId;

    // Initialize the local fbx map
    for (MapIdx = 0;
         MapIdx < (FBX_CNT / DM_CNT);
         MapIdx++)
    {
        LocalFbxPtr[MapIdx] = &DmFbx[FbxIdx];
        FbxIdx += DM_CNT;
    }

    LocalFbxPtr[MapIdx] = BIT_NULL_PTR;

    util_init_pattern(&DmFbx[0],
                      sizeof(DmFbx),
                      INIT_PATTERN_LO_VALUE);

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init (void)
{
    PCB_STRUCT *PcbPtr;

    PcbPtr = _sched_get_pcb();
    DmParm.MasterPcbPtr = PcbPtr;

    DmFlagParm.SystemStat = SYSTEM_INIT;
    dm_master_send_cmd(dm_init_malloc_fbx);
    PcbPtr->Fn = dm_init_blkinfo;

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : dm_init_blkinfo
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_blkinfo (PCB_STRUCT *PcbPtr)
{
    dm_master_send_cmd(blkinfo_init_fbx);
    PcbPtr->Fn = dm_init_check_blkinfo;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_check_blkinfo
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_check_blkinfo (PCB_STRUCT *PcbPtr)
{    
    unsigned long StatBmp;
    unsigned long FbxIdx;

    StatBmp = 0; 
    for (FbxIdx = 0; 
         FbxIdx < FBX_CNT; 
         FbxIdx++) 
    { 
        StatBmp |= (DmFlagParm.BIStat[FbxIdx] << (4 * FbxIdx)); 
    } 

    if (StatBmp != 0)
    {
        // No BlkInfo found. Build BlkInfo
        dm_master_send_cmd(blkinfo_build_fbx);
        PcbPtr->Fn = dm_init_defects;        
    }

    else
    {
        // BlkInfo found. Proceed to Defects initialization
        dm_init_defects(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_defects
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_defects (PCB_STRUCT *PcbPtr)
{
    dm_master_send_cmd(defects_init_fbx);
    PcbPtr->Fn = dm_init_check_defects;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_check_defects
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_check_defects (PCB_STRUCT *PcbPtr)
{
    unsigned long StatBmp;
    unsigned long FbxIdx;

    // Check defects list stat of each fbx
    StatBmp = 0;
    for (FbxIdx = 0;
         FbxIdx < FBX_CNT;
         FbxIdx++)
    {
        StatBmp |= (DmFlagParm.DefectsStat[FbxIdx] << (4 * FbxIdx));
    }

    // Evaluate StatBmp
    // No Defects
    if (FwVersion.EraseBoardFlag == 2)
    {
        dm_master_send_cmd(defects_erase_all_fbx);
        PcbPtr->Fn = dm_scanning;
    }

    else
    {
        // Build Done
        if (StatBmp == FBX_DONE_MSK)
        {
            if (FwVersion.EraseBoardFlag == ON)
            {
                dm_master_send_cmd(defects_erase_fbx);
                PcbPtr->Fn = dm_build_flush_thorough_defects;
            }

            else
            {
                dm_init_fetch_cntlhdr(PcbPtr);
            }
        }
        else if (StatBmp == FBX_NO_DEFECTS_MSK)
        {
            if (FwVersion.EraseBoardFlag == ON)
            {
                dm_scanning(PcbPtr);
             }

            else
            {
                // err_gross while waiting for soft reset
                // needs soft reset to lower ECC Threshold
                err_gross();
            }
        }
        // Scan Mfg Done Only
        else if (StatBmp == FBX_SCAN_MFG_MSK)
        {
            if (FwVersion.EraseBoardFlag == ON)
            {
                dm_build_thorough_scan_fbx(PcbPtr);
            }

            else
            {
                // err_gross while waiting for soft reset
                // needs soft reset to lower ECC Threshold
                err_gross();
            }
        }
        // Thorough Scan Done Only
        else if (StatBmp == FBX_THOROUGH_MSK)
        {
            if (FwVersion.EraseBoardFlag == ON)
            {
                dm_master_send_cmd(defects_erase_fbx);
                PcbPtr->Fn = dm_build;
            }

            else
            {
                // err_gross while waiting for soft reset
                // needs soft reset to lower ECC Threshold
                err_gross();
            }
        }
        // Partial built fbx is not supported
        else
        {
            err_gross();
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_fetch_cntlhdr
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_fetch_cntlhdr (PCB_STRUCT *PcbPtr)
{
    LED_START_CNTL_BLK_FETCH_SCRUB();
    dm_master_send_cmd(cntldata_fetch_fbx);
    PcbPtr->Fn = dm_init_scrub_dir1;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_scrub_dir1
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_scrub_dir1 (PCB_STRUCT *PcbPtr)
{
    dm_master_send_cmd(dir_scrub_dir1);
    PcbPtr->Fn = dm_init_scrub_si1;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_scrub_si1
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_scrub_si1 (PCB_STRUCT *PcbPtr)
{
    dm_master_send_cmd(sxninfo_scrub_si1);
    PcbPtr->Fn = dm_init_fetch_dir0;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_fetch_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_fetch_dir0 (PCB_STRUCT *PcbPtr)
{
    DmFlagParm.SystemStat = SYSTEM_SCRUB;
    dm_master_send_cmd(dir_fetch_dir0_fbx);
    PcbPtr->Fn = dm_init_scrub_vi;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_scrub_vi
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_scrub_vi (PCB_STRUCT *PcbPtr)
{
    dm_master_send_cmd(blkrecord_cd_init_entries);
    PcbPtr->Fn = dm_init_cd_reclaim;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_cd_reclaim
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_cd_reclaim (PCB_STRUCT *PcbPtr)
{
    LED_STOP_SEQUENCE();

    dm_master_send_cmd(reclaim_cd_init_reclaim_fbx);
    PcbPtr->Fn = dm_init_ud_reclaim;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_ud_reclaim
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_ud_reclaim (PCB_STRUCT *PcbPtr)
{
    dm_master_send_cmd(reclaim_ud_init_reclaim_fbx);
    PcbPtr->Fn = dm_init_ud_trigger_reclaim;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_ud_trigger_reclaim
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_ud_trigger_reclaim (PCB_STRUCT *PcbPtr)
{
    dm_master_send_cmd(reclaim_ud_init_trigger_reclaim);
    PcbPtr->Fn = dm_init_cdflushmgr;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_cdflushmgr
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_cdflushmgr (PCB_STRUCT *PcbPtr)
{
    // Fill SRAM with random data to prevent writing patterned data during rmw
    dm_fill_random_pattern((void *)0xE0100000,
                           0x300000);
    DmFlagParm.ScrubCycle = 0;
    dm_master_send_cmd(cdflushmgr_start_mgr_fbx);
    PcbPtr->Fn = dm_init_done;

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_done (PCB_STRUCT *PcbPtr)
{
    DmFlagParm.SystemStat = SYSTEM_READY;
    _sched_return_pcb(PcbPtr);

    LED_START_HOST_ACCESS();

    return;
}


//-----------------------------------------------------------------------------
// Function    : dm_init_malloc_fbx
// Description : Process by slave cores
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void dm_init_malloc_fbx (PCB_STRUCT *PcbPtr)
{
    unsigned long FbxIdx;

    FbxIdx = PcbPtr->Word.FbxIdx;
    DmFlagParm.FbxStat[FbxIdx] = FBX_INIT;

    DmFbx[FbxIdx].FbxIdx = FbxIdx;
    DmFbx[FbxIdx].DevCurPbaPtr = &DmParm.DevCurPba[FbxIdx][0];

    // Initialize fbx memory
    dmx_ops_init_malloc(FbxIdx);
    defects_init_malloc(FbxIdx);
    blkinfo_init_malloc(FbxIdx);
    dir_init_malloc(FbxIdx);
    sxninfo_init_malloc(FbxIdx);
    media_init_malloc(FbxIdx);
    cntldata_init_malloc(FbxIdx);
    blkrecord_cd_init_malloc(FbxIdx);
    blkrecord_ud_init_malloc(FbxIdx);
    freelist_cd_init_malloc(FbxIdx);
    freelist_ud_init_malloc(FbxIdx);
    reclaim_cd_init_malloc(FbxIdx);
    reclaim_ud_init_malloc(FbxIdx);
    compact_cd_init_malloc(FbxIdx);
    compact_ud_init_malloc(FbxIdx);
    remap_ud_init_malloc(FbxIdx);
    disturb_init_malloc(FbxIdx);
    cdflushmgr_init_malloc(FbxIdx);
    scrub_init_malloc(FbxIdx);
    nv_init_malloc(FbxIdx);

    SCHED_POST_PCB(PcbPtr);

    return;
}


//=============================================================================
// $Log: DmInit.c,v $
// Revision 1.9  2014/05/19 05:21:06  rcantong
// 1. BUGFIX: Support jumper rebuild for production board
// 1.1 Scanning without erase if production board - MManzo
//
// Revision 1.8  2014/05/13 13:19:51  rcantong
// 1. DEV: Support background scrubber
// 1.1 Added background scrubber process - PPestano
//
// Revision 1.7  2014/04/30 15:29:49  rcantong
// 1. BUGFIX: Enhanced scrambler pattern
// 1.1 Used tiny encryption algo to generate scrambler pattern
// 2. DEV: Support rebuild securing defects list
// 2.1 Update process flow to secure defects list - BBantigue
// 3. BUGFIX: Lack of freelist during power on
// 3.1 Triggered reclaim at end of dm init - MFenol
//
// Revision 1.6  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
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
