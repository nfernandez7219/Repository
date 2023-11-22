//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Dm/Dm.h,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/05/13 13:19:51 $
// $Id: Dm.h,v 1.7 2014/05/13 13:19:51 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__DM_H__)
#define __DM_H__

#if defined(DEBUG)
_Inline void dm_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define SYSTEM_INIT                     0x00001111
#define SYSTEM_SCRUB                    0xB00BB00B
#define SYSTEM_BUILD                    0x0000BBBB
#define SYSTEM_READY                    0xFAFADEDE

#define BUILD_NO_DEFECTS                0xA
#define BUILD_SCAN_MFG_DONE             0xB
#define BUILD_THOROUGH_DONE             0xC
#define BUILD_DONE                      0xD

#define FBX_NO_DEFECTS_MSK              0xAAAAAA
#define FBX_SCAN_MFG_MSK                0xBBBBBB
#define FBX_THOROUGH_MSK                0xCCCCCC
#define FBX_DONE_MSK                    0xDDDDDD
#define FORCE_ERASE_AND_BUILD           0xEEEEEE

#define FBX_INIT                        0x00001D1D
#define FBX_SCRUB                       0x0000600D
#define FBX_BUILD                       0x0000BDBD


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct DmConfigStruct
{
    unsigned long FlashPageSize;
    unsigned long FlashBlkSize;
    unsigned long Dir0SxnCnt;
    unsigned long Si0SxnCnt;
    unsigned long SegmentsPerFbx;
    unsigned long CntlSegmentsPerFbx;
    unsigned long BlksPerFbx;
    unsigned long CntlBlksPerFbx;
    unsigned long Filler[8];
} DM_CONFIG_STRUCT;

typedef struct DmFlagParmStruct
{
    volatile unsigned long SystemStat;
    volatile unsigned long BuildFlag;
    volatile unsigned long FbxStat[FBX_CNT];
    volatile unsigned long DefectsStat[FBX_CNT];
    volatile unsigned long BIStat[FBX_CNT];
    volatile unsigned long ScrubCycle;
    volatile unsigned long BIBuild;
    volatile unsigned long Filler[2];
} DM_FLAG_PARM_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

extern DM_CONFIG_STRUCT DmConfig;

extern DM_FLAG_PARM_STRUCT DmFlagParm;


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void dm_init_config (void);

void dm_init_domain (void);

void dm_init (void);


#endif
//=============================================================================
// $Log: Dm.h,v $
// Revision 1.7  2014/05/13 13:19:51  rcantong
// 1. DEV: Support background scrubber
// 1.1 Added background scrubber process - PPestano
//
// Revision 1.6  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.5  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.4  2013/12/05 13:06:34  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:07  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:03  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
