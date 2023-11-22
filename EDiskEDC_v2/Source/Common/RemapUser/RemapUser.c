//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/RemapUser/RemapUser.c,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/05/19 05:09:21 $
// $Id: RemapUser.c,v 1.7 2014/05/19 05:09:21 rcantong Exp $
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
#include "Compact.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "Err.h"
#include "FreeList.h"
#include "Media.h"
#include "SxnInfo.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "RemapUserI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
PCB_ALIGN_STRUCT RemapUserPcb[FBX_CNT];
L1CACHE_ALIGN(RemapUserPcb);
#pragma BSS()

#pragma BSS(".dm_buffer")
unsigned char RemapUserReadBuff[FBX_CNT][FLASH_PAGE_SIZE];
L1CACHE_ALIGN(RemapUserReadBuff);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : remap_ud_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_ud_init_malloc (unsigned long FbxIdx)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_STRUCT *PcbPtr;

    FbxPtr = &DmFbx[FbxIdx];
    PcbPtr = (void *)&RemapUserPcb[FbxIdx];

    util_init_pattern(PcbPtr,
                      sizeof(RemapUserPcb[0]),
                      INIT_PATTERN_LO_VALUE);

    PcbPtr->Word.FbxPtr = FbxPtr;
    FbxPtr->RemapUd.RemapUdPcbPtr = PcbPtr;
    FbxPtr->RemapUd.BuffAddr = (unsigned long)&RemapUserReadBuff[FbxIdx][0];

    dm_fill_random_pattern((void *)FbxPtr->RemapUd.BuffAddr,
                           sizeof(RemapUserReadBuff[0]));

    return;
}


