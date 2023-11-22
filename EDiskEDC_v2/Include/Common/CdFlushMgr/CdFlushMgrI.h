//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/CdFlushMgr/CdFlushMgrI.h,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2014/04/30 13:48:54 $
// $Id: CdFlushMgrI.h,v 1.3 2014/04/30 13:48:54 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__CDFLUSHMGRI_H__)
#define __CDFLUSHMGRI_H__

#if defined(DEBUG)
_Inline void cdflushmgri_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Local Constant Macros
//-----------------------------------------------------------------------------

#define HIGH_PRIO                       0
#define LOW_PRIO                        1

// To make sure up to 2 cad only will be used
#if (CNTL_SXNS_PER_PAGE > 12)
#define CNTL_SXNS_PER_PAGE_LIMIT        12
#else
#define CNTL_SXNS_PER_PAGE_LIMIT        CNTL_SXNS_PER_PAGE
#endif


//-----------------------------------------------------------------------------
// Local Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void cdflushmgr_start_flush_ops (PCB_STRUCT *PcbPtr,
                                 unsigned long MinCntlSxnCnt);

void cdflushmgr_cmpct_flush_page (PCB_STRUCT *PcbPtr);

void cdflushmgr_flush_page (PCB_STRUCT *PcbPtr);

void cdflushmgr_flush_page_cb (PCB_STRUCT *PcbPtr);

void cdflushmgr_flush_mirror_page (PCB_STRUCT *PcbPtr);

void cdflushmgr_flush_mirror_page_cb (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: CdFlushMgrI.h,v $
// Revision 1.3  2014/04/30 13:48:54  rcantong
// 1. DEV: Prioritized flushing of compacting CD
// 1.1 Added process for flushing of compacting CD - MFenol
// 2. DEV: Support CD mirroring
// 2.1 Added API for flush mirror page - JAbad
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
