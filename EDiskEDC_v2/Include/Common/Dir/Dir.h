//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Dir/Dir.h,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/04/30 13:56:45 $
// $Id: Dir.h,v 1.7 2014/04/30 13:56:45 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__DIR_H__)
#define __DIR_H__

#if defined(DEBUG)
_Inline void dir_h (void) { return; }
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

typedef struct Dir0CacheLookupStruct
{
    CNTL_CACHE_STRUCT *Dir0CachePtr;
} DIR0_CACHE_LOOKUP_STRUCT;

typedef struct Dir0EntryStruct
{
    PBA_INT UserPba;
} DIR0_ENTRY_STRUCT;

typedef struct Dir1EntryStruct
{
    PBA_INT Dir0Pba0;
    #if defined(CD_MIRROR)
    PBA_INT Dir0Pba1;
    #endif
} DIR1_ENTRY_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define DIR_GET_CACHE_FR_LOOKUP(Dir, Dir0SxnIdx) \
    (Dir.Dir0CacheLookupPtr[Dir0SxnIdx].Dir0CachePtr)

#define DIR_PUT_TO_CLEAN_LIST(Dir0CachePtr, Dir) \
    (util_dll_insert_at_tail(&Dir0CachePtr->Link, &Dir.CleanList))


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
extern DIR0_CACHE_LOOKUP_STRUCT Dir0CacheLookup[FBX_CNT][DIR0_SXN_CNT + 8];
#pragma BSS()

extern unsigned long _unused_sdram[];


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void dir_init_malloc (unsigned long FbxIdx);

void dir_build_fbx (PCB_STRUCT *PcbPtr);

void dir_fetch_dir0_fbx (PCB_STRUCT *PcbPtr);

void dir_init_dir1_entry (DM_FBX_STRUCT *FbxPtr,
                          IDENTITY_INT Identity,
                          SQN_INT Sqn,
                          PBA_INT Pba);

void dir_scrub_dir1 (PCB_STRUCT *PcbPtr);

BIT_STAT dir_alloc_read (DM_FBX_STRUCT *FbxPtr,
                         unsigned long UserSxnIdx,
                         CNTL_CACHE_STRUCT **Dir0CachePtr2Ptr);

BIT_STAT dir_alloc_write (DM_FBX_STRUCT *FbxPtr,
                          unsigned long UserSxnIdx,
                          CNTL_CACHE_STRUCT **Dir0CachePtr2Ptr);

BIT_STAT dir_alloc_compact (DM_FBX_STRUCT *FbxPtr,
                            unsigned long Dir0SxnIdx,
                            CNTL_CACHE_STRUCT **Dir0CachePtr2Ptr);

void dir_unlock_read (CNTL_CACHE_STRUCT *Dir0CachePtr);

void dir_post_dir0_fetching (CNTL_CACHE_STRUCT *Dir0CachePtr);


#endif
//=============================================================================
// $Log: Dir.h,v $
// Revision 1.7  2014/04/30 13:56:45  rcantong
// 1. DEV: Support CD mirroring
// 1.1 Added process to utilize the CD copy - JAbad
//
// Revision 1.6  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.4  2013/12/05 13:06:34  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:07  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:03  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
