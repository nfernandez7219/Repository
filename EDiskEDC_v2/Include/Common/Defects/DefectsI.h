//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Defects/DefectsI.h,v $
// $Revision: 1.6 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:58 $
// $Id: DefectsI.h,v 1.6 2014/05/19 04:48:58 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__DEFECTSI_H__)
#define __DEFECTSI_H__

#if defined(DEBUG)
_Inline void defectsi_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Local Constant Macros
//-----------------------------------------------------------------------------

#define DEFECTS_SEGMENTS_PER_BLK        256

#define DEFECTS_ALLOWED_BAD_CNT         (   DEFECTS_SEGMENTS_PER_BLK     \
                                          * SEGMENT_SIZE)                \
                                          / sizeof(DEFECTS_ENTRY_STRUCT)


//-----------------------------------------------------------------------------
// Local Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Structure Definitions
//-----------------------------------------------------------------------------

typedef struct DefectsEntryStruct
{
    PBA_INT Pba;
    unsigned short ErrorType;
    unsigned short ErrorStage;
} DEFECTS_ENTRY_STRUCT;

typedef struct DefectsBlkStruct
{
    unsigned long DefectsSignature;
    unsigned long BuildFlag;
    unsigned long Filler[2];
    DEFECTS_ENTRY_STRUCT DefectsEntry[0];
} DEFECTS_BLK_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void defects_fetch_defectsblk (PCB_STRUCT *PcbPtr);

void defects_fetch_defectsblk_cb (PCB_STRUCT *PcbPtr);

void defects_flush_defectsblk (PCB_STRUCT *PcbPtr);

void defects_flush_defectsblk_cb (PCB_STRUCT *PcbPtr);

void defects_post_flush (DM_FBX_STRUCT *FbxPtr);

void defects_flush_defectsblk_runtime (PCB_STRUCT *PcbPtr);

void defects_flush_defectsblk_runtime_cb (PCB_STRUCT *PcbPtr);

void defects_erase_dev (PCB_STRUCT *PcbPtr);

void defects_erase_dev_cb (PCB_STRUCT *PcbPtr);

void defects_scan_mfg_dev (PCB_STRUCT *PcbPtr);

void defects_thorough_scan_write_dev (PCB_STRUCT *PcbPtr);

void defects_thorough_scan_read_dev (PCB_STRUCT *PcbPtr);

void defects_fill_random_pattern (void *TgtPtr,
                                  unsigned long ByteLength);


#endif
//=============================================================================
// $Log: DefectsI.h,v $
// Revision 1.6  2014/05/19 04:48:58  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.5  2014/04/30 13:53:35  rcantong
// 1. DEV: Support defects screening
// 1.1 Added process for thorough scanning - BBantigue
// 2. BUGFIX: Enhanced scrambler pattern
// 2.1 Used tiny encryption algo to generate scrambler pattern
//
// Revision 1.4  2014/02/02 09:31:33  rcantong
// 1. DEV: Support more defects info and runtime flushing
// 1.1 Added more defects info and runtime flushing
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
