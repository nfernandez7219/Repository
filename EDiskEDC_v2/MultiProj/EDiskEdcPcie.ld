#==============================================================================
# All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
# The contents of this software may not be reprinted or reproduced in whole
# or part without the written consent of BiTMICRO Networks, Inc.
# Printed copies of this material are uncontrolled documents.
#
# Description: EDC Linker Directive file
#
# File:
# $Source: /repository/firmware/EDC/src/EDiskEDC_v2/MultiProj/EDiskEdcPcie.ld,v $
# $Revision: 1.5 $
# $Author: rcantong $
# $Date: 2014/04/30 15:20:57 $
# $Id: EDiskEdcPcie.ld,v 1.5 2014/04/30 15:20:57 rcantong Exp $
#
# Note: This file should only be modified by qualified personnels
#==============================================================================


MEMORY
{
    sdram_mem           : ORIGIN = 0x00000000, LENGTH = 1024M

    sram_mem            : ORIGIN = 0x60000000, LENGTH = 1M
    sram_mem_uc         : ORIGIN = 0xE0000000, LENGTH = 1M

    dccm_mem            : ORIGIN = 0xFC100000, LENGTH = 16K
    dccm_io_mem         : ORIGIN = 0xFC100000, LENGTH = 16K
    dccm_dm_mem         : ORIGIN = 0xFC100000, LENGTH = 16K
}

SECTIONS
{
    GROUP :
    {
        .vector                                 :
        .start          ALIGN(4)                :
        .fwversion      ALIGN(0x200)            :
        .bootstrapldr   ALIGN(0x100)
                        BLOCK(0x100)            :
        .text_perf      ALIGN(4)                :
        .text           ALIGN(4)                :
        .rodata         ALIGN(32)               :
        .data           ALIGN(32)               :
        .data1          ALIGN(32)               :
        .sdata          ALIGN(32)               :
        _firmware_size  = . ;
        .bss            ALIGN(32)               :
        .dm_global      ALIGN(32)               :
        .dm_bitmap      ALIGN(0x100)            :
        .dm_buffer      ALIGN(0x100)            :
        _unused_sdram   = . ;
    } > sdram_mem

    GROUP :
    {
        .pci_bar0                               :
        .dmx_desc       ALIGN(0x100)            :
        .ipcall         ALIGN(32)               :
        .usram          ALIGN(32)               :
        _eusram         = . ;
    } > sram_mem_uc

    GROUP BIND(_eusram - 0x80000000) :
    {
        .sram           ALIGN(32)               :
    } > sram_mem

    GROUP :
    {
        .stack          SIZE(0x800)             :
        .dccm           ALIGN(4)                :
        _edccm          = . ;
    } > dccm_mem

    GROUP BIND(_edccm)  LOAD(0x000000 + _edccm) :
    {
        .dccm_io        ALIGN(4)                :
    } > dccm_io_mem

    GROUP BIND(_edccm)  LOAD(0x100000 + _edccm) :
    {
        .dccm_dm        ALIGN(4)                :
    } > dccm_dm_mem

    _bootstrap_addr = ADDR(.bootstrapldr);
    _bootstrap_size = SIZEOF(.bootstrapldr);
}


#==============================================================================
# $Log: EDiskEdcPcie.ld,v $
# Revision 1.5  2014/04/30 15:20:57  rcantong
# 1. DEV: Location of _start will be same in every FW version
# 1.1 Transfer start section before text section
#
# Revision 1.4  2014/01/08 12:42:57  rcantong
# 1. DEV: Perform random write longevity
# 1.1 Codes for control and user data compacting
#
# Revision 1.3  2013/11/11 08:20:49  rcantong
# 1. DEV: Perform user data integrity
# 1.1 Codes to run user data integrity
#
# Revision 1.2  2013/08/08 16:48:28  rcantong
# 1. DEV: Perform iometer read and write
# 1.1 Codes to run iometer read and write
#
# Revision 1.1  2013/07/15 17:56:43  rcantong
# 1. DEV: Initial commit
# 1.1 Template file
#
#==============================================================================
