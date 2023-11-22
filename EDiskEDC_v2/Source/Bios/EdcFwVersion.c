//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/EdcFwVersion.c,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2014/01/08 12:42:55 $
// $Id: EdcFwVersion.c,v 1.3 2014/01/08 12:42:55 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================


//-----------------------------------------------------------------------------
// Standard Library Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Includes
//-----------------------------------------------------------------------------
#include "EdcFwVersion.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma DATA(".fwversion")
volatile FW_VERSION_STRUCT FwVersion = {

    // Standard header
    HDR_SIGNATURE_STRING,
    STD_HEADER_VERSION,
    ROM_TYPE_MICROCODE,
    (unsigned long)_firmware_size,
    0,

    // ROM versioning
    INTERFACE_PCIE,
    EDISK_VERSION_NUMBER,
    EDISK_VERSION_DATE,

    // BIOS Data Area (BDS)
    0,
    0,
    0,
    0,
};
#pragma DATA()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    :
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: EdcFwVersion.c,v $
// Revision 1.3  2014/01/08 12:42:55  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.2  2013/08/08 16:44:22  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:09  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
