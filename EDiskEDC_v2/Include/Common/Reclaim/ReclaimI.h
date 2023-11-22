//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Include/Common/Reclaim/ReclaimI.h,v $
// $Revision: 1.5 $
// $Author: rcantong $
// $Date: 2014/04/30 14:24:09 $
// $Id: ReclaimI.h,v 1.5 2014/04/30 14:24:09 rcantong Exp $
//
// Note: This file should only be modified by qualified personnels
//=============================================================================
#if !defined(__RECLAIMI_H__)
#define __RECLAIMI_H__

#if defined(DEBUG)
_Inline void reclaimi_h (void) { return; }
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

// Control Data
void reclaim_cd_init_reclaim_lane (PCB_STRUCT *PcbPtr);

void reclaim_cd_init_reclaim_lane_cb (PCB_STRUCT *PcbPtr);

void reclaim_cd_reclaim_lane (PCB_STRUCT *PcbPtr);

void reclaim_cd_reclaim_lane_cb (PCB_STRUCT *PcbPtr);

// User Data
void reclaim_ud_init_reclaim_lane (PCB_STRUCT *PcbPtr);

void reclaim_ud_init_reclaim_lane_cb (PCB_STRUCT *PcbPtr);

void reclaim_ud_reclaim_lane (PCB_STRUCT *PcbPtr);

void reclaim_ud_reclaim_lane_cb (PCB_STRUCT *PcbPtr);


#endif
//=============================================================================
// $Log: ReclaimI.h,v $
// Revision 1.5  2014/04/30 14:24:09  rcantong
// 1. DEV: Cleanup
// 1.1 Remove unused structure
//
// Revision 1.4  2013/12/05 13:06:35  rcantong
// 1. DEV: Support data retainability
// 1.1 Codes to run data retainability
//
// Revision 1.3  2013/11/11 08:20:48  rcantong
// 1. DEV: Perform user data integrity
// 1.1 Codes to run user data integrity
//
// Revision 1.2  2013/08/08 16:42:08  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/03 19:34:13  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================
