//=============================================================================
// All Rights Reserved. Copyright (@) 2009 by BiTMICRO Networks, Inc.
// The contents of this software may not be reprinted or reproduced in whole
// or part without the written consent of BiTMICRO Networks, Inc.
// Printed copies of this material are uncontrolled documents.
//
// Description:
//
// File:
// $Source: /repository/firmware/EDC/src/EDiskEDC_v2/Source/Common/NvConfig/NvConfig.c,v $
// $Revision: 1.8 $
// $Author: rcantong $
// $Date: 2014/05/13 13:51:27 $
// $Id: NvConfig.c,v 1.8 2014/05/13 13:51:27 rcantong Exp $
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
#include "Sched.h"
#include "SysConfig.h"

#include "Bios.h"
#include "Dm.h"
#include "DmCommon.h"
#include "Dmx.h"
#include "DmxLite.h"
#include "DmxRecovery.h"
#include "EdcFwVersion.h"
#include "Err.h"
#include "Interrupt.h"
#include "IpCall.h"
#include "Media.h"
#include "NvConfig.h"


//-----------------------------------------------------------------------------
// Internal Includes
//-----------------------------------------------------------------------------
#include "NvConfigI.h"


//-----------------------------------------------------------------------------
// Variable Definitions
//-----------------------------------------------------------------------------

#pragma BSS(".usram")
volatile SYSCONFIG_STRUCT SysConfigSaved;
L1CACHE_ALIGN(SysConfigSaved);

volatile SYSCONFIG_STRUCT SysConfigCurr;
L1CACHE_ALIGN(SysConfigCurr);

volatile NVCONFIG_STRUCT NvConfigParm;
L1CACHE_ALIGN(NvConfigParm);
#pragma BSS()

const unsigned char VendorId[VENDOR_ID_LENGTH]         = {"BiTMICRO"};
const unsigned char ProdId[PRODUCT_ID_LENGTH]          = {"CFP04X2A20N     "};
const unsigned char ProdSerNo[SERIAL_NUMBER_LENGTH]    = {"ENCFP000001 "};

const unsigned char ProdModelName[MODEL_NAME_LENGTH]   = {"CFP4XE  "};
const unsigned char ProdPartNo[PART_NUMBER_LENGTH]     = {"CFP04X1536NET1  "};
const unsigned char ProdLotCode[DATE_CODE_LENGTH]      = {"F1350A  "};
const unsigned char ProdCtlSerNo[BRD_SERIAL_LENGTH]    = {"CAXXXXXXXX  "};
const unsigned char ProdPwrModSerNo[BRD_SERIAL_LENGTH] = {"CTXXXXXXXX  "};
const unsigned char ProdPGSerNo[BRD_SERIAL_LENGTH]     = {"CSXXXXXXXX  "};


