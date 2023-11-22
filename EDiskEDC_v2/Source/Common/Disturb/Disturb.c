//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Disturb/Disturb.c,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/05/19 04:48:58 $
// $Id: Disturb.c,v 1.7 2014/05/19 04:48:58 rcantong Exp $
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
#include "Util.h"
#include "Sched.h"
#include "SysConfig.h"
#include "DmCommon.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".dm_global")
unsigned long DisturbReadCntPerBlk[FBX_CNT][BLKS_PER_FBX];
L1CACHE_ALIGN(DisturbReadCntPerBlk);
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : disturb_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void disturb_init_malloc (unsigned long FbxIdx)
{
    DM_FBX_STRUCT *FbxPtr;

    FbxPtr = &DmFbx[FbxIdx];
    FbxPtr->DisturbMgr.DisturbReadCntPtr = &DisturbReadCntPerBlk[FbxIdx][0];

    util_init_pattern(FbxPtr->DisturbMgr.DisturbReadCntPtr,
                      sizeof(DisturbReadCntPerBlk[0]),
                      INIT_PATTERN_LO_VALUE);

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: Disturb.c,v $
// Revision 1.7  2014/05/19 04:48:58  rcantong
// 1. BUGFIX: Replaced bad page by bad block management
// 1.1 Removed bad page management from Defects - BBantigue
// 1.2 Added bad blk management in BlkRecord - BBantigue
//
// Revision 1.6  2014/05/13 13:38:07  rcantong
// 1. BUGFIX: User data verify is perform also during remap processes
// 1.1 Perform dmx_ops_write_n_read when UD_VERIFY is enabled
//
// Revision 1.5  2014/04/30 13:42:37  rcantong
// 1. BUGFIX: Enhanced control data scrambler
// 1.1 XOR scramble pattern to Dir0Entry and Si0Entry - JParairo
// 2. DEV: Support user flash block remapping
// 2.1 Changed process flow using remap queue - PPestano
//
// Revision 1.4  2014/03/03 12:55:44  rcantong
// 1. DEV: FID hang handler
// 1.1 Added dmx stat checking for FID_HANG_TIMEOUT - JFaustino
//
// Revision 1.3  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.2  2013/08/08 16:44:23  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:16  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
