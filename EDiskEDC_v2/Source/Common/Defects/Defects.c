//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Defects/Defects.c,v $
// $Revision: 1.9 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:58 $
// $Id: Defects.c,v 1.9 2014/05/19 04:48:58 rcantong Exp $
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
#include "Defects.h"
#include "Dm.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "Err.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "DefectsI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

CNTL_CACHE_STRUCT DefectsCache[FBX_CNT];
L1CACHE_ALIGN(DefectsCache);

#pragma BSS(".dm_buffer")
unsigned char DefectsBuffer[FBX_CNT][FLASH_BLK_SIZE];
L1CACHE_ALIGN(DefectsBuffer);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : defects_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_init_malloc (unsigned long FbxIdx)
{
    CNTL_CACHE_STRUCT *CachePtr;
    DM_FBX_STRUCT *FbxPtr;

    // Initialize defects cache
    CachePtr = &DefectsCache[FbxIdx];
    CachePtr->FbxIdx = FbxIdx;
    CachePtr->Identity = TARGET_DEFECTSBLK;
    CachePtr->DataAddr = (unsigned long)&DefectsBuffer[FbxIdx][0];
    CachePtr->DirtyCnt = 0;
    CachePtr->LockCnt = 0;

    // Initialize fbx dccm
    FbxPtr = &DmFbx[FbxIdx];
    FbxPtr->DefectsCachePtr = CachePtr;

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_init_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_init_fbx (PCB_STRUCT *PcbPtr)
{
    PCB_STRUCT *ChildPcbPtr;

    // Deploy child pcb
    ChildPcbPtr = _sched_get_pcb();
    ChildPcbPtr->Fn = defects_fetch_defectsblk;
    ChildPcbPtr->Word.FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];
    SCHED_POST_PCB(ChildPcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_build_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_build_fbx (PCB_STRUCT *PcbPtr)
{
    PCB_STRUCT *ChildPcbPtr;
    volatile DEFECTS_BLK_STRUCT *BlkPtr;

    // Deploy child pcb
    ChildPcbPtr = _sched_get_pcb();
    ChildPcbPtr->Fn = defects_flush_defectsblk;
    ChildPcbPtr->Word.FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];
    BlkPtr = (void *)ChildPcbPtr->Word.FbxPtr->DefectsCachePtr->DataAddr;
    BlkPtr->DefectsSignature = DEFECTS_BLOCK;
    BlkPtr->BuildFlag = DmFlagParm.BuildFlag;
    SCHED_POST_PCB(ChildPcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_erase_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_erase_fbx (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long DevIdx;
    PCB_STRUCT *ChildPcbPtr;

    FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];
    PcbPtr->Info.DmInit.DeployCtr = DEV_CNT;

    // Deploy child pcbs
    for (DevIdx = 0;
         DevIdx < DEV_CNT;
         DevIdx++)
    {
        ChildPcbPtr = _sched_get_pcb();
        ChildPcbPtr->Fn = defects_erase_dev;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;

        // Erase baseblk too
        if (DevIdx == 0)
        {
            ChildPcbPtr->Info.DmxErase.Pba = CALC_PBA(DevIdx, 0, 0);
        }
        else
        {
            ChildPcbPtr->Info.DmxErase.Pba = CALC_PBA(DevIdx, 1, 0);
        }

        SCHED_POST_PCB(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_erase_all_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_erase_all_fbx (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long DevIdx;
    PCB_STRUCT *ChildPcbPtr;

    FbxPtr = &DmFbx[PcbPtr->Word.FbxIdx];
    PcbPtr->Info.DmInit.DeployCtr = DEV_CNT;

    // Deploy child pcbs
    for (DevIdx = 0;
         DevIdx < DEV_CNT;
         DevIdx++)
    {
        ChildPcbPtr = _sched_get_pcb();
        ChildPcbPtr->Fn = defects_erase_dev;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;

        // Erase baseblk too
        if (DevIdx < 22)
        {
            ChildPcbPtr->Info.DmxErase.Pba = CALC_PBA(DevIdx, 1, 0);
        }
        else
        {
            ChildPcbPtr->Info.DmxErase.Pba = CALC_PBA(DevIdx, 0, 0);
        }

        SCHED_POST_PCB(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_scan_mfg_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_scan_mfg_fbx (PCB_STRUCT *PcbPtr)
{
    unsigned long FbxIdx;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long DevIdx;
    PCB_STRUCT *ChildPcbPtr;

    FbxIdx = PcbPtr->Word.FbxIdx;
    FbxPtr = &DmFbx[FbxIdx];
    PcbPtr->Info.DmInit.DeployCtr = DEV_CNT;
    DmFlagParm.FbxStat[FbxIdx] = FBX_BUILD;

    FbxPtr->MfgDefectsCnt = 0;
    FbxPtr->TotalDefectsCnt = 0;
    dm_fill_random_pattern(DefectsBuffer[FbxIdx],
                           FLASH_BLK_SIZE);

    // Add all block 0 as reserved blks
    for(DevIdx = 0;
        DevIdx < DEV_CNT;
        DevIdx++)
    {
        defects_add_defect_entry(FbxPtr,
                                 CALC_PBA(DevIdx, 0, 0),
                                 DEFECTS_RESERVED_BLK);
    }

    // Deploy child pcbs
    for (DevIdx = 0;
         DevIdx < DEV_CNT;
         DevIdx++)
    {
        ChildPcbPtr = _sched_get_pcb();
        ChildPcbPtr->Fn = defects_scan_mfg_dev;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;
        ChildPcbPtr->Info.DmxRead.Pba = CALC_PBA(DevIdx, 1, 0);
        ChildPcbPtr->Info.DmxRead.BuffAddr = GET_DEV_BUFF_ADDR(FbxIdx, DevIdx);
        ChildPcbPtr->Info.DmxRead.SegCnt = 1;

        dmx_ops_read(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_thorough_scan_erase_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_thorough_scan_erase_fbx (PCB_STRUCT *PcbPtr)
{
    unsigned long FbxIdx;
    DM_FBX_STRUCT *FbxPtr;
    unsigned long DevIdx;
    PCB_STRUCT *ChildPcbPtr;

    FbxIdx = PcbPtr->Word.FbxIdx;
    DmFlagParm.FbxStat[FbxIdx] = FBX_BUILD;
    FbxPtr = &DmFbx[FbxIdx];

    PcbPtr->Info.DmInit.DeployCtr = DEV_CNT;

    // Deploy child pcbs
    for (DevIdx = 0;
         DevIdx < DEV_CNT;
         DevIdx++)
    {
        ChildPcbPtr = _sched_get_pcb();
        ChildPcbPtr->Fn = defects_erase_dev;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;
        ChildPcbPtr->Info.DmxErase.Pba = CALC_PBA(DevIdx, 1, 0);
        SCHED_POST_PCB(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_thorough_scan_write_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_thorough_scan_write_fbx (PCB_STRUCT *PcbPtr)
{
    unsigned long FbxIdx;
    unsigned long DevIdx;
    DM_FBX_STRUCT *FbxPtr;
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
        ChildPcbPtr->Fn = defects_thorough_scan_write_dev;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;

        ChildPcbPtr->Info.DmxWrite.BuffAddr = GET_DEV_BUFF_ADDR(FbxIdx, DevIdx);
        defects_fill_random_pattern((void *)ChildPcbPtr->Info.DmxWrite.BuffAddr,
                                    FLASH_PAGE_SIZE);
        ChildPcbPtr->Info.DmxWrite.Stat = SUCCESSFUL;
        ChildPcbPtr->Info.DmxWrite.SegCnt = SEGMENTS_PER_PAGE;

        ChildPcbPtr->Info.Screening.DevIdx = DevIdx;
        ChildPcbPtr->Info.Screening.BlkIdx = 0;
        ChildPcbPtr->Info.Screening.SegIdx = 0;
        SCHED_POST_PCB(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_thorough_scan_read_fbx
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_thorough_scan_read_fbx (PCB_STRUCT *PcbPtr)
{
    unsigned long FbxIdx;
    unsigned long DevIdx;
    DM_FBX_STRUCT *FbxPtr;
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
        ChildPcbPtr->Fn = defects_thorough_scan_read_dev;
        ChildPcbPtr->Word.FbxPtr = FbxPtr;

        ChildPcbPtr->Info.DmxRead.BuffAddr = GET_DEV_BUFF_ADDR(FbxIdx, DevIdx);
        ChildPcbPtr->Info.DmxRead.Stat = SUCCESSFUL;
        ChildPcbPtr->Info.DmxRead.SegCnt = SEGMENTS_PER_PAGE;

        ChildPcbPtr->Info.Screening.DevIdx = DevIdx;
        ChildPcbPtr->Info.Screening.BlkIdx = 1;
        ChildPcbPtr->Info.Screening.SegIdx = -SEGMENTS_PER_PAGE;
        SCHED_POST_PCB(ChildPcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_add_defect_entry
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_add_defect_entry (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba,
                               unsigned long ErrorType)
{
    unsigned long EntryIdx;
    DEFECTS_BLK_STRUCT *BlkPtr;
    volatile DEFECTS_ENTRY_STRUCT *EntryPtr;

    // Check if entry slot available
    EntryIdx = FbxPtr->TotalDefectsCnt;
    if (EntryIdx >= DEFECTS_ALLOWED_BAD_CNT)
    {
        return;
    }

    // Check bitmap first if already added
    Pba &= ~(SEGMENTS_PER_BLK - 1);

    if (Pba < CNTL_SEGMENTS_PER_FBX)
    {
        if (blkrecord_cd_check_bad_blk(FbxPtr, Pba) == BAD_BLK)
        {
            return;
        }

        blkrecord_cd_set_bad_blk(FbxPtr,
                                 Pba);
    }

    else
    {
        if (blkrecord_ud_check_bad_blk(FbxPtr, Pba) == BAD_BLK)
        {
            return;
        }

        blkrecord_ud_set_bad_blk(FbxPtr,
                                 Pba);
    }

    BlkPtr = (DEFECTS_BLK_STRUCT *)FbxPtr->DefectsCachePtr->DataAddr;
    EntryPtr = (void *)(DEFECTS_ENTRY_STRUCT *)BlkPtr->DefectsEntry;
    EntryPtr += EntryIdx;
    EntryPtr->Pba = Pba;
    EntryPtr->ErrorType = (unsigned short)ErrorType;
    EntryPtr->ErrorStage = (unsigned short)DmFlagParm.SystemStat;

    // Increment counter
    FbxPtr->TotalDefectsCnt++;
    if (ErrorType == DEFECTS_MFG_DEFECT_ERR)
    {
        FbxPtr->MfgDefectsCnt++;
    }

    if (DmFlagParm.SystemStat == SYSTEM_READY)
    {
        defects_post_flush(FbxPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : defects_fetch_defectsblk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_fetch_defectsblk (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;

    // Prepare dmx ops parameters
    FbxPtr = PcbPtr->Word.FbxPtr;
    PcbPtr->Info.DmxRead.Pba = CALC_PBA(22, 0, 0);
    PcbPtr->Info.DmxRead.BuffAddr = FbxPtr->DefectsCachePtr->DataAddr;
    PcbPtr->Info.DmxRead.SegCnt = DEFECTS_SEGMENTS_PER_BLK;
    PcbPtr->Fn = defects_fetch_defectsblk_cb;

    dmx_ops_read_in_place(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_fetch_defectsblk_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_fetch_defectsblk_cb (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;
    unsigned long FbxIdx;
    DEFECTS_BLK_STRUCT *BlkPtr;
    volatile DEFECTS_ENTRY_STRUCT *EntryPtr;
    unsigned long EntryIdx;
    BIT_STAT Pba;

    FbxPtr = PcbPtr->Word.FbxPtr;
    FbxIdx = FbxPtr->FbxIdx;
    BlkPtr = (DEFECTS_BLK_STRUCT *)FbxPtr->DefectsCachePtr->DataAddr;

    if (     (    (PcbPtr->Info.DmxRead.Stat == SUCCESSFUL)
               || (PcbPtr->Info.DmxRead.Stat == DMX_OPS_ECC_CORRECTABLE))
         &&  (BlkPtr->DefectsSignature == DEFECTS_BLOCK))
    {
        // Traverse defects list to populate bad bitmap
        EntryPtr = (void *)(DEFECTS_ENTRY_STRUCT *)BlkPtr->DefectsEntry;

        DmFlagParm.DefectsStat[FbxIdx] = BlkPtr->BuildFlag;

        for (EntryIdx = 0;
             EntryIdx < DEFECTS_ALLOWED_BAD_CNT;
             EntryIdx++)
        {
            Pba = EntryPtr->Pba;

            // Last indicator
            if ((Pba & INVALID_MASK) != 0)
            {
                break;
            }

            else
            {
                // Check if cntl pba
                if (Pba < CNTL_SEGMENTS_PER_FBX)
                {
                    blkrecord_cd_set_bad_blk(FbxPtr,
                                             Pba);

                    blkrecord_cd_put_to_temp_q(FbxPtr,
                                               Pba);
                }

                // Else user pba
                else
                {
                    blkrecord_ud_set_bad_blk(FbxPtr,
                                             Pba);

                    blkrecord_ud_put_to_temp_q(FbxPtr,
                                               Pba);
                }
            }

            // Mfg defect
            if (EntryPtr->ErrorType == DEFECTS_MFG_DEFECT_ERR)
            {
                FbxPtr->MfgDefectsCnt++;
            }

            FbxPtr->TotalDefectsCnt++;
            EntryPtr++;
        }
    }

    else
    {
        DmFlagParm.DefectsStat[FbxIdx] = BUILD_NO_DEFECTS;
    }

    SCHED_POST_PCB(FbxPtr->ParentPcbPtr);
    _sched_return_pcb(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_flush_defectsblk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_flush_defectsblk (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;

    // Prepare dmx ops parameters
    FbxPtr = PcbPtr->Word.FbxPtr;
    PcbPtr->Info.DmxWrite.Pba = CALC_PBA(22, 0, 0);
    PcbPtr->Info.DmxWrite.BuffAddr = FbxPtr->DefectsCachePtr->DataAddr;
    PcbPtr->Info.DmxWrite.SegCnt = DEFECTS_SEGMENTS_PER_BLK;
    PcbPtr->Fn = defects_flush_defectsblk_cb;

    dmx_ops_write_in_place(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_flush_defectsblk_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_flush_defectsblk_cb (PCB_STRUCT *PcbPtr)
{
    if (PcbPtr->Info.DmxWriteInPlace.Stat != SUCCESSFUL)
    {
        err_gross();
    }

    PcbPtr->Word.FbxPtr->DefectsCachePtr->State = CNTLDATA_INCACHE;
    SCHED_POST_PCB(PcbPtr->Word.FbxPtr->ParentPcbPtr);
    _sched_return_pcb(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_post_flush
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_post_flush (DM_FBX_STRUCT *FbxPtr)
{
    PCB_STRUCT *PcbPtr;

    if (FbxPtr->DefectsCachePtr->State == CNTLDATA_INFLUSH)
    {
        return;
    }
    else
    {
        PcbPtr = _sched_get_pcb();
        PcbPtr->Word.FbxPtr = FbxPtr;
        FbxPtr->DefectsCachePtr->State = CNTLDATA_INFLUSH;
        FbxPtr->DefectsCachePtr->DirtyCnt = 0;
        defects_flush_defectsblk_runtime(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_flush_defectsblk_runtime
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_flush_defectsblk_runtime (PCB_STRUCT *PcbPtr)
{
    DM_FBX_STRUCT *FbxPtr;

    // Prepare dmx ops parameters
    FbxPtr = PcbPtr->Word.FbxPtr;
    PcbPtr->Info.DmxWrite.Pba = CALC_PBA(22, 0, 0);
    PcbPtr->Info.DmxWrite.BuffAddr = FbxPtr->DefectsCachePtr->DataAddr;
    PcbPtr->Info.DmxWrite.SegCnt = DEFECTS_SEGMENTS_PER_BLK;
    PcbPtr->Fn = defects_flush_defectsblk_runtime_cb;

    dmx_ops_write_in_place(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_flush_defectsblk_runtime_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_flush_defectsblk_runtime_cb (PCB_STRUCT *PcbPtr)
{
    if (PcbPtr->Word.FbxPtr->DefectsCachePtr->DirtyCnt > 0)
    {
        PcbPtr->Word.FbxPtr->DefectsCachePtr->DirtyCnt = 0;
        defects_flush_defectsblk_runtime(PcbPtr);
    }
    else
    {
        PcbPtr->Word.FbxPtr->DefectsCachePtr->State = CNTLDATA_INCACHE;
        _sched_return_pcb(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_erase_dev
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_erase_dev (PCB_STRUCT *PcbPtr)
{
    PcbPtr->Fn = defects_erase_dev_cb;
    dmx_ops_erase(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_erase_dev_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_erase_dev_cb (PCB_STRUCT *PcbPtr)
{
    // Increment blk idx
    PcbPtr->Info.DmxErase.Pba += SEGMENTS_PER_SYSBLK;

    // Check if not yet reach the pba limit
    if (PcbPtr->Info.DmxErase.Pba < SEGMENTS_PER_FBX)
    {
        defects_erase_dev(PcbPtr);
    }

    else
    {
        dm_notify_completion(PcbPtr);
        _sched_return_pcb(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_scan_mfg_dev
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_scan_mfg_dev (PCB_STRUCT *PcbPtr)
{
    unsigned char *BuffPtr;

    BuffPtr = (unsigned char *)PcbPtr->Info.DmxRead.BuffAddr;
    if (BuffPtr[0] != 0xFF)
    {
        defects_add_defect_entry(PcbPtr->Word.FbxPtr,
                                 PcbPtr->Info.DmxRead.Pba,
                                 DEFECTS_MFG_DEFECT_ERR);
    }

    // Increment blk idx
    PcbPtr->Info.DmxRead.Pba += SEGMENTS_PER_SYSBLK;

    // Check if not yet reach the pba limit
    if (PcbPtr->Info.DmxRead.Pba >= SEGMENTS_PER_FBX)
    {
        dm_notify_completion(PcbPtr);
        _sched_return_pcb(PcbPtr);

        return;
    }

    dmx_ops_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_thorough_scan_write_dev
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_thorough_scan_write_dev (PCB_STRUCT *PcbPtr)
{
    BIT_STAT Stat;

    if (PcbPtr->Info.DmxWrite.Stat != SUCCESSFUL)
    {
        if (PcbPtr->Info.DmxWrite.Stat != DMX_OPS_CMD_RETRY)
        {
            defects_add_defect_entry(PcbPtr->Word.FbxPtr,
                                     PcbPtr->Info.DmxWrite.Pba,
                                     DEFECTS_SCREENING_ERR);
        }
        else
        {
            // Do not add to defects if fid hang, retry write
            dmx_ops_write(PcbPtr);

            return;
        }
    }

    while (1)
    {
        // Increment blk idx
        PcbPtr->Info.Screening.BlkIdx++;
        if (PcbPtr->Info.Screening.BlkIdx >= BLKS_PER_DEV)
        {
            PcbPtr->Info.Screening.BlkIdx = 1;
            PcbPtr->Info.Screening.SegIdx += SEGMENTS_PER_PAGE;

            if (PcbPtr->Info.Screening.SegIdx >= SEGMENTS_PER_BLK)
            {
                dm_notify_completion(PcbPtr);
                _sched_return_pcb(PcbPtr);

                return;
            }

            defects_fill_random_pattern((void *)PcbPtr->Info.DmxWrite.BuffAddr,
                                        FLASH_PAGE_SIZE);
        }

        PcbPtr->Info.DmxWrite.Pba = CALC_PBA(PcbPtr->Info.Screening.DevIdx,
                                             PcbPtr->Info.Screening.BlkIdx,
                                             PcbPtr->Info.Screening.SegIdx);

        // Check if cntl area
        if (PcbPtr->Info.DmxWrite.Pba < CNTL_SEGMENTS_PER_FBX)
        {
            Stat = blkrecord_cd_check_bad_blk(PcbPtr->Word.FbxPtr,
                                              PcbPtr->Info.DmxWrite.Pba);
        }
        else
        {
            Stat = blkrecord_ud_check_bad_blk(PcbPtr->Word.FbxPtr,
                                              PcbPtr->Info.DmxWrite.Pba);
        }

        // Check if blk pba is good
        if (Stat == GOOD_BLK)
        {
            break;
        }
    }

    dmx_ops_write(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_thorough_scan_read_dev
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_thorough_scan_read_dev (PCB_STRUCT *PcbPtr)
{
    BIT_STAT Stat;

    if (PcbPtr->Info.DmxRead.Stat != SUCCESSFUL)
    {
        defects_add_defect_entry(PcbPtr->Word.FbxPtr,
                                 PcbPtr->Info.DmxRead.Pba,
                                 DEFECTS_SCREENING_ERR);
    }

    while (1)
    {
        // Increment seg idx
        PcbPtr->Info.Screening.SegIdx += SEGMENTS_PER_PAGE;
        if (PcbPtr->Info.Screening.SegIdx >= SEGMENTS_PER_BLK)
        {
            PcbPtr->Info.Screening.SegIdx = 0;
            PcbPtr->Info.Screening.BlkIdx++;

            if (PcbPtr->Info.Screening.BlkIdx >= BLKS_PER_DEV)
            {
                dm_notify_completion(PcbPtr);
                _sched_return_pcb(PcbPtr);

                return;
            }
        }

        PcbPtr->Info.DmxRead.Pba = CALC_PBA(PcbPtr->Info.Screening.DevIdx,
                                            PcbPtr->Info.Screening.BlkIdx,
                                            PcbPtr->Info.Screening.SegIdx);

        // Check if cntl area
        if (PcbPtr->Info.DmxRead.Pba < CNTL_SEGMENTS_PER_FBX)
        {
            Stat = blkrecord_cd_check_bad_blk(PcbPtr->Word.FbxPtr,
                                              PcbPtr->Info.DmxRead.Pba);
        }
        else
        {
            Stat = blkrecord_ud_check_bad_blk(PcbPtr->Word.FbxPtr,
                                              PcbPtr->Info.DmxRead.Pba);
        }

        // Check if blk pba is good
        if (Stat == GOOD_BLK)
        {
            break;
        }
    }

    dmx_ops_read(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : defects_fill_random_pattern
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void defects_fill_random_pattern (void *TgtPtr,
                                  unsigned long ByteLength)
{
    volatile unsigned long *MemPtr;
    unsigned long MaxIdx;
    unsigned long Idx;
    unsigned long Key[4];

    MemPtr = TgtPtr;
    MaxIdx = ByteLength / 8;

    for (Idx = 0;
         Idx < MaxIdx;
         Idx++)
    {
        Key[0] = KEY1_OFFSET + GET_RANDOM_PATTERN();
        Key[1] = KEY2_OFFSET + GET_RANDOM_PATTERN();
        Key[2] = KEY3_OFFSET + GET_RANDOM_PATTERN();
        Key[3] = KEY4_OFFSET + GET_RANDOM_PATTERN();

        dm_scramble_data(MemPtr,
                         Key,
                         0);

        MemPtr += 2;
    }

    return;
}


//=============================================================================
// $Log: Defects.c,v $
// Revision 1.9  2014/05/19 04:48:58  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.8  2014/05/13 13:58:14  rcantong
// 1. DEV: Cleanup
// 1.1 Removed CntlBadCnt and UserBadCnt
//
// Revision 1.7  2014/04/30 13:53:35  rcantong
// 1. DEV: Support defects screening
// 1.1 Added process for thorough scanning - BBantigue
// 2. BUGFIX: Enhanced scrambler pattern
// 2.1 Used tiny encryption algo to generate scrambler pattern
//
// Revision 1.6  2014/02/02 09:31:33  rcantong
// 1. DEV: Support more defects info and runtime flushing
// 1.1 Added more defects info and runtime flushing
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