//-----------------------------------------------------------------------------
// Global Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : nv_lite_preinit
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_lite_preinit (void)
{
    // Init NvConfigParm
    nv_preinit_parm();

    // Memory allocation for NvConfig
    nv_preinit_malloc();

    // Initialization of NvConfig
    nv_lite_init();

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_init (void)
{
    unsigned long long LastLba;

    LastLba = USABLE_SXN_CNT;
    LastLba = (LastLba * LBAS_PER_USER_SXN * FBX_CNT) - 1;
    SysConfigCurr.DmCfg.LastLba = LastLba;
    SysConfigCurr.DmCfg.LbaSz = LBA_SIZE;

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_init_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_init_malloc (unsigned long FbxIdx)
{
    NV_FBX_STRUCT *NvFbxPtr;

    NvFbxPtr = &DmFbx[FbxIdx].NvConfig; 

    // Initialize Log Info
    util_init_pattern((void *)&(SysConfigCurr.LogInfo),
                      sizeof(NV_LOG_INFO_STRUCT),
                      INIT_PATTERN_LO_VALUE);

    // Initialize DiskHealth
    util_init_pattern((void *)&(SysConfigCurr.DiskHealth),
                      sizeof(DISK_HEALTH_STRUCT),
                      INIT_PATTERN_LO_VALUE);

    NvFbxPtr->LogInfoPtr = (void *)&(SysConfigCurr.LogInfo.FbxLogInfo[FbxIdx]);

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_sync_loginfo
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_sync_loginfo (void)
{
    SysConfigCurr.LogInfo.FbxLogInfo[0].DmxResetCnt = TotalResets;
    SysConfigCurr.LogInfo.FbxLogInfo[0].RefreshCnt = TotalRefreshes;

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_sync_diskhealth
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_sync_diskhealth (void)
{
    unsigned long FbxIdx;
    
    // Get max EraseCnt 
    for (FbxIdx = 0;
         FbxIdx < FBX_CNT;
         FbxIdx++)
    {
        if (SysConfigCurr.LogInfo.FbxLogInfo[FbxIdx].EraseCnt 
            > SysConfigCurr.DiskHealth.EraseCnt)
        {
            SysConfigCurr.DiskHealth.EraseCnt 
                = SysConfigCurr.LogInfo.FbxLogInfo[FbxIdx].EraseCnt;
        }
    }    
        
    return;
}


//-----------------------------------------------------------------------------
// Local Functions Implementation
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function    : nv_preinit_parm
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_preinit_parm (void)
{
    NvConfigParm.ConfigBuff[0]      = (unsigned long)&SysConfigSaved;
    NvConfigParm.ConfigBuff[1]      = (unsigned long)&SysConfigCurr;
    NvConfigParm.SysConfigStat[0]   = 0xFFFFFFFF;
    NvConfigParm.SysConfigStat[1]   = 0xFFFFFFFF;
    NvConfigParm.EraseFlag          = OFF;
    NvConfigParm.FlushingFlag       = OFF;

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_preinit_malloc
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_preinit_malloc (void)
{
    // Fill pattern for SysConfigSaved
    dm_fill_random_pattern((void *)&SysConfigCurr,
                           sizeof(SYSCONFIG_STRUCT));

    // Fill pattern for SysConfigCurr
    dm_fill_random_pattern((void *)&SysConfigSaved,
                           sizeof(SYSCONFIG_STRUCT));

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_lite_init
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_lite_init (void)
{
    // Temporary... Turn on to erase nvconfig
    if (NvConfigParm.EraseFlag == ON)
    {
        nv_lite_wipe_nvconfig(0);
        nv_lite_wipe_nvconfig(1);
    }

    // Fetch nvconfig
    nv_lite_fetch_config(0);
    nv_lite_fetch_config(1);

    // Analyze fetched data
    nv_lite_evaluate_config();

    // Analyze product info
    nv_lite_evaluate_prodinfo();

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_lite_wipe_nvconfig
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_lite_wipe_nvconfig (unsigned long FbxIdx)
{
    BIT_STAT Stat;

    Stat = dmx_lite_write(0,
                          SYSCONFIG_SAVED_PBA,
                          NvConfigParm.ConfigBuff[FbxIdx],
                          sizeof(SYSCONFIG_STRUCT));

    // Remove this after validation
    ASSERT(Stat == SUCCESSFUL);

    Stat = dmx_lite_read(0,
                         SYSCONFIG_SAVED_PBA,
                         NvConfigParm.ConfigBuff[FbxIdx],
                         sizeof(SYSCONFIG_STRUCT));

    ASSERT(Stat == SUCCESSFUL);

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_lite_fetch_config
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_lite_fetch_config (unsigned long FbxIdx)
{
    BIT_STAT ReadStat;

    ASSERT (FbxIdx < NV_FBX_CNT);

    ReadStat = dmx_lite_read(0,
                             SYSCONFIG_SAVED_PBA,
                             NvConfigParm.ConfigBuff[FbxIdx],
                             sizeof(SYSCONFIG_STRUCT));

    NvConfigParm.SysConfigStat[FbxIdx] = ReadStat;

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_lite_evaluate_config
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_lite_evaluate_config (void)
{
    // Evaluate both Saved and Curr copy of NvConfig.
    if (    (NvConfigParm.SysConfigStat[0] == SUCCESSFUL)
         && (NvConfigParm.SysConfigStat[1] == SUCCESSFUL)
         && (SysConfigSaved.ProdInfo.SavedIndA == NV_CONFIG_SAVED_IND_A)
         && (SysConfigSaved.ProdInfo.SavedIndB == NV_CONFIG_SAVED_IND_B)
         && (SysConfigCurr.ProdInfo.SavedIndA == NV_CONFIG_SAVED_IND_A)
         && (SysConfigCurr.ProdInfo.SavedIndB == NV_CONFIG_SAVED_IND_B))
    {
        if (SysConfigSaved.Sqn != SysConfigCurr.Sqn)
        {
            if (SysConfigSaved.Sqn > SysConfigCurr.Sqn)
            {
                // Copy from Saved
                SysConfigCurr = SysConfigSaved;

                // Flush config
                nv_init_flush_config(1);
            }
        }
    }

    else if (    (NvConfigParm.SysConfigStat[0] == SUCCESSFUL)
              && (NvConfigParm.SysConfigStat[1] != SUCCESSFUL)
              && (SysConfigSaved.ProdInfo.SavedIndA == NV_CONFIG_SAVED_IND_A)
              && (SysConfigSaved.ProdInfo.SavedIndB == NV_CONFIG_SAVED_IND_B))
    {
        // Copy from Saved
        SysConfigCurr = SysConfigSaved;

        // Flush config
        nv_init_flush_config(1);
    }

    else if (    (NvConfigParm.SysConfigStat[0] != SUCCESSFUL)
              && (NvConfigParm.SysConfigStat[1] == SUCCESSFUL)
              && (SysConfigCurr.ProdInfo.SavedIndA == NV_CONFIG_SAVED_IND_A)
              && (SysConfigCurr.ProdInfo.SavedIndB == NV_CONFIG_SAVED_IND_B))
    {
        // Copy from Curr
        SysConfigSaved = SysConfigCurr;

        // Flush config
        nv_init_flush_config(0);
    }

    else
    {
        // Initialize configs
        nv_build_config();

        // Flush both configs
        nv_build_flush_config();
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_lite_evaluate_prodinfo
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_lite_evaluate_prodinfo (void)
{
    BIT_STAT Stat;

    Stat = util_mem_compare((void *)&SysConfigSaved.ProdInfo.ProductId[0],
                            (void *)&ProdId,
                            PRODUCT_ID_LENGTH);

    if (Stat != SUCCESSFUL)
    {
        // Update ProdInfo based on correct FW version
        util_byte_copy((void *)(&ProdId),
                       (void *)(&SysConfigSaved.ProdInfo.ProductId[0]),
                       PRODUCT_ID_LENGTH);

        // Update Current config prior flusing
        SysConfigCurr.ProdInfo = SysConfigSaved.ProdInfo;

        // Flush both configs
        nv_build_flush_config();
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_build_config
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_build_config (void)
{
    // Product Info
    nv_build_init_prod_info();

    SysConfigSaved.Sqn = 0;

    // Copy saved to curr
    SysConfigCurr = SysConfigSaved;

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_build_init_prod_info
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_build_init_prod_info (void)
{
    // Fill Header
    SysConfigSaved.ProdInfo.SavedIndA = NV_CONFIG_SAVED_IND_A;

    // Memcopy to config
    // Vendor ID
    util_byte_copy((void *)(&VendorId[0]),
                   (void *)(&SysConfigSaved.ProdInfo.VendorId[0]),
                   VENDOR_ID_LENGTH);

    // Product Id
    util_byte_copy((void *)(&ProdId),
                   (void *)(&SysConfigSaved.ProdInfo.ProductId[0]),
                   PRODUCT_ID_LENGTH);

    // Part Number
    util_byte_copy((void *)(&ProdPartNo),
                   (void *)(&SysConfigSaved.ProdInfo.PartNumber[0]),
                   PART_NUMBER_LENGTH);

    // Date Code
    util_byte_copy((void *)(&ProdLotCode),
                   (void *)(&SysConfigSaved.ProdInfo.DateCode[0]),
                   DATE_CODE_LENGTH);

    // Serial Number
    util_byte_copy((void *)(&ProdSerNo),
                   (void *)(&SysConfigSaved.ProdInfo.SerialNumber[0]),
                   SERIAL_NUMBER_LENGTH);

    // CtlBoard Serial Number
    util_byte_copy((void *)(&ProdCtlSerNo),
                   (void *)(&SysConfigSaved.ProdInfo.CtlBrdSerNo[0]),
                   BRD_SERIAL_LENGTH);

    // MemBoard Serial Number
    util_byte_copy((void *)(&ProdPwrModSerNo),
                   (void *)(&SysConfigSaved.ProdInfo.PwrModSerNo[0]),
                   BRD_SERIAL_LENGTH);

    // PGBoard Serial Number
    util_byte_copy((void *)(&ProdPGSerNo),
                   (void *)(&SysConfigSaved.ProdInfo.PGBrdSerNo[0]),
                   BRD_SERIAL_LENGTH);

    // Model Name
    util_byte_copy((void *)(&ProdModelName),
                   (void *)(&SysConfigSaved.ProdInfo.ModelName[0]),
                   MODEL_NAME_LENGTH);

    // Fill Footer
    SysConfigSaved.ProdInfo.SavedIndB = NV_CONFIG_SAVED_IND_B;

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_build_flush_config
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_build_flush_config (void)
{
    unsigned long FbxIdx;
    BIT_STAT Stat;

    SysConfigSaved.Sqn++;
    SysConfigCurr.Sqn++;

    for (FbxIdx = 0;
         FbxIdx < NV_FBX_CNT;
         FbxIdx++)
    {
        Stat = dmx_lite_write(0,
                              SYSCONFIG_SAVED_PBA,
                              NvConfigParm.ConfigBuff[FbxIdx],
                              sizeof(SYSCONFIG_STRUCT));

        ASSERT(Stat == SUCCESSFUL);

        // Remove this after validation
        Stat = dmx_lite_read(0,
                             SYSCONFIG_SAVED_PBA,
                             NvConfigParm.ConfigBuff[FbxIdx],
                             sizeof(SYSCONFIG_STRUCT));

        ASSERT(Stat == SUCCESSFUL);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_init_flush_config
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_init_flush_config (unsigned long FbxIdx)
{
    BIT_STAT Stat;

    ASSERT(FbxIdx < NV_FBX_CNT);

    if (FbxIdx == NV_FBX0)
    {
        Stat = dmx_lite_write(0,
                              SYSCONFIG_SAVED_PBA,
                              (unsigned long)&SysConfigSaved,
                               sizeof(SYSCONFIG_STRUCT));

        // Remove this after validation
        ASSERT(Stat == SUCCESSFUL);

        Stat = dmx_lite_read(0,
                             SYSCONFIG_SAVED_PBA,
                             (unsigned long)&SysConfigSaved,
                             sizeof(SYSCONFIG_STRUCT));

        ASSERT(Stat == SUCCESSFUL);
    }

    else
    {
        Stat = dmx_lite_write(0,
                              SYSCONFIG_SAVED_PBA,
                              (unsigned long)&SysConfigCurr,
                              sizeof(SYSCONFIG_STRUCT));

        // Remove this after validation
        ASSERT(Stat == SUCCESSFUL);

        Stat = dmx_lite_read(0,
                             SYSCONFIG_SAVED_PBA,
                             (unsigned long)&SysConfigCurr,
                             sizeof(SYSCONFIG_STRUCT));

        ASSERT(Stat == SUCCESSFUL);
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_flush_config
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_flush_config (void)
{
    IPCALL_STRUCT *IpCallPtr;

    if (NvConfigParm.FlushingFlag == OFF)
    {
        SysConfigSaved.Sqn++;

        // Set DirtyFlag and FlushingFlag to ON
        NvConfigParm.FlushingFlag = ON;

        IpCallPtr = ipcall_add_slave_entry(0);
        IpCallPtr->Fn = media_write_in_place;
        IpCallPtr->Arg[0] = SYSCONFIG_SAVED_PBA;
        IpCallPtr->Arg[1] = sizeof(SYSCONFIG_STRUCT);
        IpCallPtr->Arg[2] = (unsigned long)&SysConfigSaved;
        IpCallPtr->Arg[3] = (unsigned long)nv_flush_config_cb;
        ipcall_slave_exec_calls(0);
    }

    else // FlushingFlag is ON
    {
        // Set DirtyFlag to ON. Reflushing of Cache will be triggered
        // during nv_flush_config_cb
        NvConfigParm.DirtyFlag = ON;
    }

    return;
}


//-----------------------------------------------------------------------------
// Function    : nv_flush_config_cb
// Description :
// Parameters  : NONE
// Returns     : NONE
//-----------------------------------------------------------------------------
void nv_flush_config_cb (unsigned long *PayLoadPtr)
{
    // Set FlushingFlag to OFF
    NvConfigParm.FlushingFlag = OFF;

    if (NvConfigParm.DirtyFlag == ON)
    {
        // Reflush Config
        nv_flush_config();
    }

    return;
}


//=============================================================================
// $Log: NvConfig.c,v $
// Revision 1.8  2014/05/13 13:51:27  rcantong
// 1. DEV: Able to rebuild by using hardware jumper
// 1.1 Added detection of jumper config to set rebuild flag - MManzo
//
// Revision 1.7  2014/04/30 14:16:58  rcantong
// 1. DEV: Support checking of updated FW version
// 1.1 Added saving of FW version in NvConfig - JParairo
// 2. DEV: Support board serial number in product info
// 2.1 Added CtlBrdSerNo, PwrModSerNo and PGBrdSerNo - JParairo
//
// Revision 1.6  2014/03/03 13:07:14  rcantong
// 1. DEV: Update FW Version
// 1.1 Changed 2A06 to 2A10
//
// Revision 1.5  2014/02/06 14:45:06  rcantong
// 1. DEV: Update FW version
// 1.1 Changed 2A05 to 2A06
//
// Revision 1.4  2014/02/02 09:54:16  rcantong
// 1. DEV: Support mode select and bit specific config commands
// 1.1 Added handling of mode select and bit specific config commands
//
// Revision 1.3  2014/01/08 12:42:57  rcantong
// 1. DEV: Perform random write longevity
// 1.1 Codes for control and user data compacting
//
// Revision 1.2  2013/08/08 16:44:24  rcantong
// 1. DEV: Perform iometer read and write
// 1.1 Codes to run iometer read and write
//
// Revision 1.1  2013/07/15 17:54:17  rcantong
// 1. DEV: Initial commit
// 1.1 Template file
//
//=============================================================================