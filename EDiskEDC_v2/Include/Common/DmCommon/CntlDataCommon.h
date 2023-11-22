//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/DmCommon/CntlDataCommon.h,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/04/30 13:34:46 $
// $Id: CntlDataCommon.h,v 1.5 2014/04/30 13:34:46 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__CNTLDATACOMMON_H__)
#define __CNTLDATACOMMON_H__

#if defined(DEBUG)
_Inline void cntldatacommon_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define CNTL_HDR_SIGNATURE              0xBEADFACE

#define IDENTITY_SXNIDX_MASK            0x00FFFFFF
#define TARGET_MASK                     0xFF000000
#define TARGET_UNKNOWN                  0xFFFFFFFF
#define TARGET_BASEBLK                  0x01000000
#define TARGET_DEFECTSBLK               0x02000000
#define TARGET_DIR0                     0x03000000
#define TARGET_SI0                      0x04000000
#define TARGET_BLKINFO                  0x05000000

#define READ_LOCK_VALUE                 0x00000001
#define WRITE_LOCK_VALUE                0x00010000


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct CntlHdrStruct
{
    unsigned long Signature;
    unsigned long long Sqn;
    IDENTITY_INT Identity[SEGMENTS_PER_PAGE];
} CNTL_HDR_STRUCT;

typedef struct CntlCacheStruct
{
    UTIL_DLL_ENTRY_STRUCT Link;
    IDENTITY_INT Identity;
    unsigned long DataAddr;
    unsigned long LockCnt;
    unsigned short DirtyCnt;
    unsigned char FbxIdx;
    unsigned char State;
    UTIL_SLL_STRUCT StateWaitQ;
} CNTL_CACHE_STRUCT;

typedef struct CntlBuffStruct
{
    unsigned char Buff[CNTL_SXN_SIZE];
} CNTL_BUFF_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define SXNIDX(IdInt) (IdInt & IDENTITY_SXNIDX_MASK)


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------


#endif
//=============================================================================
// $Log: CntlDataCommon.h,v $
// Revision 1.5  2014/04/30 13:34:46  rcantong
// 1. DEV: Support BlkInfo control data to monitor erase count
// 1.1 Added process for BlkInfo - BBantigue
//
// Revision 1.4  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.3  2013/12/05 13:06:34  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
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
