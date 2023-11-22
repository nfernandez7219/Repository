#include "tools2.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <excpt.h>
#include "except.hpp"


//------------------------------------------------------------------------
//	facility error functions
//------------------------------------------------------------------------

//
// For Network management errors, (Net*() functions; netapi32.dll; range NERR_BASE..MAX_NERR)
// "netmsg.dll" can be used. Problem is that these errors are not HRESULT's.
//
// Bellow Win32 errors (incl. probably standard COM errors) and MSMQ errors are converted
// to messages.
// At this time behavior of other modules is unknown.
//

// Win32 errors
static char *WIN32errorAsText( DWORD err, char *buf )
{
	DWORD cnt = FormatMessage
	( 
		FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		buf,
		511,
		NULL
	);
	if( cnt <= 0 )
		sprintf( buf, "Unknown Win32 error 0x%X", err ) ;
	else
	{
		// remove trailing EOL's (CF_TEXT format ending with "\r\n\0")
		while( --cnt > 0  &&  (buf[cnt] == '\n'  ||  buf[cnt] == '\r') )
			buf[cnt] = 0 ;
	}
	return buf ;
}


// MSMQ errors
static const char *MSMQ_HRESULTasText( HRESULT err, char *buf )
{
	// Registry: Local machine/Software/System/CurrentControlSet/Services/EventLog/Application/MSMQ
	const char *dllFile = TEXT("mqutil.dll") ;

	HMODULE hModule = LoadLibraryEx( dllFile, NULL, LOAD_LIBRARY_AS_DATAFILE );
	if( hModule == NULL )
	{
		sprintf( buf, "MSMQ error 0x%X (message dll %s could not be loaded)", err, dllFile ) ;
		return buf ;
	}

	DWORD cnt = FormatMessage(
		FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
		hModule,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		buf,
		511,
		NULL
	);
	if( cnt > 0 )
	{
		// remove trailing EOL's (CF_TEXT format ending with "\r\n\0")
		while( --cnt > 0  &&  (buf[cnt] == '\n'  ||  buf[cnt] == '\r') )
			buf[cnt] = 0 ;
	}

	FreeLibrary(hModule);
	return cnt > 0 ? buf : NULL ;
}

// application errors - will be replaced in the future
static const char* appErrorAsText( int module, DWORD code, char *buf )
{
	sprintf( buf, "Unknown error %d in module %d", code, module ) ;
	return buf ;
}


//------------------------------------------------------------------------
//	HRESULTasText()
//------------------------------------------------------------------------


static char *facilityNames[] = {
	"0",			// 0
	"RPC",			// 1
	"DISPATCH",		// 2
	"STORAGE", 		// 3
	"ITF", 			// 4
	"5",
	"6",
	"WIN32",		// 7
	"WINDOWS", 		// 8
	"SSPI",    		// 9
	"CONTROL", 		// 10
	"CERT",			// 11
	"INTERNET",		// 12
	"MEDIASERVER",	// 13
	"MSMQ",			// 14
	"SETUPAPI",		// 15
} ;
#define n_facilities (sizeof(facilityNames) / sizeof(char*))


#define APPMODULE_FROM_HRESULT(hr)		((((hr) & 0xFFFF) >> 10) & 0x3F)
#define APPERROR_FROM_HRESULT(hr)		((hr) & 0x3FF)

