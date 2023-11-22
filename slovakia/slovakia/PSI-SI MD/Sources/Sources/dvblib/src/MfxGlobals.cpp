/*
 *	Filename:		mfxglobals.cpp
 *
 *	Version:		1.00
 *
 *	Description:
 *		DvbGlobalId class implementation
 *		global routines for DDX/DDV support
 *		other Mfx*() utilities
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"

#include "FileIO.hpp"
#include "MfxGlobals.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
Discussion of the time measurement
----------------------------------

A.) time() has resolution of 1 sec and is very slow

B.) Functions using normal system timer.
		GetSystemTimeAsFileTime()	fast	time since Jan 1, 1601
		GetTickCount()				fast	# of system timer ticks since system started
		clock()						~slow	# of timer ticks since current process started
		GetLocalTime()				slow	time since Jan 1, 1601
	These functions use system dependent resolution - in the measured case 10 msecs.

C.) Functions using high resolution (HR) timer
		QueryPerformanceCounter()	slow	# of HR timer ticks
	It is not guarantied that high resolution timer is available on the system.
	The resolution is system dependent, but certainly much more exact than B).

Function benchmarks (for 1000000 calls):
	GetTickCount()					0.03 sec
	clock()							0.41 sec
	GetSystemTimeAsFileTime()		0.05 sec
	GetLocalTime()					2.34 sec
	time()							5.05 sec
	QueryPerformanceCounter()		4.80 sec
*/


//-----------------------------------------------------------------------------
//	time utilities
//-----------------------------------------------------------------------------


// loc_time = sys_time + adjustment
static LONGLONG getSysToLocalTimeAdjustment()
{
	FILETIME		ft_sys;
	GetSystemTimeAsFileTime( &ft_sys );

	FILETIME		ft_loc;
	SYSTEMTIME		st;
	GetLocalTime( &st );
	SystemTimeToFileTime( &st, &ft_loc );

	// Convert to LONGLONG
	LARGE_INTEGER	t_loc;
	t_loc.LowPart	= ft_loc.dwLowDateTime;
	t_loc.HighPart	= ft_loc.dwHighDateTime;

	LARGE_INTEGER	t_sys;
	t_sys.LowPart	= ft_sys.dwLowDateTime;
	t_sys.HighPart	= ft_sys.dwHighDateTime;

	LONGLONG	dt;
	dt = t_loc.QuadPart - t_sys.QuadPart ;
	return dt ;
}

// time = sys_time + adjustment
static LONGLONG getSysTimeToTimeAdjustment()
{
	time_t timeNow ;
	time( &timeNow ) ;

	FILETIME		ft_sys;
	GetSystemTimeAsFileTime( &ft_sys );

	LARGE_INTEGER	t_sys;
	t_sys.LowPart	= ft_sys.dwLowDateTime;
	t_sys.HighPart	= ft_sys.dwHighDateTime;

	LONGLONG timeNow_long = (LONGLONG)timeNow * 10000000 ;
	LONGLONG dt = timeNow_long - t_sys.QuadPart ;
	return dt ;
}

// Uses system timer (10 msecs resolution)
static void g_getLocalTime( LONGLONG &tim )	// units = 100 nanosecs [10^-7 sec]
{
	FILETIME 		ft;
	GetSystemTimeAsFileTime( &ft );

	LARGE_INTEGER tt ;
	tt.LowPart	= ft.dwLowDateTime;
	tt.HighPart = ft.dwHighDateTime;

	static LONGLONG adj ;
	static BOOL     adjustmentComputed=FALSE ;
	if( !adjustmentComputed )
	{
		adj = getSysToLocalTimeAdjustment() ;
		adjustmentComputed = TRUE ;
	}
	tt.QuadPart += adj ;

	tim = tt.QuadPart ;
}


//-----------------------------------------------------------------------------
//	EventQueue
//-----------------------------------------------------------------------------


EventQueue *EventQueue::first = NULL;
EventQueue *EventQueue::last  = NULL;
CRITICAL_SECTION EventQueue::modifyEventQueueLock;
BOOL bProcessMfxMessages	  = TRUE ;


EventQueue::EventQueue( UINT m, long wp, long lp )
{
	msg = m;
	wParam = wp;
	lParam = lp;
	isString = FALSE;

	next = NULL;
	if( last == NULL )
	{
		first = last = this;
	}
	else
	{
		last->next = this;
		last = this;
	}
	g_getLocalTime( tim ) ;
}

time_t EventQueue::setTime()
{
	static LONGLONG adj ;
	static BOOL     adjustmentComputed=FALSE ;
	if( !adjustmentComputed )
	{
		adj = getSysTimeToTimeAdjustment() - getSysToLocalTimeAdjustment() ;
		adjustmentComputed = TRUE ;
	}

	return (tim + adj) /10000000 ;
}


