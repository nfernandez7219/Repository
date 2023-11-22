//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Public/SysConfig.h,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/05/13 13:19:51 $
// $Id: SysConfig.h,v 1.7 2014/05/13 13:19:51 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__SYSCONFIG_H__)
#define __SYSCONFIG_H__

#if defined(DEBUG)
_Inline void sysconfig_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "FlashConfig.h"


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

// Preprocessor switches
//#define CD_MIRROR
#define UD_VERIFY
#define UD_SCRUB

// Bitmicro info
#define USER_SXN_SIZE                   4096
#define CNTL_SXN_SIZE                   1024
#define SEGMENT_SIZE                    1024
#define LBA_SIZE                        512
#define USER_SEGMENT_SIZE               512

// Asic info
#define CPU_CNT                         4
#define DM_CNT                          3
#define FBUS_CNT                        4
#define FMEM_CNT                        4

// Board info
#define FBX_CNT                         6
#define TGT_CNT                         8
#define DEV_CNT                         32
#define SDRAM_SIZE                      0x40000000          // 1GB
#define OUTPUT_DRIVE_SETTING            OUTPUT_DS_OVERDRIVE_1

// Calculated info
#define FBX_CNT_MASK                    ((1 << FBX_CNT) - 1)

#define LBAS_PER_SEGMENT                (SEGMENT_SIZE / LBA_SIZE)
#define LBAS_PER_USER_SXN               (USER_SXN_SIZE / LBA_SIZE)
#define LBAS_PER_PAGE                   (FLASH_PAGE_SIZE / LBA_SIZE)
#define USER_SXNS_PER_PAGE              (FLASH_PAGE_SIZE / USER_SXN_SIZE)
#define CNTL_SXNS_PER_PAGE              (FLASH_PAGE_SIZE / CNTL_SXN_SIZE)

#define USER_SXNS_PER_BLK               (FLASH_BLK_SIZE / USER_SXN_SIZE)
#define CNTL_SXNS_PER_BLK               (FLASH_BLK_SIZE / CNTL_SXN_SIZE)

#define SEGMENTS_PER_USER_SXN           (USER_SXN_SIZE / SEGMENT_SIZE)
#define SEGMENTS_PER_CNTL_SXN           (CNTL_SXN_SIZE / SEGMENT_SIZE)

#define SEGMENTS_PER_PAGE               (FLASH_PAGE_SIZE / SEGMENT_SIZE)
#define SEGMENTS_PER_BLK                (FLASH_BLK_SIZE / SEGMENT_SIZE)
#define SEGMENTS_PER_SYSBLK             (SEGMENTS_PER_BLK * DEV_CNT)

#define PAGES_PER_BLK                   (FLASH_BLK_SIZE / FLASH_PAGE_SIZE)
#define PAGES_PER_LUN                   (PAGES_PER_BLK * BLKS_PER_LUN)

#define BLKS_PER_FBX                    (BLKS_PER_DEV * DEV_CNT)
#define SEGMENTS_PER_FBX                (SEGMENTS_PER_BLK * BLKS_PER_FBX)
#define PAGES_PER_FBX                   (PAGES_PER_BLK * BLKS_PER_FBX)

#define DATA_OFFSET                     (DMX_MAX_CODE_WORDS - \
                                        (SEGMENT_SIZE + ECC_SIZE))

#define DMX_DPN                         (SEGMENT_SIZE)
#define DMX_PNFCN                       (   (DATA_OFFSET << 21) \
                                          | (SEGMENTS_PER_PAGE << 6) \
                                          | (FLASH_ENCODED_PG_SIZE << 3))

#define USER_BLKS_PER_DEV               ((BLKS_ALIGN_PER_DEV * 78) / 100)
#define CNTL_BLKS_PER_DEV               (   BLKS_PER_DEV \
                                          - ((BLKS_ALIGN_PER_DEV * 98) / 100))

#define USABLE_BLK_CNT                  (USER_BLKS_PER_DEV * DEV_CNT)
#define USABLE_SXN_CNT                  (USABLE_BLK_CNT * USER_SXNS_PER_BLK)
#define USER_SXN_CNT                    (BLKS_PER_FBX * USER_SXNS_PER_BLK)

#define CNTL_BLK_CNT                    (CNTL_BLKS_PER_DEV * DEV_CNT)
#define CNTL_SXN_CNT                    (CNTL_BLK_CNT * CNTL_SXNS_PER_BLK)

#define DIR0_SXN_SIZE                   (CNTL_SXN_SIZE)
#define DIR0_ENTRIES_PER_SXN            (DIR0_SXN_SIZE / \
                                        sizeof(DIR0_ENTRY_STRUCT))
#define DIR0_SXN_CNT                    (DIV_ROUND_UP(USABLE_SXN_CNT, \
                                        DIR0_ENTRIES_PER_SXN))

#define SI0_SXN_SIZE                    (CNTL_SXN_SIZE)
#define SI0_ENTRIES_PER_SXN             (SI0_SXN_SIZE / \
                                        sizeof(SI0_ENTRY_STRUCT))
#define SI0_SXN_CNT                     (DIV_ROUND_UP(USER_SXN_CNT, \
                                        SI0_ENTRIES_PER_SXN))

#define CNTL_BLKS_PER_FBX               (CNTL_BLKS_PER_DEV * DEV_CNT)
#define CNTL_SEGMENTS_PER_FBX           (CNTL_BLKS_PER_FBX * SEGMENTS_PER_BLK)

#define SYSCONFIG_SAVED_PBA             ((unsigned long)(2 * SEGMENTS_PER_BLK))
#define SYSCONFIG_CURR_PBA              ((unsigned long)(3 * SEGMENTS_PER_BLK))


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define DIV_ROUND_UP(D1, D2) (((D1) + ((D2) - 1)) / (D2))


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

extern const unsigned char FbxToDmId[FBX_CNT];


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------


#endif
//=============================================================================
// $Log: SysConfig.h,v $
// Revision 1.7  2014/05/13 13:19:51  rcantong
// 1. DEV: Support background scrubber
// 1.1 Added background scrubber process - PPestano
//
// Revision 1.6  2014/04/30 15:43:19  rcantong
// 1. DEV: Support flash config
// 1.1 Removed flash related info - JFaustino
// 2. DEV: Increase control data area
// 2.1 Added the spare blocks as control data area - JAbad
// 3. DEV: Support switch knob for CD_MIRROR and UD_VERIFY
// 3.1 Added preprocessor defines for CD_MIRROR and UD_VERIFY
// 4. BUGFIX: Spare data been allocated but not used
// 4.1 Corrected the data profile settings - MFenol
//
// Revision 1.5  2014/02/02 09:01:28  rcantong
// 1. DEV: Support 256Gb Toshiba MLC
// 1.1 Added configuration for 256Gb Toshiba MLC
//
// Revision 1.4  2014/01/08 12:42:57  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:06  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:00  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
