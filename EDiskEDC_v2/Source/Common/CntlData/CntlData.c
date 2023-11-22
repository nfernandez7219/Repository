//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/CntlData/CntlData.c,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/04/30 13:34:46 $
// $Id: CntlData.c,v 1.7 2014/04/30 13:34:46 rcantong Exp $
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

#include "BlkInfo.h"
#include "BlkRecord.h"
#include "CntlDataCommon.h"
#include "CntlData.h"
#include "Defects.h"
#include "Dir.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "Err.h"
#include "Media.h"
#include "SxnInfo.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "CntlDataI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

IDENTITY_INT CdSiId[FBX_CNT][CNTL_SEGMENTS_PER_FBX];
L1CACHE_ALIGN(CdSiId);


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : cntldata_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cntldata_init_malloc (unsigned long FbxIdx)
{
    CNTLDATA_FBX_STRUCT *CntlDataPtr;
    unsigned long CntlSxnIdx;
    IDENTITY_INT *IdPtr;

    CntlDataPtr = &DmFbx[FbxIdx].CntlData;
    util_dll_init(&CntlDataPtr->DirtyList);
    util_dll_init(&CntlDataPtr->FlushList);

    IdPtr = &CdSiId[FbxIdx][0];
    CntlDataPtr->IdPtr = IdPtr;

    for (CntlSxnIdx = 0;
         CntlSxnIdx < CNTL_SEGMENTS_PER_FBX;
         CntlSxnIdx++)
    {
        *IdPtr = TARGET_UNKNOWN;
        IdPtr++;
    }

    DmFbx[FbxIdx].Sqn = 0;

    return;
}


