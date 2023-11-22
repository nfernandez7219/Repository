//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Scrub/Scrub.c,v $
// $Revision: 1.4 $
// $Author: rcantong $
// $Date: 2014/05/19 04:58:14 $
// $Id: Scrub.c,v 1.4 2014/05/19 04:58:14 rcantong Exp $
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

#include "BlkRecord.h"
#include "CntlData.h"
#include "CntlDataCommon.h"
#include "Dm.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "FreeList.h"
#include "RemapUser.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "ScrubI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
PCB_ALIGN_STRUCT ScrubUdPcb[FBX_CNT];
L1CACHE_ALIGN(ScrubUdPcb);
#pragma BSS()

#pragma BSS(".usram")
unsigned char ScrubUdBuff[FBX_CNT][USER_SXN_SIZE];
L1CACHE_ALIGN(ScrubUdBuff);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : scrub_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void scrub_init_malloc (unsigned long FbxIdx)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_STRUCT *PcbPtr;

    FbxPtr = &DmFbx[FbxIdx];

    PcbPtr = &ScrubUdPcb[FbxIdx].Pcb;
    FbxPtr->ScrubUd.PcbPtr = PcbPtr;

    PcbPtr->Word.FbxPtr = FbxPtr;
    PcbPtr->Info.ScrubUd.Ongoing = OFF;
    PcbPtr->Info.ScrubUd.UserPba = CNTL_SEGMENTS_PER_FBX;
    PcbPtr->Info.DmxRead.BuffAddr = (unsigned long)&ScrubUdBuff[FbxIdx][0];
    PcbPtr->Info.DmxRead.SegCnt = SEGMENTS_PER_USER_SXN;

    return;
}


//-----------------------------------------------------------------------------
// Function    : scrub_start_scrubbing
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void scrub_start_scrubbing (DM_FBX_STRUCT *FbxPtr)
{
    PCB_STRUCT *PcbPtr;

    PcbPtr = FbxPtr->ScrubUd.PcbPtr;

    if (PcbPtr->Info.ScrubUd.Ongoing == OFF)
    {
        PcbPtr->Info.ScrubUd.Ongoing = ON;
        scrub_ud_chk_valid_sxn(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : scrub_ud_chk_valid_sxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void scrub_ud_chk_valid_sxn (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT ScrubPba;
    PBA_INT ScrubBlkPba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    ScrubPba = PcbPtr->Info.ScrubUd.UserPba;
    ScrubBlkPba = DM_ALIGN_TO(SEGMENTS_PER_BLK, ScrubPba);

    // Skip if bad blk, q blk or invalid sxn
    if (    (blkrecord_ud_check_bad_blk(FbxPtr, ScrubPba) != GOOD_BLK)
         || (blkrecord_ud_check_q_blk(FbxPtr, ScrubPba) != GOOD_BLK)
         || (blkrecord_ud_chk_valid_sxn(FbxPtr, ScrubPba) != ON)
         || (freelist_ud_compare_cur_pba(FbxPtr, ScrubBlkPba) == SAME))
    {
        scrub_ud_scrubbing_done(PcbPtr);
    }

    else
    {
        PcbPtr->Info.DmxRead.Pba = ScrubPba;
        PcbPtr->Fn = scrub_ud_read_done;
        dmx_ops_read(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : scrub_ud_read_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void scrub_ud_read_done (PCB_STRUCT *PcbPtr)
{
    if (PcbPtr->Info.DmxRead.Stat == DMX_OPS_ECC_CORRECTABLE)
    {
        // Set Quarantine Bit
        blkrecord_ud_set_q_blk(PcbPtr->Word.FbxPtr,
                               PcbPtr->Info.DmxRead.Pba);

        // Remap entire block
        remap_ud_add_to_remap_list(PcbPtr->Word.FbxPtr,
                                  PcbPtr->Info.DmxRead.Pba);
    }

    // For other condition let the normal user data read will handle it

    scrub_ud_scrubbing_done(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : scrub_ud_read_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void scrub_ud_scrubbing_done (PCB_STRUCT *PcbPtr)
{
    PcbPtr->Info.ScrubUd.UserPba += SEGMENTS_PER_USER_SXN;
    if (PcbPtr->Info.ScrubUd.UserPba >= SEGMENTS_PER_FBX)
    {
        PcbPtr->Info.ScrubUd.UserPba = CNTL_SEGMENTS_PER_FBX;
        if (PcbPtr->Word.FbxPtr->FbxIdx == 0)
        {
            DmFlagParm.ScrubCycle++;
        }
    }

    PcbPtr->Info.ScrubUd.Ongoing = OFF;

    return;
}


//=============================================================================
// $Log: Scrub.c,v $
// Revision 1.4  2014/05/19 04:58:14  rcantong
// 1. BUGFIX: Update scrub from LA-based to PA-based scrubber
// 1.1 Update Pcb for Scrub to be Pba based - PPestano
//
// Revision 1.3  2014/05/13 13:19:51  rcantong
// 1. DEV: Support background scrubber
// 1.1 Added background scrubber process - PPestano
//
// Revision 1.2  2013/08/08 16:44:25  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:18  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
