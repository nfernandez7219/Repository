//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Bios.c,v $
// $Revision: 1.9 $
// $Author: rcantong $
// $Date: 2014/05/19 05:21:06 $
// $Id: Bios.c,v 1.9 2014/05/19 05:21:06 rcantong Exp $
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
#include "Dmx.h"
#include "DmxCommon.h"
#include "DmxLite.h"
#include "DmxRecovery.h"
#include "EdcFwVersion.h"
#include "EdcLocalCsr.h"
#include "EdcClkPmu.h"
#include "Interrupt.h"
#include "Iop.h"
#include "Led.h"
#include "NvConfig.h"
#include "Sfprom.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "BiosI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------
#pragma BSS(".dccm")
BIOS_PARM_STRUCT BiosParm;
#pragma BSS()


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : bios_main
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bios_main (void)
{
    unsigned long Reg;
    unsigned long Idx;

    // Assign CoreId to DCCM
    Reg = *LC_AKTOUT_PTR;
    BiosParm.CpuId = (Reg >> 16) & 0xF;
    BiosParm.DmId = BiosParm.CpuId - 1;
    BiosParm.IdleTmrCnt = 0;

    // Set interrupt vector
    Reg = (unsigned long)_vector;
    _sr(Reg, 0x025);

    // Aux ienable
    Reg = 0xFFFFFFFF;
    _sr(Reg, 0x40C);

    if (BiosParm.CpuId == EDC_BOOT_CORE)
    {
        // CrCore initialization
        crcore_init();

        // LED initialization
        led_init();

        // Disable ic core
        Reg = *CR_PMU0_CPU_PTR;
        Reg = Reg & 0xF;
        *CR_PMU0_CPU_PTR = 0x0EE0C480 | Reg;

        //Jumper Initialization
        bios_jumper_init();

        // DMX initialization
        dmx_init();
        nv_lite_preinit();
        bios_download_thru_multi_firmware();
        dmx_init_enable_dmx_que();

        mr_recover_init();

        // Clear possible cause of interrupt pending
        Reg = REG_READ(0xFFF29000);
        REG_WRITE(0xFFF29000, Reg);

        Reg = REG_READ(0xFFCE7020);
        REG_WRITE(0xFFCE7020, Reg);

        Reg = REG_READ(0xFFCD100C);
        REG_WRITE(0xFFCD100C, Reg);

        // Enable ic core
        Reg = *CR_PMU0_CPU_PTR;
        Reg = Reg & 0xF;
        *CR_PMU0_CPU_PTR = 0x0EE0C400 | Reg;

        // Set priority level to highest
        for (Idx = 0;
             Idx < 8;
             Idx++)
        {
            IC_PLR0_PTR[Idx] = 0xFFFFFFFF;
        }

        // Set priority level of dm err and dm done at PLR4
        *IC_PLR4_PTR = (0 << 28) | (1 << 26);

        // Set priority level of pe normal interrupt at PLR5
        *IC_PLR5_PTR = 2 << 24;

        // Initialize timer
        _sr(0, 0x22);
        _sr(400000, 0x23);
    }

    dmx_recovery_init();

    return;
}


//-----------------------------------------------------------------------------
// Function    : bios_enable_cores
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bios_enable_cores (void)
{
    unsigned long Reg;

    // Enable slave cores
    Reg = *CR_PMU0_CPU_PTR;
    Reg = Reg & 0xFF;
    *CR_PMU0_CPU_PTR = 0x0000FC00 | Reg;

    return;
}


