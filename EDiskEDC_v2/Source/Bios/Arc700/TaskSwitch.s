//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Bios/Arc700/TaskSwitch.s,v $
// $Revision: 1.4 $
// $Author: rcantong $
// $Date: 2014/04/30 15:24:56 $
// $Id: TaskSwitch.s,v 1.4 2014/04/30 15:24:56 rcantong Exp $
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
    .section .text_perf, text


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
    .global _sched_prcs_pcb
    .global _sched_get_pcb
    .global _sched_return_pcb


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : _sched_prcs_pcb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_sched_prcs_pcb:

    ; Get head pcb from run queue
    mov_s   r2, RunQueue
    ld_s    r0, [r2]                    ; EntryPtr = RunQueue->HeadPtr

    ; Check null pcb
    brz_s   r0, 0, 1f                   ; if (EntryPtr != NULL)

    ; Remove pcb from list
    ld_s    r1, [r0]                    ; SllPtr->HeadPtr = EntryPtr->NextPtr
    st_s    r1, [r2]

    ; Execute pcb fn
    ld_s    r1, [r0, 4]                 ; PcbPtr->Fn(PcbPtr)
    jl_s    [r1]

1:  ; Process intrpt
    flag    6
    flag    0

    b_s     _sched_prcs_pcb


//-----------------------------------------------------------------------------
// Function    : _sched_get_pcb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_sched_get_pcb:

    ; Decrement pcb count
    mov_s   r2, PcbPool
    ld_s    r1, [r2, 16]                ; PcbPool.PcbCnt--
    sub_s   r1, r1, 1
    st_s    r1, [r2, 16]

    ; Get head pcb from fast pool
    ld_s    r0, [r2, 8]                 ; EntryPtr = PcbPool.PcbList[1].HeadPtr

    ; Check null pcb
    brz_s   r0, 0, 1f                   ; if (EntryPtr != NULL)

    ; Remove pcb from fast pool
    ld_s    r1, [r0]                    ; SllPtr->HeadPtr = EntryPtr->NextPtr
    st_s    r1, [r2, 8]
    j_s     [blink]
                                        ; else
1:  ; Get head pcb from slow pool
    ld_s    r0, [r2, 0]                 ; EntryPtr = PcbPool.PcbList[0].HeadPtr

    ; Check null pcb
    brz_s   r0, 0, 2f                   ; if (EntryPtr != NULL)

    ; Remove pcb from slow pool
    ld_s    r1, [r0]                    ; SllPtr->HeadPtr = EntryPtr->NextPtr
    st_s    r1, [r2, 0]
    j_s     [blink]
                                        ; else
2:  ; Empty pool is unexpected
    brk_s                               ; err_gross
    nop_s


//-----------------------------------------------------------------------------
// Function    : _sched_return_pcb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_sched_return_pcb:

    ; Increment pcb count
    mov_s   r2, PcbPool
    ld_s    r1, [r2, 16]                ; PcbPool.PcbCnt++
    add_s   r1, r1, 1
    st_s    r1, [r2, 16]

    ; Set pcb next ptr to null
    mov_s   r1, 0
    st_s    r1, [r0]                    ; EntryPtr->NextPtr = NULL

    ; Detect if fast or slow pcb
    lsr     r1, r0, 31                  ; Idx = EntryAddr >> 31
    add3_s  r2, r2, r1                  ; SllPtr = &PcbPool.PcbList[Idx]

    ; Check if pool is empty
    ld_s    r1, [r2]                    ; HeadPtr = SllPtr->HeadPtr
    brz_s   r1, 0, 1f                   ; if (HeadPtr != NULL)

    ; Add to tail
    ld_s    r1, [r2, 4]                 ; TailPtr = SllPtr->TailPtr
    st_s    r0, [r1]                    ; TailPtr->NextPtr = EntryPtr
    st_s    r0, [r2, 4]                 ; SllPtr->TailPtr = EntryPtr
    j_s     [blink]

1:  ; Add as first entry                ; else
    st_s    r0, [r2]                    ; SllPtr->HeadPtr = EntryPtr
    st_s    r0, [r2, 4]                 ; SllPtr->TailPtr = EntryPtr
    j_s     [blink]


end:
//=============================================================================
// $Log: TaskSwitch.s,v $
// Revision 1.4  2014/04/30 15:24:56  rcantong
// 1. BUGFIX: Insufficient PCB
// 1.1 Added PCB counter and monitoring of low PCB count - MFenol
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
