//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/FreeList/FreeListI.h,v $
// $Revision: 1.8 $
// $Author: rcantong $
// $Date: 2014/04/30 14:01:15 $
// $Id: FreeListI.h,v 1.8 2014/04/30 14:01:15 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__FREELISTI_H__)
#define __FREELISTI_H__

#if defined(DEBUG)
_Inline void freelisti_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Local Constant Macros
//-----------------------------------------------------------------------------

#define FREEBLKS_PER_CDLANE             8
#define FREELIST_CD_HI_THRES            ((FREEBLKS_PER_CDLANE * 90) / 100)
#define FREELIST_CD_LO_THRES            ((FREEBLKS_PER_CDLANE * 90) / 100)
#define MIN_CD_ENTRY_CNT                5
#define MIN_CD_CMPCT_ENTRY_CNT          3

#define FREEBLKS_PER_UDLANE             16
#define FREELIST_UD_HI_THRES            ((FREEBLKS_PER_UDLANE * 90) / 100)
#define FREELIST_UD_LO_THRES            ((FREEBLKS_PER_UDLANE * 90) / 100)
#define MIN_UD_ENTRY_CNT                5
#define MIN_UD_CMPCT_ENTRY_CNT          3

#define LAST_FAST_PAGE                 (SEGMENTS_PER_PAGE * (PAGES_PER_BLK - 2))


//-----------------------------------------------------------------------------
// Local Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Structure Definitions
//-----------------------------------------------------------------------------

typedef struct FreeListEntryStruct
{
    PBA_INT BlkPba;
} FREELIST_ENTRY_STRUCT;

typedef struct FreeListLaneStruct
{
    unsigned long EntryCnt;
    unsigned long FreeIdx;
    unsigned long RclmIdx;
    PBA_INT CurPagePba;
    PBA_INT CmpctCurPagePba;
    FREELIST_ENTRY_STRUCT *EntryPtr;
    unsigned long FastPageFlag;
    unsigned long Filler;
} FREELIST_LANE_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void freelist_cd_get_new_blk (DM_FBX_STRUCT *FbxPtr,
                              unsigned long LaneIdx,
                              PBA_INT *CurPagePbaPtr);

void freelist_ud_get_new_blk (DM_FBX_STRUCT *FbxPtr,
                              unsigned long LaneIdx,
                              PBA_INT *CurPagePbaPtr);


#endif
//=============================================================================
// $Log: FreeListI.h,v $
// Revision 1.8  2014/04/30 14:01:15  rcantong
// 1. DEV: Support freelist for compact
// 1.1 Added freelist threshold for compact page - JAbad
//
// Revision 1.7  2014/03/03 12:32:20  rcantong
// 1. BUGFIX: Reduced too much compacting
// 1.1 Limit target free blocks to less than 32 blocks - JAbad
//
// Revision 1.6  2014/02/02 10:00:32  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:57  rcantong
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
// Revision 1.2  2013/08/08 16:42:08  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:04  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