const char *HRESULTasText( HRESULT res, char *buf )
{
	DWORD fac = HRESULT_FACILITY(res) ;
	DWORD cod = HRESULT_CODE(res) ;
	if( cod == 0 )
	{
		strcpy( buf, "Success" ) ;
		return buf ;
	}

	const char *ret ;
	switch( fac )
	{
		case FACILITY_WIN32 :
			return WIN32errorAsText( cod, buf ) ;
		case FACILITY_ITF :
		{
			DWORD module = APPMODULE_FROM_HRESULT( res ) ;
			DWORD err    = APPERROR_FROM_HRESULT ( res ) ;
			if( module == 0 )
			{
				strcpy( buf, "(COM error) " ) ;
				return WIN32errorAsText( err, buf+strlen(buf) ) ;
			}
			else
			{
				return appErrorAsText( module, err, buf ) ;
			}
			break ;
		}
		case FACILITY_CONTROL :
			sprintf( buf, "Error %d (Specific to the control returning this error.)", cod ) ;
			return buf;
		case FACILITY_MSMQ :
			ret = MSMQ_HRESULTasText( res, buf ) ;
			if( ret != NULL )
				return ret ;
	}

	// default processing
	if( SUCCEEDED(res) )
	{
		strcpy( buf, "Success" ) ;
		return buf ;
	}

	if( fac < n_facilities )
		sprintf( buf, "Unknown error 0x%X (Facility %s)", res, facilityNames[fac] ) ;
	else
		sprintf( buf, "Unknown error 0x%X", cod ) ;
	return buf ;
}


//------------------------------------------------------------------------
//	other global functions
//------------------------------------------------------------------------


char *systemExceptionText( DWORD code )
{
	switch( code )
	{
		case EXCEPTION_ACCESS_VIOLATION			: return "attempt to read/write to non-accessible memory";
		case EXCEPTION_BREAKPOINT				: return "breakpoint encountered";
		case EXCEPTION_DATATYPE_MISALIGNMENT	: return "attempt to read misaligned data";
		case EXCEPTION_SINGLE_STEP				: return "trace exception";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED	: return "array bounds exceeded";
		case EXCEPTION_FLT_DENORMAL_OPERAND		: return "floating operand too small";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO		: return "(float) zero division";
		case EXCEPTION_FLT_INEXACT_RESULT		: return "inexact result for float operation";
		case EXCEPTION_FLT_INVALID_OPERATION	: return "floating point case EXCEPTION";
		case EXCEPTION_FLT_OVERFLOW				: return "floating point overflow";
		case EXCEPTION_FLT_STACK_CHECK			: return "stack over/underflow";
		case EXCEPTION_FLT_UNDERFLOW			: return "floating point underflow";
		case EXCEPTION_INT_DIVIDE_BY_ZERO		: return "(int) zero division";
		case EXCEPTION_INT_OVERFLOW				: return "integer overflow";
		case EXCEPTION_PRIV_INSTRUCTION			: return "attempt to execute privileged instruction";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION	: return "noncontinuable exception";
	}
	return "unknown exception" ;
}


char *getLastSystemError( char *buf )
{
	DWORD lastError = GetLastError ();
	return WIN32errorAsText( lastError, buf ) ;
}


void HRESULTmessageBox( HRESULT hr, const char *fmt,... )
{
	char buf[1024] ;

	va_list args ;
	va_start( args, fmt ) ;
	int len = vsprintf( buf, fmt, args ) ;
	buf[len++] = '\n' ;
	buf[len++] = '\n' ;
	strcpy( buf+len, "System message:" ) ;

	len = strlen(buf) ;
	buf[len++] = '\n' ;
	HRESULTasText( hr, buf+len ) ;

	#ifndef NO_MFC
	AfxMessageBox( buf ) ;
	#else
	char szAppPath[_MAX_PATH] ;
	char szAppName[_MAX_FNAME] ;
	GetModuleFileName( NULL, szAppPath, _MAX_PATH ) ;
	_splitpath( szAppPath, NULL, NULL, szAppName, NULL ) ;
	::MessageBox( NULL, buf, szAppName, MB_OK | MB_ICONEXCLAMATION ) ;
	#endif
}


void catchAndSaySystemException( DWORD code )
{
	char buf[1024] ;
	sprintf( buf, "System exception:\n%s", systemExceptionText(code) ) ;
	::MessageBox( NULL, buf, "!", MB_OK | MB_ICONERROR | MB_TASKMODAL ) ;
}