//-----------------------------------------------------------------------------
//	messages
//-----------------------------------------------------------------------------


#define LOCK()		EnterCriticalSection( &EventQueue::modifyEventQueueLock )
#define UNLOCK()	LeaveCriticalSection( &EventQueue::modifyEventQueueLock )


BOOL MfxPostMessage( UINT msg, long wParam, long lParam )
{
	if( !bProcessMfxMessages )
		return TRUE ;

	LOCK() ;
	/*
	if( msg == EMsg_InboxSpeed  &&  EventQueue::last != NULL )
	{
		EventQueue *ev = (EventQueue::last) ;
		if( ev->msg == EMsg_InboxSpeed  &&  ev->wParam == wParam )
		{
			// Merge subsequent channel speed messages
			// wParam=channel, lParam=packets
			ev->lParam += lParam ;
			ev->set_time() ;
			UNLOCK() ;
			return TRUE;
		}
	}
	*/

	new EventQueue( msg, wParam, lParam );

	UNLOCK() ;
	return TRUE;
}

// msg code, long, message text
BOOL MfxPostMessage( UINT msg, long wParam, const char *lParam )
{
	if( !bProcessMfxMessages )
		return TRUE ;

	LOCK() ;

	long l = (long)STRDUP(lParam);
	EventQueue *ev = new EventQueue( msg, wParam, l );
	ev->isString = TRUE ;

	UNLOCK() ;
	return TRUE;
}

EventQueue *MfxGetMsg()
{
	LOCK() ;
	if ( EventQueue::first == NULL )
	{
		UNLOCK() ;
		return NULL;
	}
	EventQueue *p = EventQueue::first;
	if ( EventQueue::first == EventQueue::last )
		EventQueue::first = EventQueue::last = NULL;
	else
		EventQueue::first = EventQueue::first->next;
	UNLOCK() ;
	return p;
}


//-----------------------------------------------------------------------------
//	global id
//-----------------------------------------------------------------------------


#include <sys/timeb.h>

ushort DvbGlobalId::_initialized = 0 ;
ushort DvbGlobalId::_cnt = 0 ;


void DvbGlobalId::create( )
{
	if( !_initialized )
	{
		struct timeb tb;
		ftime( &tb );
		srand( tb.millitm ) ;
		_cnt = rand( ) ;
		_initialized = 1 ;
	}
	if( ++_cnt == 0 )
		++_cnt ;
	cnt = _cnt ;
	::time( &tim ) ;
}


//-----------------------------------------------------------------------------
//	utilities
//-----------------------------------------------------------------------------


char *MfxGetFullPath( const char *relPath, char *path )
{
	char drive[20], dir[1024] ;
	GetModuleFileName( NULL, path, 1024 ) ;
	_splitpath( path, drive, dir, NULL, NULL ) ;

	_makepath( path, drive, dir, relPath, NULL ) ;
	return path ;
}

void MfxMakeFullAppPath( CString &path )
{
	if( path.GetAt( 1 ) != ':' && path.GetAt( 1 ) != '\\' )
	{
		char pth[1024], drive[20], dir[1024] ;
		GetModuleFileName( NULL, pth, 1024 ) ;
		_splitpath( pth, drive, dir, NULL, NULL ) ;
		_makepath ( pth, drive, dir, path, NULL ) ;
		path = pth;
	}
}

BOOL MfxOpenFileDialog( CWnd *wnd, CString &path )
{
    static char szFilter[] = "Executable Files (*.exe)|*.exe|All Files (*.*)|*.*||";
    static char defExt[]   = "exe";

	MfxMakeFullAppPath( path );

	char *ext = strrchr( path, '\\' );

	if( ext )
	{
		*ext = 0;
		makeDir( path );
		*ext = '\\';
	}
	else
		makeDir( path );

    CFileDialog fd( TRUE, defExt, path, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter );
	if( fd.DoModal() == IDOK )
    { 
		path = fd.GetPathName();
		return TRUE;
    }
	return FALSE;
}

char *getAllocatedFullPath( const char *path )
{
	if( path[1] != ':' && path[1] != '\\' )
	{
		char pth[1024], drive[20], dir[1024] ;
		GetModuleFileName( NULL, pth, 1024 ) ;
		_splitpath( pth, drive, dir, NULL, NULL ) ;
		_makepath ( pth, drive, dir, path, NULL ) ;
		return STRDUP( pth );
	}
	return STRDUP( path );
}

int getAppVersion( char *str )
{
	static char ver_string[30] ;
	static int  ver=-1 ;
	if( ver == -1 )
	{
		if( !getFileVersion( NULL, ver_string, UPPER_VERSION) )
		{
			ver = 0 ;
			ver_string[0] = 0 ;
		}
	}
	if( str != NULL )
		strcpy( str, ver_string ) ;
	return ver ;
}


