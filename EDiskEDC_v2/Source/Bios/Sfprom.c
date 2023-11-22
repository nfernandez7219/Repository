//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Sfprom.c,v $
// $Revision: 1.4 $
// $Author: rcantong $
// $Date: 2014/02/02 09:17:38 $
// $Id: Sfprom.c,v 1.4 2014/02/02 09:17:38 rcantong Exp $
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
#include "EdcClkPmu.h"
#include "Sfprom.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : sfprom_clear_spi
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sfprom_clear_spi (void)
{
    unsigned long Temp;

    while (*SI_ESR_PTR & STAT_BUS_BUSY);

    while (!(*SI_ESR_PTR & STAT_TX_REG_EMPTY))
    {
        *SI_CTL_PTR = T8B_CTL_START;
        while (!(*SI_ESR_PTR & STAT_TX_REG_EMPTY));
        *SI_CTL_PTR = T8B_CTL_END;
    }

    while (*SI_ESR_PTR & STAT_RX_REG_FULL)
    {
        Temp = *SI_RXR_PTR;
    }

    while (*SI_ESR_PTR & STAT_BUS_BUSY);

    return;
}


//-----------------------------------------------------------------------------
// Function    : sfprom_read_8b
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long sfprom_read_8b (unsigned long Opcode)
{
    unsigned long Data;

    sfprom_clear_spi();

    *SI_TXR_PTR = Opcode;

    *SI_ESE_PTR  = 0x00001FB8;
    *SI_CTL_PTR = T8B_CTL_START;

    while (!((*SI_ESR_PTR & STAT_TX_REG_EMPTY) == STAT_TX_REG_EMPTY));
    while (((*SI_ESR_PTR & STAT_RX_REG_FULL) != STAT_RX_REG_FULL));

    Data = *SI_RXR_PTR;

    sfprom_clear_spi();

    *SI_CTL_PTR = T8B_CTL_END;

    return (Data & 0xFF);
}


//-----------------------------------------------------------------------------
// Function    : sfprom_write_8b
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sfprom_write_8b (unsigned long Opcode)
{
    *SI_TXR_PTR = Opcode;

    *SI_ESE_PTR  = 0x00001FB8;
    *SI_CTL_PTR = T8B_CTL_START;
    while (!(*SI_ESR_PTR & STAT_TX_REG_EMPTY));
    *SI_CTL_PTR = T8B_CTL_END;

    return;
}


//-----------------------------------------------------------------------------
// Function    : sfprom_write_enable
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sfprom_write_enable (void)
{
    unsigned long Status;

    sfprom_clear_spi();

    do
    {
        sfprom_write_8b(AT_WRITE_EN);
        sfprom_clear_spi();
        Status = sfprom_read_8b(AT_READ_STAT_REG);

    } while (!(Status & AT_STAT_WEL));

    return;
}


//-----------------------------------------------------------------------------
// Function    : sfprom_unlock
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sfprom_unlock (void)
{
    sfprom_clear_spi();
    sfprom_read_8b(AT_UNPROTECT + AT_WRITE_STAT_REG_B1);

    while (sfprom_read_8b(AT_READ_STAT_REG) & AT_STAT_UNPROT);

    return;
}


//-----------------------------------------------------------------------------
// Function    : sfprom_read_32b
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long sfprom_read_32b (unsigned long Addr)
{
    unsigned long Data;

    *SI_TXR_PTR = Addr + AT_READ;
    *SI_CTL_PTR = T32B_CTL_START;

    while (!(*SI_ESR_PTR & STAT_TX_REG_EMPTY));
    while (!(*SI_ESR_PTR & STAT_RX_REG_FULL));

    Data = *SI_RXR_PTR;

    *SI_CTL_PTR = T32B_CTL_END;

    return Data;
}


//-----------------------------------------------------------------------------
// Function    : sfprom_write_32b
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sfprom_write_32b (unsigned long Addr,
                       unsigned long Data)
{
    sfprom_write_enable();

    *SI_TXR_PTR = Addr + AT_WRITE;
    *SI_CTL_PTR = T32B_CTL_START;
    while (!(*SI_ESR_PTR & STAT_TX_REG_EMPTY));

    *SI_TXR_PTR = Data;
    while (!(*SI_ESR_PTR & STAT_TX_REG_EMPTY));

    *SI_CTL_PTR = T32B_CTL_END;

    while (sfprom_read_8b(AT_READ_STAT_REG) & AT_STAT_BSY);

    return;
}


//-----------------------------------------------------------------------------
// Function    : sfprom_chip_erase
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sfprom_chip_erase (void)
{
    sfprom_write_enable();

    sfprom_write_8b(AT_CHIP_ERASE);

    while (sfprom_read_8b(AT_READ_STAT_REG) & AT_STAT_BSY);

    return;
}


//-----------------------------------------------------------------------------
// Function    : sfprom_sector_erase
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sfprom_sector_erase (unsigned long Addr)
{
    sfprom_write_enable();

    *SI_TXR_PTR = Addr + AT_BLOCK_ERASE_64K;
    *SI_CTL_PTR = T32B_CTL_START;
    *SI_CTL_PTR = T32B_CTL_END;

    while (sfprom_read_8b(AT_READ_STAT_REG) & AT_STAT_BSY);

    return;
}


//-----------------------------------------------------------------------------
// Function    : sfprom_program_sfprom
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT sfprom_program_sfprom (unsigned long *Src,
                                unsigned long Size,
                                unsigned long StrapConfig)
{
    unsigned long Addr;
    unsigned long SfpromVerify;

    sfprom_sector_erase(0);

    for (Addr = 0;
         Addr < Size;
         Addr += sizeof(long), Src++)
    {
        sfprom_write_32b(ADDR_PER_BYTE(Addr),
                         *Src);

        SfpromVerify = sfprom_read_32b(ADDR_PER_BYTE(Addr));

        if (SfpromVerify != *Src)
        {
            return SFPROM_BURN_FAIL;
        }
    }

    sfprom_write_32b(ADDR_PER_BYTE(0x3500),
                                   StrapConfig);

    sfprom_write_32b(ADDR_PER_BYTE(0x36E0),
                     0xFADEBEAD);

    return SFPROM_BURN_SUCCESS;
}


//-----------------------------------------------------------------------------
// Function    : sfprom_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void sfprom_init (void)
{
    if (*SI_CIR_PTR != 0x000C0101)
    {
        _brk();
    }

    *SI_ESR_PTR = 0x00001F00;
    *SI_CTL_PTR = 0x00000001;
    *SI_ESE_PTR = 0x00001FB8;
    *SI_EIM_PTR = 0x00001F00;
    *SI_EIT_PTR = 0x00000000;

    sfprom_write_enable();
    sfprom_unlock();

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: Sfprom.c,v $
// Revision 1.4  2014/02/02 09:17:38  rcantong
// 1. DEV: Support LED as status indicator
// 1.1 Added control of LEDs to indicate the device status
//
// Revision 1.3  2014/01/08 12:42:55  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.2  2013/08/08 16:44:22  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:09  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