//-----------------------------------------------------------------------------
// Function    : bios_jumper_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bios_jumper_init (void)
{
    unsigned long Jumper;

    // Jumper rebuild
    Jumper = *JUMPER_CNTLR_PTR;
    Jumper = Jumper & JUMPER_MASK;

    if (Jumper == JUMPER_ERASE_ALL)
    {
        FwVersion.EraseBoardFlag = 2;
    }
    else if (Jumper == JUMPER_REBUILD)
    {
        FwVersion.EraseBoardFlag = ON;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : bios_flush_dc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bios_flush_dc (void)
{
    unsigned long Stat;

    _sr(1, 0x4B); //REG_DC_FLSH

    do
    {
        Stat = _lr(0x48); //REG_DC_CTRL

    } while ((Stat & 0x100) != 0);

    return;
}


//-----------------------------------------------------------------------------
// Function    : bios_flush_dc_lines
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bios_flush_dc_lines (unsigned long Addr,
                          unsigned long ByteSize)
{
    unsigned long EndAddr;

    ASSERT((Addr % DCACHE_LINE_SIZE) == 0);
    EndAddr = Addr + ByteSize;

    for (;
         Addr < EndAddr;
         Addr += DCACHE_LINE_SIZE)
    {
        bios_flush_dc_line((void *)Addr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : bios_invalidate_dc_lines
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bios_invalidate_dc_lines (unsigned long Addr,
                               unsigned long ByteSize)
{
    unsigned long EndAddr;

    EndAddr = Addr + ByteSize;

    for (;
         Addr < EndAddr;
         Addr += DCACHE_LINE_SIZE)
    {
        bios_invalidate_dc_line((void *)Addr);
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : crcore_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void crcore_init (void)
{
    // Clocking for SFPROM and PIO
    *CR_PMU4_PER0_PTR = 0x6FD32A2A;

    return;
}


//-----------------------------------------------------------------------------
// Function    : bios_download_thru_multi_firmware
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bios_download_thru_multi_firmware (void)
{
    sfprom_init();

    // When user wants to burn bootstrap and bootcode
    if (FwVersion.ProgramFirmwareFlag == ON)
    {
        FwVersion.ProgramFirmwareFlag = OFF;

        bios_program_bootloader();

        bios_program_firmware();

        // Save firmware done indicator
        *LED_CNTLR_PTR = LED_GREEN_MASK
                       | LED_YELLOW_MASK
                       | LED_ORANGE_MASK;

        _brk();
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : bios_program_bootloader
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bios_program_bootloader (void)
{
    BIT_STAT ProgramStat;
    unsigned long StrapConfig = 0x0003800A;

    if (FwVersion.PciGen1Flag == ON)
    {
        StrapConfig = 0x0001800A;
    }

    ProgramStat = sfprom_program_sfprom(_bootstrap_addr,
                                        (unsigned long) _bootstrap_size,
                                        StrapConfig);

    ASSERT(ProgramStat == SFPROM_BURN_SUCCESS);

    return;
}


//-----------------------------------------------------------------------------
// Function    : bios_program_firmware
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void bios_program_firmware (void)
{
    BIT_STAT Stat;
    const unsigned long CopyBufferAddrA = (unsigned long)_unused_sdram;
    const unsigned long CopyBufferAddrB = CopyBufferAddrA + MAX_FW_SIZE;
    const unsigned long FwSize = (unsigned long )_firmware_size;

    ASSERT(FwSize <= MAX_FW_SIZE);

    // Save firmware copy A
    dmx_lite_write(0,
                   MAINFW_A_BLK_PBA,
                   0,
                   MAX_FW_SIZE);

    // Save firmware copy B
    dmx_lite_write(0,
                   MAINFW_B_BLK_PBA,
                   0,
                   MAX_FW_SIZE);

    // Verify firmware copy A
    dmx_lite_read(0,
                  MAINFW_A_BLK_PBA,
                  CopyBufferAddrA,
                  MAX_FW_SIZE);

    Stat = util_mem_compare((unsigned long *)0,
                            (unsigned long *)CopyBufferAddrA,
                            FwSize);
    if (Stat != IDENTICAL)
    {
        // Possible defective firmware block
        _brk();
    }

    // Verify firmware copy B
    dmx_lite_read(0,
                  MAINFW_B_BLK_PBA,
                  CopyBufferAddrB,
                  MAX_FW_SIZE);

    Stat = util_mem_compare((unsigned long *)0,
                            (unsigned long *)CopyBufferAddrB,
                            FwSize);

    if (Stat != IDENTICAL)
    {
        // Possible defective firmware block
        _brk();
    }

    return;
}


//=============================================================================
// $Log: Bios.c,v $
// Revision 1.9  2014/05/19 05:21:06  rcantong
// 1. BUGFIX: Support jumper rebuild for production board
// 1.1 Scanning without erase if production board - MManzo
//
// Revision 1.8  2014/05/13 13:51:27  rcantong
// 1. DEV: Able to rebuild by using hardware jumper
// 1.1 Added detection of jumper config to set rebuild flag - MManzo
//
// Revision 1.7  2014/04/30 15:25:18  rcantong
// 1. DEV: Support MRLAR hang detection
// 1.1 Added mr_recover_init - JFaustino
// 2. DEV: LED indicator for firmware saving
// 2.1 Lit three LED indicating firmware save completed - RPayumo
//
// Revision 1.6  2014/03/03 12:52:33  rcantong
// 1. DEV: FID hang handler
// 1.1 Added call to dmx_recovery_init - JFaustino
//
// Revision 1.5  2014/02/02 09:17:38  rcantong
// 1. DEV: Support LED as status indicator
// 1.1 Added control of LEDs to indicate the device status
//
// Revision 1.4  2014/01/08 12:42:55  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:44:22  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:08  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
