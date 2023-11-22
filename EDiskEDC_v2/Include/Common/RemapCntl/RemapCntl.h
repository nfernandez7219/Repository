//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/RemapCntl/RemapCntl.h,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2014/05/19 05:02:10 $
// $Id: RemapCntl.h,v 1.2 2014/05/19 05:02:10 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__REMAPCNTL_H__)
#define __REMAPCNTL_H__


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

void remap_cntl_trigger_remap (DM_FBX_STRUCT *FbxPtr,
                               PBA_INT Pba);


#endif
//=============================================================================
// $Log: RemapCntl.h,v $
// Revision 1.2  2014/05/19 05:02:10  rcantong
// 1. DEV: Support control remap
// 1.1 Added control remapping functions - BBantigue
//
// Revision 1.1  2013/07/03 19:34:13  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
