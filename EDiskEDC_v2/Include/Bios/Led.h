//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Bios/Led.h,v $
// $Revision: 1.3 $
// $Author: rcantong $
// $Date: 2014/02/02 09:50:07 $
// $Id: Led.h,v 1.3 2014/02/02 09:50:07 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__LED_H__)
#define __LED_H__

#if defined(DEBUG)
_Inline void led_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global Constant Macros
//-----------------------------------------------------------------------------

#define LED_CNTLR_PTR                   ((volatile unsigned long *)0xFFCE342C)

#define LED_RED_MASK                    (1 << 28)
#define LED_ORANGE_MASK                 (1 << 29)
#define LED_YELLOW_MASK                 (1 << 30)
#define LED_GREEN_MASK                  (1 << 31)
#define LED_ALL_MASK                    (   LED_RED_MASK    \
                                          | LED_ORANGE_MASK \
                                          | LED_YELLOW_MASK \
                                          | LED_GREEN_MASK)


//-----------------------------------------------------------------------------
// Global Type Definitions
//-----------------------------------------------------------------------------

typedef void (*LED_SEQ_FN)(void);


//-----------------------------------------------------------------------------
// Global Structure Definitions
//-----------------------------------------------------------------------------

typedef struct LedParmStruct
{
    // Will be used by all system functions
    LED_SEQ_FN SeqPtr;
    unsigned long Curr;
    unsigned long Mask;
    unsigned long PutCntLoc;
} LED_PARM_STRUCT;


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------

#define LED_STOP_SEQUENCE() \
do {                        \
    LedParm.Curr = 0;       \
    LedParm.Mask = 0;       \
} while (0)

// Scan Defects LED sequence
// Yellow led is alternate toggle with Green and Orange
#define LED_START_SCAN_DEFECTS()                                       \
do {                                                                   \
    LedParm.Curr = LED_GREEN_MASK | LED_ORANGE_MASK;                   \
    LedParm.Mask = LED_GREEN_MASK | LED_YELLOW_MASK | LED_ORANGE_MASK; \
} while (0)

// Control Blocks Fetch and Scrubbing LED sequence
// Green and Yellow blinking simultaneously
#define LED_START_CNTL_BLK_FETCH_SCRUB()             \
do {                                                 \
    LedParm.Curr = LED_GREEN_MASK | LED_YELLOW_MASK; \
    LedParm.Mask = LED_GREEN_MASK | LED_YELLOW_MASK; \
} while (0)

// Building LED LED sequence
// Orange and Yellow alternating toggle
#define LED_START_BUILDING()                          \
do {                                                  \
    LedParm.Curr = LED_ORANGE_MASK;                   \
    LedParm.Mask = LED_ORANGE_MASK | LED_YELLOW_MASK; \
} while (0)

#define LED_START_HOST_ACCESS()       \
do {                                  \
    LedParm.Curr = LED_YELLOW_MASK;   \
    LedParm.Mask = LED_YELLOW_MASK;   \
    LedParm.SeqPtr = led_host_access; \
} while (0)


//-----------------------------------------------------------------------------
// Global Variable Declaration (Extern)
//-----------------------------------------------------------------------------

#pragma BSS(".dccm_io")
extern LED_PARM_STRUCT LedParm;
#pragma BSS()


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void led_init (void);

void led_sequencer (void);

void led_host_access (void);


#endif
//=============================================================================
// $Log: Led.h,v $
// Revision 1.3  2014/02/02 09:50:07  rcantong
// 1. DEV: Support LED as status indicator
// 1.1 Added control of LEDs to indicate the device status
//
// Revision 1.2  2014/01/08 12:42:55  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.1  2013/07/03 19:34:01  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
