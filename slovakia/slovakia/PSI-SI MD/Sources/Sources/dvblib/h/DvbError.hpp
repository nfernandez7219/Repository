#ifndef __INC_DVBERROR_H__
#define __INC_DVBERROR_H__

//
// All DVB functions returning operation status, thread exit values, exception
// or logfile codes ... should use DVB coding scheme defined in this file.
//
// DVB event coding scheme encompasses following codes:
//
// - WIN32 codes
//		Simply use as return value without any modification.
//
// - errno codes
//		use DvbErrno(errno) to convert to DVB code
//
// - System exceptions
//		Use following scheme:
//			__try
//			{
//				...
//			}
//			CATCH_EXCEPTION_CODE
//			{
//				DWORD exc = EXCEPTION_CODE ;
//				// ...
//				// SAY_EXCEPTION(exc) ;
//				// ...
//				return exc ;
//			}
//		or one of the simplified versions:
//			__try { ... } CATCH_AND_RETURN_EXCEPTION	// returns from the function
//			__try { ... } CATCH_AND_SAY_EXCEPTION		// just displays the string
//
//		This scheme cannot be combined in one function with C++ try/catch block.
//		Must be used in the main function of every thread.
//
// - Application errors, log events...
//		Use constants defined below as return values.
//
//
// The event codes consist of the code (16 bit), facility (event group) and
// severity (error, warning...).
// To convert event code to the text, call DvbEventText().
//


#define CATCH_EXCEPTION_CODE		__except( EXCEPTION_EXECUTE_HANDLER )
#define EXCEPTION_CODE				(convertSystemExceptionCode( GetExceptionCode()))
#define SAY_EXCEPTION(exc)			{char buf[512]; AfxMessageBox( DvbEventText(exc,buf), MB_ICONSTOP|MB_OK );}

#define CATCH_AND_RETURN_EXCEPTION	CATCH_EXCEPTION_CODE { return EXCEPTION_CODE; }
#define CATCH_AND_SAY_EXCEPTION		CATCH_EXCEPTION_CODE { SAY_EXCEPTION(EXCEPTION_CODE) ; }


const char *DvbEventText( int code, char *buf ) ;		// max 256 chars


//
//  Error/event code system is an extension of the WIN32 scheme (winerror.h).
//  They are not compatible with HRESULT coding scheme.
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +-------+-----------------------+-------------------------------+
//  |Sev|C| |     Facility          |               Code            |
//  |    1 0|                       |                               |
//  +-------+-----------------------+-------------------------------+
//
//  where
//      Sev      = is the severity code
//          00 - Log
//          01 - Informational
//          10 - Warning
//          11 - Error
//      C        = is the Customer code flag (0 for WIN32 codes)
//      Facility = is the facility code
//      Code     = is the facility's status code
//

#define NonWin32Event			0x20000000

#define Event_Facility(h)		(  (h) & 0xfff0000 )
#define Event_IsLog(h)			( ((h) & 0xc0000000) == 0x00000000 )
#define Event_IsInfo(h)			( ((h) & 0xc0000000) == 0x40000000 )
#define Event_IsWarning(h)		( ((h) & 0xc0000000) == 0x80000000 )
#define Event_IsError(h)		( ((h) & 0xc0000000) == 0xc0000000 )
#define Event_IsWinCode(h)		( ((h) & NonWin32Event) == 0 )

// facility codes
//
#define Facility_Null			0			/* general events */
#define Facility_IO				(1   <<16)	/* I/O driver dll errors */
#define Facility_File			(2   <<16)	/* any events concerned with the disk */
#define Facility_Free4			(4   <<16)	/* */
#define Facility_Usr			(8   <<16)	/* events concerned with the user actions */
#define Facility_Exc			(16  <<16)	/* */
#define Facility_Errno			(32  <<16)	/* */
#define Facility_DVB			(64  <<16)	/* dvb driver/card events */
#define Facility_HNet			(128 <<16)	/* HNet */
#define Facility_Free10			(256 <<16)	/* */
#define Facility_Free11			(512 <<16)	/* */
#define Facility_Msg			(1024<<16)	/* */

//---------------- general codes -----------------------------------

#define Event_LogFlag					(0x00000000 | NonWin32Event)
#define Event_InfoFlag					(0x40000000 | NonWin32Event)
#define Event_WarningFlag				(0x80000000 | NonWin32Event)
#define Event_ErrorFlag					(0xc0000000 | NonWin32Event)


