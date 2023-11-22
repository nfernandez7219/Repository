//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/SxnInfo/SxnInfo.h,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/04/30 13:56:45 $
// $Id: SxnInfo.h,v 1.7 2014/04/30 13:56:45 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__SXNINFO_H__)
#define __SXNINFO_H__

#if defined(DEBUG)
_Inline void sxninfo_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct Si0CacheLookupStruct
{
    CNTL_CACHE_STRUCT *Si0CachePtr;
} SI0_CACHE_LOOKUP_STRUCT;

typedef struct Si0EntryStruct
{
    unsigned long UserSxnIdx;
} SI0_ENTRY_STRUCT;

typedef struct Si1EntryStruct
{
    PBA_INT Si0Pba0;
    #if defined(CD_MIRROR)
    PBA_INT Si0Pba1;
    #endif
} SI1_ENTRY_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define SXNINFO_PUT_TO_CLEAN_LIST(Si0CachePtr, SxnInfo) \
    (util_dll_insert_at_tail(&Si0CachePtr->Link, &SxnInfo.CleanList));


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
extern SI0_CACHE_LOOKUP_STRUCT Si0CacheLookup[FBX_CNT][SI0_SXN_CNT + 8];
#pragma BSS()


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void sxninfo_init_malloc (unsigned long FbxIdx);

void sxninfo_build_fbx (PCB_STRUCT *PcbPtr);

void sxninfo_init_si1_entry (DM_FBX_STRUCT *FbxPtr,
                             IDENTITY_INT Identity,
                             SQN_INT Sqn,
                             PBA_INT Pba);

void sxninfo_scrub_si1 (PCB_STRUCT *PcbPtr);

BIT_STAT sxninfo_alloc_read (DM_FBX_STRUCT *FbxPtr,
                             PBA_INT Pba,
                             CNTL_CACHE_STRUCT **Si0CachePtr2Ptr);

BIT_STAT sxninfo_alloc_write (DM_FBX_STRUCT *FbxPtr,
                              PBA_INT Pba,
                              CNTL_CACHE_STRUCT **Si0CachePtr2Ptr);

BIT_STAT sxninfo_alloc_compact (DM_FBX_STRUCT *FbxPtr,
                                unsigned long Si0SxnIdx,
                                CNTL_CACHE_STRUCT **Si0CachePtr2Ptr);

void sxninfo_unlock_read (CNTL_CACHE_STRUCT *Si0CachePtr);

void sxninfo_post_si0_fetching (CNTL_CACHE_STRUCT *Si0CachePtr);


#endif
//=============================================================================
// $Log: SxnInfo.h,v $
// Revision 1.7  2014/04/30 13:56:45  rcantong
// 1. DEV: Support CD mirroring
// 1.1 Added process to utilize the CD copy - JAbad
//
// Revision 1.6  2014/02/02 10:00:32  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:58  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
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
