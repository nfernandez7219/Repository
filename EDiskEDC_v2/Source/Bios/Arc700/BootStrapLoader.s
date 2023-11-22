//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Arc700/BootStrapLoader.s,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/04/30 15:23:46 $
// $Id: BootStrapLoader.s,v 1.7 2014/04/30 15:23:46 rcantong Exp $
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
    .section .bootstrapldr, text


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
    .global _bootstraploader


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : _bootstraploader
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_bootstraploader:
    .long   0xDEAFFACE
    .long   0

    mov     r0, 0x0EE0C482
    st      r0, [0xFFCE3030]

    ; change siclk and slwperclk settings
    mov     r0, 0x6FD32A2A
    st      r0, [0xFFCE3054]

    ; unlock pll
    mov     r0, 0xDEADBEEF
    st      r0, [0xFFCE3020]

    ; pll0
    mov     r0, 0x404F0082
    st      r0, [0xFFCE3008]

    ; pll1
    mov     r0, 0x001F0041
    st      r0, [0xFFCE300C]

    ; pll2
    mov     r0, 0x005F0045
    st      r0, [0xFFCE3010]

    ; pll3
    mov     r0, 0x0013800A
    st      r0, [0xFFCE3014]

    ; lock pll
    mov     r0, 0x0
    st      r0, [0xFFCE3020]

    mov     r2, 0xA
    mov     r1, 0
1:
    nop
    sub     r2, r2, 0x1
    cmp     r2, r1
    bne     1b

    mov     r2, 0xA
    mov     r1, 0
1:
    nop
    sub     r2, r2, 0x1
    cmp     r2, r1
    bne     1b

    ; set dmclk, fbarbclk, fbclk to source from pll2
    mov     r0, 0x000B8306
    st      r0, [0xFFCE3068]

    ; enable clocks
    mov     r0, 0x7FFF9FC0
    st      r0, [0xFFCE3034]

    mov r0,  0x00FF0404
    st.di    r0, [0xFFCE3064]

    ; enable sram
    mov     r0, 0x00016000
    mov     r1, 0xFFCE3038
    st      r0, [r1]

    ; enable dp
    mov     r0, 0x0003E400
    st      r0, [r1]
    sync
    mov     r0, 0x0002A400
    st      r0, [r1]
    sync
    mov     r0, 0x0002A000
    st      r0, [r1]
    sync

    ; reset and enable dp channel, engine, and aes
    mov     r0, 0xFFFFFFFF
    st      r0, [0xFFCE303C]
    sync
    st      r0, [0xFFCE3040]
    sync
    mov     r0, 0
    st      r0, [0xFFCE3040]
    sync
    st      r0, [0xFFCE3044]
    sync

    ; sdcore reset
    mov     r0, 0x0
    st      r0, [0xFFCE3048]

    ; dmx work around
    mov     r0, 0x0EFF00FF
    st.di   r0, [0xFFCE307C]

    mov     r0, 0x1EFFFFFF
    st.di   r0, [0xFFCE307C]

    mov     r0, 0x0C00FF00
    st.di   r0, [0xFFCE3078]

    mov     r0, 0x1CFFFF00
    st.di   r0, [0xFFCE3078]

    mov     r0, 0x12003BF7
    st.di   r0, [0xFFCE3080]

    mov     r0, 0x36003BF7
    st.di   r0, [0xFFCE3080]

    mov     r0, 0x0EFDFBF7
    st.di   r0, [0xFFCE306C]

    mov     r0, 0x0EFDFBF7
    st.di   r0, [0xFFCE3070]

    mov     r0, 0x00000000
    st.di   r0, [0xFFCE3074]

    ; bypdis
    mov     r0, 0x10000
    st      r0, [0xFFCE301C]
    sync

    ; mask out mpcore interrupts during boot
    mov     r0, 0x03030303
    st.di   r0, [0xFFCE7024]

    ; host detection
    ld      r0, [0xFFCE3050]
    cmp     r0, 0x8FE
    beq     __skip_pecore_init

