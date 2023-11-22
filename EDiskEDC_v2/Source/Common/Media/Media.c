//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Media/Media.c,v $
// $Revision: 1.9 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:59 $
// $Id: Media.c,v 1.9 2014/05/19 04:48:59 rcantong Exp $
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
#include "Bios.h"
#include "Util.h"
#include "Sched.h"
#include "SysConfig.h"

#include "BlkRecord.h"
#include "Compact.h"
#include "CntlDataCommon.h"
#include "CntlData.h"
#include "Defects.h"
#include "Dir.h"
#include "Disturb.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "Err.h"
#include "FreeList.h"
#include "Interrupt.h"
#include "Iop.h"
#include "IpCall.h"
#include "Media.h"
#include "RemapUser.h"
#include "SxnInfo.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "MediaI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_bitmap")
unsigned long UserSxnBmp[FBX_CNT][USABLE_SXN_CNT / BITS_PER_WORD];
L1CACHE_ALIGN(UserSxnBmp);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : media_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_init_malloc (unsigned long FbxIdx)
{
    DM_FBX_STRUCT *FbxPtr;

    FbxPtr = &DmFbx[FbxIdx];
    FbxPtr->UserSxnBmpPtr = &UserSxnBmp[FbxIdx][0];

    util_init_pattern(UserSxnBmp[FbxIdx],
                      sizeof(UserSxnBmp[0]),
                      INIT_PATTERN_LO_VALUE);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_chk_usersxn_lock
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT media_chk_usersxn_lock (DM_FBX_STRUCT *FbxPtr,
                                 unsigned long UserSxnIdx)
{
    unsigned long WordIdx;
    unsigned long BitIdx;
    unsigned long WordBmp;

    WordIdx = UserSxnIdx / BITS_PER_WORD;
    BitIdx = UserSxnIdx % BITS_PER_WORD;
    WordBmp = FbxPtr->UserSxnBmpPtr[WordIdx];

    return ((WordBmp >> BitIdx) & 1);
}


//-----------------------------------------------------------------------------
// Function    : media_chk_n_lock_usersxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT media_chk_n_lock_usersxn (DM_FBX_STRUCT *FbxPtr,
                                   unsigned long UserSxnIdx)
{
    unsigned long WordIdx;
    unsigned long BitIdx;
    unsigned long *WordBmpPtr;

    WordIdx = UserSxnIdx / BITS_PER_WORD;
    BitIdx = UserSxnIdx % BITS_PER_WORD;
    WordBmpPtr = &FbxPtr->UserSxnBmpPtr[WordIdx];

    if (((*WordBmpPtr >> BitIdx) & 1) == UNLOCKED)
    {
        *WordBmpPtr |= (1 << BitIdx);
        return UNLOCKED;
    }

    return LOCKED;
}