//-----------------------------------------------------------------------------
//	data exchange
//-----------------------------------------------------------------------------


#define IS_SPACE(x)  (x==' ' || x=='\t')
void sDDX_Text( CDataExchange *pDX, UINT id, char *txt, int maxChar, int flag )
{
	CString	str;

	if( pDX->m_bSaveAndValidate )
	{
		DDX_Text( pDX, id, str );

		txt[0] = 0;
		if( str.IsEmpty() )
		{
			if( flag&sDDX_FLAG_NOEMPTY )
				ExchangeException( pDX, id, "Field has to be filled." );
		}
		else
		{
			const char *beg = str;

			//copy without space
			while( IS_SPACE( *beg ) )	beg++;
			for( int len=strlen(beg); len>1 && IS_SPACE(beg[len-1]);)
				 len--;
			
			if( maxChar && len >= maxChar )
				len  = maxChar-1;
			strncpy( txt, beg, len );
			txt[len] = 0;

			// test for flag
			if( flag&sDDX_FLAG_NUMBER || flag&sDDX_FLAG_NOSPACE )
				for( int i = strlen( txt )-1; i >= 0; i-- )
				{
					if( flag&sDDX_FLAG_NUMBER && !isdigit( txt[i] ) )
						ExchangeException( pDX, id, "Field accepts only numerical characters." );
		
					if( flag&sDDX_FLAG_NOSPACE && ( txt[i] == ' ' || txt[i] == '\t' )) 
						ExchangeException( pDX, id, "No spaces are allowed in the edit field." );
				}
			if( flag&sDDX_FLAG_FIRSTCHARUPPER )
			{
				strlwr( txt );
				txt[0] = toupper( txt[0] );
			}
		}
	}
	else
	{
		str = txt;
		DDX_Text( pDX, id, str );
	}
	if( maxChar )
		DDV_MaxChars( pDX, str, maxChar-1 );
};

void sDDX_Path( CDataExchange *pDX, UINT id, char **txt, int maxChar, int flag )
{
	CString	path;
	char buf[1024] ;

	if( pDX->m_bSaveAndValidate )
	{
		DDX_Text( pDX, id, path );
		if( path.IsEmpty() )
		{
			if( flag&sDDX_FLAG_NOEMPTY )
				ExchangeException( pDX, id, "Field has to be filled." );
		}
		else
		{
			MfxMakeFullAppPath( path );
			if( flag&sDDX_FLAG_FILE && !fileExist( path ) )
			{
				FILE *f = fopenRetry( path, "wt" );
				if( !f )
				{
					sprintf( buf, "Failed to create file\n\t%s", path ) ;
					ExchangeException( pDX, id, buf );
				}
				else
					fclose( f );
			}
			
			if( flag&sDDX_FLAG_DIRECTORY && !dirExist( path ) && makeDir( path ) )
			{
				sprintf( buf, "Failed to create directory\n\t%s", path ) ;
				ExchangeException( pDX, id, buf );
			}
		}
		if( *txt )
			FREE( *txt );
		if( !path.IsEmpty() )
			*txt = STRDUP( path );
		else
			*txt = NULL;
		DDX_Text( pDX, id, path );
	}
	else
	{
		path = *txt;
		DDX_Text( pDX, id, path );
	}
	if( maxChar )
		DDV_MaxChars( pDX, path, maxChar-1 );
};

void sDDX_Int( CDataExchange* pDX, UINT id, int &num, int minVal, int maxVal )
{
	CString	str;
	int		val;	

	if( pDX->m_bSaveAndValidate )
	{
		DDX_Text( pDX, id, str );
		if( str.IsEmpty() )
			val = 0;
		else
			if( !isInt( str, &val ) )
				ExchangeException( pDX, id, "Please insert numerical value" );

		if( val < minVal || val > maxVal ) 
		{
			CString err;
			err.Format( "Out of range.\nValue has to be from the interval %d..%d.", minVal, maxVal );
			ExchangeException( pDX, id, err );
		}
		num = val;
	}
	else
	{
		str.Format( "%d", num );
		DDX_Text( pDX, id, str );
	}
};

void sDDX_Short( CDataExchange* pDX, UINT id, short &num, int minVal, int maxVal )
{
	int val = num;
	sDDX_Int( pDX, id, val, minVal, maxVal );
	num = val;
}

void ExchangeException( CDataExchange* pDX, UINT id, const char *str )
{
	if( str != NULL )
		AfxMessageBox( str );
	pDX->PrepareEditCtrl( id );
	pDX->Fail();
}