;******************START OF PCIE INITIALIZATION***************

    ; 1. PCIE RESET DE-ASSERTION
    ;*************************************
    ; @ POWER-ON, all PeCORE resets in CrCORE are asserted.
    ; 0xFFCE_3050[8:1]=0x1FE (default value)
    ; We can immediately de-assert 0xFFCE_3050[2]: PeCORE non-stoppable reset
    ;   - to access and modify PeCORE CSRs for GUC PHY control
    ;   - bit[6]: actually not used, but to be safe,
    ;             also included to be de-asserted
    ; ALL REMAINING RESET BITS ARE AUTO-DEASSERTED
    ; AS LONG AS 0xFFCE3050[1]=1(default)
    ld      r0, [0xFFCE3050]
    mov     r1, 0xFFFFFFBB
    and     r0, r0, r1
    st      r0, [0xFFCE3050]
    ;*************************************END OF 1. PCIE RESET DE-ASSERTION

    ; 2. PCIE PHY TUNING
    ;*************************************
    ; after bit[2] de-assertion, we can now access
    ; all registers of addr 0xFFF2_8xxx onwards
    ; these registers need to be configured as early as possible

    ; bit[16:14]: 000(single-end) to 100(differential) pll refclk
    ; bit[13:09]: 1C to 1E (charge pump current setting)
    ; bit[08:04]: 0C to 08 (ppath current setting)
    mov     r0, 0x008D1E82
    st.di   r0, [0xFFF2894C]

    ; optimal setting experiment c/o Haidee for Rx
    mov     r0, 0x006E03A8
    st.di   r0, [0xFFF28970]
    st.di   r0, [0xFFF28974]
    st.di   r0, [0xFFF28978]
    st.di   r0, [0xFFF2897c]
    st.di   r0, [0xFFF28980]
    st.di   r0, [0xFFF28984]
    st.di   r0, [0xFFF28988]
    st.di   r0, [0xFFF2898c]

    ; forcing Tx de-emphasis, -3.5dB
    mov     r0, 0x244064C4
    st.di   r0, [0xFFF28950]
    st.di   r0, [0xFFF28954]
    st.di   r0, [0xFFF28958]
    st.di   r0, [0xFFF2895c]
    st.di   r0, [0xFFF28960]
    st.di   r0, [0xFFF28964]
    st.di   r0, [0xFFF28968]
    st.di   r0, [0xFFF2896c]

    ; added fix for TX from GUC 10/03/2012
    ;****************************************START
    ld.di   r7, [0xFFF28944]            ; read 0xFFF2_8944

    mov     r0, 0x00038000
    or      r0, r0, r7
    st.di   r0, [0xFFF28944]            ; write ext_cfg_wdata

    ; 100 ns delay (1st)
    mov     r8, 0x0
    mov     r9, 0x28                    ; 28'h = 40'd x 2.5ns = 100ns
1:
    nop                                 ; 1 nop == 2.5ns
    add     r8 , r8, 0x1                ; increment count
    cmp     r8 , r9                     ; compare count
    bne     1b

    ld.di   r10, [0xFFF2893C]           ; read GUCPMAB0
    mov     r11, 0x20
    mov     r12, 0x15                   ; shift left value == 21bits
    or      r10, r10, r11               ; assert bit[5]
    mov     r11, 0xFF
    and     r10, r10, r11               ; mask for bit[7:0]
    asl     r13, r10, r12
    or      r0, r0, r13
    st.di   r0, [0xFFF28944]

    ; 100 ns delay (2nd)
    mov     r8, 0x0                     ; initialize
    mov     r9, 0x28                    ; 28'h = 40'd x 2.5ns = 100ns