//-----------------------------------------------------------------------------
// Function    : media_unlock_usersxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_unlock_usersxn (DM_FBX_STRUCT *FbxPtr,
                           unsigned long UserSxnIdx)
{
    unsigned long WordIdx;
    unsigned long BitIdx;
    unsigned long *WordBmpPtr;

    WordIdx = UserSxnIdx / BITS_PER_WORD;
    BitIdx = UserSxnIdx % BITS_PER_WORD;
    WordBmpPtr = &FbxPtr->UserSxnBmpPtr[WordIdx];
    *WordBmpPtr &= (~(1 << BitIdx));

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_prcs_read_bulk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_prcs_read_bulk (unsigned long *PayloadPtr)
{
    unsigned long FrmAddr;
    USER_CMD_FRM_STRUCT *FrmPtr;
    unsigned long FbxIdxs;
    unsigned long LbaOffsets;
    unsigned long LbaCnts;
    unsigned long BuffAddr;
    unsigned long DrvrCmdTag;
    unsigned long ValidSxnCnt;
    unsigned long *SxnIdxsPtr;
    unsigned long SxnIdx;
    PCB_STRUCT *PcbPtr;
    unsigned long FbxIdx;
    PCB_USER_READ_STRUCT *UserReadPtr;
    unsigned long LbaCnt;

    BIOS_RESET_IDLE_TMR();

    // Change uncached address to cached
    FrmAddr = PayloadPtr[0] & 0x7FFFFFFF;
    bios_invalidate_dc_line((void *)FrmAddr);
    bios_invalidate_dc_line((void *)(FrmAddr + 32));

    // Get emp cmd frame contents
    FrmPtr      = (void *)FrmAddr;
    FbxIdxs     = FrmPtr->FbxIdxs;
    LbaOffsets  = FrmPtr->SxnOfsts;
    LbaCnts     = FrmPtr->LbaCnts;
    BuffAddr    = FrmPtr->BuffAddr & 0xF1FFFFFF;
    DrvrCmdTag  = (((FrmPtr->Profile >> 8) & 0xFF) << 11) | (1 << 23);
    ValidSxnCnt = FrmPtr->CntNSize & 0xF;

    SxnIdxsPtr  = &FrmPtr->SxnIdx[0];

    // Setup read process for each valid section
    for (SxnIdx = 0;
         SxnIdx < ValidSxnCnt;
         SxnIdx++)
    {
        PcbPtr = _sched_get_pcb();

        FbxIdx = (FbxIdxs >> (SxnIdx * 4)) & 0xF;
        PcbPtr->Word.FbxPtr = &DmFbx[FbxIdx];

        UserReadPtr = (void *)&PcbPtr->Info;
        UserReadPtr->UserSxnIdx = SxnIdxsPtr[SxnIdx];
        UserReadPtr->DrvrCmdTag = DrvrCmdTag;
        UserReadPtr->CmdSxnIdx = SxnIdx;
        UserReadPtr->LbaOffset = (LbaOffsets >> (SxnIdx * 4)) & 0xF;

        LbaCnt = (LbaCnts >> (SxnIdx * 4)) & 0xF;
        UserReadPtr->LbaCnt = LbaCnt;

        UserReadPtr->BuffAddr = BuffAddr;
        BuffAddr += (LbaCnt * LBA_SIZE);

        media_read_alloc_dir0(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_prcs_write_bulk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_prcs_write_bulk (unsigned long *PayloadPtr)
{
    unsigned long CmdTag;
    WRITE_CMD_INFO_STRUCT *WrCmdInfoPtr;
    unsigned long DrvrCmdTag;
    unsigned long BuffAddr;
    unsigned long FbxIdx;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long ValidSxnCnt;

    unsigned long *SxnIdxsPtr;
    unsigned long LinkSxnCnt;
    unsigned long SxnIdx;
    PCB_STRUCT *PcbPtr;
    PCB_STRUCT *FirstPcbPtr = BIT_NULL_PTR;
    PCB_STRUCT *LastPcbPtr = BIT_NULL_PTR;

    BIOS_RESET_IDLE_TMR();

    // Invalidate lower half of cmd info to get dm related info
    CmdTag = PayloadPtr[0];
    WrCmdInfoPtr = (void *)&IopCmdInfo[CmdTag].CmdInfo;
    bios_invalidate_dc_line(WrCmdInfoPtr);

    // Get cmd info contents
    DrvrCmdTag = (CmdTag << 11) | (1 << 23);
    BuffAddr = BUFFER_BASE_ADDR + (CmdTag * BUFFER_SIZE);
    FbxIdx = WrCmdInfoPtr->FbxIdx;
    FbxPtr = &DmFbx[FbxIdx];
    ValidSxnCnt = WrCmdInfoPtr->ValidSxnCnt;
    ASSERT(ValidSxnCnt <= 4);

    SxnIdxsPtr = &WrCmdInfoPtr->UsrSxn[0];
    LinkSxnCnt = 0;

    for (SxnIdx = 0;
         SxnIdx < ValidSxnCnt;
         SxnIdx++)
    {
        PcbPtr = _sched_get_pcb();

        PcbPtr->Word.FbxPtr = FbxPtr;
        PcbPtr->Info.UserWrite.UserSxnIdx = *SxnIdxsPtr;
        SxnIdxsPtr++;

        if (LinkSxnCnt == 0)
        {
            // Save current pcb as first pcb
            FirstPcbPtr = PcbPtr;
            PcbPtr->Info.UserWrite.DrvrCmdTag = DrvrCmdTag;
            PcbPtr->Info.UserWrite.BuffAddr = BuffAddr;
        }
        else
        {
            // Link the current pcb as last pcb
            LastPcbPtr->Info.UserWrite.NxtPcbPtr = PcbPtr;
        }

        // Save current pcb as last pcb
        LastPcbPtr = PcbPtr;
        LinkSxnCnt++;

        // Reached limit, post first pcb
        if (LinkSxnCnt == USER_SXNS_PER_PAGE)
        {
            FirstPcbPtr->Info.UserWrite.SxnCnt = LinkSxnCnt;
            LastPcbPtr->Info.UserWrite.NxtPcbPtr = BIT_NULL_PTR;
            media_write_lock_usersxn(FirstPcbPtr);

            LinkSxnCnt = 0;
            BuffAddr += FLASH_PAGE_SIZE;
        }
    }

    // Lack sections, still needs posting of first pcb
    if (LinkSxnCnt != 0)
    {
        ASSERT(LinkSxnCnt < USER_SXNS_PER_PAGE);
        FirstPcbPtr->Info.UserWrite.SxnCnt = LinkSxnCnt;
        LastPcbPtr->Info.UserWrite.NxtPcbPtr = BIT_NULL_PTR;
        media_write_lock_usersxn(FirstPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_prcs_rmw_read_bulk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_prcs_rmw_read_bulk (unsigned long *PayloadPtr)
{
    unsigned long CmdTag;
    WRITE_CMD_INFO_STRUCT *WrCmdInfoPtr;
    unsigned long DrvrCmdTag;
    unsigned long BuffAddr;
    unsigned long FbxIdx;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long ValidSxnCnt;

    unsigned long *SxnIdxsPtr;
    unsigned long SxnIdx;
    PCB_STRUCT *PcbPtr;
    PCB_USER_READ_STRUCT *UserReadPtr;
    unsigned long LbaCnt;
    unsigned long LbaOffset;
    PCB_STRUCT *FirstPcbPtr = BIT_NULL_PTR;
    PCB_STRUCT *LastPcbPtr = BIT_NULL_PTR;

    BIOS_RESET_IDLE_TMR();

    // Invalidate lower half of cmd info to get dm related info
    CmdTag = PayloadPtr[0];
    WrCmdInfoPtr = (void *)&IopCmdInfo[CmdTag].CmdInfo;
    bios_invalidate_dc_line(WrCmdInfoPtr);

    // Get cmd info contents
    DrvrCmdTag = (CmdTag << 11) | (1 << 23);
    BuffAddr = BUFFER_BASE_ADDR + (CmdTag * BUFFER_SIZE);
    FbxIdx = WrCmdInfoPtr->FbxIdx;
    ASSERT(BiosParm.DmId == FbxToDmId[FbxIdx]);
    FbxPtr = &DmFbx[FbxIdx];
    ValidSxnCnt = WrCmdInfoPtr->ValidSxnCnt;
    SxnIdxsPtr = &WrCmdInfoPtr->UsrSxn[0];

    for (SxnIdx = 0;
         SxnIdx < ValidSxnCnt;
         SxnIdx++)
    {
        PcbPtr = _sched_get_pcb();

        PcbPtr->Word.FbxPtr = FbxPtr;

        UserReadPtr = (void *)&PcbPtr->Info;

        // TODO: read only needed segments
        LbaCnt = LBAS_PER_USER_SXN;
        LbaOffset = 0;

        UserReadPtr->UserSxnIdx = SxnIdxsPtr[SxnIdx];
        UserReadPtr->DrvrCmdTag = DrvrCmdTag;
        UserReadPtr->BuffAddr   = BuffAddr;
        UserReadPtr->LbaCnt     = LbaCnt;
        UserReadPtr->LbaOffset  = LbaOffset;
        UserReadPtr->CmdSxnIdx  = SxnIdx;
        UserReadPtr->NxtPcbPtr  = BIT_NULL_PTR;

        BuffAddr += USER_SXN_SIZE;

        if (SxnIdx == 0)
        {
            FirstPcbPtr = PcbPtr;
        }
        else
        {
            LastPcbPtr->Info.UserRead.NxtPcbPtr = PcbPtr;
        }

        LastPcbPtr = PcbPtr;
    }

    media_rmw_lock_usersxn(FirstPcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_prcs_rmw_write_bulk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_prcs_rmw_write_bulk (unsigned long *PayloadPtr)
{
    unsigned long CmdTag;
    WRITE_CMD_INFO_STRUCT *WrCmdInfoPtr;
    unsigned long DrvrCmdTag;
    unsigned long BuffAddr;
    unsigned long FbxIdx;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long ValidSxnCnt;

    unsigned long *SxnIdxsPtr;
    unsigned long LinkSxnCnt;
    unsigned long SxnIdx;
    PCB_STRUCT *PcbPtr;
    PCB_STRUCT *FirstPcbPtr = BIT_NULL_PTR;
    PCB_STRUCT *LastPcbPtr = BIT_NULL_PTR;

    BIOS_RESET_IDLE_TMR();

    // No need to invalidate
    CmdTag = PayloadPtr[0];
    WrCmdInfoPtr = (void *)&IopCmdInfo[CmdTag].CmdInfo;

    // Get cmd info contents
    DrvrCmdTag = (CmdTag << 11) | (1 << 23);
    BuffAddr = BUFFER_BASE_ADDR + (CmdTag * BUFFER_SIZE);
    FbxIdx = WrCmdInfoPtr->FbxIdx;
    ASSERT(BiosParm.DmId == FbxToDmId[FbxIdx]);
    FbxPtr = &DmFbx[FbxIdx];
    ValidSxnCnt = WrCmdInfoPtr->ValidSxnCnt;
    ASSERT(ValidSxnCnt <= 4);

    SxnIdxsPtr = &WrCmdInfoPtr->UsrSxn[0];
    LinkSxnCnt = 0;

    for (SxnIdx = 0;
         SxnIdx < ValidSxnCnt;
         SxnIdx++)
    {
        PcbPtr = _sched_get_pcb();

        PcbPtr->Word.FbxPtr = FbxPtr;
        PcbPtr->Info.UserWrite.UserSxnIdx = *SxnIdxsPtr;
        SxnIdxsPtr++;

        if (LinkSxnCnt == 0)
        {
            // Save current pcb as first pcb
            FirstPcbPtr = PcbPtr;
            PcbPtr->Info.UserWrite.DrvrCmdTag = DrvrCmdTag;
            PcbPtr->Info.UserWrite.BuffAddr   = BuffAddr;
        }
        else
        {
            // Link the current pcb as last pcb
            LastPcbPtr->Info.UserWrite.NxtPcbPtr = PcbPtr;
        }

        // Save current pcb as last pcb
        LastPcbPtr = PcbPtr;
        LinkSxnCnt++;

        // Reached limit, post first pcb
        if (LinkSxnCnt == USER_SXNS_PER_PAGE)
        {
            FirstPcbPtr->Info.UserWrite.SxnCnt = LinkSxnCnt;
            LastPcbPtr->Info.UserWrite.NxtPcbPtr = BIT_NULL_PTR;
            media_write_flush_ud(FirstPcbPtr);

            LinkSxnCnt = 0;
            BuffAddr += FLASH_PAGE_SIZE;
        }
    }

    // Lack sections, still needs posting of first pcb
    if (LinkSxnCnt != 0)
    {
        ASSERT(LinkSxnCnt < USER_SXNS_PER_PAGE);
        FirstPcbPtr->Info.UserWrite.SxnCnt = LinkSxnCnt;
        LastPcbPtr->Info.UserWrite.NxtPcbPtr = BIT_NULL_PTR;
        media_write_flush_ud(FirstPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_write_in_place
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_write_in_place (unsigned long *PayloadPtr)
{
    PCB_STRUCT *PcbPtr;

    PcbPtr = _sched_get_pcb();

    // Setup dmx write in place request
    PcbPtr->Word.FbxPtr = &DmFbx[0];
    PcbPtr->Info.DmxWrite.Pba = PayloadPtr[0];
    PcbPtr->Info.DmxWrite.SegCnt = PayloadPtr[1] / SEGMENT_SIZE;
    PcbPtr->Info.DmxWrite.BuffAddr = PayloadPtr[2];
    PcbPtr->Info.MediaWriteInPlace.IpCallFn = PayloadPtr[3];
    PcbPtr->Fn = media_write_in_place_done;

    dmx_ops_write_in_place(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : media_read_alloc_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_read_alloc_dir0 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_USER_READ_STRUCT *UserReadPtr;
    unsigned long Stat;

    FbxPtr = PcbPtr->Word.FbxPtr;
    UserReadPtr = (void *)&PcbPtr->Info;

    Stat = dir_alloc_read(FbxPtr,
                          UserReadPtr->UserSxnIdx,
                          &UserReadPtr->Dir0CachePtr);

    if (Stat <= CNTLDATA_INFLUSH)
    {
        media_read_fetch_ud(PcbPtr);
    }
    else if (Stat == CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = media_read_fetch_ud;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr, UserReadPtr->Dir0CachePtr);
    }
    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        PcbPtr->Fn = media_read_alloc_dir0;
        CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr, FbxPtr->Dir);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_read_fetch_ud
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_read_fetch_ud (PCB_STRUCT *PcbPtr)
{
    unsigned long UserSxnIdx;
    volatile DIR0_ENTRY_STRUCT *Dir0EntryPtr;
    PBA_INT UserPba;
    IPCALL_STRUCT *IpCallPtr;

    // Check if user sxn is locked
    UserSxnIdx = PcbPtr->Info.UserRead.UserSxnIdx;
    if (media_chk_usersxn_lock(PcbPtr->Word.FbxPtr,
                               UserSxnIdx) == LOCKED)
    {
        PcbPtr->Fn = media_read_fetch_ud;
        SCHED_POST_PCB(PcbPtr);
        return;
    }

    // Get user pba from dir
    Dir0EntryPtr = (void *)PcbPtr->Info.UserRead.Dir0CachePtr->DataAddr;
    Dir0EntryPtr += (UserSxnIdx % DIR0_ENTRIES_PER_SXN);

    // Decode userpba based on UserSxnIdx
    UserPba = Dir0EntryPtr->UserPba ^ SCRAMBLE_WORD(UserSxnIdx);

    // Unlock dir0 cache
    dir_unlock_read(PcbPtr->Info.UserRead.Dir0CachePtr);

    // Check if unmap pba
    if ((UserPba & INVALID_MASK) != 0)
    {
        // Unmapped user sxn, send reply to core 0
        IpCallPtr = ipcall_add_master_entry();
        IpCallPtr->Fn = iop_msg_read_error_reply;
        IpCallPtr->Arg[0] = (PcbPtr->Info.UserRead.DrvrCmdTag >> 11) & 0xFF;
        ipcall_master_exec_calls();

        _sched_return_pcb(PcbPtr);

        return;
    }

    // Fill up fetch userdata
    ASSERT(UserPba < SEGMENTS_PER_FBX);
    PcbPtr->Info.UserRead.Pba = UserPba;
    PcbPtr->Fn = media_read_done;
    dmx_ops_read_user(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_read_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_read_done (PCB_STRUCT *PcbPtr)
{
    unsigned long DmxStat;
    IPCALL_STRUCT *IpCallPtr;
    unsigned long BlkIdx;

    DmxStat = PcbPtr->Info.UserRead.Stat;
    switch (DmxStat) {
    case SUCCESSFUL:
        break;

    case DMX_OPS_ECC_CORRECTABLE:

        // Set Quarantine Bit
        blkrecord_ud_set_q_blk(PcbPtr->Word.FbxPtr,
                               PcbPtr->Info.UserRead.Pba);

        // Reply success user read
        IpCallPtr = ipcall_add_master_entry();
        IpCallPtr->Fn = iop_msg_read_error_reply;
        IpCallPtr->Arg[0] = (PcbPtr->Info.UserRead.DrvrCmdTag >> 11) & 0xFF;
        IpCallPtr->Arg[1] = SUCCESSFUL;
        ipcall_master_exec_calls();
        break;

    // Uncorrectable and erased
    default:

        // Set Quarantine Bit
        blkrecord_ud_set_q_blk(PcbPtr->Word.FbxPtr,
                               PcbPtr->Info.UserRead.Pba);

        // Reply error user read
        IpCallPtr = ipcall_add_master_entry();
        IpCallPtr->Fn = iop_msg_read_error_reply;
        IpCallPtr->Arg[0] = (PcbPtr->Info.UserRead.DrvrCmdTag >> 11) & 0xFF;
        IpCallPtr->Arg[1] = NOT_SUCCESSFUL;
        ipcall_master_exec_calls();
        break;
    }

    BlkIdx = PcbPtr->Info.UserRead.Pba / SEGMENTS_PER_BLK;
    DISTURB_INC_READ_CNT(PcbPtr->Word.FbxPtr, BlkIdx);
    if (    (DISTURB_CHECK_READ_THRESH(PcbPtr->Word.FbxPtr, BlkIdx))
         || (DmxStat == DMX_OPS_ECC_UNCORRECTABLE)
         || (DmxStat == DMX_OPS_ECC_CORRECTABLE))
    {
        remap_ud_add_to_remap_list(PcbPtr->Word.FbxPtr,
                                   PcbPtr->Info.UserRead.Pba);
    }

    // Return the pcb
    _sched_return_pcb(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_write_lock_usersxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_write_lock_usersxn (PCB_STRUCT *PcbPtr)
{
    unsigned long SxnCnt;
    DM_FBX_STRUCT *FbxPtr;
    PCB_STRUCT *CurPcbPtr;
    unsigned long SxnIdx;

    ASSERT(PcbPtr != BIT_NULL_PTR);
    SxnCnt = PcbPtr->Info.UserWrite.SxnCnt;
    ASSERT(SxnCnt > 0);

    FbxPtr = PcbPtr->Word.FbxPtr;
    CurPcbPtr = PcbPtr;

    for (SxnIdx = 0;
         SxnIdx < SxnCnt;
         SxnIdx++)
    {
        if (media_chk_n_lock_usersxn(FbxPtr,
                CurPcbPtr->Info.UserWrite.UserSxnIdx) == LOCKED)
        {
            CurPcbPtr = PcbPtr;

            while (SxnIdx > 0)
            {
                // Unlock all previously locked user sections
                media_unlock_usersxn(FbxPtr,
                                     CurPcbPtr->Info.UserWrite.UserSxnIdx);

                CurPcbPtr = CurPcbPtr->Info.UserWrite.NxtPcbPtr;
                SxnIdx--;
            }

            PcbPtr->Fn = media_write_lock_usersxn;
            SCHED_POST_PCB(PcbPtr);

            return;
        }

        CurPcbPtr = CurPcbPtr->Info.UserWrite.NxtPcbPtr;
    }

    ASSERT(SxnIdx > 0);

    media_write_flush_ud(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_write_flush_ud
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_write_flush_ud (PCB_STRUCT *PcbPtr)
{
    PBA_INT Pba;

    if (SCHED_LOW_PCB_CNT())
    {
        PcbPtr->Fn = media_write_flush_ud;
        SCHED_POST_PCB(PcbPtr);
        return;
    }

    // Get free page pba
    Pba = freelist_ud_get_page(PcbPtr->Word.FbxPtr);

    // There are no sufficient freelist entries
    if (Pba == INVALID_MASK)
    {
        PcbPtr->Fn = media_write_flush_ud;
        SCHED_POST_PCB(PcbPtr);
        return;
    }

    // Fill up additional parameters for write
    PcbPtr->Info.UserWrite.Pba = Pba;
    PcbPtr->Fn = media_write_flush_ud_done;
    dmx_ops_write_user(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_write_flush_ud_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_write_flush_ud_done (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    Pba = PcbPtr->Info.UserWrite.Pba;

    // Check if flushing failed
    if (PcbPtr->Info.UserWrite.Stat != SUCCESSFUL)
    {

        if (PcbPtr->Info.UserWrite.Stat != DMX_OPS_CMD_RETRY)
        {
            // Set Quarantine Bit
            blkrecord_ud_set_q_blk(FbxPtr,
                                   Pba);
        }

        // Remove from freelist
        freelist_ud_chk_n_remove_cur_pba(FbxPtr,
                                         Pba);

        // Get new page again
        media_write_flush_ud(PcbPtr);

        return;
    }

    // Populate pba assigned in each sxn
    do
    {
        // Process update of dir0
        PcbPtr->Info.UserWrite.Pba = Pba;
        Pba += SEGMENTS_PER_USER_SXN;

        media_write_alloc_si0(PcbPtr);

        PcbPtr = PcbPtr->Info.UserWrite.NxtPcbPtr;

    } while (PcbPtr != BIT_NULL_PTR);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_write_alloc_si0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_write_alloc_si0 (PCB_STRUCT *PcbPtr)
{
    PCB_USER_WRITE_STRUCT *UserWritePtr;
    BIT_STAT Stat;

    UserWritePtr = (void *)&PcbPtr->Info;

    Stat = sxninfo_alloc_write(PcbPtr->Word.FbxPtr,
                               UserWritePtr->Pba,
                               &UserWritePtr->CntlCachePtr);

    if (Stat == CNTLDATA_INCACHE)
    {
        media_write_update_si0(PcbPtr);
    }
    else if (Stat <= CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = media_write_update_si0;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr, UserWritePtr->CntlCachePtr);
    }
    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        PcbPtr->Fn = media_write_alloc_si0;
        CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr, PcbPtr->Word.FbxPtr->SxnInfo);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_write_update_si0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_write_update_si0 (PCB_STRUCT *PcbPtr)
{
    CNTL_CACHE_STRUCT *Si0CachePtr;
    unsigned long Si0EntryIdx;
    volatile SI0_ENTRY_STRUCT *Si0EntryPtr;
    unsigned long UserSxnIdx;

    // Get si0 entry derived from pba
    Si0CachePtr = PcbPtr->Info.UserWrite.CntlCachePtr;
    Si0EntryIdx = PcbPtr->Info.UserWrite.Pba / SEGMENTS_PER_USER_SXN;
    Si0EntryPtr = (void *)Si0CachePtr->DataAddr;
    Si0EntryPtr += (Si0EntryIdx % SI0_ENTRIES_PER_SXN);

    // Update si0 entry to new user sxn idx then set cache to dirty
    UserSxnIdx = PcbPtr->Info.UserWrite.UserSxnIdx;
    Si0EntryPtr->UserSxnIdx = UserSxnIdx ^ SCRAMBLE_WORD(Si0EntryIdx);
    CNTLDATA_SET_DIRTY(Si0CachePtr,
                       PcbPtr->Word.FbxPtr);

    // Unlock si0 cache
    CNTLDATA_UNLOCK_WRITE(Si0CachePtr);

	// Next stage update directory
	media_write_alloc_dir0(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_write_alloc_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_write_alloc_dir0 (PCB_STRUCT *PcbPtr)
{
    PCB_USER_WRITE_STRUCT *UserWritePtr;
    BIT_STAT Stat;

    UserWritePtr = (void *)&PcbPtr->Info;

    Stat = dir_alloc_write(PcbPtr->Word.FbxPtr,
                           UserWritePtr->UserSxnIdx,
                           &UserWritePtr->CntlCachePtr);

    if (Stat == CNTLDATA_INCACHE)
    {
        media_write_update_dir0(PcbPtr);
    }
    else if (Stat <= CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = media_write_update_dir0;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr, UserWritePtr->CntlCachePtr);
    }
    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        PcbPtr->Fn = media_write_alloc_dir0;
        CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr, PcbPtr->Word.FbxPtr->Dir);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_write_update_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_write_update_dir0 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long UserSxnIdx;
    CNTL_CACHE_STRUCT *Dir0CachePtr;
    volatile DIR0_ENTRY_STRUCT *Dir0EntryPtr;
    PBA_INT OldPba;
    PBA_INT NewPba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    UserSxnIdx = PcbPtr->Info.UserWrite.UserSxnIdx;
    ASSERT(media_chk_usersxn_lock(FbxPtr,
                                  UserSxnIdx) == LOCKED);

    // Get dir0 entry derived from user sxn idx
    Dir0CachePtr = PcbPtr->Info.UserWrite.CntlCachePtr;
    ASSERT(Dir0CachePtr != BIT_NULL_PTR);
    Dir0EntryPtr = (void *)Dir0CachePtr->DataAddr;
    Dir0EntryPtr += (UserSxnIdx % DIR0_ENTRIES_PER_SXN);

    // Get old pba from scrambled dir0 entry
    OldPba = Dir0EntryPtr->UserPba ^ SCRAMBLE_WORD(UserSxnIdx);
    NewPba = PcbPtr->Info.UserWrite.Pba;

    // Update dir0 entry to new pba then set cache to dirty
    Dir0EntryPtr->UserPba
        = NewPba ^ SCRAMBLE_WORD(UserSxnIdx);
    CNTLDATA_SET_DIRTY(Dir0CachePtr,
                       FbxPtr);

    // Unlock dir0 cache
    CNTLDATA_UNLOCK_WRITE(Dir0CachePtr);

    // set new pba to valid sxn
    blkrecord_ud_set_valid_sxn(FbxPtr,
                               NewPba);

    // Update valid info
    if ((OldPba & INVALID_MASK) == 0)
    {
        // Invalidate vi
        ASSERT(OldPba < SEGMENTS_PER_FBX);
        blkrecord_ud_set_invalid_sxn(FbxPtr,
                                     OldPba);
    }

    // Unlock user sxn lock
    media_unlock_usersxn(FbxPtr,
                         UserSxnIdx);

    _sched_return_pcb(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_rmw_lock_usersxn
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_rmw_lock_usersxn (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_STRUCT *CurPcbPtr;
    PCB_USER_READ_STRUCT *UserReadPtr;
    unsigned long SxnLockedCnt = 0;

    FbxPtr = PcbPtr->Word.FbxPtr;
    CurPcbPtr = PcbPtr;

    do
    {
        UserReadPtr = (void *)&CurPcbPtr->Info;
        if (media_chk_n_lock_usersxn(FbxPtr,
                                     UserReadPtr->UserSxnIdx) == LOCKED)
        {
            CurPcbPtr = PcbPtr;
            while (SxnLockedCnt > 0)
            {
                UserReadPtr = (void *)&CurPcbPtr->Info;
                media_unlock_usersxn(FbxPtr,
                                     UserReadPtr->UserSxnIdx);

                CurPcbPtr = UserReadPtr->NxtPcbPtr;
                SxnLockedCnt--;
            }

            PcbPtr->Fn = media_rmw_lock_usersxn;
            SCHED_POST_PCB(PcbPtr);

            return;
        }

        SxnLockedCnt++;
        CurPcbPtr = UserReadPtr->NxtPcbPtr;
    } while (CurPcbPtr != BIT_NULL_PTR);

    CurPcbPtr = PcbPtr;
    do
    {
        media_rmw_read_alloc_dir0(CurPcbPtr);
        CurPcbPtr = CurPcbPtr->Info.UserRead.NxtPcbPtr;
    } while (CurPcbPtr != BIT_NULL_PTR);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_rmw_read_alloc_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_rmw_read_alloc_dir0 (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PCB_USER_READ_STRUCT *UserReadPtr;
    BIT_STAT Stat;

    FbxPtr = PcbPtr->Word.FbxPtr;
    UserReadPtr = (void *)&PcbPtr->Info;

    Stat = dir_alloc_read(FbxPtr,
                          UserReadPtr->UserSxnIdx,
                          &UserReadPtr->Dir0CachePtr);

    if (Stat == CNTLDATA_INCACHE)
    {
        media_rmw_fetch_ud(PcbPtr);
    }
    else if (Stat <= CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = media_rmw_fetch_ud;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr, UserReadPtr->Dir0CachePtr);
    }
    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        PcbPtr->Fn = media_rmw_read_alloc_dir0;
        CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr, FbxPtr->Dir);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_rmw_fetch_ud
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_rmw_fetch_ud (PCB_STRUCT *PcbPtr)
{
    unsigned long UserSxnIdx;
    volatile DIR0_ENTRY_STRUCT *Dir0EntryPtr;
    PBA_INT UserPba;
    IPCALL_STRUCT *IpCallPtr;

    // Get user pba from dir
    UserSxnIdx = PcbPtr->Info.UserRead.UserSxnIdx;
    Dir0EntryPtr = (void *)PcbPtr->Info.UserRead.Dir0CachePtr->DataAddr;
    Dir0EntryPtr += (UserSxnIdx % DIR0_ENTRIES_PER_SXN);

    // Decode userpba based on UserSxnIdx
    UserPba = Dir0EntryPtr->UserPba ^ SCRAMBLE_WORD(UserSxnIdx);

    // Unlock dir0 cache
    dir_unlock_read(PcbPtr->Info.UserRead.Dir0CachePtr);

    if ((UserPba & INVALID_MASK) != 0)
    {
        // Unmapped user sxn, send reply to core 0
        IpCallPtr = ipcall_add_master_entry();
        IpCallPtr->Fn = iop_msg_rmw_unmapped_reply;
        IpCallPtr->Arg[0] = (PcbPtr->Info.UserRead.DrvrCmdTag >> 11) & 0xFF;
        ipcall_master_exec_calls();

        _sched_return_pcb(PcbPtr);

        return;
    }

    // Fill up fetch userdata
    ASSERT(UserPba < SEGMENTS_PER_FBX);
    PcbPtr->Info.UserRead.Pba = UserPba;
    PcbPtr->Fn = media_rmw_fetch_ud_done;
    dmx_ops_read_user(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_rmw_fetch_ud_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_rmw_fetch_ud_done (PCB_STRUCT *PcbPtr)
{
    IPCALL_STRUCT *IpCallPtr;

    if (PcbPtr->Info.UserRead.Stat != SUCCESSFUL)
    {
        // Message iop
        IpCallPtr = ipcall_add_master_entry();
        IpCallPtr->Fn = iop_msg_rmw_unmapped_reply;
        IpCallPtr->Arg[0] = (PcbPtr->Info.UserRead.DrvrCmdTag >> 11) & 0xFF;
        ipcall_master_exec_calls();

        // Set Quarantine Bit
        blkrecord_ud_set_q_blk(PcbPtr->Word.FbxPtr,
                               PcbPtr->Info.UserRead.Pba);

        remap_ud_add_to_remap_list(PcbPtr->Word.FbxPtr,
                                   PcbPtr->Info.UserRead.Pba);
    }

    _sched_return_pcb(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : media_write_in_place_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void media_write_in_place_done (PCB_STRUCT *PcbPtr)
{
    IPCALL_STRUCT *IpCallPtr;

    IpCallPtr = ipcall_add_master_entry();
    IpCallPtr->Fn = (IPCALL_FN)PcbPtr->Info.MediaWriteInPlace.IpCallFn;
    IpCallPtr->Arg[0] = PcbPtr->Info.DmxWriteInPlace.Stat;
    ipcall_master_exec_calls();

    _sched_return_pcb(PcbPtr);

    return;
}


//=============================================================================
// $Log: Media.c,v $
// Revision 1.9  2014/05/19 04:48:59  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.8  2014/04/30 14:08:23  rcantong
// 1. BUGFIX: Deadlock in user section lock during RMW
// 1.1 Implement atomic locking of user sections - MFenol
// 2. BUGFIX: Insufficient PCB
// 2.1 Hold media process when low PCB count - MFenol
// 3. BUGFIX: Enhanced control data scrambler
// 3.1 XOR scramble pattern to Dir0Entry and Si0Entry - JParairo
//
// Revision 1.7  2014/03/03 13:06:00  rcantong
// 1. BUGFIX: User section lock deadlock
// 1.1 Corrected the typo error FirstPcbPtr to LastPcbPtr - MFenol
//
// Revision 1.6  2014/02/02 10:00:32  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:57  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.4  2013/12/05 13:06:35  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
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
