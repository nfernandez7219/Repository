//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Public/FlashConfig.h,v $
// $Revision: 1.1 $
// $Author: rcantong $
// $Date: 2014/04/30 15:16:57 $
// $Id: FlashConfig.h,v 1.1 2014/04/30 15:16:57 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__FLASHCONFIG_H__)
#define __FLASHCONFIG_H__

#if defined(DEBUG)
_Inline void flashconfig_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

// Flash type
#define FLASH_TYPE_TOSH_256_MLC
//#define FLASH_TYPE_TOSH_256_EMLC
//#define FLASH_TYPE_TOSH_128

// Flash info
#if defined(FLASH_TYPE_TOSH_256_MLC)
#define FLASHID                         0xDE98 //0x3A98
#define TECH_CODE                       0xD700
#define BLKS_PER_DEV                    2116
#define BLKS_ALIGN_PER_DEV              2048
#define FLASH_BLK_SIZE                  0x400000            // 4MB
#define FLASH_PAGE_SIZE                 0x4000              // 16KB
#define FLASH_ENCODED_PG_SIZE           FLASH_ENCODED_PG_SZ_16384
#define FLASH_ENCODED_PG_PER_BLK        FLASH_ENCODED_PG_PER_BLK_256
#define FLASH_FEAT_ADDR_OUTPUT_DS       0x10
#define ECC_CONFIG                      9
#define ECC_SIZE                        72
#define FLASH_PE_THRES                  3000

#elif defined(FLASH_TYPE_TOSH_256_EMLC)
#define FLASHID                         0xDE98
#define TECH_CODE                       0xD500
#define BLKS_PER_DEV                    8296
#define BLKS_ALIGN_PER_DEV              8192
#define BLKS_PER_LUN                    4148
#define FLASH_BLK_SIZE                  0x100000            // 1MB
#define FLASH_PAGE_SIZE                 0x2000              // 8KB
#define FLASH_LUN_MASK                  0x100000
#define FLASH_ENCODED_PG_SIZE           FLASH_ENCODED_PG_SZ_8192
#define FLASH_ENCODED_PG_PER_BLK        FLASH_ENCODED_PG_PER_BLK_128
#define FLASH_FEAT_ADDR_OUTPUT_DS       0x10
#define FLASH_LUN_SUPPORTED
#define ECC_CONFIG                      9
#define ECC_SIZE                        72
#define FLASH_PE_THRES                  10000

#elif defined(FLASH_TYPE_TOSH_128)
#define FLASHID                         0xD798
#define TECH_CODE                       0xD500
#define BLKS_PER_DEV                    8296
#define BLKS_ALIGN_PER_DEV              8192
#define BLKS_PER_LUN                    4148
#define FLASH_BLK_SIZE                  0x80000             // 512KB
#define FLASH_PAGE_SIZE                 0x2000              // 8KB
#define FLASH_LUN_MASK                  0x80000
#define FLASH_ENCODED_PG_SIZE           FLASH_ENCODED_PG_SZ_8192
#define FLASH_ENCODED_PG_PER_BLK        FLASH_ENCODED_PG_PER_BLK_64
#define FLASH_FEAT_ADDR_OUTPUT_DS       0x10
#define FLASH_LUN_SUPPORTED
#define ECC_CONFIG                      9
#define ECC_SIZE                        72
#define FLASH_PE_THRES                  30000
#endif


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


#endif
//=============================================================================
// $Log: FlashConfig.h,v $
// Revision 1.1  2014/04/30 15:16:57  rcantong
// 1. DEV: Initial commit of flash config
// 1.1 Added selection of flash type and its specific info - JFaustino
//
//=============================================================================
