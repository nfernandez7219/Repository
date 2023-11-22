//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/BlkInfo/BlkInfo.c,v $
// $Revision: 1.4 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:58 $
// $Id: BlkInfo.c,v 1.4 2014/05/19 04:48:58 rcantong Exp $
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
#include "BlkRecord.h"
#include "CntlData.h"
#include "CntlDataCommon.h"
#include "Dm.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "EdcFwVersion.h"
#include "Err.h"
#include "NvConfig.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "BlkInfoI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
CNTL_CACHE_STRUCT BlkInfoCache[FBX_CNT];
L1CACHE_ALIGN(BlkInfoCache);

BLKINFO_PARM_STRUCT BlkInfoParm[FBX_CNT];
L1CACHE_ALIGN(BlkInfoParm);

PCB_ALIGN_STRUCT BlkInfoPcb[FBX_CNT];
L1CACHE_ALIGN(BlkInfoPcb);
#pragma BSS()

#pragma BSS(".dm_buffer")
unsigned char BlkInfoBuffer[FBX_CNT][FLASH_BLK_SIZE];
L1CACHE_ALIGN(BlkInfoBuffer);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : blkinfo_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_init_malloc (unsigned long FbxIdx)
{
    CNTL_CACHE_STRUCT *CachePtr;
    BLKINFO_FBX_STRUCT *BlkInfoPtr;

    // Initialize defects cache
    BlkInfoPtr = &DmFbx[FbxIdx].BlkInfo;
    CachePtr = &BlkInfoCache[FbxIdx];
    CachePtr->FbxIdx = FbxIdx;
    CachePtr->Identity = TARGET_BLKINFO;
    CachePtr->DataAddr = (unsigned long)&BlkInfoBuffer[FbxIdx][0];
    CachePtr->DirtyCnt = 0;
    CachePtr->LockCnt = 0;

    // Initialize BlkInfo Buffer
    dm_fill_random_pattern((void *)CachePtr->DataAddr,
                           FLASH_BLK_SIZE);

    BlkInfoPtr->BlkInfoCachePtr = CachePtr;
    // Initialize Shadow copy of EraseCnt
    BlkInfoPtr->EraseCntPtr
        = (void *)&(SysConfigCurr.LogInfo.FbxLogInfo[FbxIdx].EraseCnt);
    BlkInfoPtr->BlkInfoParmPtr = (void *)&(BlkInfoParm[FbxIdx]);

    BlkInfoPtr->BlkInfoParmPtr->BlkInfoPcbPtr = (void *)&BlkInfoPcb[FbxIdx];
    BlkInfoPtr->BlkInfoParmPtr->MinEraseCnt = 0;
    BlkInfoPtr->BlkInfoParmPtr->MaxEraseCnt = 0;
    BlkInfoPtr->BlkInfoParmPtr->RunningTotalBlkErase = 0;
    BlkInfoPtr->BlkInfoParmPtr->CurrDevIdx = 0xFFFFFFFF;
    BlkInfoPtr->BlkInfoParmPtr->Sqn = 0;
    BlkInfoPtr->BlkInfoParmPtr->UpdateCnt = 0;

    // DevIdx to 12 and Set 0-3 and 22-31 to Bad
    BlkInfoPtr->BlkInfoParmPtr->DevIdx = BLKINFO_DEF_DEV;
    BlkInfoPtr->BlkInfoParmPtr->BadBmp = 0xFFCF;

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_init_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_init_fbx (PCB_STRUCT *PcbPtr)
{
    PCB_STRUCT *ChildPcbPtr;

    // Deploy child pcb
    ChildPcbPtr = _sched_get_pcb();
    ChildPcbPtr->Fn = blkinfo_fetch_dev;
    ChildPcbPtr->Word.FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];
    SCHED_POST_PCB(ChildPcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_build_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_build_fbx (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    volatile BLKINFO_STRUCT *BlkInfoDataPtr;
    unsigned long EraseCnt;
    unsigned long BlkIdx;
    PCB_STRUCT *ChildPcbPtr;

    FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];

    BlkInfoDataPtr = (void *)FbxPtr->BlkInfo.BlkInfoCachePtr->DataAddr;

    // Initialize Blk Info Buffer
    util_init_pattern((void *)BlkInfoDataPtr,
                      FLASH_BLK_SIZE,
                      INIT_PATTERN_LO_VALUE);

    BlkInfoDataPtr->BlkInfoHdr.Signature = CNTL_HDR_SIGNATURE;
    BlkInfoDataPtr->BlkInfoHdr.Sqn = 1;
    BlkInfoDataPtr->BlkInfoHdr.Identity = TARGET_BLKINFO;

    // Force BlkInfo building and initialize EraseCnt to non-zero
    EraseCnt = 0;
    if (FwVersion.EraseBoardFlag  != OFF)
    {
        EraseCnt = PE_INIT_CNT;
    }

    for (BlkIdx = 0;
         BlkIdx < BLKS_PER_FBX;
         BlkIdx++)
    {
        BlkInfoDataPtr->BlkInfoEntry[BlkIdx] = EraseCnt;
    }

    PcbPtr->Info.DmInit.DeployCtr = 1;

    ChildPcbPtr = _sched_get_pcb();
    ChildPcbPtr->Fn = blkinfo_build_flush_blkinfo;
    ChildPcbPtr->Word.FbxPtr = FbxPtr;

    SCHED_POST_PCB(ChildPcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_update_entry
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_update_entry (DM_FBX_STRUCT *FbxPtr,
                           PBA_INT Pba)
{
    BLKINFO_FBX_STRUCT *BlkInfoPtr;
    volatile BLKINFO_STRUCT *BlkInfoDataPtr;
    unsigned long SysBlkIdx;
    unsigned long EraseCnt;
    PCB_STRUCT *PcbPtr;

    BlkInfoPtr = (void *)&(FbxPtr->BlkInfo);
    BlkInfoDataPtr = (void *)BlkInfoPtr->BlkInfoCachePtr->DataAddr;

    SysBlkIdx = Pba / SEGMENTS_PER_BLK;
    BlkInfoDataPtr->BlkInfoEntry[SysBlkIdx]++;
    EraseCnt = BlkInfoDataPtr->BlkInfoEntry[SysBlkIdx];
    BlkInfoPtr->BlkInfoParmPtr->RunningTotalBlkErase++;
    BlkInfoPtr->BlkInfoParmPtr->UpdateCnt++;

    if (EraseCnt < BlkInfoPtr->BlkInfoParmPtr->MinEraseCnt)
    {
        BlkInfoPtr->BlkInfoParmPtr->MinEraseCnt = EraseCnt;
    }
    else if (EraseCnt > BlkInfoPtr->BlkInfoParmPtr->MaxEraseCnt)
    {
        BlkInfoPtr->BlkInfoParmPtr->MaxEraseCnt = EraseCnt;
        *BlkInfoPtr->EraseCntPtr = EraseCnt;
    }

    if (BlkInfoPtr->BlkInfoParmPtr->UpdateCnt >= BLKINFO_UPDATE_THRES)
    {
        PcbPtr = ((void *)BlkInfoPtr->BlkInfoParmPtr->BlkInfoPcbPtr);
        PcbPtr->Fn = blkinfo_flush_blkinfo;
        PcbPtr->Word.FbxPtr = FbxPtr;

        // Reset UpdateCnt
        BlkInfoPtr->BlkInfoParmPtr->UpdateCnt = 0;

        // Flush BlkInfo
        SCHED_POST_PCB(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_get_min_erase_cnt
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long blkinfo_get_min_erase_cnt (DM_FBX_STRUCT *FbxPtr,
                                         unsigned long BlkIdx)
{
    BLKINFO_PARM_STRUCT *BlkInfoParmPtr;

    BlkInfoParmPtr = (void *)(FbxPtr->BlkInfo.BlkInfoParmPtr);

    return BlkInfoParmPtr->MinEraseCnt;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_get_max_erase_cnt
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long blkinfo_get_max_erase_cnt (DM_FBX_STRUCT *FbxPtr,
                                         unsigned long BlkIdx)
{
    BLKINFO_PARM_STRUCT *BlkInfoParmPtr;

    BlkInfoParmPtr = (void *)(FbxPtr->BlkInfo.BlkInfoParmPtr);

    return BlkInfoParmPtr->MaxEraseCnt;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : blkinfo_fetch_dev
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_fetch_dev (PCB_STRUCT *PcbPtr)
{
    BLKINFO_FBX_STRUCT *BlkInfoPtr;

    BlkInfoPtr = (void *)&PcbPtr->Word.FbxPtr->BlkInfo;

    // Prepare dmx ops parameters
    PcbPtr->Info.DmxRead.Pba
        = CALC_PBA(BlkInfoPtr->BlkInfoParmPtr->DevIdx, 0, 0);
    PcbPtr->Info.DmxRead.BuffAddr = BlkInfoPtr->BlkInfoCachePtr->DataAddr;
    PcbPtr->Info.DmxRead.SegCnt = SEGMENTS_PER_BLK;
    PcbPtr->Fn = blkinfo_fetch_dev_cb;
    dmx_ops_read_in_place(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_fetch_dev_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_fetch_dev_cb (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long FbxIdx;
    BLKINFO_FBX_STRUCT *BlkInfoPtr;
    volatile BLKINFO_STRUCT *BlkInfoDataPtr;
    unsigned long Sqn;
    unsigned long DevIdx;

    FbxPtr = PcbPtr->Word.FbxPtr;
    FbxIdx = FbxPtr->FbxIdx;
    BlkInfoPtr = (void *)&FbxPtr->BlkInfo;
    BlkInfoDataPtr = (void *)BlkInfoPtr->BlkInfoCachePtr->DataAddr;
    Sqn = BlkInfoDataPtr->BlkInfoHdr.Sqn;
    DevIdx = GET_DEV_FROM_PBA(PcbPtr->Info.DmxRead.Pba);
    ASSERT(DevIdx < BLKINFO_END_DEV);

    if (    (PcbPtr->Info.DmxRead.Stat == SUCCESSFUL)
         || (PcbPtr->Info.DmxRead.Stat == DMX_OPS_ECC_CORRECTABLE))
    {
        if (    (Sqn > BlkInfoPtr->BlkInfoParmPtr->Sqn)
             && (BlkInfoDataPtr->BlkInfoHdr.Signature == CNTL_HDR_SIGNATURE)
             && (BlkInfoDataPtr->BlkInfoHdr.Identity == TARGET_BLKINFO))
        {
            BlkInfoPtr->BlkInfoParmPtr->Sqn = Sqn;
            BlkInfoPtr->BlkInfoParmPtr->CurrDevIdx = DevIdx;
        }
    }

    else if (    (PcbPtr->Info.DmxRead.Stat == DMX_OPS_ECC_UNCORRECTABLE)
             ||  (PcbPtr->Info.DmxRead.Stat > DMX_OPS_ERASED))
    {
        BlkInfoPtr->BlkInfoParmPtr->BadBmp |= (1 << DevIdx);
    }

    // Proceed to next Blk
    DevIdx++;

    if (DevIdx < BLKINFO_END_DEV)
    {
        BlkInfoPtr->BlkInfoParmPtr->DevIdx = DevIdx;
        blkinfo_fetch_dev(PcbPtr);
    }

    else
    {
        if (BlkInfoPtr->BlkInfoParmPtr->CurrDevIdx == 0xFFFFFFFF)
        {
            // No BlkInfo found
            DmFlagParm.BIStat[FbxIdx] = NOT_SUCCESSFUL;

            SCHED_POST_PCB(FbxPtr->ParentPcbPtr);
            _sched_return_pcb(PcbPtr);
        }
        else
        {
            DmFlagParm.BIStat[FbxIdx] = SUCCESSFUL;

            // Done. Proceed to checking of BlkInfo
            blkinfo_fetch_blkinfo(PcbPtr);
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_fetch_blkinfo
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_fetch_blkinfo (PCB_STRUCT *PcbPtr)
{
    BLKINFO_FBX_STRUCT *BlkInfoPtr;

    BlkInfoPtr = (void *)&PcbPtr->Word.FbxPtr->BlkInfo;

    PcbPtr->Info.DmxRead.Pba
        = CALC_PBA(BlkInfoPtr->BlkInfoParmPtr->CurrDevIdx, 0, 0);
    PcbPtr->Info.DmxRead.BuffAddr = BlkInfoPtr->BlkInfoCachePtr->DataAddr;
    PcbPtr->Info.DmxRead.SegCnt = SEGMENTS_PER_BLK;
    PcbPtr->Fn = blkinfo_check_blkinfo;

    dmx_ops_read_in_place(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_check_blkinfo
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_check_blkinfo (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    BLKINFO_FBX_STRUCT *BlkInfoPtr;
    volatile BLKINFO_STRUCT *BlkInfoDataPtr;
    unsigned long Sqn;
    unsigned long long TotalBlkErase = 0;
    unsigned long BlkIdx;
    unsigned long MaxErase = 0;

    FbxPtr = PcbPtr->Word.FbxPtr;
    BlkInfoPtr = (void *)&FbxPtr->BlkInfo;
    BlkInfoDataPtr = (void *)BlkInfoPtr->BlkInfoCachePtr->DataAddr;
    Sqn = BlkInfoDataPtr->BlkInfoHdr.Sqn;

    if (    (PcbPtr->Info.DmxRead.Stat == SUCCESSFUL)
         || (PcbPtr->Info.DmxRead.Stat == DMX_OPS_ECC_CORRECTABLE))
    {
        ASSERT(BlkInfoDataPtr->BlkInfoHdr.Signature == CNTL_HDR_SIGNATURE);
        ASSERT(BlkInfoDataPtr->BlkInfoHdr.Identity == TARGET_BLKINFO);
        ASSERT(Sqn == BlkInfoPtr->BlkInfoParmPtr->Sqn);

        // Initialize MaxErase and MinErse
        for (BlkIdx = 0;
             BlkIdx < BLKS_PER_FBX;
             BlkIdx++)
        {
            TotalBlkErase += BlkInfoDataPtr->BlkInfoEntry[BlkIdx];

            if (BlkInfoDataPtr->BlkInfoEntry[BlkIdx] > MaxErase)
            {
                MaxErase = BlkInfoDataPtr->BlkInfoEntry[BlkIdx];
            }
        }

        // Sync to Parm and Nv
        *BlkInfoPtr->EraseCntPtr = MaxErase;
        BlkInfoPtr->BlkInfoParmPtr->MaxEraseCnt = MaxErase;
        BlkInfoPtr->BlkInfoParmPtr->RunningTotalBlkErase = TotalBlkErase;
    }

    else
    {
        // This should not happen since the drive was supposedly built
        err_gross();
    }

    SCHED_POST_PCB(FbxPtr->ParentPcbPtr);
    _sched_return_pcb(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_build_flush_blkinfo
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_build_flush_blkinfo (PCB_STRUCT *PcbPtr)
{
    BLKINFO_FBX_STRUCT *BlkInfoPtr;

    BlkInfoPtr = (void *)&PcbPtr->Word.FbxPtr->BlkInfo;

    PcbPtr->Info.DmxWrite.Pba
        = CALC_PBA(BlkInfoPtr->BlkInfoParmPtr->DevIdx, 0, 0);
    PcbPtr->Info.DmxWrite.BuffAddr = BlkInfoPtr->BlkInfoCachePtr->DataAddr;
    PcbPtr->Info.DmxWrite.SegCnt = SEGMENTS_PER_BLK;
    PcbPtr->Fn = blkinfo_build_flush_blkinfo_cb;

    dmx_ops_write_in_place(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_build_flush_blkinfo_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_build_flush_blkinfo_cb (PCB_STRUCT *PcbPtr)
{
    BLKINFO_FBX_STRUCT *BlkInfoPtr;
    volatile BLKINFO_STRUCT *BlkInfoDataPtr;
    unsigned long DevIdx;

    BlkInfoPtr = (void *)&PcbPtr->Word.FbxPtr->BlkInfo;
    BlkInfoDataPtr = (void *)BlkInfoPtr->BlkInfoCachePtr->DataAddr;
    DevIdx = GET_DEV_FROM_PBA(PcbPtr->Info.DmxRead.Pba);

    ASSERT(DevIdx == BlkInfoPtr->BlkInfoParmPtr->DevIdx);

    if (PcbPtr->Info.DmxWrite.Stat != SUCCESSFUL)
    {
        // Set BadBmp and Increment Pba and SQN
        DevIdx++;
        ASSERT(DevIdx < BLKINFO_END_DEV);

        BlkInfoPtr->BlkInfoParmPtr->DevIdx = CALC_PBA(DevIdx, 0, 0);
        BlkInfoPtr->BlkInfoParmPtr->BadBmp |= (1 << DevIdx);
        BlkInfoDataPtr->BlkInfoHdr.Sqn++;

        blkinfo_build_flush_blkinfo(PcbPtr);

        return;
    }

    // Update CurrDevIdx
    BlkInfoPtr->BlkInfoParmPtr->CurrDevIdx = DevIdx;

    dm_notify_completion(PcbPtr);
    _sched_return_pcb(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_flush_blkinfo
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_flush_blkinfo (PCB_STRUCT *PcbPtr)
{
    BLKINFO_PARM_STRUCT *BlkInfoParmPtr;
    volatile BLKINFO_STRUCT *BlkInfoDataPtr;
    unsigned long DevIdx;
    unsigned long BadBmp;

    BlkInfoParmPtr
        = (void *)PcbPtr->Word.FbxPtr->BlkInfo.BlkInfoParmPtr;
    BlkInfoDataPtr
        = (void *)PcbPtr->Word.FbxPtr->BlkInfo.BlkInfoCachePtr->DataAddr;

    DevIdx = BlkInfoParmPtr->CurrDevIdx;
    BadBmp = BlkInfoParmPtr->BadBmp;
    ASSERT(BadBmp != 0xFFFFFFFF);

    while (1)
    {
        // Check Dev in in BadBmp
        DevIdx++;

        // Must Check if PBA has already been used and/or overflow
        if (DevIdx == BLKINFO_END_DEV)
        {
            DevIdx = BLKINFO_DEF_DEV;
        }

        if ((1 << DevIdx) & BadBmp)
        {
            continue;
        }

        break;
    }

    //Update Sqn and DevIdx not CurrDevIdx; Increment TotalRunning BlkErase
    BlkInfoParmPtr->DevIdx = DevIdx;
    BlkInfoParmPtr->RunningTotalBlkErase++;
    BlkInfoDataPtr->BlkInfoHdr.Sqn++;

    PcbPtr->Info.DmxWrite.Pba = CALC_PBA(DevIdx, 0, 0);
    PcbPtr->Info.DmxWrite.BuffAddr = (unsigned long)BlkInfoDataPtr;
    PcbPtr->Info.DmxWrite.SegCnt = SEGMENTS_PER_BLK;
    PcbPtr->Fn = blkinfo_flush_blkinfo_cb;

    dmx_ops_write_in_place(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : blkinfo_flush_blkinfo_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void blkinfo_flush_blkinfo_cb (PCB_STRUCT *PcbPtr)
{
    BLKINFO_FBX_STRUCT *BlkInfoPtr;
    volatile BLKINFO_STRUCT *BlkInfoDataPtr;
    unsigned long DevIdx;

    BlkInfoPtr
        = (void *)&PcbPtr->Word.FbxPtr->BlkInfo;
    BlkInfoDataPtr
        = (void *)BlkInfoPtr->BlkInfoCachePtr->DataAddr;

    DevIdx = GET_DEV_FROM_PBA(PcbPtr->Info.DmxWrite.Pba);
    ASSERT(DevIdx == BlkInfoPtr->BlkInfoParmPtr->DevIdx);

    if (PcbPtr->Info.DmxWrite.Stat != SUCCESSFUL)
    {
        // Set BadBmp
        BlkInfoPtr->BlkInfoParmPtr->BadBmp |= (1 << DevIdx);

        // We may need to increment the EraseCnt of these blks
        blkinfo_flush_blkinfo(PcbPtr);

        return;
    }

    // Update BlkInfoParm Current DevIdx
    BlkInfoPtr->BlkInfoParmPtr->CurrDevIdx = DevIdx;

    return;
}


//=============================================================================
// $Log: BlkInfo.c,v $
// Revision 1.4  2014/05/19 04:48:58  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.3  2014/04/30 13:34:46  rcantong
// 1. DEV: Support BlkInfo control data to monitor erase count
// 1.1 Added process for BlkInfo - BBantigue
//
// Revision 1.2  2013/08/08 16:44:22  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:10  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
