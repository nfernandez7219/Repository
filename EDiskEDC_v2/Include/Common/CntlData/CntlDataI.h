//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/CntlData/CntlDataI.h,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2013/11/11 08:20:48 $
// $Id: CntlDataI.h,v 1.3 2013/11/11 08:20:48 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__CNTLDATAI_H__)
#define __CNTLDATAI_H__

#if defined(DEBUG)
_Inline void cntldatai_h (void) { return; }
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

void cntldata_fetch_dev (PCB_STRUCT *PcbPtr);

void cntldata_fetch_dev_cb (PCB_STRUCT *PcbPtr);

void cntldata_write_update_dir0 (PCB_STRUCT *PcbPtr);

void cntldata_write_update_si0 (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: CntlDataI.h,v $
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:07  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:02  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
