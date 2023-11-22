//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Public/Util.h,v $
// $Revision: 1.7 $
// $Author: rcantong $
// $Date: 2014/02/06 14:30:53 $
// $Id: Util.h,v 1.7 2014/02/06 14:30:53 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__UTIL_H__)
#define __UTIL_H__

#if defined(DEBUG)
_Inline void util_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define INIT_PATTERN_LO_VALUE           0x00000000
#define INIT_PATTERN_HI_VALUE           0xFFFFFFFF
#define IDENTICAL                       0


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

// Generic util dll entry
typedef struct UtilDllEntryStruct
{
    struct UtilDllEntryStruct *NextPtr;
    struct UtilDllEntryStruct *PrevPtr;
} UTIL_DLL_ENTRY_STRUCT;

// Generic util dll control
typedef struct UtilDllStruct
{
    UTIL_DLL_ENTRY_STRUCT *NextPtr;
    UTIL_DLL_ENTRY_STRUCT *PrevPtr;
} UTIL_DLL_STRUCT;

// Generic util sll entry
typedef struct UtilSllEntryStruct
{
    struct UtilSllEntryStruct *NextPtr;
} UTIL_SLL_ENTRY_STRUCT;

// Generic util sll control
typedef struct UtilSllStruct
{
    UTIL_SLL_ENTRY_STRUCT *HeadPtr;
    UTIL_SLL_ENTRY_STRUCT *TailPtr;
} UTIL_SLL_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

// Calculate bits from start to size
#define UTIL_CALC_BITS(Start, Size) (((1 << (Size)) - 1) << (Start))

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void util_delay (unsigned long Num);

void util_init_pattern (void *TgtPtr,
                        unsigned long ByteLength,
                        unsigned long InitPattern);

BIT_STAT util_mem_compare (void *Mem1Ptr,
                           void *Mem2Ptr,
                           unsigned long MemSz);

void util_mem_copy (void *MemDstPtr,
                    void *MemSrcPtr,
                    unsigned long MemSz);

void util_byte_copy (unsigned char *SrcPtr,
                     unsigned char *TgtPtr,
                     unsigned long ByteLen);

void util_swap (unsigned long DataAddr,
                unsigned long Len);

void util_swap8 (unsigned long Addr);

unsigned long util_ffs (unsigned long Bitmap);

void util_dll_init (UTIL_DLL_STRUCT *DllPtr);

void util_sll_init (UTIL_SLL_STRUCT *SllPtr);

unsigned long util_switch_endian_word (unsigned long Pattern);

void util_switch_endian_words (unsigned long SrcAddr,
                               unsigned long TgtAddr,
                               unsigned long ByteLength);


//-----------------------------------------------------------------------------
// Function    : util_clz
// Description : Count Leading Zeroes in a 32 bit long, which has the
//               effect of giving the index of the first set bit.
//
//               User must make sure to check if the Word is already fully
//               zeroed since it's pointless to call clz on a zero word.
// Parameters  : Bitmap
// Returns     : BitOffset
//-----------------------------------------------------------------------------
#define util_clz(Word, ZeroCount) \
do                                \
{                                 \
    (ZeroCount) = 0;              \
    ASSERT((Word) != 0);          \
                                  \
    if (((Word) & 0xFFFF) == 0)   \
    {                             \
        (Word) >>= 16;            \
        (ZeroCount) += 16;        \
    }                             \
                                  \
    if (((Word) & 0xFF) == 0)     \
    {                             \
        (Word) >>= 8;             \
        (ZeroCount) += 8;         \
    }                             \
                                  \
    if (((Word) & 0xF) == 0)      \
    {                             \
        (Word) >>= 4;             \
        (ZeroCount) += 4;         \
    }                             \
                                  \
    if (((Word) & 3) == 0)        \
    {                             \
        (Word) >>= 2;             \
        (ZeroCount) += 2;         \
    }                             \
                                  \
    if (((Word) & 1) == 0)        \
    {                             \
        (ZeroCount)++;            \
    }                             \
} while (0)


//-----------------------------------------------------------------------------
// Function    : util_count_bits
// Description : Count number of set bits
// Parameters  : Bmp
// Returns     : Count
//-----------------------------------------------------------------------------
#define util_count_bits(Bmp, Count)                         \
do                                                          \
{                                                           \
    Bmp = Bmp - ((Bmp >> 1) & 0x55555555);                  \
    Bmp = (((Bmp >> 2) & 0x33333333) + (Bmp & 0x33333333)); \
    Bmp = (((Bmp >> 4) + Bmp) & 0x0f0f0f0f);                \
    Bmp = Bmp + (Bmp >> 8);                                 \
    Bmp = Bmp + (Bmp >> 16);                                \
    Count += (Bmp & 0x0000003f);                            \
} while (0)


