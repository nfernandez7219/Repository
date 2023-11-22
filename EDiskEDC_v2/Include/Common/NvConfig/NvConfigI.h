//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/NvConfig/NvConfigI.h,v $
// $Revision: 1.4 $
// $Author: rcantong $
// $Date: 2014/04/30 14:13:52 $
// $Id: NvConfigI.h,v 1.4 2014/04/30 14:13:52 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__NVCONFIGI_H__)
#define __NVCONFIGI_H__

#if defined(DEBUG)
_Inline void nvconfigi_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Local Constant Macros
//-----------------------------------------------------------------------------

#define NV_FBX0                         0
#define NV_FBX1                         1


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

void nv_preinit_parm (void);

void nv_preinit_malloc (void);

void nv_lite_init (void);

void nv_lite_wipe_nvconfig (unsigned long FbxIdx);

void nv_lite_fetch_config (unsigned long FbxIdx);

void nv_lite_evaluate_config (void);

void nv_lite_evaluate_prodinfo (void);

void nv_build_config (void);

void nv_build_init_prod_info(void);

void nv_init_flush_config (unsigned long FbxIdx);

void nv_build_flush_config (void);

void nv_flush_config_cb (unsigned long *PayLoadPtr);


#endif
//=============================================================================
// $Log: NvConfigI.h,v $
// Revision 1.4  2014/04/30 14:13:52  rcantong
// 1. DEV: Support checking of updated FW version
// 1.1 Added saving of FW version in NvConfig - JParairo
//
// Revision 1.3  2014/02/02 09:54:16  rcantong
// 1. DEV: Support mode select and bit specific config commands
// 1.1 Added handling of mode select and bit specific config commands
//
// Revision 1.2  2014/01/08 12:42:58  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.1  2013/07/03 19:34:05  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
