//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/RemapCntl/RemapCntlI.h,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2014/05/19 05:02:10 $
// $Id: RemapCntlI.h,v 1.2 2014/05/19 05:02:10 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__REMAPCNTLI_H__)
#define __REMAPCNTLI_H__


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

void remap_cntl_block (PCB_STRUCT *PcbPtr);

void remap_cntl_write_alloc_dir0 (PCB_STRUCT *PcbPtr);

void remap_cntl_write_alloc_si0 (PCB_STRUCT *PcbPtr);

void remap_cntl_dirty_dir0 (PCB_STRUCT *PcbPtr);

void remap_cntl_dirty_si0 (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: RemapCntlI.h,v $
// Revision 1.2  2014/05/19 05:02:10  rcantong
// 1. DEV: Support control remap
// 1.1 Added control remapping functions - BBantigue
//
// Revision 1.1  2013/07/03 19:34:13  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