1:
    nop                                 ; 1 nop == 2.5ns
    add     r8, r8, 0x1                 ; increment count
    cmp     r8, r9                      ; compare count
    bne     1b

    mov     r3, 0x20000000              ; assert ext_cfg_wr
    or      r5, r0, r3
    st.di   r5, [0xFFF28944]

    ; 100 ns delay (3rd)
    mov     r8, 0x0                     ; initialize
    mov     r9, 0x28                    ; 28'h = 40'd x 2.5ns = 100ns
1:
    nop                                 ; 1 nop == 2.5ns
    add     r8, r8, 0x1                 ; increment count
    cmp     r8, r9                      ; compare count
    bne     1b

    ; de-assert
    mov     r0, 0x00038000
    st.di   r0, [0xFFF28944]
    ;*************************************END OF FIX
    ;*************************************END OF 2. PCIE PHY TUNING

    ; 3. WAIT FOR PHY READY
    ;*************************************
1:
    ld      r0, [0xFFF28000]
    and     r0, r0, 0x00000003
    sub.f   0, r0, 0x00000003
    bnz     1b

    ; WE CAN ADD TIMEOUT/RECOVERY/WORKAROUND HERE
    ; IF OUR PHY FAILED IN OUR RESET SEQUENCE
    ;*************************************END Of 3

    ; 4. START INITIALIZING PeCORE CONFIGURATION REGISTERS
    ;*************************************

    ; TRANSFER THE CONTROL OF PeCORE RESETS TO FW
    mov     r0, 0x00000200
    st.di   r0, [0xFFCE3050]
    ; ADD ENABLE AUTOMATIC HARDWARE RESET

    ; unmasking of PERST# and Hardware System Interrupt
    ld      r0, [0xFFF29000]
    mov     r1, 0xFFBBFFFF
    and     r0, r0, r1
    st      r0, [0xFFF29000]

    ; checking for stable pe clock
1:
    ld      r0, [0xFFF20000]
    sub.f   0, r0, 0x00070101
    bnz     1b

    ; set max decode error
    mov     r0, 0x00000FFF
    st      r0, [0xFFF20028]

    ; force to -3.5dB de-emphasis to be advertised to the Host
    mov     r0, 0x8042A240
    st      r0, [0xFFF20808]

    ; if you want to force select de-emphasis and compliance de-emphasis to -6dB
    ;ld.di   r0, [0xFFF21090]
    ;mov     r2, 0xFFFFEFBF
    ;and     r0, r0, r2
    ;st.di   r0, [0xFFF21090]

    ; enable extended synch
    ld.di   r0, [0xFFF21070]
    mov     r2, 0x00000080
    or      r0, r0, r2
    st.di   r0, [0xFFF21070]
    ;*************************************END OF 4
