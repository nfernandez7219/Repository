//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Arc700/BootCodeLoaderC.c,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/04/30 13:23:41 $
// $Id: BootCodeLoaderC.c,v 1.5 2014/04/30 13:23:41 rcantong Exp $
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

#include "Bios.h"
#include "BitPciEpRegs.h"
#include "Dmx.h"
#include "DmxInit.h"
#include "DmxLite.h"
#include "DmxRegs.h"
#include "DpcRegs.h"
#include "EdcClkPmu.h"
#include "Err.h"
#include "FbxInit.h"
#include "Led.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "BiosI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------

#pragma CODE(".bootstrapldr")


//-----------------------------------------------------------------------------
// Function    : bootcodeldr_setup_ecc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bootcodeldr_setup_ecc (void)
{
    bootcodeldr_init_sd(0x5D120ECC,
                        0x40000000,
                        0);

    return;
}


//-----------------------------------------------------------------------------
// Function    : bootcodeldr_main
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bootcodeldr_main (void)
{
    BIT_STAT Stat;

    bootcodeldr_crcore_init();

    dmx_lite_init();

    Stat = dmx_lite_read(0,
                         MAINFW_A_BLK_PBA,
                         0,
                         MAX_FW_SIZE);

    if (Stat != SUCCESSFUL)
    {
        Stat = dmx_lite_read(0,
                             MAINFW_B_BLK_PBA,
                             0,
                             MAX_FW_SIZE);
    }

    if (Stat != SUCCESSFUL)
    {
        bootcodeldr_init_maypayload();
        err_gross();
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : bootcodeldr_init_maypayload
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bootcodeldr_init_maypayload (void)
{
    unsigned long Bar0Val;
    unsigned long NewMaxPayload;

    while (1)
    {
        Bar0Val = *PE_BAR_PTR_0_PTR;

        if (    (Bar0Val != 0xFFFFC000)
             && (Bar0Val != 0x0))
        {
            break ;
        }
    }

    NewMaxPayload = *PE_DEV_STA_CTRL_PTR;

    _sr(0xFADE4800 | (NewMaxPayload & 0xE0),
        0x418);

    return;
}


//-----------------------------------------------------------------------------
// Function    : bootcodeldr_crcore_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bootcodeldr_crcore_init (void)
{
    // DM CRCore initialization
    *CR_PMU7_DM4_PTR = 0x1CFFFF00;      // Reset FbClk and DmCORE domains
    *CR_PMU7_DM5_PTR = 0x1EFFFFFF;      // Reset Arb, Sys and IFDB domains
    *CR_PMU7_DM6_PTR = 0x12003BF7;      // Reset DmFdClk domains
    *CR_PMU7_DM6_PTR = 0x36003BF7;
    *CR_PMU7_DM1_PTR = 0x0EFDFBF7;      // Set FmClk to lowest freq
    *CR_PMU7_DM2_PTR = 0x0EFDFBF7;      // Set FmClk to lowest freq
    *CR_PMU7_DM3_PTR = 0x00000000;      // Disable FmClk

    return;
}


//-----------------------------------------------------------------------------
// Function    : bootcodeldr_init_sd
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bootcodeldr_init_sd (unsigned long Pattern,
                          unsigned long SdramSz,
                          unsigned long StartAddr)
{
    unsigned long Idx;
    unsigned long AllocDesc;
    unsigned long DpTxCnt = SdramSz >> 18;
    unsigned long CurAddr = StartAddr;

    bootcodeldr_setup_dgp_s2d();

    for (Idx = 0;
         Idx < DpTxCnt;
         Idx++)
    {
        bootcodeldr_setup_dpr_s2d(Idx,
                                  CurAddr,
                                  DP_DMA_WD_CNT);

        CurAddr += SIZE_PER_FLUSH;
    }

    bootcodeldr_setup_end_dpr_s2d(Idx,
                                  CurAddr,
                                  (SdramSz - (CurAddr - StartAddr)) >> 2);

    AllocDesc = *DP_CH_ALLOCDESCID_BASE_PTR(0);

    bootcodeldr_copy_dpr_s2d(0,
                             StartAddr,
                             DP_DMA_WD_CNT);

    *DP_CH_RDYDESC_BASE_PTR(0) = (DPR_RDY | DPRQ0_RDY);

    // Start sram to dram transfer
    bootcodeldr_copy_dgp_s2d();

    return;
}


//-----------------------------------------------------------------------------
// Function    : bootcodeldr_setup_dgp_s2d
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bootcodeldr_setup_dgp_s2d (void)
{
    *BOOT_DP_DGP_W0_SRAM_PTR(0) = 0xF0000007;
    *BOOT_DP_DGP_W1_SRAM_PTR(0) = 0x00200040;
    *BOOT_DP_DGP_W2_SRAM_PTR(0) = 0x00000000;
    *BOOT_DP_DGP_W3_SRAM_PTR(0) = 0x00000000;

    return;
}


//-----------------------------------------------------------------------------
// Function    : bootcodeldr_copy_dgp_s2d
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bootcodeldr_copy_dgp_s2d (void)
{
    *DP_CH_DGP_W3_BASE_PTR(0) = 0x0010000C;
    *DP_CH_DGP_W2_BASE_PTR(0) = 0x00000000;
    *DP_CH_DGP_W1_BASE_PTR(0) = 0x00200040;

    _ASM("sync");

    *DP_CH_DGP_W0_BASE_PTR(0) = 0xF0000007;

    return;
}


//-----------------------------------------------------------------------------
// Function    : bootcodeldr_setup_dpr_s2d
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bootcodeldr_setup_dpr_s2d (unsigned long Idx,
                                unsigned long DestAddr,
                                unsigned long WdCnt)
{
    *BOOT_DP_DPR_W0_SRAM_PTR(Idx) = COMPUTE_NXT_DPR(Idx);
    *BOOT_DP_DPR_W1_SRAM_PTR(Idx) = WdCnt;
    *BOOT_DP_DPR_W2_SRAM_PTR(Idx) = SRAM_BUFFER;
    *BOOT_DP_DPR_W3_SRAM_PTR(Idx) = DestAddr;
    *BOOT_DP_DPR_W4_SRAM_PTR(Idx) = 0;
    *BOOT_DP_DPR_W5_SRAM_PTR(Idx) = 0;
    *BOOT_DP_DPR_W6_SRAM_PTR(Idx) = 0;
    *BOOT_DP_DPR_W7_SRAM_PTR(Idx) = 0;

    return;
}


//-----------------------------------------------------------------------------
// Function    : bootcodeldr_setup_end_dpr_s2d
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bootcodeldr_setup_end_dpr_s2d (unsigned long Idx,
                                    unsigned long DestAddr,
                                    unsigned long WdCnt)
{
    *BOOT_DP_DPR_W0_SRAM_PTR(Idx) = 0x83000000;
    *BOOT_DP_DPR_W1_SRAM_PTR(Idx) = WdCnt;
    *BOOT_DP_DPR_W2_SRAM_PTR(Idx) = SRAM_BUFFER;
    *BOOT_DP_DPR_W3_SRAM_PTR(Idx) = DestAddr;
    *BOOT_DP_DPR_W4_SRAM_PTR(Idx) = 0;
    *BOOT_DP_DPR_W5_SRAM_PTR(Idx) = 0;
    *BOOT_DP_DPR_W6_SRAM_PTR(Idx) = 0;
    *BOOT_DP_DPR_W7_SRAM_PTR(Idx) = 0;

    return;
}


//-----------------------------------------------------------------------------
// Function    : bootcodeldr_copy_dpr_s2d
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bootcodeldr_copy_dpr_s2d (unsigned long Idx,
                               unsigned long DestAddr,
                               unsigned long WdCnt)
{
    *DP_CH_DPR3_W0_BASE_PTR(0) = 0x03100032;
    *DP_CH_DPR3_W1_BASE_PTR(0) = WdCnt;
    *DP_CH_DPR3_W2_BASE_PTR(0) = SRAM_BUFFER;
    *DP_CH_DPR3_W3_BASE_PTR(0) = DestAddr;

    return;
}


//-----------------------------------------------------------------------------
// Function    : bootcodeldr_util_delay
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bootcodeldr_util_delay (unsigned long Num)
{
    unsigned long Idx;

    for (Idx = 0;
         Idx < Num;
         Idx++)
    {
        _nop();
    }

    return;
}


#pragma CODE()


//=============================================================================
// $Log: BootCodeLoaderC.c,v $
// Revision 1.5  2014/04/30 13:23:41  rcantong
// 1. DEV: Cleanup
// 1.1 Removed the redundant bootcodeldr_dmx_lite - TPingol
//
// Revision 1.4  2014/02/02 09:14:07  rcantong
// 1. DEV: Support LED as status indicator
// 1.1 Added control of LEDs to indicate the device status
//
// Revision 1.3  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.2  2013/08/08 16:44:21  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:08  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