//-----------------------------------------------------------------------------
// Function    : util_dll_insert_at_head
// Description : Insert one util dll entry to the head of the specified list
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void util_dll_insert_at_head (UTIL_DLL_ENTRY_STRUCT *EntryPtr,
                                      UTIL_DLL_STRUCT *DllPtr)
{
    UTIL_DLL_ENTRY_STRUCT *NxtPtr;

    NxtPtr = DllPtr->NextPtr;
    EntryPtr->PrevPtr = (void *)DllPtr;
    EntryPtr->NextPtr = NxtPtr;
    DllPtr->NextPtr = EntryPtr;
    NxtPtr->PrevPtr = EntryPtr;

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_dll_insert_at_tail
// Description : Insert one util dll entry to the tail of the specified list
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void util_dll_insert_at_tail (UTIL_DLL_ENTRY_STRUCT *EntryPtr,
                                      UTIL_DLL_STRUCT *DllPtr)
{
    UTIL_DLL_ENTRY_STRUCT *PrvPtr;

    PrvPtr = DllPtr->PrevPtr;
    EntryPtr->NextPtr = (void *)DllPtr;
    EntryPtr->PrevPtr = PrvPtr;
    DllPtr->PrevPtr = EntryPtr;
    PrvPtr->NextPtr = EntryPtr;

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_dll_remove_from_middle
// Description : Remove one util dll entry from the list
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void util_dll_remove_from_middle (UTIL_DLL_ENTRY_STRUCT *EntryPtr)
{
    UTIL_DLL_ENTRY_STRUCT *NxtPtr;
    UTIL_DLL_ENTRY_STRUCT *PrvPtr;

    NxtPtr = EntryPtr->NextPtr;
    PrvPtr = EntryPtr->PrevPtr;
    PrvPtr->NextPtr = NxtPtr;
    NxtPtr->PrevPtr = PrvPtr;

    #if defined(DEBUG)
    EntryPtr->NextPtr = BIT_NULL_PTR;
    EntryPtr->PrevPtr = BIT_NULL_PTR;
    #endif

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_dll_get_head_entry
// Description : Get the head entry of the queue
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void *util_dll_get_head_entry (UTIL_DLL_STRUCT *DllPtr)
{
    UTIL_DLL_ENTRY_STRUCT *EntryPtr;
    UTIL_DLL_ENTRY_STRUCT *NxtPtr;

    EntryPtr = DllPtr->NextPtr;

    if (EntryPtr == (void *)DllPtr)
    {
        return BIT_NULL_PTR;
    }

    NxtPtr = EntryPtr->NextPtr;
    DllPtr->NextPtr = NxtPtr;
    NxtPtr->PrevPtr = (void *)DllPtr;

    return EntryPtr;
}


//-----------------------------------------------------------------------------
// Function    : util_dll_peek_head_entry
// Description : Peek the head entry of the queue
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void *util_dll_peek_head_entry (UTIL_DLL_STRUCT *DllPtr)
{
    UTIL_DLL_ENTRY_STRUCT *EntryPtr;

    EntryPtr = DllPtr->NextPtr;

    if (EntryPtr == (void *)DllPtr)
    {
        return BIT_NULL_PTR;
    }

    return EntryPtr;
}


//-----------------------------------------------------------------------------
// Function    : util_dll_peek_next
// Description : Return the next dll entry if possible
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void *util_dll_peek_next (UTIL_DLL_ENTRY_STRUCT *EntryPtr,
                                  UTIL_DLL_STRUCT *DllPtr)
{
    EntryPtr = EntryPtr->NextPtr;

    if (EntryPtr == (void *)DllPtr)
    {
        return BIT_NULL_PTR;
    }

    return EntryPtr;
}


//-----------------------------------------------------------------------------
// Function    : util_dll_move_entry
// Description : Get head entry of src then put to tail of dst
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void *util_dll_move_entry (UTIL_DLL_STRUCT *SrcDllPtr,
                                   UTIL_DLL_STRUCT *DstDllPtr)
{
    UTIL_DLL_ENTRY_STRUCT *EntryPtr;
    UTIL_DLL_ENTRY_STRUCT *NxtPtr;
    UTIL_DLL_ENTRY_STRUCT *PrvPtr;

    // Get head entry
    EntryPtr = SrcDllPtr->NextPtr;

    if (EntryPtr == (void *)SrcDllPtr)
    {
        return BIT_NULL_PTR;
    }

    NxtPtr = EntryPtr->NextPtr;
    SrcDllPtr->NextPtr = NxtPtr;
    NxtPtr->PrevPtr = (void *)SrcDllPtr;

    // Insert at tail
    PrvPtr = DstDllPtr->PrevPtr;
    EntryPtr->NextPtr = (void *)DstDllPtr;
    EntryPtr->PrevPtr = PrvPtr;
    DstDllPtr->PrevPtr = EntryPtr;
    PrvPtr->NextPtr = EntryPtr;

    return EntryPtr;
}


//-----------------------------------------------------------------------------
// Function    : util_dll_move_to_tail
// Description : Remove entry from middle then put to tail of specified list
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void util_dll_move_to_tail (UTIL_DLL_ENTRY_STRUCT *EntryPtr,
                                    UTIL_DLL_STRUCT *DllPtr)
{
    UTIL_DLL_ENTRY_STRUCT *NxtPtr;
    UTIL_DLL_ENTRY_STRUCT *PrvPtr;

    // Remove from middle
    NxtPtr = EntryPtr->NextPtr;
    PrvPtr = EntryPtr->PrevPtr;
    PrvPtr->NextPtr = NxtPtr;
    NxtPtr->PrevPtr = PrvPtr;

    // Insert at tail
    PrvPtr = DllPtr->PrevPtr;
    EntryPtr->NextPtr = (void *)DllPtr;
    EntryPtr->PrevPtr = PrvPtr;
    DllPtr->PrevPtr = EntryPtr;
    PrvPtr->NextPtr = EntryPtr;

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_dll_move_all_entries
// Description : Move all entries of src then put to dst
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void util_dll_move_all_entries (UTIL_DLL_STRUCT *SrcDllPtr,
                                        UTIL_DLL_STRUCT *DstDllPtr)
{
    // Check if src is empty
    if (SrcDllPtr->NextPtr == (void *)SrcDllPtr)
    {
        return;
    }

    // Remove entries from src
    SrcDllPtr->NextPtr->PrevPtr = DstDllPtr->PrevPtr;
    SrcDllPtr->PrevPtr->NextPtr = (void *)DstDllPtr;

    // Insert to tail of dst
    DstDllPtr->PrevPtr->NextPtr = SrcDllPtr->NextPtr;
    DstDllPtr->PrevPtr = SrcDllPtr->PrevPtr;

    // Emptied the src
    SrcDllPtr->NextPtr = (void *)SrcDllPtr;
    SrcDllPtr->PrevPtr = (void *)SrcDllPtr;

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_sll_insert_at_head
// Description : Insert one util sll entry to the head of the specified list
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void util_sll_insert_at_head (UTIL_SLL_ENTRY_STRUCT *EntryPtr,
                                      UTIL_SLL_STRUCT *SllPtr)
{
    if (SllPtr->HeadPtr != BIT_NULL_PTR)
    {
        EntryPtr->NextPtr = SllPtr->HeadPtr;
        SllPtr->HeadPtr = EntryPtr;
    }
    else
    {
        EntryPtr->NextPtr = BIT_NULL_PTR;
        SllPtr->HeadPtr = EntryPtr;
        SllPtr->TailPtr = EntryPtr;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_sll_insert_at_tail
// Description : Insert one util sll entry to the tail of the specified list
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void util_sll_insert_at_tail (UTIL_SLL_ENTRY_STRUCT *EntryPtr,
                                      UTIL_SLL_STRUCT *SllPtr)
{
    EntryPtr->NextPtr = BIT_NULL_PTR;

    if (SllPtr->HeadPtr != BIT_NULL_PTR)
    {
        SllPtr->TailPtr->NextPtr = EntryPtr;
        SllPtr->TailPtr = EntryPtr;
    }
    else
    {
        SllPtr->HeadPtr = EntryPtr;
        SllPtr->TailPtr = EntryPtr;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_sll_get_head_entry
// Description : Get the head entry of the queue
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void *util_sll_get_head_entry (UTIL_SLL_STRUCT *SllPtr)
{
    UTIL_SLL_ENTRY_STRUCT *EntryPtr;

    EntryPtr = SllPtr->HeadPtr;

    if (EntryPtr != BIT_NULL_PTR)
    {
        SllPtr->HeadPtr = EntryPtr->NextPtr;
    }

    return EntryPtr;
}


//-----------------------------------------------------------------------------
// Function    : util_sll_move_entry
// Description : Get head entry of src then put to tail of dst
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
_Inline void *util_sll_move_entry (UTIL_SLL_STRUCT *SrcSllPtr,
                                   UTIL_SLL_STRUCT *DstSllPtr)
{
    UTIL_SLL_ENTRY_STRUCT *EntryPtr;

    // Get head entry
    EntryPtr = SrcSllPtr->HeadPtr;

    if (EntryPtr != BIT_NULL_PTR)
    {
        SrcSllPtr->HeadPtr = EntryPtr->NextPtr;

        // Insert at tail
        EntryPtr->NextPtr = BIT_NULL_PTR;

        if (DstSllPtr->HeadPtr != BIT_NULL_PTR)
        {
            DstSllPtr->TailPtr->NextPtr = EntryPtr;
            DstSllPtr->TailPtr = EntryPtr;
        }
        else
        {
            DstSllPtr->HeadPtr = EntryPtr;
            DstSllPtr->TailPtr = EntryPtr;
        }
    }

    return EntryPtr;
}


#endif
//=============================================================================
// $Log: Util.h,v $
// Revision 1.7  2014/02/06 14:30:53  rcantong
// 1. BUGFIX: Fix DLL move all entries
// 1.1 Update util_dll_move_all_entries code
//
// Revision 1.6  2014/02/02 09:07:07  rcantong
// 1. DEV: Support mode select and bit specific config commands
// 1.1 Added handling of mode select and bit specific config commands
//
// Revision 1.5  2014/01/08 12:42:57  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.4  2013/12/05 13:06:33  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:49  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:06  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:00  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
