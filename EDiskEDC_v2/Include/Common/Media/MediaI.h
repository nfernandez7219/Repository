//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Media/MediaI.h,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/04/30 14:04:54 $
// $Id: MediaI.h,v 1.5 2014/04/30 14:04:54 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__MEDIAI_H__)
#define __MEDIAI_H__

#if defined(DEBUG)
_Inline void mediai_h (void) { return; }
#endif


//-----------------------------------------------------------------------------
// Local Constant Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Type Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Structure Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

// Media read stages
void media_read_alloc_dir0 (PCB_STRUCT *PcbPtr);

void media_read_fetch_ud (PCB_STRUCT *PcbPtr);

void media_read_done (PCB_STRUCT *PcbPtr);

// Media write stages
void media_write_lock_usersxn (PCB_STRUCT *PcbPtr);

void media_write_flush_ud (PCB_STRUCT *PcbPtr);

void media_write_flush_ud_done (PCB_STRUCT *PcbPtr);

void media_write_alloc_si0 (PCB_STRUCT *PcbPtr);

void media_write_update_si0 (PCB_STRUCT *PcbPtr);

void media_write_alloc_dir0 (PCB_STRUCT *PcbPtr);

void media_write_update_dir0 (PCB_STRUCT *PcbPtr);

// Media read modify write stages
void media_rmw_lock_usersxn (PCB_STRUCT *PcbPtr);

void media_rmw_read_alloc_dir0 (PCB_STRUCT *PcbPtr);

void media_rmw_fetch_ud (PCB_STRUCT *PcbPtr);

void media_rmw_fetch_ud_done (PCB_STRUCT *PcbPtr);

// Media write in place
void media_write_in_place_done (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: MediaI.h,v $
// Revision 1.5  2014/04/30 14:04:54  rcantong
// 1. BUGFIX: Deadlock in user section lock during RMW
// 1.1 Implement atomic locking of user sections - MFenol
//
// Revision 1.4  2014/02/02 10:00:32  rcantong
// 1. DEV: Support remapping and read disturb management
// 1.1 Codes for user data remapping and read disturb management
//
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:08  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:04  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
