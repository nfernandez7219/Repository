//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Arc700/Vectors.s,v $
// $Revision: 1.4 $
// $Author: rcantong $
// $Date: 2014/02/02 09:12:19 $
// $Id: Vectors.s,v 1.4 2014/02/02 09:12:19 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#define __ASSEMBLY__


//-----------------------------------------------------------------------------
// Global Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Section Definitions
//-----------------------------------------------------------------------------
    .section .vector, text


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
    .global _vector


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------
.macro IRQ_ENTRY, _irq_handler
    bl      _irq_handler
    j.f     [ilink1]
.endm

.macro EXC_ENTRY
    brk
    nop
.endm


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : _vector
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_vector:        j _start                        ; (0x00) Vector Reset
_ev_mem_err:    EXC_ENTRY                       ; (0x08) Exception Memory Error
_ev_instr_err:  EXC_ENTRY                       ; (0x10) Exception Instr Error
_int_vec_irq3:  IRQ_ENTRY arc_timer0_int_hdl    ; (0x18) Interrupt 3 Timer 0
_int_vec_irq4:  IRQ_ENTRY arc_timer1_int_hdl    ; (0x20) Interrupt 4 Timer 1
_int_vec_irq5:  IRQ_ENTRY int_arc_isr           ; (0x28) Interrupt 5 UART
_int_vec_irq6:  IRQ_ENTRY int_arc_isr           ; (0x30) Interrupt 6 EMAC
_int_vec_irq7:  IRQ_ENTRY int_arc_isr           ; (0x38) Interrupt 7 XY Memory
_int_vec_irq8:  IRQ_ENTRY int_arc_isr           ; (0x40) Interrupt Vector 8
_int_vec_irq9:  IRQ_ENTRY arc_service_ipirq     ; (0x48) Interrupt Vector 9
_int_vec_irq10: IRQ_ENTRY int_arc_isr           ; (0x50) Interrupt Vector 10
_int_vec_irq11: IRQ_ENTRY int_arc_isr           ; (0x58) Interrupt Vector 11
_int_vec_irq12: IRQ_ENTRY int_arc_isr           ; (0x60) Interrupt Vector 12
_int_vec_irq13: IRQ_ENTRY int_arc_isr_pg        ; (0x68) Interrupt Vector 13
_int_vec_irq14: IRQ_ENTRY int_arc_isr           ; (0x70) Interrupt Vector 14
_int_vec_irq15: IRQ_ENTRY int_arc_isr           ; (0x78) Interrupt Vector 15
_int_vec_irq16: IRQ_ENTRY int_arc_isr           ; (0x80) Interrupt Vector 16
_int_vec_irq17: IRQ_ENTRY int_arc_isr           ; (0x88) Interrupt Vector 17
_int_vec_irq18: IRQ_ENTRY int_arc_isr_dm_err    ; (0x90) Interrupt Vector 18
_int_vec_irq19: IRQ_ENTRY int_arc_isr_dm_done   ; (0x98) Interrupt Vector 19
_int_vec_irq20: IRQ_ENTRY int_arc_isr_pe        ; (0xA0) Interrupt Vector 20
_int_vec_irq21: IRQ_ENTRY int_arc_isr_normal    ; (0xA8) Interrupt Vector 21
_int_vec_irq22: EXC_ENTRY                       ; (0xB0) Interrupt Vector 22
_int_vec_irq23: EXC_ENTRY                       ; (0xB8) Interrupt Vector 23
_int_vec_irq24: EXC_ENTRY                       ; (0xC0) Interrupt Vector 24
_int_vec_irq25: EXC_ENTRY                       ; (0xC8) Interrupt Vector 25
_int_vec_irq26: EXC_ENTRY                       ; (0xD0) Interrupt Vector 26
_int_vec_irq27: EXC_ENTRY                       ; (0xD8) Interrupt Vector 27
_int_vec_irq28: EXC_ENTRY                       ; (0xE0) Interrupt Vector 28
_int_vec_irq29: EXC_ENTRY                       ; (0xE8) Interrupt Vector 29
_int_vec_irq30: EXC_ENTRY                       ; (0xF0) Interrupt Vector 30
_int_vec_irq31: EXC_ENTRY                       ; (0xF8) Interrupt Vector 31
_ev_mach_check: EXC_ENTRY                       ; (0x100) Machine Check
_ev_i_tlb_miss: EXC_ENTRY                       ; (0x108) Instr Fetch TLB Miss
_ev_d_tlb_miss: EXC_ENTRY                       ; (0x110) Data TLB Miss
_ev_tlb_prot_v: EXC_ENTRY                       ; (0x118) TLB Prot Violation
_ev_priv_v:     EXC_ENTRY                       ; (0x120) Privilege Violation
_ev_trap:       EXC_ENTRY                       ; (0x128) Trap Instruction
_ev_ext_instr:  EXC_ENTRY                       ; (0x130) Extension Instruction


end:
//=============================================================================
// $Log: Vectors.s,v $
// Revision 1.4  2014/02/02 09:12:19  rcantong
// 1. DEV: Support LED as status indicator
// 1.1 Added control of LEDs to indicate the device status
//
// Revision 1.3  2013/12/05 13:06:35  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
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
