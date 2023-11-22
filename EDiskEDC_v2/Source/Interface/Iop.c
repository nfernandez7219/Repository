//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Interface/Iop.c,v $
// $Revision: 1.10 $
// $Author: rcantong $
// $Date: 2014/05/13 14:01:29 $
// $Id: Iop.c,v 1.10 2014/05/13 14:01:29 rcantong Exp $
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
#include "Dm.h"
#include "EdcClkPmu.h"
#include "EdcFwVersion.h"
#include "EdcLocalCsr.h"
#include "Err.h"
#include "Interrupt.h"
#include "Iop.h"
#include "IpCall.h"
#include "Media.h"
#include "Scsi.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "BitIcbmRegs.h"
#include "BitPciEpRegs.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".pci_bar0")
unsigned char Bar0Buffer[0x10000];
#pragma BSS()

#pragma BSS(".dccm_io")
MRDMA_PARM_STRUCT MrDmaParm;
PCB_STRUCT *Ch0PcbPtr;
PCB_STRUCT *Ch1PcbPtr;
PCB_STRUCT *Ch2PcbPtr;
PCB_STRUCT *Ch3PcbPtr;
PCB_STRUCT UcodePcb;
#pragma BSS()

#pragma BSS(".sram")
IOP_CMD_INFO_STRUCT IopCmdInfo[BUFFER_CNT];
L1CACHE_ALIGN(IopCmdInfo);
#pragma BSS()

const IOP_PRCS_CMD_FN iop_prcs_cmd_fn[] = {
    iop_prcs_cmd_err_gross,              // 0x00
    iop_prcs_cmd_err_gross,              // 0x01
    iop_prcs_cmd_err_gross,              // 0x02
    iop_prcs_cmd_err_gross,              // 0x03
    iop_prcs_cmd_err_gross,              // 0x04
    iop_prcs_cmd_err_gross,              // 0x05
    iop_prcs_cmd_err_gross,              // 0x06
    iop_prcs_cmd_err_gross,              // 0x07
    iop_prcs_cmd_err_gross,              // 0x08
    iop_prcs_cmd_err_gross,              // 0x09
    iop_prcs_cmd_ucode,                  // 0x0A
    iop_prcs_non_user_cmd,               // 0x0B
    iop_prcs_cmd_err_gross,              // 0x0C
    iop_prcs_cmd_err_gross,              // 0x0D
    iop_prcs_cmd_err_gross,              // 0x0E
    iop_prcs_cmd_err_gross,              // 0x0F
    iop_prcs_cmd_err_gross,              // 0x10
    iop_prcs_cmd_err_gross,              // 0x11
    iop_prcs_cmd_err_gross,              // 0x12
    iop_prcs_cmd_err_gross,              // 0x13
    iop_prcs_cmd_err_gross,              // 0x14
    iop_prcs_cmd_err_gross,              // 0x15
    iop_prcs_cmd_err_gross,              // 0x16
    iop_prcs_cmd_err_gross,              // 0x17
    iop_prcs_cmd_err_gross,              // 0x18
    iop_prcs_cmd_err_gross,              // 0x19
    iop_prcs_cmd_err_gross,              // 0x1A
    iop_prcs_cmd_err_gross,              // 0x1B
    iop_prcs_cmd_err_gross,              // 0x1C
    iop_prcs_cmd_err_gross,              // 0x1D
    iop_prcs_cmd_err_gross,              // 0x1E
    iop_prcs_cmd_err_gross,              // 0x1F
    iop_prcs_cmd_err_gross,              // 0x20
    iop_prcs_cmd_err_gross,              // 0x21
    iop_prcs_cmd_err_gross,              // 0x22
    iop_prcs_cmd_err_gross,              // 0x23
    iop_prcs_cmd_read_bulk,              // 0x24
    iop_prcs_cmd_write_bulk,             // 0x25
    iop_prcs_cmd_write_bulk,             // 0x26
    iop_prcs_cmd_write_bulk,             // 0x27
    iop_prcs_cmd_write_bulk,             // 0x28
    iop_prcs_cmd_rmw_bulk,               // 0x29
    iop_prcs_cmd_err_gross,              // 0x2A
    iop_prcs_cmd_err_gross,              // 0x2B
    iop_prcs_cmd_err_gross,              // 0x2C
    iop_prcs_cmd_err_gross,              // 0x2D
    iop_prcs_cmd_err_gross,              // 0x2E
    iop_prcs_cmd_err_gross,              // 0x2F
    iop_prcs_cmd_err_gross,              // 0x30
    iop_prcs_cmd_err_gross,              // 0x31
    iop_prcs_cmd_check_system_stat,      // 0x32
    iop_prcs_cmd_err_gross,              // 0x33
    iop_prcs_cmd_shutdown,               // 0x34
    iop_prcs_cmd_err_gross,              // 0x35
    iop_prcs_cmd_err_gross,              // 0x36
    iop_prcs_cmd_err_gross,              // 0x37
    iop_prcs_cmd_err_gross,              // 0x38
    iop_prcs_cmd_err_gross,              // 0x39
    iop_prcs_cmd_err_gross,              // 0x3A
    iop_prcs_cmd_err_gross,              // 0x3B
    iop_prcs_cmd_err_gross,              // 0x3C
    iop_prcs_cmd_err_gross,              // 0x3D
    iop_prcs_cmd_err_gross,              // 0x3E
    iop_prcs_cmd_err_gross,              // 0x3F
};
L1CACHE_ALIGN(iop_prcs_cmd_fn);


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : iop_post_mrlar
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
PCB_STRUCT *iop_post_mrlar (void)
{
    PCB_STRUCT *PcbPtr;

    PcbPtr = _sched_get_pcb();

    util_sll_insert_at_tail(&PcbPtr->Link,
                            &MrDmaParm.MrDmaQue);

    return PcbPtr;
}