#define DvbErr_UnknownComDevice          (Event_ErrorFlag | 1)
#define DvbErr_BadConnectStr             (Event_ErrorFlag | 2)
#define DvbErr_UnknownError              (Event_ErrorFlag | 3)
#define DvbErr_BadUserRights			 (Event_ErrorFlag | 4)
#define DvbErr_InboxKilled				 (Event_ErrorFlag | 5)
#define DvbErr_LoadDriverDll			 (Event_ErrorFlag | 6)


//---------------- exception codes ---------------------------------
//
// Exceptions caught in the __try block

#define Event_ExcErrorFlag				 (Facility_Exc | Event_ErrorFlag)

#define ExcErr_ACCESS_VIOLATION          (Event_ExcErrorFlag | 1)
#define ExcErr_DATATYPE_MISALIGNMENT     (Event_ExcErrorFlag | 2)
#define ExcErr_BREAKPOINT                (Event_ExcErrorFlag | 3)
#define ExcErr_SINGLE_STEP               (Event_ExcErrorFlag | 4)
#define ExcErr_ARRAY_BOUNDS_EXCEEDED     (Event_ExcErrorFlag | 5)
#define ExcErr_FLT_DENORMAL_OPERAND      (Event_ExcErrorFlag | 6)
#define ExcErr_FLT_DIVIDE_BY_ZERO        (Event_ExcErrorFlag | 7)
#define ExcErr_FLT_INEXACT_RESULT        (Event_ExcErrorFlag | 8)
#define ExcErr_FLT_INVALID_OPERATION     (Event_ExcErrorFlag | 9)
#define ExcErr_FLT_OVERFLOW              (Event_ExcErrorFlag | 10)
#define ExcErr_FLT_STACK_CHECK           (Event_ExcErrorFlag | 11)
#define ExcErr_FLT_UNDERFLOW             (Event_ExcErrorFlag | 12)
#define ExcErr_INT_DIVIDE_BY_ZERO        (Event_ExcErrorFlag | 13)
#define ExcErr_INT_OVERFLOW              (Event_ExcErrorFlag | 14)
#define ExcErr_PRIV_INSTRUCTION          (Event_ExcErrorFlag | 15)
#define ExcErr_IN_PAGE_ERROR             (Event_ExcErrorFlag | 16)
#define ExcErr_ILLEGAL_INSTRUCTION       (Event_ExcErrorFlag | 17)
#define ExcErr_NONCONTINUABLE_EXCEPTION  (Event_ExcErrorFlag | 18)
#define ExcErr_STACK_OVERFLOW            (Event_ExcErrorFlag | 19)
#define ExcErr_INVALID_DISPOSITION       (Event_ExcErrorFlag | 20)
#define ExcErr_GUARD_PAGE                (Event_ExcErrorFlag | 21)
#define ExcErr_INVALID_HANDLE            (Event_ExcErrorFlag | 22)
#define ExcErr_UnknownException          (Event_ExcErrorFlag | 30000)


//---------------- Driver IO codes ---------------------------------

#define Event_IOLogFlag			(Facility_IO | Event_LogFlag	)
#define Event_IOInfoFlag		(Facility_IO | Event_InfoFlag	)
#define Event_IOWarningFlag		(Facility_IO | Event_WarningFlag)
#define Event_IOErrorFlag		(Facility_IO | Event_ErrorFlag	)

//---------------- file codes --------------------------------------

#define Event_FileLogFlag		(Facility_File | Event_LogFlag)
#define Event_FileInfoFlag		(Facility_File | Event_InfoFlag	)
#define Event_FileWarningFlag	(Facility_File | Event_WarningFlag)
#define Event_FileErrorFlag		(Facility_File | Event_ErrorFlag)

#define FileErr_OpenError		(Event_FileErrorFlag | 1)
#define FileErr_ReadError		(Event_FileErrorFlag | 2)
#define FileErr_WriteError		(Event_FileErrorFlag | 3)
#define FileErr_NoSuchFile		(Event_FileErrorFlag | 4)

#define FileInf_OutboxRemoved	(Event_FileInfoFlag  | 1)

//---------------- Errno codes ---------------------------------------

#define Event_ErrnoLogFlag		(Facility_Errno | Event_LogFlag	)
#define Event_ErrnoInfoFlag		(Facility_Errno | Event_InfoFlag	)
#define Event_ErrnoWarningFlag	(Facility_Errno | Event_WarningFlag)
#define Event_ErrnoErrorFlag	(Facility_Errno | Event_ErrorFlag	)

