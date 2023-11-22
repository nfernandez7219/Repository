//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/Util/Util.c,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/02/02 09:48:32 $
// $Id: Util.c,v 1.5 2014/02/02 09:48:32 rcantong Exp $
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
// Function    : util_delay
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void util_delay (unsigned long Num)
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


//-----------------------------------------------------------------------------
// Function    : util_init_pattern
// Description : Initialize block of memory with a given pattern of four bytes.
// Parameters  : *TgtPtr - pointer to the beginning of block to initialize.
//               ByteLength - length in bytes which must be divisible by 4.
//               InitPattern - Fill pattern
// Returns     : NONE
//-----------------------------------------------------------------------------
void util_init_pattern (void *TgtPtr,
                        unsigned long ByteLength,
                        unsigned long InitPattern)
{
    unsigned long *MemPtr;
    unsigned long MaxIdx;
    unsigned long Idx;

    ASSERT((ByteLength & 3) == 0);
    MemPtr = TgtPtr;
    MaxIdx = ByteLength / sizeof(long);

    for (Idx = 0;
         Idx < MaxIdx;
         Idx++)
    {
        *MemPtr = InitPattern;
        MemPtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_mem_copy
// Description : Compare two segments of memory for equality
// Parameters  : Mem1Ptr - Start address memory region 1
//               Mem2Ptr - Start address memory region 2
//               MemSz - Size of region to compare
// Returns     : NONE
//-----------------------------------------------------------------------------
BIT_STAT util_mem_compare (void *Mem1Ptr,
                           void *Mem2Ptr,
                           unsigned long MemSz)
{
    unsigned long *Src1Ptr;
    unsigned long *Src2Ptr;
    unsigned long MaxIdx;
    unsigned long Idx;

    ASSERT((MemSz & 3) == 0);
    Src1Ptr = Mem1Ptr;
    Src2Ptr = Mem2Ptr;
    MaxIdx = MemSz / sizeof(long);

    for (Idx = 0;
         Idx < MaxIdx;
         Idx++)
    {
        if (*Src1Ptr != *Src2Ptr)
        {
            return !IDENTICAL;
        }

        Src1Ptr++;
        Src2Ptr++;
    }

    return IDENTICAL;
}


//-----------------------------------------------------------------------------
// Function    : util_mem_copy
// Description : Copy a segment of memory from one location to another location
// Parameters  : MemDstPtr - Start address of destination memory
//               MemSrcPtr - Start address of source memory
//               MemSz - Size of memory to copy
// Returns     : NONE
//-----------------------------------------------------------------------------
void util_mem_copy (void *MemDstPtr,
                    void *MemSrcPtr,
                    unsigned long MemSz)
{
    unsigned long *SrcPtr;
    unsigned long *DstPtr;
    unsigned long MaxIdx;
    unsigned long Idx;

    ASSERT((MemSz & 3) == 0);
    SrcPtr = MemSrcPtr;
    DstPtr = MemDstPtr;
    MaxIdx = MemSz / sizeof(long);

    for (Idx = 0;
         Idx < MaxIdx;
         Idx++)
    {
        *DstPtr = *SrcPtr;
        SrcPtr++;
        DstPtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_byte_copy
// Description : Copy bytes of a given length from a given source to a given
//               destination.
// Parameters  : *SrcPtr - pointer to the string to copy from.
//               *TgtPtr - pointer to the string to copy to.
//               ByteLen - length of the string to copy.
// Returns     : NONE
//-----------------------------------------------------------------------------
void util_byte_copy (unsigned char *SrcPtr,
                     unsigned char *TgtPtr,
                     unsigned long ByteLen)
{
    unsigned long Idx;

    for (Idx = 0;
         Idx < ByteLen;
         Idx++)
    {
        *TgtPtr = *SrcPtr;
        SrcPtr++;
        TgtPtr++;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_swap
// Description : Swaps per 4 bytes of data
// Parameters  : DataAddr - start address
//               Len - data length to be swapped
// Returns     : NONE
//-----------------------------------------------------------------------------
void util_swap (unsigned long DataAddr,
                unsigned long Len)
{
    unsigned long *P;
    unsigned long Ctr = ((Len % 4) > 0) ? ((Len >> 2) + 1) : (Len >> 2);
    unsigned long Idx = 0;

    P = (unsigned long *)DataAddr;

    for (Idx = 0;
         Idx < Ctr;
         Idx++)
    {
        *(P + Idx) = _swap32(*(P + Idx));
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_swap8
// Description : Swap 2 dwords (given 8bytes of data 0x11223344_55667788,
//               swap as follows: 0x55667788_11223344
// Parameters  : Addr - start address
// Returns     : NONE
//-----------------------------------------------------------------------------
void util_swap8 (unsigned long Addr)
{
    unsigned long Tmp;
    unsigned long *TwoWords;

    TwoWords = (unsigned long *)Addr;

    Tmp = TwoWords[1];
    TwoWords[1] = TwoWords[0];
    TwoWords[0] = Tmp;

    return;
}


//-----------------------------------------------------------------------------
// Function    : util_ffs
// Description : Returns bit offset of the 1st bit that set
// Parameters  : Bitmap
// Returns     : BitOffset
//-----------------------------------------------------------------------------
unsigned long util_ffs (unsigned long Bitmap)
{
    unsigned long BitOffset = 1;

    if (!Bitmap)
    {
        return 0;
    }

    if (!(Bitmap & 0xFFFF))
    {
        Bitmap >>= 16;
        BitOffset += 16;
    }

    if (!(Bitmap & 0xFF))
    {
        Bitmap >>= 8;
        BitOffset += 8;
    }

    if (!(Bitmap & 0xF))
    {
        Bitmap >>= 4;
        BitOffset += 4;
    }

    if (!(Bitmap & 3))
    {
        Bitmap >>= 2;
        BitOffset += 2;
    }

    if (!(Bitmap & 1))
    {
        Bitmap >>= 1;
        BitOffset += 1;
    }

    return BitOffset;
}


//-----------------------------------------------------------------------------
// Function    : util_dll_init
// Description : Initialize the util dll control struct
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void util_dll_init (UTIL_DLL_STRUCT *DllPtr)
{
    DllPtr->NextPtr = (void *)DllPtr;
    DllPtr->PrevPtr = (void *)DllPtr;
    return;
}


//-----------------------------------------------------------------------------
// Function    : util_sll_init
// Description : Initialize the util sll control struct
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void util_sll_init (UTIL_SLL_STRUCT *SllPtr)
{
    SllPtr->HeadPtr = BIT_NULL_PTR;
    SllPtr->TailPtr = BIT_NULL_PTR;
    return;
}


//-----------------------------------------------------------------------------
// Function    : util_switch_endian_word
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
unsigned long util_switch_endian_word (unsigned long Pattern)
{
    unsigned char t;

    union
    {
        unsigned long FourBytes;
        unsigned char OneByte[4];
    } Temp;

    Temp.FourBytes = Pattern;
    t = Temp.OneByte[3];
    Temp.OneByte[3] = Temp.OneByte[0];
    Temp.OneByte[0] = t;

    t = Temp.OneByte[2];
    Temp.OneByte[2] = Temp.OneByte[1];
    Temp.OneByte[1] = t;

    return Temp.FourBytes;
}


//-----------------------------------------------------------------------------
// Function    : util_switch_endian_words
// Description : Given a series of 4 bytes, swap the bytes as follows:
//               0:1 to 2:3 and 2:3 to 0:1.
// Parameters  : SrcAddr - address of the series of four bytes to swap.
//               TgtAddr - address where to put the results of the swap.
//               ByteLength - length in bytes which must be divisible by 4.
// Returns     : NONE
//-----------------------------------------------------------------------------
void util_switch_endian_words (unsigned long SrcAddr,
                               unsigned long TgtAddr,
                               unsigned long ByteLength)
{
    unsigned long Idx;
    unsigned long MaxIdx = ByteLength / sizeof(long);
    unsigned long *SrcPtr = (unsigned long *)SrcAddr;
    unsigned long *TgtPtr = (unsigned long *)TgtAddr;

    ASSERT((ByteLength & 3) == 0);

    for (Idx = 0;
         Idx < MaxIdx;
         Idx++)
    {
        TgtPtr[Idx] = util_switch_endian_word(SrcPtr[Idx]);
    }

    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//=============================================================================
// $Log: Util.c,v $
// Revision 1.5  2014/02/02 09:48:32  rcantong
// 1. DEV: Support mode select and bit specific config commands
// 1.1 Added handling of mode select and bit specific config commands
//
// Revision 1.4  2014/01/08 12:42:58  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.3  2013/11/11 08:20:50  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:44:24  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:08  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