;******************END OF PCIE INITIALIZATION***************

    mov     r0, 0x00000003
    st      r0, [0xFFF20030]

    mov     r0, 0x00000006
    st      r0, [0xFFF20038]

    lr      r6, [0x416]

    mov     r0, 0x00000006
    st      r0, [0xFFFD0024]

    mov     r0, 0x000016FF
    st      r0, [0xFFFD0034]

    mov     r0, 0x0000166F
    st      r0, [0xFFFD0028]

    mov     r0, 0x00000000
    st      r0, [0xFFFD003C]

    mov     r0, 0x00000000
    st      r0, [0xFFFD0040]

    mov     r0, 0x00000000
    st      r0, [0xFFFD0054]

    mov     r0, 0x00000000
    st      r0, [0xFFFD0058]

    mov     r0, 0x000CC318
    st      r0, [0xFFFD0094]

    mov     r0, 0x03000400
    st      r0, [0xFFFD0098]

    mov     r0, 0x00000000
    st      r0, [0xFFFD0008]

    mov     r0, 0x00000000
    st      r0, [0xFFFD0004]

    mov     r0, 0x00000000
    st      r0, [0xFFFD000C]

    mov     r0, 0x00001002
    st      r0, [0xFFFD0010]

    mov     r0, 0x00000801
    st      r0, [0xFFFD0018]

    mov     r0, 0x00000FF1
    st      r0, [0xFFFD001C]

    mov     r0, 0x00140C03
    st      r0, [0xFFFD006C]

    mov     r0, 0x140F0A05
    st      r0, [0xFFFD0070]

    mov     r0, 0x00013210
    st      r0, [0xFFFD0074]

    mov     r0, 0x10000000
    st      r0, [0xFFFD008c]

    mov     r0, 0x001F8202
    st      r0, [0xFFCE304C]

    mov     r0, 0x7FFF9FC0
    st      r0, [0xFFCE3034]

    mov     r0, 0x200
    st      r0, [0xFFFD0008]

    mov     r0, 0xFF3
    st      r0, [0xFFFD001C]

    mov     r0, 0x1660
    st      r0, [0xFFFD0028]

    mov     r0, 0x110
    st      r0, [0xFFFD002C]

    mov     r0, 0x1A0F
    st      r0, [0xFFFD0034]

    mov     r0, 0x0
    st      r0, [0xFFFD0038]

    mov     r0, 0x60
    st      r0, [0xFFFD005C]

    mov     r0, 0x100
    st      r0, [0xFFFD0064]

    mov     r0, 0x0
    st      r0, [0xFFFD0094]

    mov     r0, 0x00004800
    st      r0, [0xFFF21068]

    lr      r7, [0x416]

    ; mass storage controller
    mov     r0, 0x01800001
    st      r0, [0xFFF21008]

    ; bar0
    mov     r0, 0xFFFFC000
    st      r0, [0xFFF21010]

    mov     r0, 0xFFFFC001
    st      r0, [0xFFF210B0]

    mov     r0, 0x180000
    st      r0, [0xFFF210A0]

    ; bar1
    mov     r0, 0xFFFFC000
    st      r0, [0xFFF21014]

    mov     r0, 0xFFFFC001
    st      r0, [0xFFF210B4]

    mov     r0, 0xFFF20000
    st      r0, [0xFFF210A4]

    ; claim IoSpace bar2
    mov     r0, 0xFFFFFF01
    st      r0, [0xFFF21018]

    mov     r0, 0xFFFFFF01
    st      r0, [0xFFF210B8]

    mov     r0, 0xFFF20100
    st      r0, [0xFFF210A8]

    ; claim IoSpace bar3
    mov     r0, 0xFFFFFF01
    st      r0, [0xFFF2101C]

    mov     r0, 0xFFFFFF01
    st      r0, [0xFFF210BC]

    mov     r0, 0xFFF20400
    st      r0, [0xFFF210AC]

    mov     r0, 0x100407
    st      r0, [0xFFF21004]

    ; set expansion ROM
    mov     r0, 0x1
    st      r0, [0xFFF21030]

    mov     r0, 0xFFFF0000
    st      r0, [0xFFF21254]

    mov     r0, 0x10000
    st      r0, [0xFFF21250]

    ; disable un-used BAR
    mov      r0, 0x0
    st       r0, [0xFFF21020]

    mov      r0, 0x0
    st       r0, [0xFFF21024]

    mov      r0, 0x0
    st       r0, [0xFFF21210]

    mov      r0, 0x0
    st       r0, [0xFFF21214]

    ; added for ASPM initial value
    ld.di    r0, [0xFFF2106C]
    mov      r1, 0xFFFFF3FF
    and      r0, r0, r1
    st.di    r0, [0xFFF2106C]

    ; added for NextCap Offset
    ld.di    r0, [0xFFF21100]
    mov      r1, 0x000FFFFF
    and      r0, r0, r1
    st.di    r0, [0xFFF21100]

    ; added for NextCap Offset
    ld.di    r0, [0xFFF21138]
    mov      r1, 0x000FFFFF
    and      r0, r0, r1
    st.di    r0, [0xFFF21138]

    mov      r0, 0x1F02
    st       r0, [0xFFF20004]

    ; unmasking of PERST# and Hardware System Interrupt
    ld.di    r0, [0xFFF29000]
    mov      r1, 0x0023000F
    or       r0, r0, r1
    st.di    r0, [0xFFF29000]

