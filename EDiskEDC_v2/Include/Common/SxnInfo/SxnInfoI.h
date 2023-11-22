//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/SxnInfo/SxnInfoI.h,v $
// $Revision: 1.6 $
// $Author: rcantong $
// $Date: 2014/04/30 13:56:45 $
// $Id: SxnInfoI.h,v 1.6 2014/04/30 13:56:45 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__SXNINFOI_H__)
#define __SXNINFOI_H__

#if defined(DEBUG)
_Inline void sxninfoi_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Local Constant Macros
//-----------------------------------------------------------------------------

#define SI0_CACHE_LINE_CNT              ((SI0_SXN_CNT * 5) / 100)


//-----------------------------------------------------------------------------
// Local Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define SXNINFO_GET_CACHE_FR_LOOKUP(SxnInfo, Si0SxnIdx) \
    (SxnInfo.Si0CacheLookupPtr[Si0SxnIdx].Si0CachePtr)


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void sxninfo_build_dev_stg1 (PCB_STRUCT *PcbPtr);

void sxninfo_build_dev_stg2 (PCB_STRUCT *PcbPtr);

void sxninfo_build_dev_stg3 (PCB_STRUCT *PcbPtr);

void sxninfo_build_dev_stg4 (PCB_STRUCT *PcbPtr);

void sxninfo_build_dev_stg5 (PCB_STRUCT *PcbPtr);

void sxninfo_fetch_si0 (PCB_STRUCT *PcbPtr);

void sxninfo_fetch_si0_done (PCB_STRUCT *PcbPtr);

void sxninfo_fetch_si0_mirror (PCB_STRUCT *PcbPtr);

void sxninfo_fetch_si0_mirror_done (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: SxnInfoI.h,v $
// Revision 1.6  2014/04/30 13:56:45  rcantong
// 1. DEV: Support CD mirroring
// 1.1 Added process to utilize the CD copy - JAbad
//
// Revision 1.5  2014/03/03 12:27:24  rcantong
// 1. BUGFIX: PCB become empty due to lack of clean CD cache
// 1.1 Decreased SI0 cache count to give more Dir0 cache
//
// Revision 1.4  2013/12/05 13:06:35  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:08  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:13  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
