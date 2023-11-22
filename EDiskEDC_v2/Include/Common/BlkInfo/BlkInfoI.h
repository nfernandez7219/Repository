//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/BlkInfo/BlkInfoI.h,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2014/04/30 13:34:46 $
// $Id: BlkInfoI.h,v 1.2 2014/04/30 13:34:46 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__BLKINFOI_H__)
#define __BLKINFOI_H__

#if defined(DEBUG)
_Inline void blkinfoi_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Local Constant Macros
//-----------------------------------------------------------------------------

#define PE_INIT_CNT                     300
#define BLKINFO_DEF_DEV                 4
#define BLKINFO_END_DEV                 22
#define BLKINFO_UPDATE_THRES            60000


//-----------------------------------------------------------------------------
// Local Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Structure Definitions
//-----------------------------------------------------------------------------

typedef struct BlkInfoHdrStruct
{
    unsigned long Signature;
    unsigned long Sqn;
    IDENTITY_INT Identity;
} BLKINFO_HDR_STRUCT;

typedef struct BlkInfoStruct
{
    BLKINFO_HDR_STRUCT BlkInfoHdr;
    unsigned short BlkInfoEntry[BLKS_PER_FBX];
} BLKINFO_STRUCT;

typedef struct BlkInfoParmStruct
{
    PCB_STRUCT *BlkInfoPcbPtr;
    unsigned long MinEraseCnt;
    unsigned long MaxEraseCnt;
    unsigned long long RunningTotalBlkErase;
    unsigned long CurrDevIdx; // Currently being used
    unsigned long DevIdx; // running Pba during init
    unsigned long BadBmp;
    unsigned long Sqn;
    unsigned long UpdateCnt;
    unsigned long Filler[6];
} BLKINFO_PARM_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void blkinfo_fetch_dev (PCB_STRUCT *PcbPtr);

void blkinfo_fetch_dev_cb (PCB_STRUCT *PcbPtr);

void blkinfo_fetch_blkinfo (PCB_STRUCT *PcbPtr);

void blkinfo_check_blkinfo (PCB_STRUCT *PcbPtr);

void blkinfo_build_flush_blkinfo (PCB_STRUCT *PcbPtr);

void blkinfo_build_flush_blkinfo_cb (PCB_STRUCT *PcbPtr);

void blkinfo_flush_blkinfo (PCB_STRUCT *PcbPtr);

void blkinfo_flush_blkinfo_cb (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: BlkInfoI.h,v $
// Revision 1.2  2014/04/30 13:34:46  rcantong
// 1. DEV: Support BlkInfo control data to monitor erase count
// 1.1 Added process for BlkInfo - BBantigue
//
// Revision 1.1  2013/07/03 19:34:02  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
