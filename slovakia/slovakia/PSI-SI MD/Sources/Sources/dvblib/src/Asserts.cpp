#include "tools2.hpp"
#include "DvbError.hpp"
#include "MfxGlobals.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//---------------------------------------------------------------------------
//	logging _CrtDbgReport() calls
//---------------------------------------------------------------------------


static BOOL __bIgnoreWarnings=TRUE;

static int __cdecl myCrtReportHook(int nRptType, char *szMsg, int* pResult)
{
	// get only reduced path "\projectdir\src\file"
	static char lastBuff[1024] = "";
	char buff[1024], *ptr, *msg=szMsg, *p;

	*pResult = 0;		// 1 = debug break
	if( __bIgnoreWarnings && nRptType ==_CRT_WARN )
		return FALSE;	// use original report hook

	if( (ptr = strchr( szMsg, '(' )) != NULL )
	{
		int iCount = 0;
		for( p=ptr-1; p>=szMsg; p-- )
		{
			if( *p=='\\' && ++iCount==3 )
			{
				msg = p;
				break;
			}
		}
	}
	while( *msg==10 || *msg==13 || *msg==' ' || *msg=='\t' )
		msg++;

	strcpy( buff, msg );
	for( p=buff+strlen(buff)-1; p>=buff; p-- )
		if( *p==13 || *p==10 )
			*p = p[1]==0 ? 0 : ' ';

	if( !lastBuff[0] || strcmp( buff, lastBuff ) )
	{
		// send message
		strcpy( lastBuff, buff );
		switch( nRptType )
		{
			case _CRT_ASSERT: MfxPostMessage( DVB_Assert,		0, buff ); break ;
			case _CRT_WARN  : MfxPostMessage( DVB_AssertWarning,0, buff ); break ;
			case _CRT_ERROR : MfxPostMessage( DVB_AssertError,  0, buff ); break ;
		}
	}
	return TRUE ;		// report handled
}


void MfxSetAssertHandling( BOOL bIgnoreWarnings )
{
	__bIgnoreWarnings = bIgnoreWarnings;
	_CrtSetReportHook( myCrtReportHook ) ;
}
