//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Application/SysConfig.c,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2013/08/08 16:44:23 $
// $Id: SysConfig.c,v 1.2 2013/08/08 16:44:23 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================


//-----------------------------------------------------------------------------
// Standard Library Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Includes
//-----------------------------------------------------------------------------
#include "BitDefs.h"
#include "SysConfig.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

const unsigned char FbxToDmId[FBX_CNT] = {
    0, 1, 2, 0, 1, 2,
};
L1CACHE_ALIGN(FbxToDmId);


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    :
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sysconfig_init (void)
{
    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: SysConfig.c,v $
// Revision 1.2  2013/08/08 16:44:23  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:16  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
