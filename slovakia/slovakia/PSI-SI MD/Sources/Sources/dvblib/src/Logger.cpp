/*
 *	Filename:		logger.cpp
 *
 *	Version:		1.00
 *
 *	Description: support for log windows
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"

#include "logger.hpp"
#include "BaseCfg.hpp"
#include "MfxGlobals.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DELNUMITEMS		200
#define MAXNUMITEMS		1000


//-----------------------------------------------------------------------------
//	app context
//	(e.g. volume transferred via a channel in 1 day)
//-----------------------------------------------------------------------------


// Context strings store info about log files between consecutive runs of server/receiver.

static void MfxSetContextString( const char *key, const char *string )
{
	CfgContext	*cfg = MfxContext();
	ASSERT( cfg != NULL ) ;
	cfg->set( "context", key, string ) ;
}

static const char *MfxGetContextString( const char *key )
{
	CfgContext	*cfg = MfxContext();
	ASSERT( cfg != NULL ) ;
	return cfg->get( "context", key ) ;
}

void MfxSetChannelContextString( const char *key, const char *string )
{
	CfgContext *cfg = MfxContext();
	ASSERT( cfg != NULL );
	cfg->set( "channels", key, string );
}

const char *MfxGetChannelContextString( const char *key )
{
	CfgContext *cfg = MfxContext();
	ASSERT( cfg != NULL );
	return cfg->get( "channels", key );
}



//------------------------------------------------------------------------------------------
//	ListBase
//------------------------------------------------------------------------------------------


IMPLEMENT_DYNCREATE( ListBase, CListView)

void ListBase::initView( )
{
	GetListCtrl().SetExtendedStyle( LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_INFOTIP );
	GetListCtrl().DeleteAllItems();
	while( GetListCtrl().DeleteColumn( 0 ) );
	createColumns();
} 

BOOL ListBase :: PreCreateWindow (CREATESTRUCT& cs)
{
    if (!CListView::PreCreateWindow (cs))
        return FALSE;

    cs.style &= ~LVS_TYPEMASK;
    cs.style |= LVS_REPORT;
    return TRUE;
}

long ListBase::getItem( int row	 )
{
	ASSERT( row > -1 && row < GetListCtrl().GetItemCount () );
	return GetListCtrl().GetItemData( row );
}

int ListBase::getRow( long value )
{
	LV_FINDINFO	pFindInfo;

	pFindInfo.flags = LVFI_PARAM;
	pFindInfo.lParam= value ;
	return GetListCtrl().FindItem( &pFindInfo );
}

BOOL ListBase::addItem( long lParam )
{
    LV_ITEM lvi;

    lvi.mask	= LVIF_TEXT | LVIF_PARAM; 
    lvi.iItem	= 0; 
    lvi.iSubItem= 0; 
    lvi.pszText = LPSTR_TEXTCALLBACK; 
    lvi.lParam	= (LPARAM)lParam;
    if (GetListCtrl().InsertItem( &lvi ) == -1)
        return FALSE;
	return TRUE;
}

BOOL ListBase::refreshItem( long value )
{
	int index = getRow( value );
	
	if( index < 0 )
		return FALSE;
	GetListCtrl().Update( index );
	return TRUE;
}

void ListBase::refreshAllItems()
{ 
	GetListCtrl().RedrawItems( 0, GetListCtrl().GetItemCount()-1 ); 
	UpdateWindow();
}

BOOL ListBase::deleteItem( long value )
{
	int index = getRow( value );
	
	if( index < 0 )
		return FALSE;
	GetListCtrl().DeleteItem( index );
	return TRUE;
}



//------------------------------------------------------------------------------------------
//	LogEventList
//------------------------------------------------------------------------------------------


IMPLEMENT_DYNCREATE( LogEventList, ListBase )

BEGIN_MESSAGE_MAP( LogEventList, ListBase)
    //ON_WM_CREATE ()
    ON_WM_DESTROY	 ()
    ON_NOTIFY_REFLECT( LVN_GETDISPINFO, OnGetDispInfo )
    ON_NOTIFY_REFLECT( LVN_COLUMNCLICK, OnColumnClick )
END_MESSAGE_MAP()

LogEventList::LogEventList()
{
	logFile_capacity = 0;
	file = NULL;
	numInLogFile = 0;
	savedContext = FALSE;
	numInLogWindow = MAXNUMITEMS;
}

LogEventList::~LogEventList()
{
	if ( !savedContext )
	{
		savedContext = TRUE;
		if ( file )
		{
			char buf[1024];

			sprintf( buf, "%d", numInLogFile );
			MfxSetContextString( "NumOfEntriesInLog", buf );

			fclose( file );
			file = NULL;
		}
	}
}

void LogEventList::clearLog()
{
	deleteAllItems();
	_makeBakLog( ) ;
}


//-----------------------
//	Listbox support
//-----------------------


void LogEventList::createColumns( )
{
    GetListCtrl().InsertColumn (0, "Time",		LVCFMT_LEFT, 100 );
	GetListCtrl().InsertColumn (1, "Channel Id",LVCFMT_LEFT, 70 );
	GetListCtrl().InsertColumn (2, "Channel",	LVCFMT_LEFT, 80 );
    GetListCtrl().InsertColumn (3, "State",		LVCFMT_LEFT, 80  );
	GetListCtrl().InsertColumn (4, "Event",		LVCFMT_LEFT, 455 );
}

#define COMPARE_NUM(i1, i2)	 (( i1 > i2) ? -1 : (( i1 < i2) ? 1 :0))	
static int CALLBACK CompareLogEventList(LPARAM lParam1, LPARAM lParam2, LPARAM sortType )
{
	LogEventStruct *event1 = (LogEventStruct*)lParam1;
	LogEventStruct *event2 = (LogEventStruct*)lParam2;

	switch( sortType )
	{
	case 0:
		return COMPARE_NUM( event1->m_timeNow,event2->m_timeNow);
	case 1:
		return COMPARE_NUM( event1->m_channelID, event2->m_channelID );
	case 2:
		return stricmp( event1->m_channelName, event2->m_channelName );
	case 3:
		return COMPARE_NUM( event1->m_state, event2->m_state );
	default:
		return stricmp( event1->m_string, event2->m_string   );
	}	
}

void LogEventList :: OnColumnClick( LPNMHDR pnmhdr, LRESULT *pResult)
{
    int          code = pnmhdr->code;
    NM_LISTVIEW *pnmtv= (NM_LISTVIEW FAR *)pnmhdr; 
    
    if( code == LVN_COLUMNCLICK )
        GetListCtrl().SortItems( CompareLogEventList, long(pnmtv->iSubItem));
   *pResult = 0;
}

void LogEventList :: OnGetDispInfo (NMHDR* pnmh, LRESULT* pResult)
{
    CString string;
    LV_DISPINFO* plvdi = (LV_DISPINFO*) pnmh;

    if (plvdi->item.mask & LVIF_TEXT)
	{
    	char *txt = getText( plvdi->item.iSubItem, plvdi->item.lParam );
        ::lstrcpy( plvdi->item.pszText, (LPCTSTR)txt );
	}
}

void LogEventList :: OnDestroy()
{
	deleteAllItems();
	ListBase :: OnDestroy();
}

char *LogEventList::getText( int col, long item )
{
	LogEventStruct *event = (LogEventStruct*)item;

	switch( col )
	{
	case 0:
		return (char*)(const char*)event->m_timeStr;
	case 1:
		return (char*)(const char*)event->m_channelIDStr;
	case 2:
		return (char*)(const char*)event->m_channelName;
	case 3:
		return (char*)(const char*)event->m_state;
	default:
		return (char*)(const char*)event->m_string;
	}	
}  


//-----------------------
//	add/delete item
//-----------------------


void LogEventList::deleteNItems( int num )
{
    int nCount = GetListCtrl().GetItemCount ();
	numInLogWindow -= num;
    for( int i = nCount - 1; i > numInLogWindow; i-- )
	{
		LogEventStruct *str = (LogEventStruct*)GetListCtrl().GetItemData( i );
		delete str;
		GetListCtrl().DeleteItem( i );
	}
}

void LogEventList :: deleteAllItems ()
{
	CListCtrl &lst = GetListCtrl() ;
    int nCount = lst.GetItemCount ();
    if( nCount != 0 )
	{
		 int i ;

		// Collect all events stored in the list control
		LogEventStruct **events = (LogEventStruct **)_alloca( nCount*sizeof(LogEventStruct*) ) ;
		for( i=0; i<nCount; i++ )
			events[i] = (LogEventStruct*)lst.GetItemData( i );

		// Clean list control
		lst.DeleteAllItems();

		// Destroy events
		// (Must be done after DeleteAllItems() as otherwise display mayt interfere.)
		for( i=0; i<nCount; i++ )
			delete events[i] ;
	}
	numInLogWindow = 0;
}


BOOL LogEventList::addLogerItem( ushort channelID, CString status, const char *stri, time_t *setTime )
{
	LogEventStruct *str = new LogEventStruct( channelID, status, stri, setTime );	
	return addLogerItem( str );
}


BOOL LogEventList::addLogerItem( LogEventStruct *str )
{
	if( addItem(long(str)) )
	{
		_saveFirstToLogFile();
		return TRUE;
	}
	delete str;
	return FALSE;
}


BOOL LogEventList::deleteLogerItem( long lParam )
{
	int row = getRow( lParam );

	if( row == -1 )
		return FALSE;
	LogEventStruct *str = (LogEventStruct*)GetListCtrl().GetItemData( row );
	if( deleteItem(lParam) )
	{
		delete str;
		return TRUE;
	}
	return FALSE;
}


//-----------------------
//	private utilities
//-----------------------


void LogEventList::_testLogFileCapacityLimit( )
{
	if( logFile_capacity > 0 )		// otherwise unlimited capacity
	{
		if ( numInLogFile >= logFile_capacity )
			_makeBakLog( ) ;
	}
}

#define CANTMAKELOGFILE_MSG	"Can not create log file %s.\nPlease correct the path in the setup dialog or in the config file."

void LogEventList::_makeBakLog( const char *newFileName )
{
	if ( file )
		fclose( file );

	char buf1[1024], buf2[1024];
	strcpy( buf1, logFile );
	strcat( buf1, ".old"  );
	strcpy( buf2, buf1	  );
	strcat( buf2, ".old"  );
	copyFile( buf1, buf2  );
	copyFile( logFile, buf1 );

	if( newFileName != NULL )
		logFile = newFileName ;

	file = fopenRetry( logFile, "wt", sFILESHARE_READ );
	if ( !file )
		stdWarningDialog( CANTMAKELOGFILE_MSG, logFile );
	numInLogFile = 0;
}

void LogEventList::_saveOneItem( LogEventStruct *log )
{
	if( log != NULL )
	{
		fprintf( file, "%-6s  %-15s   %s   %s\n", log->m_channelIDStr, log->m_channelName, log->m_timeStr, log->m_string );
		fflush( file );
		_commit( _fileno( file ) );//fflush( file );
		numInLogFile++;
		numInLogWindow++;
	}
}

void LogEventList::_saveFirstToLogFile()		// server
{
	int	num = GetListCtrl().GetItemCount();

	if( logFile.IsEmpty() || num <= 0 || !file )
		return;

	LogEventStruct *log = (LogEventStruct*)GetListCtrl().GetItemData( 0 );
	_saveOneItem( log ) ;

	if( numInLogWindow >= MAXNUMITEMS )
		deleteNItems( DELNUMITEMS );

	if( logFile_capacity > 0 )
	{
		if( num >= logFile_capacity )
			deleteAllItems();
		_testLogFileCapacityLimit( ) ;
	}
}


//-----------------------
//	Logfile
//-----------------------


void LogEventList::defineLogFile( const char *fileName, int numItems )
{
	logFile_capacity = numItems;
	if( file != NULL  &&  (fileName==NULL || stricmp(logFile,fileName)==0) )
		return ;

	if ( fileName )
		logFile = fileName;
	MfxMakeFullAppPath( logFile ) ;

	if ( file )
		fclose( file );

	file = fopenRetry( logFile, "at", sFILESHARE_READ );
	if ( !file )
		stdWarningDialog( CANTMAKELOGFILE_MSG, logFile );

	const char *buf = MfxGetContextString( "NumOfEntriesInLog" );
	if ( buf == NULL  ||  sscanf( buf, "%d", &numInLogFile ) != 1 )
	{
		numInLogFile = 0;
		//throwException();
	}
}

void LogEventList::changeLogFile( const char *fileName, int numItems )
{
	logFile_capacity = numItems;
	if( fileName )
	{
		if ( !logFile.CompareNoCase( fileName ) )
		{
			_testLogFileCapacityLimit() ;
			if ( GetListCtrl().GetItemCount() >= logFile_capacity && logFile_capacity != 0 )
				deleteAllItems();
		}
		else
		{
			_makeBakLog( fileName ) ;
			deleteAllItems();
		}
	}
}

void LogEventList::saveToLogFile( LogEventStruct *log )		// server
{
	if( logFile.IsEmpty() || !file )
		return;

	_saveOneItem( log ) ;

	if( numInLogWindow >= MAXNUMITEMS )
		deleteNItems( DELNUMITEMS );

	_testLogFileCapacityLimit() ;
}

void LogEventList::saveToLogFile( ushort channelID, CString status, const char *stri, time_t *setTime )		// server
{
	LogEventStruct log( channelID, status, stri, setTime );
	saveToLogFile( &log ) ;
}


//------------------------------------------------------------------------------------------
//	ErrorList
//------------------------------------------------------------------------------------------


IMPLEMENT_DYNCREATE( ErrorList, LogEventList )

BEGIN_MESSAGE_MAP( ErrorList, LogEventList)
    //ON_WM_CREATE ()
    ON_NOTIFY_REFLECT( LVN_GETDISPINFO, OnGetDispInfo )
    ON_NOTIFY_REFLECT( LVN_COLUMNCLICK, OnColumnClick )
END_MESSAGE_MAP()

ErrorList::~ErrorList()
{
	if ( !savedContext )
	{
		savedContext = TRUE;

		if ( file )
		{
			char buf[1024];

			sprintf( buf, "%d", numInLogFile );
			MfxSetContextString( "NumOfEntriesInErr", buf );

			fclose( file );
			file = NULL;
		}
	}
}

void ErrorList::defineLogFile( const char *fileName, int numItems )
{
	logFile_capacity = numItems;
	if( file != NULL  &&  (fileName==NULL || stricmp(logFile,fileName)==0) )
		return ;

	if ( fileName )
		logFile = fileName;
	MfxMakeFullAppPath( logFile ) ;

	if ( file )
		fclose( file );

	file = fopenRetry( logFile, "at", sFILESHARE_READ );
	if ( !file )
		stdWarningDialog( CANTMAKELOGFILE_MSG, logFile );

	const char *buf = MfxGetContextString( "NumOfEntriesInErr" );
	if( buf == NULL  ||  sscanf( buf, "%d", &numInLogFile ) != 1 )
	{
		numInLogFile = 0;
		//throwException();
	}

}

void ErrorList::createColumns( )
{
    GetListCtrl().InsertColumn (0, "Time",		LVCFMT_LEFT, 100 );
	GetListCtrl().InsertColumn (1, "Channel Id",LVCFMT_LEFT, 70 );
	GetListCtrl().InsertColumn (2, "Channel",	LVCFMT_LEFT, 80 );
	GetListCtrl().InsertColumn (3, "Error",		LVCFMT_LEFT, 535 );
}

static int CALLBACK CompareErrorList(LPARAM lParam1, LPARAM lParam2, LPARAM sortType )
{
	LogEventStruct *event1 = (LogEventStruct*)lParam1;
	LogEventStruct *event2 = (LogEventStruct*)lParam2;

	switch( sortType )
	{
	case 0:
		return COMPARE_NUM( event1->m_timeNow,event2->m_timeNow);
	case 1:
		return COMPARE_NUM( event1->m_channelID, event2->m_channelID );
	case 2:
		return stricmp( event1->m_channelName, event2->m_channelName );
	default:
		return stricmp( event1->m_string, event2->m_string   );
	}	
}

void ErrorList :: OnColumnClick( LPNMHDR pnmhdr, LRESULT *pResult)
{
    int          code = pnmhdr->code;
    NM_LISTVIEW *pnmtv= (NM_LISTVIEW FAR *)pnmhdr; 
    
    if( code == LVN_COLUMNCLICK )
        GetListCtrl().SortItems( CompareErrorList, long(pnmtv->iSubItem));
   *pResult = 0;
}

void ErrorList :: OnGetDispInfo (NMHDR* pnmh, LRESULT* pResult)
{
    CString string;
    LV_DISPINFO* plvdi = (LV_DISPINFO*) pnmh;

    if (plvdi->item.mask & LVIF_TEXT)
	{
    	char *txt = getErrorText( plvdi->item.iSubItem, plvdi->item.lParam );
        ::lstrcpy( plvdi->item.pszText, (LPCTSTR)txt );
	}
}

char *ErrorList::getErrorText( int col, long item )
{
	LogEventStruct *event = (LogEventStruct*)item;

	switch( col )
	{
	case 0:
		return (char*)(const char*)event->m_timeStr;
	case 1:
		return (char*)(const char*)event->m_channelIDStr;
	case 2:
		return (char*)(const char*)event->m_channelName;
	default:
		return (char*)(const char*)event->m_string;
	}	
}  