#define DvbErrno( err )			(err | Event_ErrnoErrorFlag)

//---------------- Usr codes ---------------------------------------

#define Event_UsrLogFlag		(Facility_Usr | Event_LogFlag	)
#define Event_UsrInfoFlag		(Facility_Usr | Event_InfoFlag	)
#define Event_UsrWarningFlag	(Facility_Usr | Event_WarningFlag)
#define Event_UsrErrorFlag		(Facility_Usr | Event_ErrorFlag	)

#define Usr_UnknownError		( Event_UsrErrorFlag | 1 )

//---------------- Event codes ---------------------------------------

#define Event_MsgLogFlag		(Facility_Msg | Event_LogFlag	)
#define Event_MsgInfoFlag		(Facility_Msg | Event_InfoFlag	)
#define Event_MsgWarningFlag	(Facility_Msg | Event_WarningFlag)
#define Event_MsgErrorFlag		(Facility_Msg | Event_ErrorFlag	)

//---------------- DVB codes -----------------------------------------

#define Event_DVBErrorFlag			(Facility_DVB | Event_ErrorFlag )

#define HNETERR_UnknownErr			( Event_DVBErrorFlag | 21)

// Assert messages
#define	DVB_Assert					( Event_DVBErrorFlag | 50)
#define	DVB_AssertWarning			( Event_DVBErrorFlag | 51)
#define	DVB_AssertError				( Event_DVBErrorFlag | 52)

//---------------- HNet codes -----------------------------------------

#define Event_HNErrorFlag			(Facility_HNet | Event_ErrorFlag )

inline int HNetDrvError2DVBError( int x)
{
	return  Event_HNErrorFlag | (UINT(x)) ;
}
inline int DVBError2HNetDrvError ( int x)
{
	if( ( Event_HNErrorFlag & (UINT)(x) ) == Event_HNErrorFlag )
		return  (UINT(x)) & 0xFFFF ;
	else
		return 0 ;
}

//---------------- Server -----------------------------------------
#define IsInboxMessage(msg)			(msg>=EMsg_InboxStart && msg<=EMsg_InboxVolumeExceeded)

#define	EMsg_InboxStart				(Event_MsgLogFlag | 1)
#define	EMsg_InboxEnd				(Event_MsgLogFlag | 2)
#define	EMsg_InboxSendStarted		(Event_MsgLogFlag | 3)
#define	EMsg_InboxSendWaiting		(Event_MsgLogFlag | 4)
#define	EMsg_InboxSendCompleted		(Event_MsgLogFlag | 7)
#define EMsg_InboxSendFailed		(Event_MsgLogFlag | 8)
#define	EMsg_InboxError				(Event_MsgLogFlag | 9)
#define	EMsg_InboxVolumeExceeded	(Event_MsgLogFlag |10)
#define	EMsg_InboxStatus			(Event_MsgLogFlag |11)
#define	EMsg_InboxSpeed				(Event_MsgLogFlag |12)
#define	EMsg_InboxBroadcasting		(Event_MsgLogFlag |13)	// lParam=string wParam=channelId
#define	EMsg_InboxUnicasting		(Event_MsgLogFlag |14)	// lParam=string wParam=*userId
#define	EMsg_InboxMulticasting		(Event_MsgLogFlag |15)	// lParam=string wParam=channelId
#define	EMsg_InboxPriority			(Event_MsgLogFlag |16)
#define	EMsg_InboxOutputRate		(Event_MsgLogFlag |17)
#define	EMsg_InboxFECChanged		(Event_MsgLogFlag |18)
#define	EMsg_StreamFormatChanged	(Event_MsgLogFlag |19)

#define IsCfgUserMessage(msg)		(msg>=EMsg_CfgNewUser && msg<=EMsg_CfgSendToUser)

#define	EMsg_CfgNewUser				(Event_MsgLogFlag |21)		// lParam = CfgUser
#define	EMsg_CfgUserDeleted			(Event_MsgLogFlag |22)
#define	EMsg_CfgUserModified		(Event_MsgLogFlag |23)
#define	EMsg_CfgSendToUser			(Event_MsgLogFlag |24)		//			, wParam = file path

#define IsCfgProviderMessage(msg)	(msg>=EMsg_CfgNewProvider && msg<=EMsg_CfgProviderModified)

