//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/RemapCntl/RemapCntl.c,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2014/05/19 05:02:10 $
// $Id: RemapCntl.c,v 1.3 2014/05/19 05:02:10 rcantong Exp $
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
#include "Dir.h"
#include "Freelist.h"
#include "SxnInfo.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "RemapCntlI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : remap_cntl_trigger_remap
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_cntl_trigger_remap (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba)
{
    PBA_INT BlkPba;
    PCB_STRUCT *PcbPtr;

    ASSERT(Pba < CNTL_SEGMENTS_PER_FBX);

    // Align to block PBA
    BlkPba = DM_ALIGN_TO(SEGMENTS_PER_BLK, Pba);

    if (blkrecord_cd_check_remap_blk(FbxPtr, BlkPba) == REMAP_BLK)
    {
        return;
    }

    blkrecord_cd_set_remap_blk(FbxPtr,
                               BlkPba);

    blkrecord_cd_set_q_blk(FbxPtr,
                           BlkPba);

    freelist_cd_chk_n_remove_cur_pba(FbxPtr,
                                     BlkPba);

    PcbPtr = _sched_get_pcb();
    PcbPtr->Word.FbxPtr = FbxPtr;
    PcbPtr->Info.RemapCd.Pba = BlkPba;

    remap_cntl_block(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : remap_cntl_block
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_cntl_block (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    PBA_INT SxnPba;
    IDENTITY_INT Identity;
    unsigned long Target;

    FbxPtr = PcbPtr->Word.FbxPtr;

    while (1)
    {
        SxnPba = blkrecord_cd_get_valid_sxn(FbxPtr,
                                            PcbPtr->Info.RemapCd.Pba);

        if (SxnPba == INVALID_MASK)
        {
            _sched_return_pcb(PcbPtr);
            break;
        }

        Identity = FbxPtr->CntlData.IdPtr[SxnPba];
        Target = Identity & TARGET_MASK;
        PcbPtr->Info.RemapCd.SxnIdx = SXNIDX(Identity);
        PcbPtr->Info.RemapCd.Pba = SxnPba;

        if (Target == TARGET_DIR0)
        {
            remap_cntl_write_alloc_dir0(PcbPtr);
            break;
        }
        else if (Target == TARGET_SI0)
        {
            remap_cntl_write_alloc_si0(PcbPtr);
            break;
        }
        else
        {
            PcbPtr->Info.RemapCd.Pba += SEGMENTS_PER_CNTL_SXN;
            if ((PcbPtr->Info.RemapCd.Pba % SEGMENTS_PER_BLK) == 0)
            {
                _sched_return_pcb(PcbPtr);
                break;
            }
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : remap_cntl_write_alloc_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_cntl_write_alloc_dir0 (PCB_STRUCT *PcbPtr)
{
    PCB_REMAP_CD_STRUCT *RemapCdPtr;
    BIT_STAT Stat;

    // Reuse our remap pcb
    RemapCdPtr = (void *)&PcbPtr->Info;

    Stat = dir_alloc_compact(PcbPtr->Word.FbxPtr,
                             RemapCdPtr->SxnIdx,
                             &RemapCdPtr->Dir0CachePtr);

    if (Stat == CNTLDATA_INCACHE)
    {
        remap_cntl_dirty_dir0(PcbPtr);
    }

    else if (Stat == CNTLDATA_INFLUSH)
    {
        CNTLDATA_UNLOCK_WRITE(RemapCdPtr->Dir0CachePtr);
        PcbPtr->Info.RemapCd.Pba += SEGMENTS_PER_CNTL_SXN;
        PcbPtr->Fn = remap_cntl_block;
        SCHED_POST_PCB(PcbPtr);
    }

    else if (Stat == CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = remap_cntl_dirty_dir0;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr,
                                    RemapCdPtr->Dir0CachePtr);
    }

    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        PcbPtr->Fn = remap_cntl_write_alloc_dir0;
        CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr,
                                   PcbPtr->Word.FbxPtr->Dir);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : remap_cntl_write_alloc_si0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_cntl_write_alloc_si0 (PCB_STRUCT *PcbPtr)
{
    PCB_REMAP_CD_STRUCT *RemapCdPtr;
    BIT_STAT Stat;

    RemapCdPtr = (void *)&PcbPtr->Info;

    Stat = sxninfo_alloc_compact(PcbPtr->Word.FbxPtr,
                                 RemapCdPtr->SxnIdx,
                                 &RemapCdPtr->Si0CachePtr);

    if (Stat == CNTLDATA_INCACHE)
    {
        remap_cntl_dirty_si0(PcbPtr);
    }

    else if (Stat == CNTLDATA_INFLUSH)
    {
        CNTLDATA_UNLOCK_WRITE(RemapCdPtr->Si0CachePtr);
        PcbPtr->Info.RemapCd.Pba += SEGMENTS_PER_CNTL_SXN;
        PcbPtr->Fn = remap_cntl_block;
        SCHED_POST_PCB(PcbPtr);
    }

    else if (Stat == CNTLDATA_INFETCH)
    {
        PcbPtr->Fn = remap_cntl_dirty_si0;
        CNTLDATA_PUT_TO_STATE_WAITQ(PcbPtr,
                                    RemapCdPtr->Si0CachePtr);
    }

    else
    {
        ASSERT(Stat == CNTLDATA_NOCACHE);
        PcbPtr->Fn = remap_cntl_write_alloc_si0;
        CNTLDATA_PUT_TO_RQST_WAITQ(PcbPtr,
                                   PcbPtr->Word.FbxPtr->SxnInfo);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : remap_cntl_dirty_dir0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_cntl_dirty_dir0 (PCB_STRUCT *PcbPtr)
{
    CNTL_CACHE_STRUCT *Dir0CachePtr;

    Dir0CachePtr = PcbPtr->Info.RemapCd.Dir0CachePtr;

    // Set cache to dirty
    COMPACT_CD_SET_DIRTY(Dir0CachePtr,
                         PcbPtr->Word.FbxPtr);

    // Unlock dir0 cache
    CNTLDATA_UNLOCK_WRITE(Dir0CachePtr);

    PcbPtr->Info.RemapCd.Pba += SEGMENTS_PER_CNTL_SXN;
    if ((PcbPtr->Info.RemapCd.Pba % SEGMENTS_PER_BLK) != 0)
    {
        PcbPtr->Fn = remap_cntl_block;
        SCHED_POST_PCB(PcbPtr);
    }

    else
    {
        _sched_return_pcb(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : remap_cntl_dirty_si0
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void remap_cntl_dirty_si0 (PCB_STRUCT *PcbPtr)
{
    CNTL_CACHE_STRUCT *Si0CachePtr;

    Si0CachePtr = PcbPtr->Info.RemapCd.Si0CachePtr;

    // Set cache to dirty
    COMPACT_CD_SET_DIRTY(Si0CachePtr,
                         PcbPtr->Word.FbxPtr);

    // Unlock dir0 cache
    CNTLDATA_UNLOCK_WRITE(Si0CachePtr);

    PcbPtr->Info.RemapCd.Pba += SEGMENTS_PER_CNTL_SXN;
    if ((PcbPtr->Info.RemapCd.Pba % SEGMENTS_PER_BLK) != 0)
    {
        PcbPtr->Fn = remap_cntl_block;
        SCHED_POST_PCB(PcbPtr);
    }

    else
    {
        _sched_return_pcb(PcbPtr);
    }

    return;
}


//=============================================================================
// $Log: RemapCntl.c,v $
// Revision 1.3  2014/05/19 05:02:10  rcantong
// 1. DEV: Support control remap
// 1.1 Added control remapping functions - BBantigue
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
