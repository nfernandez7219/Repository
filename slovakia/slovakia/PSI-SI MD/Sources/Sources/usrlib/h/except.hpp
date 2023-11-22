#ifndef __INC_EXCEPT_HPP__
#define __INC_EXCEPT_HPP__

//#include <excpt.h>

char *systemExceptionText( DWORD code ) ;
void catchAndSaySystemException( DWORD code ) ;

#define CATCH_AND_SAY_SYSTEM_EXCEPTION		\
	__except( EXCEPTION_EXECUTE_HANDLER )	{ catchAndSaySystemException( GetExceptionCode()) ; }

// usage:
//	__try
//	{
//		...
//	}
//	CATCH_AND_SAY_SYSTEM_EXCEPTION		// this displays error report

// fills text corresp. to GetLastError() into <buf> and returns <buf>
char *getLastSystemError( char *buf ) ;

const char *HRESULTasText( HRESULT res, char *buf ) ;
void HRESULTmessageBox( HRESULT hr, const char *fmt,... ) ;


#endif
