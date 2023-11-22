//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Bios/EdcFwVersion.h,v $
// $Revision: 1.8 $
// $Author: rcantong $
// $Date: 2014/05/19 06:12:58 $
// $Id: EdcFwVersion.h,v 1.8 2014/05/19 06:12:58 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__EDCFWVERSION_H__)
#define __EDCFWVERSION_H__

#if defined(DEBUG)
_Inline void edcfwversion_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define EDISK_VERSION_NUMBER            "2A20"
#define EDISK_VERSION_DATE              "04-June-2014"

#define HDR_SIGNATURE_STRING            "BiTVersionHdr"
#define STD_HEADER_VERSION              1
#define HDR_STR_LENGTH                  0x10

// ROM type
#define ROM_TYPE_BOOTSTRAPLOADER        1
#define ROM_TYPE_BOOTCODELOADER         2
#define ROM_TYPE_MICROCODE              3

// Interface
#define INTERFACE_PCIE                  1
#define INTERFACE_SATA                  2
#define INTERFACE_SAS                   3

// Bootup Status
#define BOOT_FROM_A                     1
#define BOOT_FROM_B                     2


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef enum TypeEnum
{
    FWTYPE_BOOTSTRAPLOADER,
    FWTYPE_BOOTCODELOADER,
    FWTYPE_BOOTLOADER,
    FWTYPE_MICROCODE
} TYPE_ENUM;

typedef enum FwProdEnum
{
    FWPROD_EDCDE,
    FWPROD_EDCSE
} FW_PROD_ENUM;

typedef enum FwIntfcEnum
{
    FWINTFC_PCIE,
    FWINTFC_SATA,
    FWINTFC_SAS,
} FW_INTFC_ENUM;

typedef struct FwVersionStruct
{
    // Standard header
    unsigned char HdrSignature[HDR_STR_LENGTH];
    unsigned long StdHeaderVersion;
    unsigned long RomType;
    unsigned long RomSize;
    unsigned long RomCSum;

    // ROM Versioning
    unsigned long Interface;
    unsigned char FwVersion[HDR_STR_LENGTH];
    unsigned char FwVersionDate[HDR_STR_LENGTH];

    // BIOS Data Area (BDS)
    unsigned long EraseBoardFlag;
    unsigned long SysConfigSaveFlag;
    unsigned long ProgramFirmwareFlag;
    unsigned long PciGen1Flag;
    unsigned long Filler[3];
} FW_VERSION_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

extern unsigned long _firmware_size[];
extern volatile FW_VERSION_STRUCT FwVersion;


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------


#endif
//=============================================================================
// $Log: EdcFwVersion.h,v $
// Revision 1.8  2014/05/19 06:12:58  rcantong
// 1. DEV: Update FW version
// 1.1 Changed 2A16 to 2A18
//
// Revision 1.7  2014/04/30 13:27:30  rcantong
// 1. DEV: Update FW version
// 1.1 Changed 2A10 to 2A16
//
// Revision 1.6  2014/03/03 13:07:14  rcantong
// 1. DEV: Update FW Version
// 1.1 Changed 2A06 to 2A10
//
// Revision 1.5  2014/02/06 14:45:06  rcantong
// 1. DEV: Update FW version
// 1.1 Changed 2A05 to 2A06
//
// Revision 1.4  2014/02/02 10:00:31  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.3  2014/01/08 12:42:55  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.2  2013/11/11 08:20:47  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.1  2013/07/03 19:34:01  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
