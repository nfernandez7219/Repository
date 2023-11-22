//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Dir/DirI.h,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/04/30 13:56:45 $
// $Id: DirI.h,v 1.5 2014/04/30 13:56:45 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__DIRI_H__)
#define __DIRI_H__

#if defined(DEBUG)
_Inline void diri_h (void) { return; }
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

void dir_build_dev_stg1 (PCB_STRUCT *PcbPtr);

void dir_build_dev_stg2 (PCB_STRUCT *PcbPtr);

void dir_build_dev_stg3 (PCB_STRUCT *PcbPtr);

void dir_build_dev_stg4 (PCB_STRUCT *PcbPtr);

void dir_build_dev_stg5 (PCB_STRUCT *PcbPtr);

void dir_fetch_dir0_dev_stg1 (PCB_STRUCT *PcbPtr);

void dir_fetch_dir0_dev_stg2 (PCB_STRUCT *PcbPtr);

void dir_fetch_dir0_dev_stg1_mirror (PCB_STRUCT *PcbPtr);

void dir_fetch_dir0_dev_stg2_mirror (PCB_STRUCT *PcbPtr);

void dir_fetch_dir0 (PCB_STRUCT *PcbPtr);

void dir_fetch_dir0_done (PCB_STRUCT *PcbPtr);

void dir_fetch_dir0_mirror (PCB_STRUCT *PcbPtr);

void dir_fetch_dir0_mirror_done (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: DirI.h,v $
// Revision 1.5  2014/04/30 13:56:45  rcantong
// 1. DEV: Support CD mirroring
// 1.1 Added process to utilize the CD copy - JAbad
//
// Revision 1.4  2013/12/05 13:06:34  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:07  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:03  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
