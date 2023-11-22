//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/CdFlushMgr/CdFlushMgr.h,v $
// $Revision: 1.6 $
// $Author: rcantong $
// $Date: 2014/03/03 12:26:05 $
// $Id: CdFlushMgr.h,v 1.6 2014/03/03 12:26:05 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__CDFLUSHMGR_H__)
#define __CDFLUSHMGR_H__

#if defined(DEBUG)
_Inline void cdflushmgr_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define FLUSH_WORKER_CNT                16 // always power of two


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct CdFlushWorkerStruct
{
    PCB_STRUCT Pcb;
    unsigned long Padding[4]; // to make the workers cache aligned
} CD_FLUSH_WORKER_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void cdflushmgr_init_malloc (unsigned long FbxIdx);

void cdflushmgr_start_mgr_fbx (PCB_STRUCT *PcbPtr);

void arc_timer1_cdflushmgr (void);


#endif
//=============================================================================
// $Log: CdFlushMgr.h,v $
// Revision 1.6  2014/03/03 12:26:05  rcantong
// 1. BUGFIX: PCB become empty due to lack of clean CD cache
// 1.1 Increase CD flush worker from 8 to 16
//
// Revision 1.5  2014/02/06 14:41:38  rcantong
// 1. BUGFIX: PCB become empty due to lack of clean CD cache
// 1.1 Increased CD flush worker from 4 to 8
//
// Revision 1.4  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.3  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.2  2013/12/05 13:06:33  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.1  2013/07/03 19:34:02  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
