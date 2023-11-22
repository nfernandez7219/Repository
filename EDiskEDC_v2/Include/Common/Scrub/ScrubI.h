//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Scrub/ScrubI.h,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2014/05/19 04:58:14 $
// $Id: ScrubI.h,v 1.3 2014/05/19 04:58:14 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__SCRUBI_H__)
#define __SCRUBI_H__

#if defined(DEBUG)
_Inline void scrubi_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Local Constant Macros
//-----------------------------------------------------------------------------


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

void scrub_ud_chk_valid_sxn (PCB_STRUCT *PcbPtr);

void scrub_ud_read_done (PCB_STRUCT *PcbPtr);

void scrub_ud_scrubbing_done (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: ScrubI.h,v $
// Revision 1.3  2014/05/19 04:58:14  rcantong
// 1. BUGFIX: Update scrub from LA-based to PA-based scrubber
// 1.1 Update Pcb for Scrub to be Pba based - PPestano
//
// Revision 1.2  2014/05/13 13:19:51  rcantong
// 1. DEV: Support background scrubber
// 1.1 Added background scrubber process - PPestano
//
// Revision 1.1  2013/07/03 19:34:13  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