#define	EMsg_CfgNewProvider			(Event_MsgLogFlag |31)		// lParam = CfgProvider
#define	EMsg_CfgProviderDeleted		(Event_MsgLogFlag |32)
#define	EMsg_CfgProviderModified	(Event_MsgLogFlag |33)

#define IsCfgChannelMessage(msg)	(msg>=EMsg_CfgNewChannel && msg<=EMsg_CfgChannelModified)

#define	EMsg_CfgNewChannel			(Event_MsgLogFlag |41)		// lParam = CfgChannel
#define	EMsg_CfgChannelDeleted		(Event_MsgLogFlag |42)
#define	EMsg_CfgChannelModified		(Event_MsgLogFlag |43)

#define	EMsg_CfgDVBSettingsChanged	(Event_MsgLogFlag |44)		// nothing
#define	EMsg_CfgOutputRateChanged	(Event_MsgLogFlag |45)		// lParam = new transfer rate (float)
#define	EMsg_CfgUpdateScheduler		(Event_MsgLogFlag |46)

#define	EMsg_CfgUpgradeSoftware		(Event_MsgLogFlag |47)		// lParam = path to file to sent

#define	EMsg_AppConnected			(Event_MsgLogFlag |48)
#define	EMsg_AppDisconnected		(Event_MsgLogFlag |49)
#define	EMsg_SrvLogMessage			(Event_MsgLogFlag |50)
#define	EMsg_CfgDVBLogFileChanged	(Event_MsgLogFlag |51)


//---------------- Client -----------------------------------------
// File receiver messages
#define	EMsg_FRcvrStartProcess		(Event_MsgLogFlag |100)
#define	EMsg_FRcvrEndProcess		(Event_MsgLogFlag |101)
#define	EMsg_FRcvrError				(Event_MsgLogFlag |102)
#define	EMsg_FRcvrStatus			(Event_MsgLogFlag |103)
#define	EMsg_FRcvrProgress			(Event_MsgLogFlag |104)
#define	EMsg_FRcvrSpeed				(Event_MsgLogFlag |105)
#define	EMsg_FRcvrStartReceiving	(Event_MsgLogFlag |116)
#define	EMsg_FRcvrEndReceiving		(Event_MsgLogFlag |117)
#define	EMsg_FRcvrFileOpened		(Event_MsgLogFlag |108)
#define	EMsg_FRcvrFileCompleted		(Event_MsgLogFlag |109)
#define	EMsg_FRcvrFileWithoutHeader	(Event_MsgLogFlag |110)
#define	EMsg_FRcvrFileIncompleteData (Event_MsgLogFlag |111)
#define	EMsg_FRcvrFileDamagedData	(Event_MsgLogFlag |112)
#define	EMsg_FRcvrFileRefused		(Event_MsgLogFlag |113)
#define	EMsg_FRcvrFileAllreadyExisting	(Event_MsgLogFlag |114)
#define	EMsg_FRcvrFileName			(Event_MsgLogFlag |115)
#define	EMsg_FRcvrUnknown			(Event_MsgLogFlag |125)

#define	EMsg_NewInstallation		(Event_MsgLogFlag |126)

// Service receiver messages
#define	EMsg_SRcvrCreateDlg			(Event_MsgLogFlag |130)
#define	EMsg_SCAChanged				(Event_MsgLogFlag |131)
#define	EMsg_ChannelsUpdateFailed	(Event_MsgLogFlag |132)
#define	EMsg_ChannelNamesChanged	(Event_MsgLogFlag |133)
#define	EMsg_ChannelDeleted			(Event_MsgLogFlag |134)
#define	EMsg_ChannelAdded			(Event_MsgLogFlag |135)

// config. messages
#define	EMsg_SetupChanged			(Event_MsgLogFlag |47)


// config. messages
#define	EMsg_AliveSignal			(Event_MsgLogFlag |150)


// communication error
#define	EMsg_CommunicationError		(Event_MsgLogFlag |1000)
#define EMsg_DvbError				(Event_MsgLogFlag |1001)
#define EMsg_DvbInfo				(Event_MsgLogFlag |1002)

// HNet messages
#define	EMsg_HNetError				(Event_MsgLogFlag |1003)
#define	EMsg_HNetStart				(Event_MsgLogFlag |1004)
#define	EMsg_HNetSetOnOff			(Event_MsgLogFlag |1005)
#define EMsg_HNetInfo				(Event_MsgLogFlag |1006)


//---------------- internals ---------------------------------------
DWORD       convertSystemExceptionCode( DWORD code ) ;	// converts system code to DVB code

#endif