//-----------------------------------------------------------------------------
// Function    : remap_ud_add_to_remap_list
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_ud_add_to_remap_list (DM_FBX_STRUCT *FbxPtr,
                                 PBA_INT BlkPba)
{
    PCB_STRUCT *PcbPtr;

    BlkPba = DM_ALIGN_TO(SEGMENTS_PER_BLK, BlkPba);

    freelist_ud_chk_n_remove_cur_pba(FbxPtr,
                                     BlkPba);

    // Set blk to REMAP and add blk to remap list
    blkrecord_ud_set_remap_blk(FbxPtr,
                               BlkPba);

    PcbPtr = FbxPtr->RemapUd.RemapUdPcbPtr;

    if (PcbPtr->Info.RemapUd.ActiveFlag == OFF)
    {
        PcbPtr->Info.RemapUd.ActiveFlag = ON;

        PcbPtr->Info.DmxRead.BuffAddr
            = FbxPtr->RemapUd.BuffAddr
            + (USER_SXN_SIZE * (USER_SXNS_PER_PAGE - 1));

        PcbPtr->Info.RemapUd.RemapSxnCnt = 0;
        PcbPtr->Info.RemapUd.NxtPcbPtr = BIT_NULL_PTR;
        PcbPtr->Info.RemapUd.RemapPba = BlkPba;

        remap_ud_prcs_get_remap_blk(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : remap_ud_prcs_get_remap_blk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_ud_prcs_get_remap_blk (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_REMAP_UD_STRUCT *RemapUdPtr;
    PBA_INT RemapPba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    RemapUdPtr = (void *)&PcbPtr->Info;
    RemapPba = RemapUdPtr->RemapPba;

    while (1)
    {
        // Make sure we have remap pba
        if ((RemapPba % SEGMENTS_PER_BLK) == 0)
        {
            // Get remap blk from blk record
            RemapPba = blkrecord_ud_get_rmp_blk(FbxPtr);

            if (RemapPba == INVALID_MASK)
            {
                if (RemapUdPtr->RemapSxnCnt == 0)
                {
                    RemapUdPtr->ActiveFlag = OFF;
                    return;
                }
                else
                {
                    remap_ud_write_userdata(PcbPtr);
                    return;
                }
            }
        }

        // Get valid sxn within the remap blk
        RemapPba = blkrecord_ud_get_valid_sxn(FbxPtr,
                                              RemapPba);

        if (RemapPba != INVALID_MASK)
        {
            break;
        }
    }

    RemapUdPtr->RemapPba = RemapPba;

    remap_ud_read_sxninfo(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : remap_ud_read_userdata
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_ud_read_sxninfo (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_REMAP_UD_STRUCT *RemapUdPtr;
    BIT_STAT Stat;

    FbxPtr = PcbPtr->Word.FbxPtr;
    RemapUdPtr = (void *)&PcbPtr->Info;

    // Si0 alloc read
    Stat = sxninfo_alloc_read(FbxPtr,
                              RemapUdPtr->RemapPba,
                              &RemapUdPtr->CntlCachePtr);

    if (Stat <= CNTLDATA_INFLUSH)
    {
        remap_ud_read_userdata(PcbPtr);
    }
    else if (Stat == CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = remap_ud_read_userdata;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr, RemapUdPtr->CntlCachePtr);
    }
    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        PcbPtr->Fn = remap_ud_read_sxninfo;
        CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr, PcbPtr->Word.FbxPtr->SxnInfo);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : remap_ud_read_userdata
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_ud_read_userdata (PCB_STRUCT *PcbPtr)
{
    PCB_REMAP_UD_STRUCT *RemapUdPtr;
    volatile SI0_ENTRY_STRUCT *Si0EntryPtr;
    unsigned long Si0EntryIdx;
    unsigned long UserSxnIdx;

    RemapUdPtr = (void *)&PcbPtr->Info;
    Si0EntryPtr = (void *)RemapUdPtr->CntlCachePtr->DataAddr;
    Si0EntryIdx = RemapUdPtr->RemapPba / SEGMENTS_PER_USER_SXN;
    Si0EntryPtr += (Si0EntryIdx % SI0_ENTRIES_PER_SXN);

    UserSxnIdx = Si0EntryPtr->UserSxnIdx ^ SCRAMBLE_WORD(Si0EntryIdx);

    // Check if invalid sxn idx
    // in case the sxnidx is invalid get new pba for remap
    // Check usersxn lock
    // if locked, ignore this usersxn, other process will update this userdata
    if (    (UserSxnIdx >= USABLE_SXN_CNT)
         || ((blkrecord_ud_chk_valid_sxn(PcbPtr->Word.FbxPtr,
                                         RemapUdPtr->RemapPba)) == OFF)
         || (media_chk_n_lock_usersxn(PcbPtr->Word.FbxPtr, UserSxnIdx) == LOCKED))

    {
        RemapUdPtr->RemapPba += SEGMENTS_PER_USER_SXN;
        PcbPtr->Fn = remap_ud_prcs_get_remap_blk;
        SCHED_POST_PCB(PcbPtr);

        // Unlock si0 cache
        sxninfo_unlock_read(RemapUdPtr->CntlCachePtr);

        return;
    }

    // Unlock si0 cache
    sxninfo_unlock_read(RemapUdPtr->CntlCachePtr);

    RemapUdPtr->UserSxnIdx = UserSxnIdx;

    // Prepare dmx info for userdata read
    PcbPtr->Info.DmxRead.Pba = RemapUdPtr->RemapPba;
    PcbPtr->Info.DmxRead.SegCnt = SEGMENTS_PER_USER_SXN;
    PcbPtr->Fn = remap_ud_read_userdata_done;
    dmx_ops_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : remap_ud_read_userdata_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_ud_read_userdata_done (PCB_STRUCT *PcbPtr)
{
    BIT_STAT Stat;
    PCB_STRUCT *ChildPcbPtr;

    Stat = PcbPtr->Info.DmxRead.Stat;
    if (    (Stat != SUCCESSFUL)
         && (Stat != DMX_OPS_ECC_CORRECTABLE))
    {
        // Ignore this section and then get new section
        media_unlock_usersxn(PcbPtr->Word.FbxPtr,
                             PcbPtr->Info.RemapUd.UserSxnIdx);

        PcbPtr->Info.RemapUd.RemapPba += SEGMENTS_PER_USER_SXN;
        remap_ud_prcs_get_remap_blk(PcbPtr);

        return;
    }

    // Get pcb
    ChildPcbPtr = _sched_get_pcb();

    // Fill up child pcb for control update
    ChildPcbPtr->Word.FbxPtr = PcbPtr->Word.FbxPtr;
    ChildPcbPtr->Info.UserWrite.UserSxnIdx = PcbPtr->Info.RemapUd.UserSxnIdx;
    ChildPcbPtr->Info.UserWrite.OldPba = PcbPtr->Info.RemapUd.RemapPba;
    ChildPcbPtr->Info.UserWrite.NxtPcbPtr = PcbPtr->Info.RemapUd.NxtPcbPtr;
    PcbPtr->Info.RemapUd.NxtPcbPtr = ChildPcbPtr;

    PcbPtr->Info.RemapUd.RemapSxnCnt++;
    if (PcbPtr->Info.RemapUd.RemapSxnCnt < USER_SXNS_PER_PAGE)
    {
        PcbPtr->Info.DmxRead.BuffAddr -= USER_SXN_SIZE;
        PcbPtr->Info.RemapUd.RemapPba += SEGMENTS_PER_USER_SXN;
        remap_ud_prcs_get_remap_blk(PcbPtr);
    }
    else
    {
        remap_ud_write_userdata(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : remap_ud_write_userdata
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_ud_write_userdata (PCB_STRUCT *PcbPtr)
{
    PBA_INT Pba;

    if (SCHED_LOW_PCB_CNT())
    {
        PcbPtr->Fn = remap_ud_write_userdata;
        SCHED_POST_PCB(PcbPtr);
        return;
    }

    Pba = freelist_ud_cmpct_get_page(PcbPtr->Word.FbxPtr);
    if (Pba == INVALID_MASK)
    {
        PcbPtr->Fn = remap_ud_write_userdata;
        SCHED_POST_PCB(PcbPtr);
        return;    
    }

    // Fill up dmx info for user write
    PcbPtr->Info.DmxWrite.Pba = Pba;
    PcbPtr->Info.DmxWrite.SegCnt = SEGMENTS_PER_PAGE;
    PcbPtr->Info.DmxWrite.BuffAddr = PcbPtr->Word.FbxPtr->RemapUd.BuffAddr;
    PcbPtr->Fn = remap_ud_write_userdata_done;

    #if defined(UD_VERIFY)
    dmx_ops_write_n_read(PcbPtr);
    #else
    dmx_ops_write(PcbPtr);
    #endif

    return;
}


//-----------------------------------------------------------------------------
// Function    : remap_ud_write_userdata_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_ud_write_userdata_done (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT Pba;
    PCB_STRUCT *CurPcbPtr;
    unsigned long SxnToInv;

    FbxPtr = PcbPtr->Word.FbxPtr;
    Pba = PcbPtr->Info.DmxWrite.Pba;

    // Check if flushing failed
    if (PcbPtr->Info.DmxWrite.Stat != SUCCESSFUL)
    {

        if (PcbPtr->Info.DmxWrite.Stat != DMX_OPS_CMD_RETRY)
        {
            // Set Quarantine Bit
            blkrecord_ud_set_q_blk(FbxPtr,
                                   Pba);
        }

        // Remove from freelist
        freelist_ud_chk_n_remove_cur_pba(FbxPtr,
                                         Pba);

        // Get new page again
        remap_ud_write_userdata(PcbPtr);

        return;
    }

    // Check SxnCnt if enough UserSxn has been written to page
    for (SxnToInv = 0;
         SxnToInv < (USER_SXNS_PER_PAGE - PcbPtr->Info.RemapUd.RemapSxnCnt);
         SxnToInv++)
    {
        Pba += SEGMENTS_PER_USER_SXN;
    }

    CurPcbPtr = PcbPtr->Info.RemapUd.NxtPcbPtr;

    // Loop page pba based on number of sxn
    do
    {
        // Save new user pba
        CurPcbPtr->Info.UserWrite.Pba = Pba;
        Pba += SEGMENTS_PER_USER_SXN;

        cntldata_write_alloc_si0(CurPcbPtr);

        CurPcbPtr = CurPcbPtr->Info.UserWrite.NxtPcbPtr;

    } while (CurPcbPtr != BIT_NULL_PTR);

    // Reset remap buffer address and remap sxn count
    PcbPtr->Info.DmxRead.BuffAddr
        = FbxPtr->RemapUd.BuffAddr
        + (USER_SXN_SIZE * (USER_SXNS_PER_PAGE - 1));

    PcbPtr->Info.RemapUd.RemapSxnCnt = 0;
    PcbPtr->Info.RemapUd.NxtPcbPtr = BIT_NULL_PTR;
    PcbPtr->Info.RemapUd.RemapPba += SEGMENTS_PER_USER_SXN;

    PcbPtr->Fn = remap_ud_prcs_get_remap_blk;
    SCHED_POST_PCB(PcbPtr);

    return;
}


//=============================================================================
// $Log: RemapUser.c,v $
// Revision 1.7  2014/05/19 05:09:21  rcantong
// 1. DEV: Cleanup
// 1.1 Removed unused user remap per section functions
//
// Revision 1.6  2014/05/13 13:38:07  rcantong
// 1. BUGFIX: User data verify is perform also during remap processes
// 1.1 Perform dmx_ops_write_n_read when UD_VERIFY is enabled
//
// Revision 1.5  2014/04/30 13:45:51  rcantong
// 1. BUGFIX: Enhanced control data scrambler
// 1.1 XOR scramble pattern to Dir0Entry and Si0Entry - JParairo
//
// Revision 1.4  2014/03/03 12:55:44  rcantong
// 1. DEV: FID hang handler
// 1.1 Added dmx stat checking for FID_HANG_TIMEOUT - JFaustino
//
// Revision 1.3  2014/02/02 10:00:32  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.2  2013/08/08 16:44:24  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:17  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================