__skip_pecore_init:

__sdraminit:

    mov      r0, 0x00000018
    st       r0, [0xFFCD1004]

    mov      r0, 0x00000098
    st       r0, [0xFFCD1008]

    mov      r0, 0x00001B91
    st       r0, [0xFFCD1018]

    mov      r0, 0x00000000
    st       r0, [0xFFCD102C]

    mov      r0, 0x00000120
    st       r0, [0xFFCD1034]

    mov      r0, 0x0D66013C
    st       r0, [0xFFCD1038]

    mov      r0, 0x00000044
    st       r0, [0xFFCD103C]

    mov      r0, 0x0000002A
    st       r0, [0xFFCD1058]

    mov      r0, 0x0000FE9E
    st       r0, [0xFFCD101C]

    mov      r0, 0x00000002
    st       r0, [0xFFCD1020]

    mov      r0, 0x00810001             ; 1GB
    st       r0, [0xFFCD1040]

    mov      r0, 0x01800100
    st       r0, [0xFFCD1044]

    mov      r0, 0x000C095A
    st       r0, [0xFFCD1028]

    mov      r0, 0x000000A0
    st       r0, [0xFFCD1030]

    mov      r0, 0x0710038E
    st       r0, [0xFFCD1048]

    mov      r0, 0x0A800840
    st       r0, [0xFFCD104C]

    mov      r0, 0x00000282
    st       r0, [0xFFCD1050]

    mov      r0, 0x00000083
    st       r0, [0xFFCD1054]

    mov      r0, 0x0A000840
    st       r0, [0xFFCD104C]

    mov      r0, 0x00000202
    st       r0, [0xFFCD1050]

    mov      r0, 0x00000003
    st       r0, [0xFFCD1054]

1:
    ld       r0, [0xFFCD1024]
    and      r0, r0, 0x7F0008
    brne     r0, 0x7F0008, 1b

    mov      r0, 1500
1:
    cmp_s    r0, 0
    sub_s    r0, r0, 1
    bnz_s    1b

    mov      r0, 0xFFCD1024

    mov      r0, 0x0000FE9F
    st       r0, [0xFFCD101C]

_xfer_bootcodeldr:

    ; setup memcpy() parameters
    mov      r0, (_bootcodeentry - _bootstraploader) - 8    ; src address
    mov      r1, 0x60000000                                 ; dst address
    mov      r2, _ebootstrapldr - _bootcodeentry            ; byte count
    xor      r5, r5, r5

    ; do memcpy()
1:
    sub.f    0, r2, r5
    bz       2f
    ld.di    r3, [r0, r5]
    st.di    r3, [r1]
    add      r5, r5, 4
    add      r1, r1, 4
    b        1b

2:
    j        0x60000000
    brk


end:
//=============================================================================
// $Log: BootStrapLoader.s,v $
// Revision 1.7  2014/04/30 15:23:46  rcantong
// 1. BUGFIX: Improved PCIe signal and eliminate decoding error
// 1.1 Changed GUC PHY RX settings to 0x006E03A8 - Kranthi
// 2. DEV: Unmask hardware system interrupt
// 2.1 Changed CINT2 to 0xFFBBFFFF - FSambilay
// 3. DEV: Support board powerup in backplane
// 3.1 Added detection if board is in backplane - RPayumo
//
// Revision 1.6  2014/03/03 13:00:41  rcantong
// 1. DEV: PCIe server compatibility boot-up
// 1.1 Early unmasking of PERST interrupt - AMarohomsalic
//
// Revision 1.5  2014/02/06 14:12:35  rcantong
// 1. DEV: PCIe compatibility boot-up
// 1.1 Modified Phy Sequence
//
// Revision 1.4  2014/02/02 09:16:08  rcantong
// 1. DEV: PCIe compatibility boot-up
// 1.1 Enable expansion ROM
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
