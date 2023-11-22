//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Bios/Sfprom.h,v $
// $Revision: 1.2 $
// $Author: rcantong $
// $Date: 2014/01/08 12:42:56 $
// $Id: Sfprom.h,v 1.2 2014/01/08 12:42:56 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__SFPROM_H__)
#define __SFPROM_H__

#if defined(DEBUG)
_Inline void sfprom_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define SI_CIR_PTR                      ((volatile unsigned long *)0xFFCE5000)
#define SI_ESR_PTR                      ((volatile unsigned long *)0xFFCE5004)
#define SI_CTL_PTR                      ((volatile unsigned long *)0xFFCE5008)
#define SI_TXR_PTR                      ((volatile unsigned long *)0xFFCE500C)
#define SI_RXR_PTR                      ((volatile unsigned long *)0xFFCE5010)
#define SI_ESE_PTR                      ((volatile unsigned long *)0xFFCE5014)
#define SI_EIM_PTR                      ((volatile unsigned long *)0xFFCE5018)
#define SI_EIT_PTR                      ((volatile unsigned long *)0xFFCE501C)

#define T32B_CTL_START                  0x322
#define T32B_CTL_END                    0x302
#define T8B_CTL_START                   0x022
#define T8B_CTL_END                     0x002

#define STAT_RX_REG_FULL                (1 << 3)
#define STAT_TX_REG_EMPTY               (1 << 4)
#define STAT_BUS_BUSY                   (1 << 5)

#define SFPROM_BURN_SUCCESS             0x1
#define SFPROM_BURN_FAIL                (!SFPROM_BURN_SUCCESS)

// ATMEL SFPROM Opcodes
#define AT_READ                         0x03
#define AT_BLOCK_ERASE_64K              0xD8
#define AT_WRITE                        0x02
#define AT_WRITE_EN                     0x06
#define AT_WRITE_DIS                    0x04
#define AT_READ_STAT_REG                0x05
#define AT_WRITE_STAT_REG_B1            0x01
#define AT_WRITE_STAT_REG_B2            0x31
#define AT_READ_MFG_DEV_ID              0x9F
#define AT_DEEP_POW_DWN                 0xB9
#define AT_RSM_FRM_PW_DWN               0xAB
#define AT_SW_UNPROTECT                 0x39
#define AT_CHIP_ERASE                   0xC7

#define ATMEL                           0x0002461F
#define AT_STAT_BSY                     (1 << 0)
#define AT_STAT_WEL                     (1 << 1)
#define AT_STAT_UNPROT                  (3 << 2)
#define AT_UNPROTECT                    0x0


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define ADDR_PER_BYTE(Addr)             (   ((Addr & 0xFF) << 24) \
                                          | ((Addr & 0x00FF00) << 8) \
                                          | ((Addr & 0xFF0000) >> 8))


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void sfprom_clear_spi (void);

unsigned long sfprom_read_8b (unsigned long Opcode);

void sfprom_write_8b (unsigned long Opcode);

void sfprom_write_enable (void);

void sfprom_unlock (void);

unsigned long sfprom_read_32b (unsigned long Addr);

void sfprom_write_32b (unsigned long Addr,
                       unsigned long Data);

void sfprom_chip_erase (void);

void sfprom_sector_erase (unsigned long Addr);

BIT_STAT sfprom_program_sfprom (unsigned long *Src,
                                unsigned long Size,
                                unsigned long StrapConfig);

void sfprom_init (void);


#endif
//=============================================================================
// $Log: Sfprom.h,v $
// Revision 1.2  2014/01/08 12:42:56  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.1  2013/07/03 19:34:01  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
