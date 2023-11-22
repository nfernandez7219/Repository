//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Scrub/Scrub.h,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2014/05/13 13:19:51 $
// $Id: Scrub.h,v 1.2 2014/05/13 13:19:51 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__SCRUB_H__)
#define __SCRUB_H__

#if defined(DEBUG)
_Inline void scrub_h (void) { return; }
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


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void scrub_init_malloc (unsigned long FbxIdx);

void scrub_start_scrubbing (DM_FBX_STRUCT *FbxPtr);


#endif
//=============================================================================
// $Log: Scrub.h,v $
// Revision 1.2  2014/05/13 13:19:51  rcantong
// 1. DEV: Support background scrubber
// 1.1 Added background scrubber process - PPestano
//
// Revision 1.1  2013/07/03 19:34:13  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