//-----------------------------------------------------------------------------
// Function    : cntldata_fetch_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cntldata_fetch_fbx (PCB_STRUCT *PcbPtr)
{
    unsigned long FbxIdx;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long DevIdx;
    PCB_STRUCT *ChildPcbPtr;

    FbxIdx = PcbPtr->Word.FbxIdx;
    FbxPtr = &DmFbx[FbxIdx];
    PcbPtr->Info.DmInit.DeployCtr = DEV_CNT;

    // Deploy child pcbs
    for (DevIdx = 0;
         DevIdx < DEV_CNT;
         DevIdx++)
    {
        ChildPcbPtr = _sched_get_pcb();
        ChildPcbPtr->Fn = cntldata_fetch_dev;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;
        ChildPcbPtr->Info.DmxRead.SegCnt = 1;
        ChildPcbPtr->Info.DmxRead.BuffAddr = GET_DEV_BUFF_ADDR(FbxIdx, DevIdx);
        ChildPcbPtr->Info.CntlFetch.CurPba = CALC_PBA(DevIdx, 1, 0);
        SCHED_POST_PCB(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : cntldata_write_alloc_si0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cntldata_write_alloc_si0 (PCB_STRUCT *PcbPtr)
{
    PCB_USER_WRITE_STRUCT *UserWritePtr;
    BIT_STAT Stat;

    UserWritePtr = (void *)&PcbPtr->Info;

    Stat = sxninfo_alloc_write(PcbPtr->Word.FbxPtr,
                               UserWritePtr->Pba,
                               &UserWritePtr->CntlCachePtr);

    if (Stat == CNTLDATA_INCACHE)
    {
        cntldata_write_update_si0(PcbPtr);
    }
    else if (Stat <= CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = cntldata_write_update_si0;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr, UserWritePtr->CntlCachePtr);
    }
    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        PcbPtr->Fn = cntldata_write_alloc_si0;
        CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr, PcbPtr->Word.FbxPtr->SxnInfo);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : cntldata_write_alloc_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cntldata_write_alloc_dir0 (PCB_STRUCT *PcbPtr)
{
    PCB_USER_WRITE_STRUCT *UserWritePtr;
    BIT_STAT Stat;

    UserWritePtr = (void *)&PcbPtr->Info;

    Stat = dir_alloc_write(PcbPtr->Word.FbxPtr,
                           UserWritePtr->UserSxnIdx,
                           &UserWritePtr->CntlCachePtr);

    if (Stat == CNTLDATA_INCACHE)
    {
        cntldata_write_update_dir0(PcbPtr);
    }
    else if (Stat <= CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = cntldata_write_update_dir0;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr, UserWritePtr->CntlCachePtr);
    }
    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        PcbPtr->Fn = cntldata_write_alloc_dir0;
        CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr, PcbPtr->Word.FbxPtr->Dir);
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : cntldata_fetch_dev
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cntldata_fetch_dev (PCB_STRUCT *PcbPtr)
{
    PBA_INT Pba;

    Pba = PcbPtr->Info.CntlFetch.CurPba;

    // Increment pba by page
    PcbPtr->Info.CntlFetch.CurPba = dmx_incr_pba_by_devpage(Pba);

    // Don't check if pba is good anymore. There is a possibility
    // that the page we read is a bad page with valid cntl sxns remaining.

    PcbPtr->Fn = cntldata_fetch_dev_cb;
    PcbPtr->Info.DmxRead.Pba = Pba;
    dmx_ops_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : cntldata_fetch_dev_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cntldata_fetch_dev_cb (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    volatile CNTL_HDR_STRUCT *CntlHdrPtr;
    SQN_INT Sqn;
    PBA_INT Pba;
    unsigned long EntryIdx;
    IDENTITY_INT Identity;
    unsigned long Target;

    if (    (PcbPtr->Info.DmxRead.Stat == SUCCESSFUL)
         || (PcbPtr->Info.DmxRead.Stat == DMX_OPS_ECC_CORRECTABLE))
    {
        FbxPtr = PcbPtr->Word.FbxPtr;

        // Get buffer
        CntlHdrPtr = (void *)PcbPtr->Info.CntlFetch.Dmx.BuffAddr;

        // Check signature
        if (CntlHdrPtr->Signature == CNTL_HDR_SIGNATURE)
        {
            Sqn = CntlHdrPtr->Sqn;
            Pba = PcbPtr->Info.CntlFetch.Dmx.Pba + 1;

            // Get max sqn
            if (Sqn > FbxPtr->Sqn)
            {
                FbxPtr->Sqn = Sqn;
            }

            // Traverse identity
            for (EntryIdx = 0;
                 EntryIdx < (SEGMENTS_PER_PAGE - 1);
                 EntryIdx++)
            {
                Identity = CntlHdrPtr->Identity[EntryIdx];
                Target = Identity & TARGET_MASK;

                if (Target == TARGET_DIR0)
                {
                    dir_init_dir1_entry(PcbPtr->Word.FbxPtr,
                                        Identity,
                                        Sqn,
                                        Pba);
                }

                else if (Target == TARGET_SI0)
                {
                    sxninfo_init_si1_entry(PcbPtr->Word.FbxPtr,
                                           Identity,
                                           Sqn,
                                           Pba);
                }

                else
                {
                    break;
                }

                Pba++;
            }
        }
    }

    if (PcbPtr->Info.CntlFetch.CurPba < CNTL_SEGMENTS_PER_FBX)
    {
        // Read next page
        cntldata_fetch_dev(PcbPtr);
    }
    else
    {
        // Done
        dm_notify_completion(PcbPtr);
        _sched_return_pcb(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : cntldata_write_update_si0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cntldata_write_update_si0 (PCB_STRUCT *PcbPtr)
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

	// next stage update directory
	cntldata_write_alloc_dir0(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : cntldata_write_update_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void cntldata_write_update_dir0 (PCB_STRUCT *PcbPtr)
{
    CNTL_CACHE_STRUCT *Dir0CachePtr;
    volatile DIR0_ENTRY_STRUCT *Dir0EntryPtr;
    unsigned long UserSxnIdx;
    DM_FBX_STRUCT *FbxPtr;
    PCB_STRUCT *WakePcbPtr;
    PBA_INT OldPba;
    PBA_INT NewPba;

    // Get dir0 entry derived from user sxn idx
    Dir0CachePtr = PcbPtr->Info.UserWrite.CntlCachePtr;
    Dir0EntryPtr = (void *)Dir0CachePtr->DataAddr;
    UserSxnIdx = PcbPtr->Info.UserWrite.UserSxnIdx;
    Dir0EntryPtr += (UserSxnIdx % DIR0_ENTRIES_PER_SXN);

    // Decode data to get the real pba
    OldPba = Dir0EntryPtr->UserPba ^ SCRAMBLE_WORD(UserSxnIdx);

    FbxPtr = PcbPtr->Word.FbxPtr;
    NewPba = PcbPtr->Info.UserWrite.Pba;

    if (OldPba == PcbPtr->Info.UserWrite.OldPba)
    {

        // set new pba to valid sxn
        blkrecord_ud_set_valid_sxn(FbxPtr,
                                   NewPba);
 
        // Invalidate old pba
        blkrecord_ud_set_invalid_sxn(FbxPtr,
                                     OldPba);

        // Update dir0 entry to new pba then set cache to dirty
        Dir0EntryPtr->UserPba
            = NewPba ^ SCRAMBLE_WORD(UserSxnIdx);
        CNTLDATA_SET_DIRTY(Dir0CachePtr,
                           FbxPtr);

        // Unlock dir0 cache
        CNTLDATA_UNLOCK_WRITE(Dir0CachePtr);
		
		// Unlock user sxn lock
		media_unlock_usersxn(FbxPtr,
							 UserSxnIdx);

		_sched_return_pcb(PcbPtr);
    }

    else
    {
        // Since old pba is not sync to userpba from dir0
        blkrecord_ud_move_blk(FbxPtr,
                              NewPba);

        // Unlock dir0 cache
        CNTLDATA_UNLOCK_WRITE(Dir0CachePtr);

		// Unlock user sxn lock
		media_unlock_usersxn(FbxPtr,
							 UserSxnIdx);

		_sched_return_pcb(PcbPtr);

        // In case the cache line is floating need to put back in clean list
        if (    (Dir0CachePtr->LockCnt == 0)
             && (Dir0CachePtr->DirtyCnt == 0))
        {
            // Put to clean list
            DIR_PUT_TO_CLEAN_LIST(Dir0CachePtr,FbxPtr->Dir);

            // Wake sleep pcb
            WakePcbPtr = CNTLDATA_GET_HEAD_RQST_WAITQ(FbxPtr->Dir);
            if (WakePcbPtr != BIT_NULL_PTR)
            {
                WakePcbPtr->Fn(WakePcbPtr);
            }
        }
    }

    return;
}


//=============================================================================
// $Log: CntlData.c,v $
// Revision 1.7  2014/04/30 13:34:46  rcantong
// 1. DEV: Support BlkInfo control data to monitor erase count
// 1.1 Added process for BlkInfo - BBantigue
//
// Revision 1.6  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.4  2013/12/05 13:06:33  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:44:22  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:15  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
