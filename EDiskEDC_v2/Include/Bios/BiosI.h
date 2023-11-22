//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Bios/BiosI.h,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2014/04/30 13:23:41 $
// $Id: BiosI.h,v 1.3 2014/04/30 13:23:41 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__BIOSI_H__)
#define __BIOSI_H__

#if defined(DEBUG)
_Inline void biosi_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Local Constant Macros
//-----------------------------------------------------------------------------

#define DGP_SRAM_BASE_ADDR              0x60100000
#define DPR_SRAM_BASE_ADDR              0x60100010
#define SRAM_BUFFER                     0x60180000
#define DP_DMA_WD_CNT                   0xFFFF
#define DP_WORD_SIZE                    0x4
#define SIZE_PER_FLUSH                  (DP_DMA_WD_CNT * DP_WORD_SIZE)
#define DGP_DONE                        0x1
#define DGP_INTERRUPT_DONE              0x1
#define DPR_RDY                         0x1
#define DPRQ0_RDY                       0x2


//-----------------------------------------------------------------------------
// Local Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define BOOT_DP_DGP_W0_SRAM_ADDR(x)     (DGP_SRAM_BASE_ADDR + (x * 0x20))
#define BOOT_DP_DGP_W1_SRAM_ADDR(x)     (DGP_SRAM_BASE_ADDR + 0x4 + (x * 0x20))
#define BOOT_DP_DGP_W2_SRAM_ADDR(x)     (DGP_SRAM_BASE_ADDR + 0x8 + (x * 0x20))
#define BOOT_DP_DGP_W3_SRAM_ADDR(x)     (DGP_SRAM_BASE_ADDR + 0xC + (x * 0x20))

#define BOOT_DP_DPR_W0_SRAM_ADDR(x)     (DPR_SRAM_BASE_ADDR + (x * 0x20))
#define BOOT_DP_DPR_W1_SRAM_ADDR(x)     (DPR_SRAM_BASE_ADDR + 0x4 + (x * 0x20))
#define BOOT_DP_DPR_W2_SRAM_ADDR(x)     (DPR_SRAM_BASE_ADDR + 0x8 + (x * 0x20))
#define BOOT_DP_DPR_W3_SRAM_ADDR(x)     (DPR_SRAM_BASE_ADDR + 0xC + (x * 0x20))
#define BOOT_DP_DPR_W4_SRAM_ADDR(x)     (DPR_SRAM_BASE_ADDR + 0x10 + (x * 0x20))
#define BOOT_DP_DPR_W5_SRAM_ADDR(x)     (DPR_SRAM_BASE_ADDR + 0x14 + (x * 0x20))
#define BOOT_DP_DPR_W6_SRAM_ADDR(x)     (DPR_SRAM_BASE_ADDR + 0x18 + (x * 0x20))
#define BOOT_DP_DPR_W7_SRAM_ADDR(x)     (DPR_SRAM_BASE_ADDR + 0x1C + (x * 0x20))

#define BOOT_DP_DPR_W0_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DPR_W0_SRAM_ADDR(x)
#define BOOT_DP_DPR_W1_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DPR_W1_SRAM_ADDR(x)
#define BOOT_DP_DPR_W2_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DPR_W2_SRAM_ADDR(x)
#define BOOT_DP_DPR_W3_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DPR_W3_SRAM_ADDR(x)
#define BOOT_DP_DPR_W4_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DPR_W4_SRAM_ADDR(x)
#define BOOT_DP_DPR_W5_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DPR_W5_SRAM_ADDR(x)
#define BOOT_DP_DPR_W6_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DPR_W6_SRAM_ADDR(x)
#define BOOT_DP_DPR_W7_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DPR_W7_SRAM_ADDR(x)

#define BOOT_DP_DGP_W0_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DGP_W0_SRAM_ADDR(x)
#define BOOT_DP_DGP_W1_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DGP_W1_SRAM_ADDR(x)
#define BOOT_DP_DGP_W2_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DGP_W2_SRAM_ADDR(x)
#define BOOT_DP_DGP_W3_SRAM_PTR(x)      (volatile unsigned long *)BOOT_DP_DGP_W3_SRAM_ADDR(x)

#define COMPUTE_NXT_DPR(x)  (0x03100012 + 0x20 + (0x20 * x))


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

// Firware BIOS specific
void bios_download_thru_multi_firmware (void);

void bios_program_bootloader (void);

void bios_program_firmware (void);

// Boot code loader specific
void bootcodeldr_crcore_init (void);

void bootcodeldr_main (void);

void bootcodeldr_init_maypayload (void);

void bootcodeldr_init_sd (unsigned long Pattern,
                          unsigned long SdramSz,
                          unsigned long StartAddr);

void bootcodeldr_setup_dgp_s2d (void);

void bootcodeldr_copy_dgp_s2d (void);

void bootcodeldr_setup_dpr_s2d (unsigned long Idx,
                                unsigned long DestAddr,
                                unsigned long WdCnt);

void bootcodeldr_setup_end_dpr_s2d (unsigned long Idx,
                                    unsigned long DestAddr,
                                    unsigned long WdCnt);

void bootcodeldr_copy_dpr_s2d (unsigned long Idx,
                               unsigned long DestAddr,
                               unsigned long WdCnt);

void bootcodeldr_util_delay (unsigned long Num);


#endif
//=============================================================================
// $Log: BiosI.h,v $
// Revision 1.3  2014/04/30 13:23:41  rcantong
// 1. DEV: Cleanup
// 1.1 Removed the redundant bootcodeldr_dmx_lite - TPingol
//
// Revision 1.2  2014/01/08 12:42:55  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.1  2013/07/03 19:34:01  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
