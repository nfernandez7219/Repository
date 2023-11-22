//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Public/Err.h,v $
// $Revision: 1.4 $
// $Author: rcantong $
// $Date: 2014/04/30 15:15:07 $
// $Id: Err.h,v 1.4 2014/04/30 15:15:07 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__ERR_H__)
#define __ERR_H__

#if defined(DEBUG)
_Inline void err_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Led.h"


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


//-----------------------------------------------------------------------------
// Function    : err_gross
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
#define err_gross()                \
{                                  \
    *LED_CNTLR_PTR = LED_RED_MASK; \
    _brk();                        \
}


#endif
//=============================================================================
// $Log: Err.h,v $
// Revision 1.4  2014/04/30 15:15:07  rcantong
// 1. DEV: Cleanup
// 1.1 Removed err_gross function prototype
//
// Revision 1.3  2014/02/02 08:53:51  rcantong
// 1. DEV: Support LED as status indicator
// 1.1 Added control of LEDs to indicate the device status
//
// Revision 1.2  2013/08/08 16:42:06  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:00  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
