/*
 *	Filename:		dvberror.cpp
 *
 *	Version:		1.00
 *
 *	Description: implementation of DvbEventText() (see DvbError.hpp)
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"

#include "except.hpp"
#include "internet.hpp"


//--- exceptions ----------------------------------------------------------------


DWORD convertSystemExceptionCode( DWORD code )
{
	switch( code )
	{
		case EXCEPTION_ACCESS_VIOLATION			: return ExcErr_ACCESS_VIOLATION        ;
		case EXCEPTION_BREAKPOINT				: return ExcErr_BREAKPOINT              ;
		case EXCEPTION_DATATYPE_MISALIGNMENT	: return ExcErr_DATATYPE_MISALIGNMENT   ;
		case EXCEPTION_SINGLE_STEP				: return ExcErr_SINGLE_STEP             ;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED	: return ExcErr_ARRAY_BOUNDS_EXCEEDED   ;
		case EXCEPTION_FLT_DENORMAL_OPERAND		: return ExcErr_FLT_DENORMAL_OPERAND    ;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO		: return ExcErr_FLT_DIVIDE_BY_ZERO      ;
		case EXCEPTION_FLT_INEXACT_RESULT		: return ExcErr_FLT_INEXACT_RESULT      ;
		case EXCEPTION_FLT_INVALID_OPERATION	: return ExcErr_FLT_INVALID_OPERATION   ;
		case EXCEPTION_FLT_OVERFLOW				: return ExcErr_FLT_OVERFLOW            ;
		case EXCEPTION_FLT_STACK_CHECK			: return ExcErr_FLT_STACK_CHECK         ;
		case EXCEPTION_FLT_UNDERFLOW			: return ExcErr_FLT_UNDERFLOW           ;
		case EXCEPTION_INT_DIVIDE_BY_ZERO		: return ExcErr_INT_DIVIDE_BY_ZERO      ;
		case EXCEPTION_INT_OVERFLOW				: return ExcErr_INT_OVERFLOW            ;
		case EXCEPTION_PRIV_INSTRUCTION			: return ExcErr_PRIV_INSTRUCTION        ;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION	: return ExcErr_NONCONTINUABLE_EXCEPTION;
		case EXCEPTION_IN_PAGE_ERROR            : return ExcErr_IN_PAGE_ERROR           ;
		case EXCEPTION_ILLEGAL_INSTRUCTION      : return ExcErr_ILLEGAL_INSTRUCTION     ;
		case EXCEPTION_STACK_OVERFLOW           : return ExcErr_STACK_OVERFLOW          ;
		case EXCEPTION_INVALID_DISPOSITION      : return ExcErr_INVALID_DISPOSITION     ;
		case EXCEPTION_GUARD_PAGE               : return ExcErr_GUARD_PAGE              ;
		case EXCEPTION_INVALID_HANDLE           : return ExcErr_INVALID_HANDLE          ;
	}
	return ExcErr_UnknownException ;
}


//--- DvbEventText() ------------------------------------------------------------


const char *DvbEventText( int code, char *buf )		// max 256 chars
{
	if( code == 0 )
	{
		strcpy( buf, "success" ) ;
		return buf ;
	}

	// WIN32 codes
	if( Event_IsWinCode(code) )
	{
		FormatMessage
		( 
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			code,
			0,					// default language
			//MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			buf,				// buffer
			256,				// size
			NULL				// args (va_list)
		);
		char *ch = buf;
		while ( ch && *ch != 0 )
		{
			if( *ch == '\r' || *ch == '\n' )
				*ch = ' ';
			ch++;
		}
		return buf ;
	}

	if( code & Facility_IO )
	{
		if( bigComIO != NULL )
			return bigComIO->errorCodeAsText( code, buf ) ;
		sprintf( buf, "I/O driver error %d", code & 0xFFFF ) ;
		return buf ;
	}

	const char *msg=NULL ;

	if( Event_Facility(code) == 0 )
	{
		switch( code )
		{
			case DvbErr_UnknownComDevice:	msg = "Unknown device." ; break ;
			case DvbErr_BadConnectStr   :	msg = "Bad connect string." ; break ;
			case DvbErr_BadUserRights:		msg = "User log file corrupted." ; break ;
			case DvbErr_InboxKilled:		msg = "File send interrupted." ; break ;
			case DvbErr_LoadDriverDll:		msg = "Failed to load driver Dll." ; break ;
		}
	}
	else
	if( code & Facility_File )
	{
		switch( code )
		{
			case FileErr_OpenError:			msg = "Open file error." ; break ;
			case FileErr_ReadError:			msg = "Read file error." ; break ;
			case FileErr_WriteError:		msg = "Write file error" ; break ;
			case FileErr_NoSuchFile:		msg = "File not found"   ; break ;
		}
	}
	else
	if( code & Facility_Usr )
	{
		switch( code )
		{
			case Usr_UnknownError:			msg = "Unknown error."; break;
		}
	}
	else
	if( code & Facility_Exc )
	{
		DWORD excCode=0 ;
		switch( code )
		{
			case ExcErr_ACCESS_VIOLATION        : excCode = EXCEPTION_ACCESS_VIOLATION		; break ;
			case ExcErr_BREAKPOINT              : excCode = EXCEPTION_BREAKPOINT			; break ;
			case ExcErr_DATATYPE_MISALIGNMENT   : excCode = EXCEPTION_DATATYPE_MISALIGNMENT	; break ;
			case ExcErr_SINGLE_STEP             : excCode = EXCEPTION_SINGLE_STEP			; break ;
			case ExcErr_ARRAY_BOUNDS_EXCEEDED   : excCode = EXCEPTION_ARRAY_BOUNDS_EXCEEDED	; break ;
			case ExcErr_FLT_DENORMAL_OPERAND    : excCode = EXCEPTION_FLT_DENORMAL_OPERAND	; break ;
			case ExcErr_FLT_DIVIDE_BY_ZERO      : excCode = EXCEPTION_FLT_DIVIDE_BY_ZERO	; break ;
			case ExcErr_FLT_INEXACT_RESULT      : excCode = EXCEPTION_FLT_INEXACT_RESULT	; break ;
			case ExcErr_FLT_INVALID_OPERATION   : excCode = EXCEPTION_FLT_INVALID_OPERATION	; break ;
			case ExcErr_FLT_OVERFLOW            : excCode = EXCEPTION_FLT_OVERFLOW			; break ;
			case ExcErr_FLT_STACK_CHECK         : excCode = EXCEPTION_FLT_STACK_CHECK		; break ;
			case ExcErr_FLT_UNDERFLOW           : excCode = EXCEPTION_FLT_UNDERFLOW			; break ;
			case ExcErr_INT_DIVIDE_BY_ZERO      : excCode = EXCEPTION_INT_DIVIDE_BY_ZERO	; break ;
			case ExcErr_INT_OVERFLOW            : excCode = EXCEPTION_INT_OVERFLOW			; break ;
			case ExcErr_PRIV_INSTRUCTION        : excCode = EXCEPTION_PRIV_INSTRUCTION		; break ;
			case ExcErr_NONCONTINUABLE_EXCEPTION: excCode = EXCEPTION_NONCONTINUABLE_EXCEPTION; break ;
			case ExcErr_IN_PAGE_ERROR           : excCode = EXCEPTION_IN_PAGE_ERROR         ; break ;
			case ExcErr_ILLEGAL_INSTRUCTION     : excCode = EXCEPTION_ILLEGAL_INSTRUCTION   ; break ;
			case ExcErr_STACK_OVERFLOW          : excCode = EXCEPTION_STACK_OVERFLOW        ; break ;
			case ExcErr_INVALID_DISPOSITION     : excCode = EXCEPTION_INVALID_DISPOSITION   ; break ;
			case ExcErr_GUARD_PAGE              : excCode = EXCEPTION_GUARD_PAGE            ; break ;
			case ExcErr_INVALID_HANDLE          : excCode = EXCEPTION_INVALID_HANDLE        ; break ;
		}
		msg = systemExceptionText( excCode ) ;
	}
	else
	if( code & Facility_Errno )
	{
		int err = code & ~Event_ErrnoErrorFlag;
		msg = strerror( err );
	}
	else
	if( code & Facility_DVB )
	{
		switch ( code )
		{
 			case HNETERR_UnknownErr		:	msg = "Unknown error in the Internet Channel." ; break ;
		}
	}
	else
	if( code & Facility_HNet )
	{
		if( hnetConnection != NULL )
			return hnetConnection->errorCodeAsText( code, buf ) ;
		sprintf( buf, "HNet error %d", code & 0xFFFF ) ;
		return buf ;
	}

	if( msg == NULL )					// non processed codes
	{
		if( Event_IsLog(code) )
			msg = "Unknown log event." ;
		else
		if( Event_IsInfo(code) )
			msg = "Unknown info event." ;
		else
		if( Event_IsWarning(code) )
			msg = "Unknown warning." ;
		else
			msg = "Unknown error." ;
	}
	strcpy( buf, msg ) ;
	return buf ;
}