//-----------------------------------------------------------------------------
// Function    : iop_activate_mrlar
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_activate_mrlar (void)
{
    PCB_STRUCT *PcbPtr;
    PCB_MRDMA_STRUCT *MrDmaPtr;

    while (1)
    {
        // Check vacant engine
        if (MrDmaParm.UsedEngine == 0xF)
        {
            break;
        }

        // Get head entry
        PcbPtr = util_sll_get_head_entry(&MrDmaParm.MrDmaQue);
        if (PcbPtr == BIT_NULL_PTR)
        {
            break;
        }

        MrDmaPtr = (void *)&PcbPtr->Info;

        if ((MrDmaParm.UsedEngine & 1) == 0)
        {
            *PE_MEM_REQ_LEN_0_PTR      = MrDmaPtr->Size;
            if (MrDmaPtr->HighAddr > 0)
            {
                *PE_MEM_REQ_CTRL_0_PTR = MR_DMA_CONFIG | 1;
            }
            else
            {
                *PE_MEM_REQ_CTRL_0_PTR = MR_DMA_CONFIG;
            }
            *PE_MEM_REQ_PTR_HIGH_0_PTR = MrDmaPtr->HighAddr;
            Ch0PcbPtr = PcbPtr;
            *PE_MEM_REQ_PTR_LOW_0_PTR  = MrDmaPtr->LowAddr;
            MR_POST_TO_WATCHDOG(0);
            MrDmaParm.UsedEngine |= 1;
            *PE_MEM_REQ_LOC_0_PTR      = MrDmaPtr->DevAddr;
        }

        else if ((MrDmaParm.UsedEngine & 2) == 0)
        {
            *PE_MEM_REQ_LEN_1_PTR      = MrDmaPtr->Size;
            if (MrDmaPtr->HighAddr > 0)
            {
                *PE_MEM_REQ_CTRL_1_PTR = MR_DMA_CONFIG | 1;
            }
            else
            {
                *PE_MEM_REQ_CTRL_1_PTR = MR_DMA_CONFIG;
            }
            *PE_MEM_REQ_PTR_HIGH_1_PTR = MrDmaPtr->HighAddr;
            Ch1PcbPtr = PcbPtr;
            *PE_MEM_REQ_PTR_LOW_1_PTR  = MrDmaPtr->LowAddr;
            MR_POST_TO_WATCHDOG(1);
            MrDmaParm.UsedEngine |= 2;
            *PE_MEM_REQ_LOC_1_PTR      = MrDmaPtr->DevAddr;
        }

        else if ((MrDmaParm.UsedEngine & 4) == 0)
        {
            *PE_MEM_REQ_LEN_2_PTR      = MrDmaPtr->Size;
            if (MrDmaPtr->HighAddr > 0)
            {
                *PE_MEM_REQ_CTRL_2_PTR = MR_DMA_CONFIG | 1;
            }
            else
            {
                *PE_MEM_REQ_CTRL_2_PTR = MR_DMA_CONFIG;
            }
            *PE_MEM_REQ_PTR_HIGH_2_PTR = MrDmaPtr->HighAddr;
            Ch2PcbPtr = PcbPtr;
            *PE_MEM_REQ_PTR_LOW_2_PTR  = MrDmaPtr->LowAddr;
            MR_POST_TO_WATCHDOG(2);
            MrDmaParm.UsedEngine |= 4;
            *PE_MEM_REQ_LOC_2_PTR      = MrDmaPtr->DevAddr;
        }

        else
        {
            *PE_MEM_REQ_LEN_3_PTR      = MrDmaPtr->Size;
            if (MrDmaPtr->HighAddr > 0)
            {
                *PE_MEM_REQ_CTRL_3_PTR = MR_DMA_CONFIG | 1;
            }
            else
            {
                *PE_MEM_REQ_CTRL_3_PTR = MR_DMA_CONFIG;
            }
            *PE_MEM_REQ_PTR_HIGH_3_PTR = MrDmaPtr->HighAddr;
            Ch3PcbPtr = PcbPtr;
            *PE_MEM_REQ_PTR_LOW_3_PTR  = MrDmaPtr->LowAddr;
            MR_POST_TO_WATCHDOG(3);
            MrDmaParm.UsedEngine |= 8;
            *PE_MEM_REQ_LOC_3_PTR      = MrDmaPtr->DevAddr;
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : int_arc_isr_pe
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void IRQ_FN int_arc_isr_pe (void)
{
    unsigned long IsrTableIdx;
    unsigned long RingAttn;
    unsigned long PeIntStat;
    unsigned long PutCsr;
    unsigned long PutLoc;
    PCB_STRUCT *PcbPtr;
    PCB_MRDMA_STRUCT *MrDmaPtr;
    unsigned long PutTgt;
    unsigned long SecLoc;
    unsigned long SecTgt;
    unsigned long CmdCnt;

    IsrTableIdx = *IC_CPU0_IN20_IVR_PTR;

    PeIntStat = *PE_CORE_INT_STA_PTR;

    // Process mem done
    if ((PeIntStat & MEM_REQ_DONE) != 0)
    {
        iop_serv_mrlar();
    }

    // Process emp interrupt
    if ((PeIntStat & EMP_PORT_ATTN) != 0)
    {
        RingAttn = *PE_EMP_PORT_ATTN_PTR;
        *PE_EMP_PORT_ATTN_PTR =  RingAttn;

        if ((RingAttn & RING_ZERO_ATTN) != 0)
        {
            PutCsr = *PE_EMP_CMD_PUT_0_PTR;
            PutLoc = MrDmaParm.PutCntLoc;

            while (PutCsr != PutLoc)
            {
                // Check ring boundary
                if (PutCsr > PutLoc)
                {
                    PutTgt = PutCsr;
                }
                else
                {
                    PutTgt = PutCsr + RING_DEPTH;
                }

                SecLoc = PutLoc / CMD_CNT_PER_BULK;
                SecTgt = PutTgt / CMD_CNT_PER_BULK;

                // Check section boundary
                if (SecLoc != SecTgt)
                {
                    PutTgt = (SecLoc + 1) * CMD_CNT_PER_BULK;
                }

                CmdCnt = PutTgt - PutLoc;

                // Prepare cmd fetch dma
                PcbPtr = iop_post_mrlar();
                PcbPtr->Fn = iop_process_cmd;
                MrDmaPtr = (void *)&PcbPtr->Info;
                MrDmaPtr->DevAddr
                    = MrDmaParm.DevCmdFrmBase + (PutLoc * CMD_FRAME_SIZE);
                MrDmaPtr->LowAddr
                    = MrDmaParm.HostCmdFrmBase + (PutLoc * CMD_FRAME_SIZE);
                MrDmaPtr->HighAddr = MrDmaParm.HostHighBase;
                MrDmaPtr->Size = CmdCnt * CMD_FRAME_WORD_SIZE;
                MrDmaPtr->Ctrl1 = CmdCnt;
                MrDmaPtr->Ctrl2 = PutLoc;

                PutLoc = PutTgt % RING_DEPTH;
                MrDmaParm.RcvCmdMax += CmdCnt;
            }

            MrDmaParm.PutCntLoc = PutLoc;
        }
        else
        {
            ASSERT(0);
        }

    }

    iop_activate_mrlar();

    // Acknowledge irq in iccore layer
    IsrTableIdx = *IC_CPU0_IN20_IIRR_PTR;

    return;
}


//-----------------------------------------------------------------------------
// Function    : pecore_crit_int_handler
// Description : This function handles the critical interrupt received from
//               PeCORE Interrupt Register 2
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long PerstCnt = 0;
void IRQ_FN pecore_crit_int_handler (void)
{
    unsigned long RegStat;
    unsigned long RegVal;
    unsigned long TempVal;
    DEV_CONFIG_PARM_STRUCT *DevConfigPtr;

    RegStat = *PE_CINT2_PTR;

    if ((RegStat & 4) == 4)
    {
        PerstCnt++;

        //============================
        // RESET PECORE and PCS
        //============================
        // [2]=0: will transfer control of PeCORE PHY Rst to CrCORE(0xffce_3050)
        // [10:9]=2'b00: will enable auto-deassertion of PeCORE and PCS Resets
        *((volatile unsigned long *)0xFFCE3050) = 0x000;

        // Assert PCIe resets excluding bit[4] PHY Reset maintaining GUC fix settings during power on
        // [2]=1: PeCORE NS Reset
        // [3]=1: PCS Reset
        // [5]=1: PIPE STP Reset
        // [6]=1: PIPE NON Reset (not used by PeCORE but included just to be safe)
        // [7]=1: PIPE STK Reset
        *((volatile unsigned long *)0xFFCE3050) = 0x0EC;

        // De-assertion of PeCORE NON-STOPPABLE Reset
        // [2]=0: CSR with offset 0xfff2_8xxx onwards are now accessible.
        // [6]=0: PIPE NON Reset (not used by PeCORE but just to be safe)
        RegVal = *((volatile unsigned long *)0xFFCE3050);
        *((volatile unsigned long *)0xFFCE3050) = RegVal & 0xFFFFFFBB;

        //==============================================
        // START CONFIGURING PeCORE PHY CSR's FOR TUNING
        //==============================================
        util_delay(4); // just a small delay after de-assertion

        // change pll refclk to differential
        // charge pump and ppath current setting
        *PE_PHY_PLL_CTRLR_PTR       = 0x008D1E82;

        // RX Tuning
        *PE_PHY_RX_CTRLR_LANE_0_PTR = 0x006E03A8;
        *PE_PHY_RX_CTRLR_LANE_1_PTR = 0x006E03A8;
        *PE_PHY_RX_CTRLR_LANE_2_PTR = 0x006E03A8;
        *PE_PHY_RX_CTRLR_LANE_3_PTR = 0x006E03A8;
        *PE_PHY_RX_CTRLR_LANE_4_PTR = 0x006E03A8;
        *PE_PHY_RX_CTRLR_LANE_5_PTR = 0x006E03A8;
        *PE_PHY_RX_CTRLR_LANE_6_PTR = 0x006E03A8;
        *PE_PHY_RX_CTRLR_LANE_7_PTR = 0x006E03A8;

        // Tx Tuning
        *PE_PHY_TX_CTRLR_LANE_0_PTR = 0x244064C4;
        *PE_PHY_TX_CTRLR_LANE_1_PTR = 0x244064C4;
        *PE_PHY_TX_CTRLR_LANE_2_PTR = 0x244064C4;
        *PE_PHY_TX_CTRLR_LANE_3_PTR = 0x244064C4;
        *PE_PHY_TX_CTRLR_LANE_4_PTR = 0x244064C4;
        *PE_PHY_TX_CTRLR_LANE_5_PTR = 0x244064C4;
        *PE_PHY_TX_CTRLR_LANE_6_PTR = 0x244064C4;
        *PE_PHY_TX_CTRLR_LANE_7_PTR = 0x244064C4;

        // GUC Fix already implemented at power-on
        TempVal = *PE_PHY_BIST_CTRLR_2_PTR;
        TempVal |= 0x38000;
        *PE_PHY_BIST_CTRLR_2_PTR = TempVal;

        util_delay(40);
        RegVal = *PE_PHY_BIST_OBSERVATION_PTR;
        RegVal = RegVal | 0x20;
        RegVal = RegVal & 0xFF;
        RegVal = RegVal << 0x15;
        RegVal = RegVal | TempVal;
        *PE_PHY_BIST_OBSERVATION_PTR = RegVal;

        util_delay(40);
        RegVal |= 0x20000000;
        *PE_PHY_BIST_CTRLR_2_PTR = RegVal;

        util_delay(40);
        *PE_PHY_BIST_CTRLR_2_PTR = 0x38000;

        //=================================================================
        // END OF PHY TUNING
        //=================================================================

        // De-assertion of PeCORE Resets.
        //   [3]=0: PCS Reset
        //   [5]=0: PIPE STP Reset
        //   [7]=0: PIPE STK Reset
        RegVal = *((volatile unsigned long *)0xFFCE3050);
        *((volatile unsigned long *)0xFFCE3050) = RegVal & 0xFFFFFF57;

        //=================================================================
        // END OF PeCORE and PCS RESET ASSERTION and DE-ASSERTION
        //=================================================================

        //=================================================================
        // START INITIALIZING PeCORE CSRs
        //=================================================================
        // WAIT FOR PHY READY 0xfff2_8000[1:0]
        // should be equal to 3 before we can proceed initialization
        while (1)
        {
            RegVal = *PE_PHY_STAT_PTR;
            if ((RegVal & 0x3) == 0x3)
            {
                break;
            }
        }

        // maximum error code value
        *PE_DECODE_ERR_LIMIT_REG = 0xFFF;

        // util_delay(2000);
        // ALTERNATIVE TO WAITING IS TO INSERT DELAY (ESTIMATE)

        // Initialize registers
        iop_init_config_regs();
        iop_init_internal_regs();

        // Mass storage controller
        *PE_CORE_INT_STA_PTR = 0x01800001;

        // Set msi auto clear
        *PE_HOST_ATT_AUTO_CLR_PTR       = 0x8FFFFFFF;
        *PE_ADV_ERR_UNCOR_ERR_MSK_PTR   = 0;

        // BAR0 - exposed SDRAM Area
        *PE_BAR_PTR_0_PTR               = 0xFFFFC000; // HTBAR0
        *PE_PCIE_IN_MAP_SZ_0_PTR        = 0xFFFFC001; // PIMS0
        *PE_PCIE_IN_MAP_LOCAL_0_PTR     = (unsigned long)Bar0Buffer; // PIML0

        // Set-up bar1 as exposed CSRs
        *PE_BAR_PTR_1_PTR               = 0xFFFFC000; // HTBAR1
        *PE_PCIE_IN_MAP_SZ_1_PTR        = 0xFFFFC001; // PIMS1
        *PE_PCIE_IN_MAP_LOCAL_1_PTR     = 0xFFF20000; // PIML1

        // Set-up bar 2 as io space for message protocol
        *PE_BAR_PTR_2_PTR               = 0xFFFFFF01; // HTBAR2
        *PE_PCIE_IN_MAP_SZ_2_PTR        = 0xFFFFFF01; // PIMS2
        *PE_PCIE_IN_MAP_LOCAL_2_PTR     = 0xFFF20100; // PIML2

        // Set-up bar 3 as io space for message protocol
        *PE_BAR_PTR_3_PTR               = 0xFFFFFF01; // HTBAR3
        *PE_PCIE_IN_MAP_SZ_3_PTR        = 0xFFFFFF01; // PIMS3
        *PE_PCIE_IN_MAP_LOCAL_3_PTR     = 0xFFF20400; // PIML3

        // Set-up bar 4
        *PE_BAR_PTR_4_PTR               = 0x00000000;
        *PE_PCIE_IN_MAP_SZ_4_PTR        = 0x00000000;

        // Set-up bar 5
        *PE_BAR_PTR_5_PTR               = 0x00000000;
        *PE_PCIE_IN_MAP_SZ_5_PTR        = 0x00000000;

        // Redirect msi interrupt
        *PE_MSI_CONFIG_4_PTR = 0x03020100;
        *PE_MSI_CONFIG_5_PTR = 0x07060504;
        *PE_MSI_CONFIG_7_PTR = 0x07060504;

        // CPMR
        *PE_CPL_PER_MRD_PTR = 0x00000010;

        // Completion credit return
        *PE_CORE_OPT_PTR        = 0x00001F00; // don't set HCE yet
        *PE_STA_CMD_PTR         = 0x00100407;

        *PE_ADV_ERR_UNCOR_ERR_MSK_PTR   = 0xFFFFFFFF;
        *PE_ADV_ERR_COR_ERR_MSK_PTR     = 0xFFFFFFFF;

        // IFDB settings
        *IFDB_AMCTL_0_PTR = 0x7F;
        *IFDB_AMCTL_1_PTR = 0x7F;
        *IFDB_AMCTL_2_PTR = 0x7F;
        *IFDB_AMCTL_3_PTR = 0x7F;

        // Dual engine
        *BM_MAIN_CTRL_PTR       = 0x200;
        // AWB Pairing
        *BM_AWB_ADDR_PAIR_PTR   = 0xE0E6E2E0;
        // Arb Ctrl
        *BM_ARB_CTRL_PTR        = 0x3;
        //=======================================
        // END OF PeCORE INITIALIZATION
        //=======================================

        // ENABLE HOST CONFIGURATION ENABLE (HCE)
        iop_init_enable_host_cfg();
        iop_init_parm();

        if ((_lr(0x418) >> 16) == 0xFADE)
        {
            MrDmaParm.MaxPayloadSize = 0xFFFF4800 | (_lr(0x418) & 0xE0);
            if ((MrDmaParm.MaxPayloadSize & 0xE0) == 0x0)
            {
                *PE_TX_DATA_MIN_THRES = TX_DATA_MIN_THRES_128;
            }
            else
            {
                ASSERT((MrDmaParm.MaxPayloadSize & 0xE0) == 0x20);
                *PE_TX_DATA_MIN_THRES = TX_DATA_MIN_THRES_256;
            }
        }

        // add delay for LTSSM to finish
        util_delay(1000);

        // clearing of interrupts
        *PE_CINT2_PTR = 0x0023000C;
        *((volatile unsigned long *)0xFFCE3050) = 0x00000200;

        DevConfigPtr = (void *)Bar0Buffer;
        DevConfigPtr->DetectPhase = 0;

        return;
    }

    *PE_CINT2_PTR = RegStat;

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_main
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_main (void)
{
    DEV_CONFIG_PARM_STRUCT *DevConfigPtr;

    DevConfigPtr = (void *)Bar0Buffer;

    if (DevConfigPtr->DetectPhase != IOP_RING_READY)
    {
        iop_init_ring();
    }

    // Workaround for ASPM control byte-enable
    *PE_LINK_STA_CTRL_PTR = 0x80;

    // Bus master enable
    *PE_STA_CMD_PTR = REG_PCSTCMD;

    // Max payload size
    if (MrDmaParm.MaxPayloadSize != 0)
    {
        *PE_DEV_STA_CTRL_PTR = MrDmaParm.MaxPayloadSize;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_prepare_read_rsp
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_prepare_read_rsp (PCB_STRUCT *PcbPtr)
{
    unsigned long RspFrameOffset;
    GEN_RSP_FRM_STRUCT *RspFramePtr;
    PCB_MRDMA_STRUCT *MrDmaPtr;

    RspFrameOffset = MrDmaParm.RspCntLoc * RSP_FRAME_SIZE;
    RspFramePtr = &MrDmaParm.DevRspFrmPtr[MrDmaParm.RspCntLoc];

    MrDmaParm.RcvCmdMax--;
    MrDmaParm.RspCntInf++;
    MrDmaParm.RspCntLoc = (MrDmaParm.RspCntLoc + 1) % RING_DEPTH;

    MrDmaPtr = (void *)&PcbPtr->Info;

    RspFramePtr->Word[0] = MrDmaPtr->Ctrl1;
    RspFramePtr->Word[1] = MrDmaPtr->Ctrl2;
    RspFramePtr->Word[7] = MrDmaParm.RspCntInf;

    // Reuse pcb for mrlar
    util_sll_insert_at_tail(&PcbPtr->Link,
                            &MrDmaParm.MrDmaQue);

    PcbPtr->Fn = iop_process_rsp;
    MrDmaPtr = (void *)&PcbPtr->Info;
    MrDmaPtr->HighAddr = MrDmaParm.HostHighBase;
    MrDmaPtr->LowAddr = MrDmaParm.HostRspFrmBase + RspFrameOffset;
    MrDmaPtr->DevAddr = MrDmaParm.DevRspFrmBase + RspFrameOffset;
    MrDmaPtr->Size = RSP_FRAME_WORD_SIZE;
    MrDmaPtr->Ctrl1 = MrDmaParm.RspCntLoc;

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_msg_read_error_reply
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_msg_read_error_reply (unsigned long *PayloadPtr)
{
    unsigned long CmdTag;
    IOP_CMD_INFO_STRUCT *CmdInfoPtr;
    unsigned long BuffOffset;
    PCB_STRUCT *PcbPtr;
    PCB_MRDMA_STRUCT *MrDmaPtr;

    CmdTag = PayloadPtr[0];
    CmdInfoPtr = &IopCmdInfo[CmdTag];

    CmdInfoPtr->DmxDeployCnt--;
    if (CmdInfoPtr->DmxDeployCnt > 0)
    {
        return;
    }

    // Process data
    ASSERT(CmdInfoPtr->InUse == ON);
    CmdInfoPtr->InUse = OFF;
    BuffOffset = (CmdTag * BUFFER_SIZE);

    PcbPtr = iop_post_mrlar();
    MrDmaPtr = (void *)&PcbPtr->Info;
    PcbPtr->Fn = iop_prepare_read_rsp;
    MrDmaPtr->HighAddr = MrDmaParm.HostHighBase;
    MrDmaPtr->LowAddr = MrDmaParm.HostDataBase + BuffOffset;
    MrDmaPtr->DevAddr = TX_BUFFER_BASE_ADDR + BuffOffset;
    MrDmaPtr->Size = CmdInfoPtr->CmdInfo.RdCmdInfo.DmaSize;
    MrDmaPtr->Ctrl1 = CmdInfoPtr->Profile;
    MrDmaPtr->Ctrl2 = 0;
    iop_activate_mrlar();

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_msg_rmw_unmapped_reply
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_msg_rmw_unmapped_reply (unsigned long *PayloadPtr)
{
    unsigned long CmdTag;
    IOP_CMD_INFO_STRUCT *CmdInfoPtr;

    CmdTag = PayloadPtr[0];
    CmdInfoPtr = &IopCmdInfo[CmdTag];
    ASSERT(CmdInfoPtr->InUse == ON);

    CmdInfoPtr->DmxDeployCnt--;
    if (CmdInfoPtr->DmxDeployCnt != 0)
    {
        return;
    }

    iop_process_data_rmw_read(CmdTag);

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : iop_init_parm
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_init_parm (void)
{
    util_sll_init(&MrDmaParm.MrDmaQue);

    util_init_pattern(&MrDmaParm,
                      sizeof(MrDmaParm),
                      INIT_PATTERN_LO_VALUE);

    util_init_pattern(&IopCmdInfo[0],
                      sizeof(IopCmdInfo),
                      INIT_PATTERN_LO_VALUE);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_init_cont
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long Gen2CntInit = 0;
void iop_init_cont (void)
{
    unsigned long RegVal;

    RegVal = *PE_EMP_PORT_STAT_PTR;
    RegVal |= 0x00400000;
    *PE_EMP_PORT_STAT_PTR = RegVal;

    *PE_EMP_PORT_ATTN_PTR = 0;

    iop_init_config_regs();
    iop_init_internal_regs();
    iop_init_bm();
    iop_init_enable_host_cfg();

    // Mass storage controller
    *PE_CORE_INT_STA_PTR = 0x01800001;

    // Set msi auto clear
    *PE_HOST_ATT_AUTO_CLR_PTR = 0x8FFFFFFF;
    *PE_ADV_ERR_UNCOR_ERR_MSK_PTR = 0;

    // BAR0 - exposed SDRAM Area
    *PE_PCIE_IN_MAP_LOCAL_0_PTR = (unsigned long)Bar0Buffer; // PIML0

    // Redirect msi interrupt
    *PE_MSI_CONFIG_4_PTR = 0x03020100;
    *PE_MSI_CONFIG_5_PTR = 0x07060504;
    *PE_MSI_CONFIG_7_PTR = 0x07060504;

    // CPMR
    *PE_CPL_PER_MRD_PTR = 0x00000010;

    // Completion credit return
    *PE_CORE_OPT_PTR = 0x00001F02;

    *PE_STA_CMD_PTR = 0x00100407;

    *PE_ADV_ERR_UNCOR_ERR_MSK_PTR = 0xFFFFFFFF;
    *PE_ADV_ERR_COR_ERR_MSK_PTR = 0xFFFFFFFF;

    // IFDB settings
    *IFDB_AMCTL_0_PTR = 0x7F;
    *IFDB_AMCTL_1_PTR = 0x7F;
    *IFDB_AMCTL_2_PTR = 0x7F;
    *IFDB_AMCTL_3_PTR = 0x7F;

    // Dual engine
    *BM_MAIN_CTRL_PTR = 0x200;

    // AWB Pairing
    *BM_AWB_ADDR_PAIR_PTR = 0xE0E6E2E0;

    // Arb Ctrl
    *BM_ARB_CTRL_PTR = 0x3;

    *PE_MEM_REQ_STA_0_PTR = 1;
    *PE_MEM_REQ_STA_1_PTR = 1;
    *PE_MEM_REQ_STA_2_PTR = 1;
    *PE_MEM_REQ_STA_3_PTR = 1;

    *PE_MEM_REQ_CTRL_0_PTR = MR_DMA_CONFIG;
    *PE_MEM_REQ_CTRL_1_PTR = MR_DMA_CONFIG;
    *PE_MEM_REQ_CTRL_2_PTR = MR_DMA_CONFIG;
    *PE_MEM_REQ_CTRL_3_PTR = MR_DMA_CONFIG;

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_detect_max_payload_size
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_detect_max_payload_size (void)
{
    unsigned long MaxPayloadSize;
    PCB_STRUCT *PcbPtr;

    MaxPayloadSize = _lr(0x418);

    if ((MaxPayloadSize >> 16) == 0xFADE)
    {
        MrDmaParm.MaxPayloadSize = 0xFFFF4800 | (MaxPayloadSize & 0xE0);
        *PE_DEV_STA_CTRL_PTR = MrDmaParm.MaxPayloadSize;

        if ((MrDmaParm.MaxPayloadSize & 0xE0) == 0x0)
        {
            *PE_TX_DATA_MIN_THRES = TX_DATA_MIN_THRES_128;
        }
        else
        {
            ASSERT((MrDmaParm.MaxPayloadSize & 0xE0) == 0x20);
            *PE_TX_DATA_MIN_THRES = TX_DATA_MIN_THRES_256;
        }
    }
    else
    {
        MrDmaParm.MaxPayloadSize = 0;

        MaxPayloadSize = *PE_DEV_STA_CTRL_PTR;

        _sr(0xAAAA4800 | (MaxPayloadSize & 0xE0),
            0x418);

        PcbPtr = _sched_get_pcb();
        PcbPtr->Fn = iop_check_bar;

        SCHED_POST_PCB(PcbPtr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_check_bar
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned char CheckBarFlag = 0;
void iop_check_bar (PCB_STRUCT *PcbPtr)
{
    unsigned long Bar0Val;

    Bar0Val = *PE_BAR_PTR_0_PTR;

    if (    (Bar0Val == 0xFFFFC000)
         || (Bar0Val == 0x0))
    {
        CheckBarFlag = 1;
        SCHED_POST_PCB(PcbPtr);

        return;
    }

    CheckBarFlag++;
    PcbPtr->Fn = iop_init_max_payload_size;
    SCHED_POST_PCB(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_init_max_payload_size
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long CheckPayloadFlag = 0;
void iop_init_max_payload_size (PCB_STRUCT *PcbPtr)
{
    unsigned long NewMaxPayload;
    unsigned long PrevMaxPayload;

    PrevMaxPayload = (_lr(0x418) & 0xE0);

    NewMaxPayload = (*PE_DEV_STA_CTRL_PTR & 0xE0);

    if (    (PrevMaxPayload == NewMaxPayload)
         && (CheckPayloadFlag != MAX_PLD_SZ_STOP_CHECK)) // end timer
    {
        CheckPayloadFlag = MAX_PLD_SZ_START_CHECK;
        SCHED_POST_PCB(PcbPtr);
        return;
    }

    CheckPayloadFlag++; // 1 - tmr not activated, 3 - tmr not reached, 4 - !3
    _sr(0xFADE4800 | NewMaxPayload,
        0x418);

    MrDmaParm.MaxPayloadSize = 0xFFFF4800 | NewMaxPayload;

    if ((MrDmaParm.MaxPayloadSize & 0xE0) == 0x0)
    {
        *PE_TX_DATA_MIN_THRES = TX_DATA_MIN_THRES_128;
    }
    else
    {
        ASSERT((MrDmaParm.MaxPayloadSize & 0xE0) == 0x20);
        *PE_TX_DATA_MIN_THRES = TX_DATA_MIN_THRES_256;
    }

    _sched_return_pcb(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_check_max_payload_size_done
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_check_max_payload_size_done (void)
{
    CheckPayloadFlag = MAX_PLD_SZ_STOP_CHECK;

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_read_max_payload_size_flag
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long iop_read_max_payload_size_flag (void)
{
    return CheckPayloadFlag;
}


//-----------------------------------------------------------------------------
// Function    : iop_init_internal_regs
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_init_internal_regs (void)
{
    unsigned long RegVal;

    RegVal = *PE_CINT2_PTR;
    RegVal |= 0x0000000C;
    *PE_CINT2_PTR = RegVal;

    *PE_CORE_ID_PTR = PE_MSI_MSK;

    RegVal = *PE_CINT2_PTR;
    RegVal &= PE_CINT2_MASK;
    *PE_CINT2_PTR = RegVal;

    RegVal = *PE_CORE_OPT_PTR;
    RegVal &= PE_CORE_OPT_MASK;
    RegVal |= PE_CORE_OPT;
    *PE_CORE_OPT_PTR = RegVal;

    RegVal = *PE_CORE_INT_STA_PTR;
    RegVal &= PE_CORE_INT_STA_MASK;
    *PE_CORE_INT_STA_PTR = RegVal;

    *PE_UPD_FC_LAT_PTR = PE_UP_FC_LAT;

    *PE_CPL_PER_MRD_PTR = PE_CPL_PER_MRD;

    *PE_ACK_NAK_LAT_CTRL_PTR = PE_ACK_NAK_LAT_CTRL;

    *PE_MAX_OUT_REQ_PAR_PTR = PE_MAX_OUT_REQ_PAR;

    *PE_HOST_ATT_AUTO_CLR_PTR = 0x0;

    *PE_LTSSM_AUX_STA_CTRL_CAP_PTR = PE_LTSSM_AUX_STA_CTRL_CAP;

    *PE_SYS_PWR_MGMT_CTRL_PTR = PE_SYS_PWR_MGMT_CTRL;

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_init_config_regs
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_init_config_regs (void)
{
    unsigned long RegVal;

    // IOP INIT CONFIG REGISTERS

    *PE_DEV_VEN_ID_PTR = PE_DEV_VEN_ID;

    *PE_STA_CMD_PTR = PE_STA_CMD;

    *PE_CLASS_CODE_REV_ID_PTR = PE_CLASS_CODE_REV_ID;

    *PE_HDR_TYPE_CACHE_LINE_SZ_PTR = 0;

    *PE_EXP_ROM_BASE_PTR_PTR = 0xFFFF0001;

    *PE_SUBSYS_VEN_DEV_ID_PTR = PE_SUB_SYS_VEN_DEV_ID;

    *PE_CAP_POINTER_PTR = PE_CAP_POINTER;

    *PE_INT_PIN_LINE_PTR = PE_INT_PIN_LINE;

    *PE_PCIE_CAP_HDR_PTR = 0x0;

    *PE_LINK_CAP_PTR = PE_LINK_CAP_MASK & 0xFFFF; // ???
    *PE_LINK_CAP_PTR = 0x0003E482; // L0s Entry Supported

    RegVal = *PE_LINK_STA_CTRL_PTR;
    RegVal &= PE_LINK_STA_CTRL_MASK;
    RegVal |= 0x80;                 // set extended sync
    *PE_LINK_STA_CTRL_PTR = RegVal;

    *PE_DEV_CAP_2_PTR = 0x0;

    *PE_ADV_ERR_REP_CAP_HDR_PTR = PE_ADV_ERR_REP_CAP_HDR;

    *PE_ADV_ERR_UNCOR_ERR_MSK_PTR = 0x0;

    *PE_ADV_ERR_UNCOR_ERR_SEV_PTR = PE_ADV_ERR_UN_COR_ERR_SEV;

    *PE_ADV_ERR_COR_ERR_MSK_PTR = 0;

    *PE_ADV_ERR_CAP_CTRL_PTR = PE_ADV_ERR_CAP_CTRL;

    *PE_ADV_ERR_HDR_LOG_0_PTR = 0x0;

    *PE_ADV_ERR_HDR_LOG_1_PTR = 0x0;

    *PE_ADV_ERR_HDR_LOG_2_PTR = 0x0;

    *PE_ADV_ERR_HDR_LOG_3_PTR = 0x0;

    *PE_MSI_CAP_HDR_PTR = PE_MSI_CAP_HDR;

    *PE_MSI_MSK_PTR = 0xFFFFFF00;

    *PE_DEV_SRL_NUM_CAP_HDR_PTR = PE_DEV_SRL_NUM_CAP_HDR;

    // IOP INIT CONFIG REGISTERS

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_init_bm
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_init_bm (void)
{
    *BM_ARB_CTRL_PTR |= BM_ARB_CTRL;

    *BM_CSR_CTRL_PTR |= BM_CSR_CTRL;

    *BM_TXI_CTRL_PTR |= BM_TX_INST_CTRL;

    *BM_RXD_CTRL_PTR |= BM_RX_DMA_CTRL;

    *BM_TXD_CTRL_PTR |= BM_TX_DMA_CTRL;

    *BM_NORM_DNE_INT_EN_PTR |= BM_NORM_DNE_INT;

    *BM_NULL_RCH_INT_EN_PTR |= BM_NULL_RCH_INT;

    *BM_PAR_ERR_INT_EN_PTR |=  BM_PAR_ERR_INT;

    *BM_GEN_ERR1_INT_EN_PTR |= BM_GEN_ERR1_INT;

    *BM_NORM_DNE_INT_MSK_PTR &= ~BM_NORM_DNE_INT_MSK;

    *BM_NULL_RCH_INT_MSK_PTR &= ~BM_NULL_RCH_INT_MSK;

    *BM_NORM_DNE_INT_C1N0_PTR &= ~BM_NORM_DNE_INT_NORM;

    *BM_NULL_RCH_INT_C1N0_PTR
        |= ( BM_NULL_RCH_INT_CRIT | ~BM_NULL_RCH_INT_NORM );

    *BM_GEN_ERR1_INT_C1N0_PTR
        |= (BM_GEN_ERR1_INT_CRIT | ~BM_GEN_ERR1_INT_NORM);

    *BM_CNT_COAL_INT_TRGR_PTR =  BM_CNT_COAL_INT_TRGR;

    *BM_TIME_COAL_INT_TRGR_PTR = BM_TIME_COAL_INT_TRGR;

    *BM_NORM_COAL_INT_CONF_CTRL_PTR = BM_NORM_COAL_INT_CFG_CTRL;

    *BM_MAX_TAG_DEV_NUM_PTR = BM_MAX_TAG_DEV_NUM;

    *BM_TX_INJ_DEPTH_PTR = BM_TX_INJ_DEPTH;

    *BM_TXD_CTRL_PTR = BM_TX_DMA_CTRL2;

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_init_enable_host_cfg
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_init_enable_host_cfg (void)
{
    unsigned long RegVal;

    RegVal = *PE_CORE_OPT_PTR;
    RegVal |= HOST_CFG_ENA;
    *PE_CORE_OPT_PTR = RegVal;

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_init_ring
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_init_ring (void)
{
    unsigned long HostPortCtrl;

    DEV_CONFIG_PARM_STRUCT *DevConfigPtr = (void *)Bar0Buffer;

    HostPortCtrl = *PE_EMP_PORT_CTRL_PTR;

    if (HostPortCtrl & 0xFF)
    {
        MrDmaParm.HostHighBase = DevConfigPtr->Host.RingHigh;
        MrDmaParm.HostCmdFrmBase = DevConfigPtr->Host.RingLow + MBX_SZ;
        MrDmaParm.HostRspFrmBase = DevConfigPtr->Host.RingLow + MBX_SZ
            + (RING_DEPTH * CMD_FRAME_SIZE);
        MrDmaParm.HostDataBase = DevConfigPtr->Host.DataLow;

        MrDmaParm.DevCmdFrmPtr
            = (void *)((unsigned long)Bar0Buffer + MBX_SZ + DEV_CONFIG_SIZE);

        MrDmaParm.DevCmdFrmBase
            = (unsigned long)MrDmaParm.DevCmdFrmPtr | MRDMA_RX_DIR_N_AWB;

        MrDmaParm.DevRspFrmPtr
            = (void *)((unsigned long)Bar0Buffer + MBX_SZ + DEV_CONFIG_SIZE
            + (RING_DEPTH * CMD_FRAME_SIZE));

        MrDmaParm.DevRspFrmBase
            = (unsigned long)MrDmaParm.DevRspFrmPtr | MRDMA_TX_DIR_N_AWB;

        *PE_MEM_REQ_PTR_HIGH_0_PTR = DevConfigPtr->Host.RingHigh;
        *PE_MEM_REQ_PTR_HIGH_1_PTR = DevConfigPtr->Host.RingHigh;
        *PE_MEM_REQ_PTR_HIGH_2_PTR = DevConfigPtr->Host.RingHigh;
        *PE_MEM_REQ_PTR_HIGH_3_PTR = DevConfigPtr->Host.RingHigh;

        if (DevConfigPtr->Host.RingHigh == 0)
        {
            *PE_MEM_REQ_CTRL_0_PTR = MR_DMA_CONFIG;
            *PE_MEM_REQ_CTRL_1_PTR = MR_DMA_CONFIG;
            *PE_MEM_REQ_CTRL_2_PTR = MR_DMA_CONFIG;
            *PE_MEM_REQ_CTRL_3_PTR = MR_DMA_CONFIG;
        }
        else
        {
            *PE_MEM_REQ_CTRL_0_PTR = MR_DMA_CONFIG | 1;
            *PE_MEM_REQ_CTRL_1_PTR = MR_DMA_CONFIG | 1;
            *PE_MEM_REQ_CTRL_2_PTR = MR_DMA_CONFIG | 1;
            *PE_MEM_REQ_CTRL_3_PTR = MR_DMA_CONFIG | 1;
        }

        *PE_MEM_REQ_STA_0_PTR = 1;
        *PE_MEM_REQ_STA_1_PTR = 1;
        *PE_MEM_REQ_STA_2_PTR = 1;
        *PE_MEM_REQ_STA_3_PTR = 1;

        DevConfigPtr->Signature     = 0xaa55aa55;
        DevConfigPtr->DetectPhase   = IOP_RING_READY;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_init (void)
{
    unsigned long RegVal;
    DEV_CONFIG_PARM_STRUCT *DevConfigPtr;

    if (*CR_PMU3_PE1_PTR == 0x8FE)
    {
        return;
    }

    iop_init_parm();
    scsi_init();
    iop_ucode_init();

    DevConfigPtr = (void *)Bar0Buffer;

    util_init_pattern(Bar0Buffer,
                      sizeof(DEV_CONFIG_PARM_STRUCT),
                      INIT_PATTERN_LO_VALUE);

    DevConfigPtr->DetectPhase = 0;
    DevConfigPtr->DevCacheStatus.Union.Cache.WriteCacheDisable =
        ~SysConfigCurr.ChnlMode.ProtMode.Scsi.Caching.WriteCacheEnable & 1;

    // Clear errors during link up
    RegVal = *PE_ADV_ERR_UNCOR_ERR_STA_PTR;
    *PE_ADV_ERR_UNCOR_ERR_STA_PTR = RegVal;

    RegVal = *PE_ADV_ERR_COR_ERR_STA_PTR;
    *PE_ADV_ERR_COR_ERR_STA_PTR = RegVal;
    *PE_ADV_ERR_COR_ERR_STA_PTR = 0x1;

    *LC_SLIO0_PTR = 0xFFFFFCFF;

    // Other init before maxpayload detection
    iop_init_cont();

    iop_detect_max_payload_size();

    // Initialize timer 0
    _sr(3, 0x22);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_serv_mrlar
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_serv_mrlar(void)
{
    unsigned long MrDmaStat;
    unsigned long UsedEngine;

    UsedEngine = MrDmaParm.UsedEngine;

    if (UsedEngine & 1)
    {
        MrDmaStat = *PE_MEM_REQ_STA_0_PTR;

        if (MrDmaStat == 1)
        {
            *PE_MEM_REQ_STA_0_PTR = 1;

            MrDmaParm.UsedEngine &= ~1;

            // Execute call back
            Ch0PcbPtr->Fn(Ch0PcbPtr);
        }
    }

    if (UsedEngine & 2)
    {
        MrDmaStat = *PE_MEM_REQ_STA_1_PTR;

        if (MrDmaStat == 1)
        {
           *PE_MEM_REQ_STA_1_PTR = 1;

            MrDmaParm.UsedEngine &= ~2;

            // Execute call back
            Ch1PcbPtr->Fn(Ch1PcbPtr);
        }
    }

    if (UsedEngine & 4)
    {
        MrDmaStat = *PE_MEM_REQ_STA_2_PTR;

        if (MrDmaStat == 1)
        {
            *PE_MEM_REQ_STA_2_PTR = 1;

            MrDmaParm.UsedEngine &= ~4;

            // Execute call back
            Ch2PcbPtr->Fn(Ch2PcbPtr);
        }
    }

    if (UsedEngine & 8)
    {
        MrDmaStat = *PE_MEM_REQ_STA_3_PTR;

        if (MrDmaStat == 1)
        {
            *PE_MEM_REQ_STA_3_PTR = 1;

            MrDmaParm.UsedEngine &= ~8;

            // Execute call back
            Ch3PcbPtr->Fn(Ch3PcbPtr);
        }
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_prcs_cmd_read_bulk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_prcs_cmd_read_bulk (GEN_CMD_FRM_STRUCT *FramePtr)
{
    USER_CMD_FRM_STRUCT *RdFrmPtr;
    unsigned long Profile;
    unsigned long CmdTag;
    IOP_CMD_INFO_STRUCT *CmdInfoPtr;
    unsigned long CntNSize;
    IPCALL_STRUCT *IpCallPtr;
    unsigned long DmId;

    RdFrmPtr = (void *)FramePtr;

    Profile = RdFrmPtr->Profile;
    CmdTag = (Profile >> 8) & 0xFF;
    CmdInfoPtr = &IopCmdInfo[CmdTag];
    CntNSize = RdFrmPtr->CntNSize;

    ASSERT(CmdInfoPtr->InUse == OFF);
    CmdInfoPtr->InUse = ON;

    CmdInfoPtr->DmxDeployCnt = CntNSize & 0xF;
    CmdInfoPtr->Profile = Profile;
    CmdInfoPtr->CmdInfo.RdCmdInfo.DmaSize = CntNSize >> 16;
    CmdInfoPtr->CmdInfo.RdCmdInfo.SectionStat = 0;
    CmdInfoPtr->CmdInfo.RdCmdInfo.ErrorBmp[0] = 0;
    CmdInfoPtr->CmdInfo.RdCmdInfo.ErrorBmp[1] = 0;

    DmId = ((Profile >> 6) & 3) - 1;
    IpCallPtr = ipcall_add_slave_entry(DmId);
    IpCallPtr->Fn = media_prcs_read_bulk;
    IpCallPtr->Arg[0] = (unsigned long)FramePtr;

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_prcs_cmd_write_bulk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_prcs_cmd_write_bulk (GEN_CMD_FRM_STRUCT *FramePtr)
{
    USER_CMD_FRM_STRUCT *WrFrmPtr;
    unsigned long Profile;
    unsigned long CmdTag;
    IOP_CMD_INFO_STRUCT *CmdInfoPtr;
    unsigned long CntNSize;
    unsigned long ValidSxnCnt;
    PCB_STRUCT *PcbPtr;
    PCB_MRDMA_STRUCT *MrDmaPtr;

    WrFrmPtr = (void *)FramePtr;

    Profile = WrFrmPtr->Profile;
    CmdTag = (Profile >> 8) & 0xFF;
    CmdInfoPtr = &IopCmdInfo[CmdTag];
    CntNSize = WrFrmPtr->CntNSize;
    ValidSxnCnt = CntNSize & 0xF;

    ASSERT(CmdInfoPtr->InUse == OFF);
    CmdInfoPtr->InUse = ON;

    CmdInfoPtr->DmxDeployCnt = DIV_ROUND_UP(ValidSxnCnt, USER_SXNS_PER_PAGE);
    CmdInfoPtr->Profile = Profile;
    CmdInfoPtr->CmdInfo.WrCmdInfo.FbxIdx = (WrFrmPtr->FbxIdxs & 0xF);
    CmdInfoPtr->CmdInfo.WrCmdInfo.UsrSxn[0] = WrFrmPtr->SxnIdx[0];
    CmdInfoPtr->CmdInfo.WrCmdInfo.UsrSxn[1] = WrFrmPtr->SxnIdx[1];
    CmdInfoPtr->CmdInfo.WrCmdInfo.UsrSxn[2] = WrFrmPtr->SxnIdx[2];
    CmdInfoPtr->CmdInfo.WrCmdInfo.UsrSxn[3] = WrFrmPtr->SxnIdx[3];
    CmdInfoPtr->CmdInfo.WrCmdInfo.ValidSxnCnt = ValidSxnCnt;

    // Flush lower half of cmd info so that it can be seen by dm
    bios_flush_dc_line(&CmdInfoPtr->CmdInfo);

    PcbPtr = iop_post_mrlar();
    PcbPtr->Fn = iop_process_data_write;
    MrDmaPtr = (void *)&PcbPtr->Info;
    MrDmaPtr->HighAddr = MrDmaParm.HostHighBase;
    MrDmaPtr->LowAddr = WrFrmPtr->HostAddrLow;
    MrDmaPtr->Size = CntNSize >> 16;
    MrDmaPtr->DevAddr = WrFrmPtr->BuffAddr;
    MrDmaPtr->Ctrl1 = CmdTag;

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_prcs_cmd_rmw_bulk
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_prcs_cmd_rmw_bulk (GEN_CMD_FRM_STRUCT *FramePtr)
{
    USER_CMD_FRM_STRUCT *WrFrmPtr;
    unsigned long Profile;
    unsigned long CmdTag;
    IOP_CMD_INFO_STRUCT *CmdInfoPtr;
    unsigned long CntNSize;
    unsigned long ValidSxnCnt;
    unsigned long DmId;
    IPCALL_STRUCT *IpCallPtr;

    WrFrmPtr = (void *)FramePtr;

    Profile = WrFrmPtr->Profile;
    CmdTag = (Profile >> 8) & 0xFF;
    CmdInfoPtr = &IopCmdInfo[CmdTag];
    CntNSize = WrFrmPtr->CntNSize;
    ValidSxnCnt = CntNSize & 0xF;

    ASSERT(CmdInfoPtr->InUse == OFF);
    CmdInfoPtr->InUse = ON;

    CmdInfoPtr->DmxDeployCnt = ValidSxnCnt;
    CmdInfoPtr->Profile = Profile;
    CmdInfoPtr->CmdInfo.WrCmdInfo.IoDeployCnt = ValidSxnCnt;
    CmdInfoPtr->CmdInfo.WrCmdInfo.Phase = RMW_READ_PHASE;

    // Dm related info
    CmdInfoPtr->CmdInfo.WrCmdInfo.FbxIdx = (WrFrmPtr->FbxIdxs & 0xF);
    CmdInfoPtr->CmdInfo.WrCmdInfo.UsrSxn[0] = WrFrmPtr->SxnIdx[0];
    CmdInfoPtr->CmdInfo.WrCmdInfo.UsrSxn[1] = WrFrmPtr->SxnIdx[1];
    CmdInfoPtr->CmdInfo.WrCmdInfo.UsrSxn[2] = WrFrmPtr->SxnIdx[2];
    CmdInfoPtr->CmdInfo.WrCmdInfo.UsrSxn[3] = WrFrmPtr->SxnIdx[3];
    CmdInfoPtr->CmdInfo.WrCmdInfo.LbaCnts = WrFrmPtr->LbaCnts;
    CmdInfoPtr->CmdInfo.WrCmdInfo.LbaOffsets = WrFrmPtr->SxnOfsts;
    CmdInfoPtr->CmdInfo.WrCmdInfo.ValidSxnCnt = ValidSxnCnt;

    // Flush lower half of cmd info so that it can be seen by dm
    bios_flush_dc_line(&CmdInfoPtr->CmdInfo);

    DmId = ((Profile >> 6) & 3) - 1;
    ASSERT(DmId == FbxToDmId[CmdInfoPtr->CmdInfo.WrCmdInfo.FbxIdx]);
    IpCallPtr = ipcall_add_slave_entry(DmId);
    IpCallPtr->Fn = media_prcs_rmw_read_bulk;
    IpCallPtr->Arg[0] = CmdTag;

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_prcs_cmd_check_system_stat
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_prcs_cmd_check_system_stat (GEN_CMD_FRM_STRUCT *FramePtr)
{
    GEN_RSP_FRAME_STRUCT RspFrame;
    unsigned long Value;

    if (DmFlagParm.SystemStat == SYSTEM_READY)
    {
        Value = 0xDEADEAFF;
    }
    else
    {
        Value = 0;
    }

    RspFrame.Word[0] = FramePtr->Word[0];
    RspFrame.Word[1] = Value;
    iop_prepare_rsp(&RspFrame);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_prcs_cmd_shutdown
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_prcs_cmd_shutdown (GEN_CMD_FRM_STRUCT *FramePtr)
{
    unsigned long CmdTag;
    GEN_RSP_FRAME_STRUCT RspFrame;

    for (CmdTag = 0;
         CmdTag < BUFFER_CNT;
         CmdTag++)
    {
        ASSERT(IopCmdInfo[CmdTag].InUse == OFF);
    }

    // Send profile
    RspFrame.Word[0] = FramePtr->Word[0];
    iop_prepare_rsp(&RspFrame);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_prcs_non_user_cmd
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_prcs_non_user_cmd (GEN_CMD_FRM_STRUCT *FramePtr)
{
    PCB_STRUCT *PcbPtr;
    SCSI_CMD_FRAME_STRUCT *ScsiCmdFramePtr;
    unsigned char *CdbPtr;
    unsigned char OpCd;
    BIT_STAT Stat;
    GEN_RSP_FRAME_STRUCT RspFrame;

    // Get pcb to process this command frame
    PcbPtr = _sched_get_pcb();

    // Get command frame generic parameters
    ScsiCmdFramePtr = (void *)FramePtr;
    PcbPtr->Info.MrDma.Ctrl1 = ScsiCmdFramePtr->Profile;
    PcbPtr->Info.MrDma.HighAddr = ScsiCmdFramePtr->HostAddrHi;
    PcbPtr->Info.MrDma.LowAddr = ScsiCmdFramePtr->HostAddrLo;

    CdbPtr = ScsiCmdFramePtr->Cdb;
    OpCd = *CdbPtr;
    PcbPtr->Word.FrameHdl = ScsiCmdFramePtr;

    // Command validation
    Stat = scsi_validate_cdb[OpCd](PcbPtr,
                                   CdbPtr);

    // Check validation result
    if (    (Stat != SUCCESSFUL)
         && (Stat != PROT_CMD_DONE))
    {
        // Change to sense address
        PcbPtr->Info.MrDma.HighAddr = ScsiCmdFramePtr->SenseBuffAddrHi;
        PcbPtr->Info.MrDma.LowAddr = ScsiCmdFramePtr->SenseBuffAddrLo;
        scsi_send_sense_data(PcbPtr,
                             OFF);
        return;
    }

    if (Stat == PROT_CMD_DONE)
    {
        _sched_return_pcb(PcbPtr);
        RspFrame.Word[0] = ScsiCmdFramePtr->Profile;
        RspFrame.Word[1] = SUCCESSFUL;
        iop_prepare_rsp(&RspFrame);
        return;
    }

    SCHED_POST_PCB(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_prcs_cmd_err_gross
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_prcs_cmd_err_gross (GEN_CMD_FRM_STRUCT *FramePtr)
{
    ASSERT(0);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_check_sys_ready
// Description :
// Parameters  : NONE
// Returns     : Status
//-----------------------------------------------------------------------------
BIT_STAT iop_check_sys_ready (void)
{
    if (DmFlagParm.SystemStat == SYSTEM_READY)
    {
        return SUCCESSFUL;
    }

    return SYSTEM_NOT_READY;
}


//-----------------------------------------------------------------------------
// Function    : iop_process_cmd
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_process_cmd (PCB_STRUCT *PcbPtr)
{
    unsigned long CmdCnt;
    unsigned long PutLoc;
    GEN_CMD_FRM_STRUCT *FramePtr;
    unsigned long OpCode;
    PCB_MRDMA_STRUCT *MrDmaPtr;

    MrDmaPtr = (void *)&PcbPtr->Info;
    CmdCnt = MrDmaPtr->Ctrl1;
    PutLoc = MrDmaPtr->Ctrl2;
    FramePtr = &MrDmaParm.DevCmdFrmPtr[PutLoc];

    _sched_return_pcb(PcbPtr);
    util_delay(100);

    while (CmdCnt)
    {
        OpCode = FramePtr->Word[0] >> 24;

        ASSERT(OpCode < 0x40); // todo: markb change function array size
        if (OpCode != 0)
        {
            iop_prcs_cmd_fn[OpCode](FramePtr);
        }
        else
        {
            // Four byte shift occur, refetch cmd frame
            PcbPtr = iop_post_mrlar();
            PcbPtr->Fn = iop_process_cmd;
            MrDmaPtr = (void *)&PcbPtr->Info;
            MrDmaPtr->DevAddr
                = MrDmaParm.DevCmdFrmBase + (PutLoc * CMD_FRAME_SIZE);
            MrDmaPtr->LowAddr
                = MrDmaParm.HostCmdFrmBase + (PutLoc * CMD_FRAME_SIZE);
            MrDmaPtr->HighAddr = MrDmaParm.HostHighBase;
            MrDmaPtr->Size = CmdCnt * CMD_FRAME_WORD_SIZE;
            MrDmaPtr->Ctrl1 = CmdCnt;
            MrDmaPtr->Ctrl2 = PutLoc;

            iop_activate_mrlar();

            // Count occurrences
            MrDmaParm.FourByteShiftCnt++;
            break;
        }

        CmdCnt--;
        PutLoc++;
        FramePtr++;
    }

    ipcall_slaves_exec_calls();

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_process_data_write
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_process_data_write (PCB_STRUCT *PcbPtr)
{
    unsigned long CmdTag;
    IOP_CMD_INFO_STRUCT *CmdInfoPtr;
    unsigned long Profile;
    IPCALL_STRUCT *IpCallPtr;
    unsigned long DmId;

    CmdTag = PcbPtr->Info.MrDma.Ctrl1;

    CmdInfoPtr = &IopCmdInfo[CmdTag];
    ASSERT(CmdInfoPtr->InUse == ON);

    Profile = CmdInfoPtr->Profile;
    DmId = ((Profile >> 6) & 3) - 1;

    util_delay(200);

    IpCallPtr = ipcall_add_slave_entry(DmId);
    IpCallPtr->Fn = media_prcs_write_bulk;
    IpCallPtr->Arg[0] = CmdTag;
    ipcall_slave_exec_calls(DmId);

    _sched_return_pcb(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_process_data_rmw_read
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_process_data_rmw_read (unsigned long CmdTag)
{
    unsigned long ValidSxnCnt;
    unsigned long LbaCnts;
    unsigned long LbaOffsets;
    unsigned long BuffOffset;
    unsigned long SxnIdx;
    PCB_STRUCT *PcbPtr;
    PCB_MRDMA_STRUCT *MrDmaPtr;
    IOP_CMD_INFO_STRUCT *CmdInfoPtr;

    // rcantong??? Rename to iop_prcs_rmw_rx_data
    CmdInfoPtr = &IopCmdInfo[CmdTag];
    ASSERT(CmdInfoPtr->InUse == ON);

    ValidSxnCnt = CmdInfoPtr->CmdInfo.WrCmdInfo.ValidSxnCnt;
    ASSERT(CmdInfoPtr->CmdInfo.WrCmdInfo.IoDeployCnt > 0);
    ASSERT(CmdInfoPtr->DmxDeployCnt == 0);
    CmdInfoPtr->DmxDeployCnt = DIV_ROUND_UP(ValidSxnCnt, USER_SXNS_PER_PAGE);

    LbaCnts = CmdInfoPtr->CmdInfo.WrCmdInfo.LbaCnts;
    LbaOffsets = CmdInfoPtr->CmdInfo.WrCmdInfo.LbaOffsets;

    // Perform write stage rmw
    CmdInfoPtr->CmdInfo.WrCmdInfo.Phase = RMW_WRITE_PHASE;
    for (SxnIdx = 0;
         SxnIdx < ValidSxnCnt;
         SxnIdx++)
    {
        PcbPtr = iop_post_mrlar();
        PcbPtr->Fn = iop_process_data_rmw_write;
        MrDmaPtr = (void *)&PcbPtr->Info;

        BuffOffset
            = (CmdTag * BUFFER_SIZE)
            + (SxnIdx * USER_SXN_SIZE)
            + (((LbaOffsets >> (SxnIdx * 4)) & 0xF) * LBA_SIZE);

        MrDmaPtr->HighAddr = MrDmaParm.HostHighBase;
        MrDmaPtr->LowAddr = MrDmaParm.HostDataBase + BuffOffset;
        MrDmaPtr->DevAddr = RX_BUFFER_BASE_ADDR + BuffOffset;
        MrDmaPtr->Size = (((LbaCnts >> (SxnIdx * 4)) & 0xF) * LBA_SIZE) / 4;
        MrDmaPtr->Ctrl1 = CmdTag;
    }

    iop_activate_mrlar();

    return;
}


//-----------------------------------------------------------------------------
// Function    :
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_process_data_rmw_write (PCB_STRUCT *PcbPtr)
{
    unsigned long CmdTag;
    IOP_CMD_INFO_STRUCT *CmdInfoPtr;
    unsigned long DmId;
    IPCALL_STRUCT *IpCallPtr;

    CmdTag = PcbPtr->Info.MrDma.Ctrl1;
    CmdInfoPtr = &IopCmdInfo[CmdTag];
    ASSERT(CmdInfoPtr->InUse == ON);

    _sched_return_pcb(PcbPtr);

    CmdInfoPtr->CmdInfo.WrCmdInfo.IoDeployCnt--;
    if (CmdInfoPtr->CmdInfo.WrCmdInfo.IoDeployCnt != 0)
    {
        return;
    }

    DmId = ((CmdInfoPtr->Profile >> 6) & 3) - 1;
    IpCallPtr = ipcall_add_slave_entry(DmId);
    IpCallPtr->Fn = media_prcs_rmw_write_bulk;
    IpCallPtr->Arg[0] = CmdTag;

    ipcall_slave_exec_calls(DmId);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_prepare_rsp
// Description : Generic response frame for non critical path commands
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_prepare_rsp (GEN_RSP_FRAME_STRUCT *RspFrmPtr)
{
    unsigned long RspFrameOffset;
    GEN_RSP_FRAME_STRUCT *RspFramePtr;
    PCB_STRUCT *PcbPtr;

    // Prepare response frame
    RspFrameOffset = MrDmaParm.RspCntLoc * RSP_FRAME_SIZE;
    RspFramePtr = &MrDmaParm.DevRspFrmPtr[MrDmaParm.RspCntLoc];

    MrDmaParm.RcvCmdMax--;
    MrDmaParm.RspCntInf++;
    MrDmaParm.RspCntLoc = (MrDmaParm.RspCntLoc + 1) % RING_DEPTH;

    // Transfer contents to response frame ring
    RspFramePtr->Word[0] = RspFrmPtr->Word[0]; // Profile
    RspFramePtr->Word[1] = RspFrmPtr->Word[1];
    RspFramePtr->Word[2] = RspFrmPtr->Word[2];
    RspFramePtr->Word[3] = RspFrmPtr->Word[3];
    RspFramePtr->Word[4] = RspFrmPtr->Word[4];
    RspFramePtr->Word[5] = RspFrmPtr->Word[5];
    RspFramePtr->Word[6] = RspFrmPtr->Word[6];
    RspFramePtr->Word[7] = MrDmaParm.RspCntInf;

    // Transmit response frame
    PcbPtr = iop_post_mrlar();
    PcbPtr->Fn = iop_process_rsp;
    PcbPtr->Info.MrDma.HighAddr = MrDmaParm.HostHighBase;
    PcbPtr->Info.MrDma.LowAddr = MrDmaParm.HostRspFrmBase + RspFrameOffset;
    PcbPtr->Info.MrDma.DevAddr = MrDmaParm.DevRspFrmBase + RspFrameOffset;
    PcbPtr->Info.MrDma.Size = RSP_FRAME_WORD_SIZE;
    PcbPtr->Info.MrDma.Ctrl1 = MrDmaParm.RspCntLoc;

    return;
}


//-----------------------------------------------------------------------------
// Function    :
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_prepare_write_rsp (unsigned long Profile)
{
    unsigned long RspFrameOffset;
    GEN_RSP_FRAME_STRUCT *RspFramePtr;
    PCB_STRUCT *PcbPtr;
    PCB_MRDMA_STRUCT *MrDmaPtr;

    RspFrameOffset = MrDmaParm.RspCntLoc * RSP_FRAME_SIZE;
    RspFramePtr = &MrDmaParm.DevRspFrmPtr[MrDmaParm.RspCntLoc];

    MrDmaParm.RcvCmdMax--;
    MrDmaParm.RspCntInf++;
    MrDmaParm.RspCntLoc = (MrDmaParm.RspCntLoc + 1) % RING_DEPTH;

    RspFramePtr->Word[0] = Profile;
    RspFramePtr->Word[7] = MrDmaParm.RspCntInf;

    PcbPtr = iop_post_mrlar();
    PcbPtr->Fn = iop_process_rsp;
    MrDmaPtr = (void *)&PcbPtr->Info;
    MrDmaPtr->HighAddr = MrDmaParm.HostHighBase;
    MrDmaPtr->LowAddr = MrDmaParm.HostRspFrmBase + RspFrameOffset;
    MrDmaPtr->DevAddr = MrDmaParm.DevRspFrmBase + RspFrameOffset;
    MrDmaPtr->Size = RSP_FRAME_WORD_SIZE;
    MrDmaPtr->Ctrl1 = MrDmaParm.RspCntLoc;
    iop_activate_mrlar();

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_process_rsp
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_process_rsp (PCB_STRUCT *PcbPtr)
{
    *PE_EMP_RSP_PUT_0_PTR = PcbPtr->Info.MrDma.Ctrl1;
    _sched_return_pcb(PcbPtr);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_process_ucode
// Description : Initialization of download microcode module
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_ucode_init(void)
{
    PCB_UCODE_STRUCT *UcodePtr;

    UcodePtr = (void *)&UcodePcb.Info;

    UcodePtr->DevAddr = UCODE_BASE_ADDR;
    UcodePtr->FwSize = 0;
    UcodePtr->CurrSize = 0;
    UcodePtr->CheckSum = 0;
    UcodePtr->CurrCheckSum = 0;

    return;
}

//-----------------------------------------------------------------------------
// Function    : iop_process_ucode_cb1
// Description : Callback after DM flush firmware, and manage host reply
// Parameters  : IPI payload
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_process_ucode_cb2 (unsigned long *PayLoadPtr)
{
    GEN_RSP_FRAME_STRUCT RspFrame;
    PCB_UCODE_STRUCT *UcodePtr;

    UcodePtr = (void*)&UcodePcb.Info;

    RspFrame.Word[0] =UcodePtr->Profile;
    RspFrame.Word[1] = PayLoadPtr[0];

    iop_prepare_rsp(&RspFrame);
    iop_activate_mrlar();

    iop_ucode_init();

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_process_ucode_cb1
// Description : Callback after DM erase firmware, and send cmd to flush firmware
// Parameters  : IPI payload
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_process_ucode_cb1 (unsigned long *PayLoadPtr)
{
    GEN_RSP_FRAME_STRUCT RspFrame;
    PCB_UCODE_STRUCT *UcodePtr;
    IPCALL_STRUCT *IpCallPtr;

    UcodePtr = (void*)&UcodePcb.Info;

    if (PayLoadPtr[0] != SUCCESSFUL)
    {
        RspFrame.Word[0] =UcodePtr->Profile;
        RspFrame.Word[1] = PayLoadPtr[0];
        iop_ucode_init();

        return;
    }

    IpCallPtr = ipcall_add_slave_entry(0);
    IpCallPtr->Fn = media_write_in_place;
    IpCallPtr->Arg[0] = MAINFW_B_BLK_PBA;
    IpCallPtr->Arg[1] = MAX_FW_SIZE; //UcodePtr->FwSize;
    IpCallPtr->Arg[2] = UCODE_BASE_ADDR;
    IpCallPtr->Arg[3] = (unsigned long)iop_process_ucode_cb2;
    ipcall_slave_exec_calls(0);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_ucode_parse_hdr
// Description : Validate firmware
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long iop_ucode_parse_hdr (void)
{
    unsigned long Status;
    PCB_UCODE_STRUCT *UcodePtr;
    FW_VERSION_STRUCT *FwVerPtr;

    FW_HEADER_STRUCT *FwHdrPtr;

    FwHdrPtr = (void *)UCODE_BASE_ADDR;
    FwVerPtr = (void *)&FwHdrPtr->HeaderStart;

    Status = util_mem_compare((void *)FwVerPtr->HdrSignature,
                              (void *)FwVersion.HdrSignature,
                              sizeof(FwVersion.HdrSignature));

    if (Status != IDENTICAL)
    {
        return NOT_SUCCESSFUL;
    }

    /*if (FwVerPtr->Interface != FWPROD_EDCDE)
    {
        return NOT_SUCCESSFUL;
    }

    if (FwVerPtr->RomType != FWTYPE_MICROCODE)
    {
        return NOT_SUCCESSFUL;
    }*/

    UcodePtr = (void *)&UcodePcb.Info;
    UcodePtr->DevAddr = UCODE_BASE_ADDR;

    util_switch_endian_words((unsigned long)&FwHdrPtr->FwSize,
            (unsigned long)&UcodePtr->FwSize,sizeof(unsigned long));

    util_switch_endian_words((unsigned long)&FwHdrPtr->Checksum,
            (unsigned long)&UcodePtr->CheckSum,sizeof(unsigned long));

    return SUCCESSFUL;
}


//-----------------------------------------------------------------------------
// Function    : iop_ucode_get_csum(void)
// Description : Calculate additive checksum
// Parameters  : Base address of raw data, and length of raw data
// Returns     : Checksum
//-----------------------------------------------------------------------------
unsigned long iop_ucode_get_csum (void *AddPtr,
                                  unsigned long Length)
{
    unsigned char *LocalPtr;
    unsigned long CheckSum;
    unsigned long Idx;

    CheckSum = 0;
    LocalPtr = (unsigned char *) AddPtr;

    for (Idx = 0;
            Idx < Length;
            Idx++)
    {
        CheckSum += LocalPtr[Idx];
    }

    return CheckSum;
}


//-----------------------------------------------------------------------------
// Function    : iop_process_ucode
// Description : Process ucode raw data
// Parameters  : Pcb
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_process_ucode (PCB_STRUCT *PcbPtr)
{
    unsigned long Status = 0;
    unsigned long XferSize;
    unsigned long LocalCSum;
    GEN_RSP_FRAME_STRUCT RspFrame;
    PCB_UCODE_STRUCT *UcodePtr;
    IPCALL_STRUCT *IpCallPtr;

    XferSize = PcbPtr->Info.MrDma.Size * 4;
    UcodePtr = (void *)&UcodePcb.Info;

    if (UcodePtr->FwSize != 0)
    {
        UcodePtr->CurrSize += XferSize;


        if (UcodePtr->CurrSize >= UcodePtr->FwSize)
        {
            LocalCSum = iop_ucode_get_csum((unsigned long*)UCODE_BASE_ADDR,
                UcodePtr->FwSize);

            if (LocalCSum == UcodePtr->CheckSum)
            {
                // Validate FW
                UcodePtr->Profile = PcbPtr->Info.MrDma.Ctrl1;

                _sched_return_pcb(PcbPtr);

                IpCallPtr = ipcall_add_slave_entry(0);
                IpCallPtr->Fn = media_write_in_place;
                IpCallPtr->Arg[0] = MAINFW_A_BLK_PBA;
                IpCallPtr->Arg[1] = MAX_FW_SIZE; //UcodePtr->FwSize;
                IpCallPtr->Arg[2] = UCODE_BASE_ADDR;
                IpCallPtr->Arg[3] = (unsigned long)iop_process_ucode_cb1;
                ipcall_slave_exec_calls(0);

                return;
            }
            else
            {
                Status = 1;
            }
        }

        UcodePtr->DevAddr += XferSize;
    }
    else
    {
        if (iop_ucode_parse_hdr())
        {
            Status = 1;
        }
    }

    if (Status)
    {
        iop_ucode_init();
    }

    // Response to host
    RspFrame.Word[0] = PcbPtr->Info.MrDma.Ctrl1;
    RspFrame.Word[1] = Status;

    _sched_return_pcb(PcbPtr);
    iop_prepare_rsp(&RspFrame);

    return;
}


//-----------------------------------------------------------------------------
// Function    : iop_prcs_cmd_ucode
// Description : Process ucode command
// Parameters  : Frame Pointer
// Returns     : NONE
//-----------------------------------------------------------------------------
void iop_prcs_cmd_ucode (GEN_CMD_FRM_STRUCT *FramePtr)
{
    SCSI_CMD_FRAME_STRUCT *UcodeFrmPtr;
    PCB_STRUCT *PcbPtr;
    PCB_MRDMA_STRUCT *MrDmaPtr;

    UcodeFrmPtr = (void *)FramePtr;

    PcbPtr = iop_post_mrlar();
    PcbPtr->Fn = iop_process_ucode;
    PcbPtr->Info.MrDma.Ctrl1 = UcodeFrmPtr->Profile;

    MrDmaPtr = (void *)&PcbPtr->Info;
    MrDmaPtr->HighAddr = UcodeFrmPtr->HostAddrHi;
    MrDmaPtr->LowAddr = UcodeFrmPtr->HostAddrLo;
    MrDmaPtr->Size = UcodeFrmPtr->ByteSz / 4;
    MrDmaPtr->DevAddr = UcodePcb.Info.Ucode.DevAddr;

    return;
}


//=============================================================================
// $Log: Iop.c,v $
// Revision 1.10  2014/05/13 14:01:29  rcantong
// 1. BUGFIX: Silent data corruption suspect IO DMA race condition
// 1.1 Added 1000 cpu cycles delay after IO DMA completion
//
// Revision 1.9  2014/04/30 15:08:11  rcantong
// 1. DEV: Detection of max payload for Dell server
// 1.1 Added timing process in getting max payload size - ROrcullo
// 2. DEV: Support MRLAR hang detection
// 2.1 Added process for detecting MRLAR hang - JFaustino
// 3. BUGFIX: Improved PCIe signal and eliminate decoding error
// 3.1 Changed GUC PHY RX settings to 0x006E03A8 - Kranthi
// 4. BUGFIX: MRLAR hang caused by LC timeout in accessing MRSR
// 4.1 Disabled LC timeout setting - FSambilay
//
// Revision 1.8  2014/03/03 13:01:53  rcantong
// 1. DEV: PCIe server compatibility boot-up
// 1.1 Handle early PERST interrupt - ROrcullo
// 1.2 Remove Gen1 to Gen2 workaround
//
// Revision 1.7  2014/02/06 14:18:28  rcantong
// 1. DEV: PCIe compatibility init and reset
// 1.1 Added delay before we clear interrupts at pecore_crit_int_handler
// 1.2 Max out decode error value at pecore_crit_int_handler
// 1.3 Modified pecore_crit_int_handler sequence
// 1.4 Force to Gen2 when link up is in Gen1
// 1.5 Adjusted delay before each GUC register writes
// 1.6 Asserted PeCore Resets excluding Phy Reset
//
// Revision 1.6  2014/02/02 09:22:03  rcantong
// 1. DEV: PCIe compatibility restart
// 1.1 Modified the handling of PE reset
//
// Revision 1.5  2014/01/08 12:42:57  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.4  2013/12/05 13:06:35  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:50  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:44:24  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:17  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
