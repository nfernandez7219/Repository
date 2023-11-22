#include "tools2.hpp"
#include "loadRes.hpp"
#include "except.hpp"
#include "screen.msg"		// Screen string table used to minimize need of app.msg in higher libs

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static const char *getStr( int id )
{
	const char *s = SCRMSG(id) ;
	if( *s != '?' )
		return s ;
	// english defaults
	switch( id )
	{
		case SCRMSG_DlgFailed		: s = "The dialog failed" ; break ;
		case SCRMSG_DlgFailedUnknown: s = "Unknown error during loading of the dialog." ; break ;
		case SCRMSG_ResLoadFailed	: s = "Failure at loading the resource" ; break ;
		case SCRMSG_NativeDllFailed	: s = "Load of native resource DLL failed." ; break ;
		case SCRMSG_DefaultDllLoaded: s = "Default (english) version loaded." ; break ;
		case SCRMSG_LoadResDllFailed: s = "Load of resource DLL failed" ; break ;
		case SCRMSG_CheckWhereIsDll : s = "\n\nCheck whether you have this DLL and whether it is located\nin %s directory\nor in the PATH.\n\nDll version can be checked e.g. via Windows Explorer (File Properties)." ; break ;
		case SCRMSG_BadDllVersion	: s = "The DLL has incorrect version: '%s'\n('%s' or higher expected.)" ; break ;
		case SCRMSG_LoadError		: s = "Load error." ; break ;
		case SCRMSG_NoDllVersion	: s = "DLL version missing." ; break ;
	}
	return s ;
}


//-----------------------------------------------------------------------------
//	ResDllDesc
//-----------------------------------------------------------------------------


ResDllDesc::ResDllDesc( const char *module_name, HMODULE h )
{
	if( module_name == NULL )
	{
		_moduleName[0] = 0 ;
		_useDLL = 0 ;
	}
	else
	{
		strncpy( _moduleName, module_name, sizeof(_moduleName)-1 ) ;
		_moduleName[sizeof(_moduleName)-1] = 0 ;
	}
	strcpy( _libname, _moduleName ) ;

	_versionCheckRequired= 0 ;
	_minVersion[12]	= 0 ;
	_mod			= h ;
	_country[0]		= 0 ;
	_version[0]		= 0 ;
	_useDLL			= 0 ;
	_useGlbHelp		= 0 ;
	_helpFile		= NULL ;

}

ResDllDesc::ResDllDesc( const char *module_name, const char *min_version, BOOL version_check_required,
					   BOOL useGlobalHelpFile )
{
	if( module_name == NULL )
	{
		_moduleName[0] = 0 ;
		_useDLL = 0 ;
	}
	else
	{
		strncpy( _moduleName, module_name, sizeof(_moduleName)-1 ) ;
		_moduleName[sizeof(_moduleName)-1] = 0 ;
		_useDLL = 1 ;
	}
	if( min_version == NULL  ||  !version_check_required )
		_minVersion[0] = 0 ;
	else
	{
		ASSERT( min_version ) ;
		strncpy( _minVersion, min_version, sizeof(_minVersion)-1 ) ;
		_minVersion[sizeof(_minVersion)-1] = 0 ;
	}
	_versionCheckRequired = version_check_required ;
	_country[0] = 0 ;
	_mod		= 0 ;
	_version[0] = 0 ;
	_libname[0] = 0 ;
	_useGlbHelp = useGlobalHelpFile ;
	_helpFile   = NULL ;
}


BOOL ResDllDesc::checkVersion( const char *actVersion )
{
	float ver, min_ver ;
	if( !isFloat(_minVersion, &min_ver) )
		return FALSE ;
	if( !isFloat( actVersion, &ver) )
		return FALSE ;
	return ver > min_ver - 0.0001 ;
}


BOOL ResDllDesc::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
    UINT         hCtrl= pNMHDR->idFrom;		// idFrom is actually the HWND of the tool

	UINT nID=0 ;
    if (pTTT->uFlags & TTF_IDISHWND)
        nID = ::GetDlgCtrlID((HWND)hCtrl);
	else
		nID = pTTT->hdr.idFrom ;

    if( nID == 0 )
		return FALSE ;

	static char buf[150] ;
	loadSafeString( nID, buf, sizeof(buf)-1) ;
	char *s = strchr( buf, '\n' ) ;
	if( s != NULL )
		*s = 0 ;
	pTTT->lpszText = buf ;
	pTTT->hinst = 0;
    return  TRUE ;
}


//-----------------------------------------------------------------------------
//	ResDllDesc::load()
//-----------------------------------------------------------------------------


BOOL ResDllDesc::_loadModule( const char *dir, const char *libName, char *errStr )
{
	_version[0] = 0 ;
	_mod = NULL ;

	if( dir != NULL )
	{
		char path[1024] ;
		_makepath( path, NULL, dir, libName, NULL ) ;
		if( !fileExist(path) )
		{
			return FALSE ;		// no error report; better report will be done later
		}
		_mod = LoadLibrary( path ) ;
	}
	else
		_mod = LoadLibrary( libName ) ;

	char buf[256] ;
	if( _mod == 0 )
	{
		if( errStr[0] == 0 )
			strcpy( errStr, getStr(SCRMSG_LoadError) ) ;
		return FALSE ;
	}

	if( _versionCheckRequired )			// version control required
	{
		try
		{
			getFileVersion( (char*)libName, buf, UPPER_VERSION );
			if( checkVersion(buf) )
			{
				strncpy( _version, buf, sizeof(_version)-1 ) ;
				_version[ sizeof(_version)-1] = 0 ;
			}
			else
				sprintf( errStr, getStr(SCRMSG_BadDllVersion), isStrEmpty(buf)?"(NULL)":buf, _minVersion ) ;
		}
		catch( ... )
		{
			strcpy( errStr, getStr(SCRMSG_NoDllVersion) ) ;
		}
		if( errStr[0] != 0 )
		{
			FreeLibrary( _mod ) ;
			_mod = 0 ;
			return FALSE ;
		}
	}
	return TRUE ;
}


HINSTANCE ResDllDesc::_LoadLibrary( const char *lib_name, char *errStr )
{
	errStr[0] = 0 ;
	sLoginClass *log = getGlbLogin() ;

	// 1. try current program directory
	_loadModule( log->basePath, lib_name, errStr ) ;

	// 2. try CS_DIR
	if( _mod == 0 )
	{
		if( stricmp( log->getSysTskDir(), log->basePath) != 0 )
			_loadModule( log->getSysTskDir(), lib_name, errStr ) ;
	}

	// 3. try standard system directories
	if( _mod == 0 )
		_loadModule( NULL, lib_name, errStr ) ;

	return _mod ;
}


BOOL ResDllDesc::load( const char *country, BOOL say_warning )
{
	char buf[1024] ;
	if( !_useDLL )
		return TRUE ;
	sLoginClass *log = getGlbLogin() ;

	BOOL tryEnglish=FALSE ;
	if( !country  ||  isStrEmpty(country) )
	{
		country = log->language ;
		if( country[0] == 0 )
			country = "E" ;
		if( _mod == NULL  &&  stricmp(country,"E") != 0 )
			tryEnglish = TRUE ;
	}

	HMODULE oldMod = _mod ;
	if( oldMod != NULL )
		if( stricmp( _country, country) == 0 )		// already loaded
			return TRUE ;

	// load DLL
	BOOL ret=TRUE ;
	char oldlibname[100] ;
	strcpy( oldlibname, _libname ) ;
	concat( _libname, _moduleName, "_", country, "_RC.DLL", NULL ) ;

	char  errStr[256] ;
	errStr[0] = 0 ;
	_mod = _LoadLibrary( _libname, errStr ) ;
	if( _mod == NULL )
	{
		if( tryEnglish )
		{
			concat( _libname , _moduleName, "_E_RC.DLL", NULL ) ;
			_mod = _LoadLibrary( _libname, buf ) ;
		}
		if( _mod == NULL )
		{
			int cnt = sprintf( buf,"%s - %s:\n  %s", _libname, getStr(SCRMSG_LoadResDllFailed), errStr ) ;
			if( say_warning )
			{
				sprintf( buf+cnt, getStr(SCRMSG_CheckWhereIsDll), log->getSysTskDir() ) ;
			}
			strcpy( _libname, oldlibname ) ;
			_mod = oldMod ;
			throw Msg( -1, buf ) ;
		}
		else    
		{
			if( say_warning )
				stdWarningDialog( "%s: %s\n(%s)\n%s",
					_libname, getStr(SCRMSG_NativeDllFailed), errStr, getStr(SCRMSG_DefaultDllLoaded) ) ;
			ret = FALSE ;
		}
	}

	// all ok; free old library
	strncpy( _country, country, sizeof(_country)-1 ) ;
	_country[ sizeof(_country)-1] = 0 ;
	if( oldMod != 0 )
		FreeLibrary( oldMod ) ;
	return ret ;
}


//-----------------------------------------------------------------------------
//	ResDllDesc - help
//-----------------------------------------------------------------------------


inline const char *getHelpDir( const char *prgDir, char *buf )
{
	char drv[100], dir[512] ;
	_splitpath( prgDir, drv, dir, NULL, NULL ) ;
	_makepath( buf, drv, dir, "Hlp", NULL ) ;
	return buf ;
}

BOOL ResDllDesc::getHelpFile( const char *baseHelpName, char *hFile )
{
	sLoginClass *log     = getGlbLogin() ;
	const char  *country = log->language ;
	BOOL tryEnglish = (stricmp(country,"E") != 0) ;

	BOOL hasHelp = FALSE ;

	char locHelpDir[512] ;
	getHelpDir( log->basePath, locHelpDir ) ;

	// 1. try local help file
	sprintf( hFile, "%s\\%s_%s.hlp", locHelpDir, baseHelpName, country ) ;
	if( fileExist(hFile) )
		hasHelp = TRUE ;
	else
	{
		char sysHelpDir[512] ;
		getHelpDir( log->getSysTskDir(), sysHelpDir ) ;

		// 2. try system help file
		sprintf( hFile, "%s\\%s_%s.hlp", sysHelpDir, baseHelpName, country ) ;
		if( fileExist(hFile) )
			hasHelp = TRUE ;
		else
		// 3. try english defaults
		if( tryEnglish )
		{
			sprintf( hFile, "%s\\%s_%s.hlp", locHelpDir, baseHelpName, "E" ) ;
			if( fileExist(hFile) )
				hasHelp = TRUE ;
			else
			{
				sprintf( hFile, "%s\\%s_%s.hlp", sysHelpDir, baseHelpName, "E" ) ;
				if( fileExist(hFile) )
					hasHelp = TRUE ;
			}
		}
	}
	return hasHelp ;
}

const char *ResDllDesc::helpFile()
{
	if( _helpFile != NULL )
		return _helpFile ;

	char baseHelpName[80] ;
	strcpy( baseHelpName, _useGlbHelp ? getGlbLogin()->prgName : _moduleName ) ;

	char hFile[512] ;
	if( getHelpFile( baseHelpName, hFile) )
	{
		_helpFile = strdup( hFile ) ;
	}
	return _helpFile;
}


//-----------------------------------------------------------------------------
//	ResDllDesc - specific load functions
//-----------------------------------------------------------------------------

HGLOBAL ResDllDesc::loadResource( WORD id , LPCSTR type )
{
	if( _useDLL  &&  _mod == NULL )
		load() ;
	HGLOBAL h=0 ;
    HRSRC hrsc = FindResource( _mod, MAKEINTRESOURCE(id), type ) ;
	if( hrsc != 0 )
	    h = LoadResource( _mod, hrsc ) ;
	if( h == 0 )
		throw Msg( -1, "%s: %s (%sid %d)",
			_libname, getStr(SCRMSG_ResLoadFailed), type == RT_DIALOG ? "dialog " : "", id ) ;
    return h ;
}

HACCEL ResDllDesc::loadAccelerators( WORD id )
{
	if( _useDLL  &&  _mod == NULL )
		load() ;
	HACCEL h = LoadAccelerators( _mod, MAKEINTRESOURCE(id) ) ;
	if( h == 0 )
		throw Msg( -1, "%s: %s (accelerators id %d)", _libname, getStr(SCRMSG_ResLoadFailed), id ) ;
	return h ;
}
HBITMAP ResDllDesc::loadBitmap( WORD id )
{
	if( _useDLL  &&  _mod == NULL )
		load() ;
	HBITMAP h = LoadBitmap( _mod, MAKEINTRESOURCE(id) ) ;
	if( h == 0 )
		throw Msg( -1, "%s: %s (bitmap id %d)", _libname, getStr(SCRMSG_ResLoadFailed), id ) ;
	return h ;
}
HCURSOR ResDllDesc::loadCursor( WORD id )
{
	if( _useDLL  &&  _mod == NULL )
		load() ;
	HCURSOR h = LoadCursor( _mod, MAKEINTRESOURCE(id) ) ;
	if( h == 0 )
		throw Msg( -1, "%s: %s (cursor id %d)", _libname, getStr(SCRMSG_ResLoadFailed), id ) ;
	return h ;
}
HICON ResDllDesc::loadIcon( WORD id )
{
	if( _useDLL  &&  _mod == NULL )
		load() ;
	HICON h = LoadIcon( _mod, MAKEINTRESOURCE(id) ) ;
	if( h == 0 )
		throw Msg( -1, "%s: %s (icon id %d)", _libname, getStr(SCRMSG_ResLoadFailed), id ) ;
	return h ;
}
HMENU ResDllDesc::loadMenu( WORD id )
{
	if( _useDLL  &&  _mod == NULL )
		load() ;
	HMENU h = LoadMenu( _mod, MAKEINTRESOURCE(id) ) ;
	if( h == 0 )
		throw Msg( -1, "%s: %s (menu id %d)", _libname, getStr(SCRMSG_ResLoadFailed), id ) ;
	return h ;
}
int ResDllDesc::loadString( UINT id, LPTSTR lpBuffer, int nBufferMax )
{
	if( _useDLL  &&  _mod == NULL )
		load() ;
	int h = LoadString( _mod, id, lpBuffer, nBufferMax ) ;
	if( h == 0 )
		throw Msg( -1, "%s: %s (string id %d)", _libname, getStr(SCRMSG_ResLoadFailed), id ) ;
	return h ;
}
const char* ResDllDesc::loadSafeString( UINT id, LPTSTR lpBuffer, int nBufferMax )
{
	lpBuffer[0] = '?' ;
	lpBuffer[1] = 0 ;
	try
	{
		if( _useDLL  &&  _mod == NULL )
			load() ;
		LoadString( _mod, id, lpBuffer, nBufferMax ? nBufferMax : 10000 ) ;
		return lpBuffer ;
	}
	catch(...) { }
	{
		return lpBuffer ;
	}
}


//----------------------------------------------------------------------------------
//		sDllDialog::loadToolBar()
//----------------------------------------------------------------------------------


static BOOL setButtons( CToolBarCtrl *ctrl, const UINT* lpIDArray, int nIDCount)
{
	ASSERT(nIDCount >= 1);  // must be at least one of them
	ASSERT(lpIDArray == NULL ||
		AfxIsValidAddress(lpIDArray, sizeof(UINT) * nIDCount, FALSE));

	// delete all existing buttons
	int nCount = ctrl->GetButtonCount();
	while (nCount--)
		VERIFY(ctrl->DeleteButton(0));

	if (lpIDArray != NULL)
	{
		// add new buttons to the common control
		TBBUTTON button; memset(&button, 0, sizeof(TBBUTTON));
		int iImage = 0;
		for (int i = 0; i < nIDCount; i++)
		{
			button.fsState = TBSTATE_ENABLED;
			if ((button.idCommand = *lpIDArray++) == 0)
			{
				// separator
				button.fsStyle = TBSTYLE_SEP;
				// width of separator includes 8 pixel overlap
				button.iBitmap = 8;
			}
			else
			{
				// a command button with image
				button.fsStyle = TBSTYLE_BUTTON;
				button.iBitmap = iImage++;
			}
			if (!ctrl->AddButtons(1, &button))
				return FALSE;
		}
	}
	else
	{
		// add 'blank' buttons
		TBBUTTON button; memset(&button, 0, sizeof(TBBUTTON));
		button.fsState = TBSTATE_ENABLED;
		for (int i = 0; i < nIDCount; i++)
		{
			ASSERT(button.fsStyle == TBSTYLE_BUTTON);
			if (!ctrl->AddButtons(1, &button))
				return FALSE;
		}
	}
	return TRUE;
}

BOOL ResDllDesc::loadToolBar( CToolBarCtrl *ctrl, UINT nIDResource )
{
	// Use this data structure to help parse the toolbar resource.
	struct CToolBarData
	{
		WORD wVersion;
		WORD wWidth;
		WORD wHeight;
		WORD wItemCount;
		WORD* items()       { return (WORD*)(this+1); }
	};

	if( _useDLL  &&  _mod == NULL )
		load() ;
	ASSERT(nIDResource != 0);

	HINSTANCE oldHInst = AfxGetResourceHandle();
	HINSTANCE hInst = oldHInst ;
	if( _mod != NULL )
	{
		hInst = _mod ;
		AfxSetResourceHandle( hInst ) ;
	}

	BOOL bResult=FALSE ;

	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nIDResource), RT_TOOLBAR);
	if( hRsrc != NULL )
	{
		HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
		if (hGlobal != NULL)
		{
			CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
			if (pData != NULL)
			{
				ASSERT(pData->wVersion == 1);

				if( ctrl->AddBitmap(pData->wItemCount, nIDResource) != -1)	// Load the bitmap
				{
					UINT* pItems = new UINT[pData->wItemCount];
					for (int i = 0; i < pData->wItemCount; i++)
						pItems[i] = pData->items()[i];
					if( setButtons( ctrl, pItems, pData->wItemCount) )
					{
						CSize sizeImage(pData->wWidth, pData->wHeight);
						CSize sizeButton(pData->wWidth + 7, pData->wHeight + 7);

						ctrl->SetBitmapSize(sizeImage);
						ctrl->SetButtonSize(sizeButton);

						bResult = TRUE ;
					}
					delete[] pItems;
				}
				UnlockResource(hGlobal);
			}
			FreeResource(hGlobal);
		}
	}

	if( _mod != NULL )
		AfxSetResourceHandle( oldHInst ) ;
	return bResult;
}


//----------------------------------------------------------------------------------
//		sDllDialog
//----------------------------------------------------------------------------------


#define MAX_TOOLTIP_WIDTH	450


//IMPLEMENT_DYNAMIC( sDllDialog, CDialog )
IMPLEMENT_DYNCREATE( sDllDialog, CDialog )
BEGIN_MESSAGE_MAP( sDllDialog, CDialog )
	//{{AFX_MSG_MAP(sDllDlg)
    ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNotify )
	#if (_MFC_VER >= 0x400)
		ON_WM_HELPINFO()
		ON_WM_CONTEXTMENU()
	#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

sDllDialog::sDllDialog( )
{
	m_resMod	= NULL ;
	m_dlgId		= 0 ;
	m_parent	= NULL ;
	m_hResource	= 0 ;
	m_title		= NULL ;
}

sDllDialog::sDllDialog( ResDllDesc *res_mod, UINT dlg_id, CWnd *pParent, const char *title ) : CDialog()
{
	m_resMod	= res_mod ;
	m_dlgId		= dlg_id  ;
	m_parent	= pParent ;
	m_hResource	= NULL ;
	m_title		= title ? strdup(title) : NULL ;
}

sDllDialog::~sDllDialog()
{
	if( m_title != NULL )
		free( (void*)m_title ) ;
	FreeResource( m_hResource ) ;
}

BOOL sDllDialog::OnInitDialog()
{
    BOOL ret = CDialog::OnInitDialog();
	EnableToolTips( TRUE );
	if( m_title )
		SetWindowText( m_title ) ;
    return ret;  
}


void sDllDialog::_load()
{
	if( m_resMod != NULL )
    {
        m_hResource = m_resMod->loadDialog( m_dlgId ) ;
    }
	else
	{
		HINSTANCE hInst = AfxGetResourceHandle();
		HRSRC     hrsc  = FindResource( hInst, MAKEINTRESOURCE(m_dlgId), RT_DIALOG ) ;
		if( hrsc != NULL )
			m_hResource = LoadResource( hInst, hrsc ) ;
	}
	if( m_hResource == NULL )
		throw Msg( -1, "Dialog resource #%d not found.", m_dlgId ) ;
}

int sDllDialog::_DoModal()
{
	try
	{
		_load() ;
		if (m_hDialogTemplate == NULL)
			InitModalIndirect( m_hResource, m_parent );
		return CDialog::DoModal() ;
	}
	catch( Msg &msg )
	{
		char buf[1024] ;
		sprintf( buf, "%s:\n\n%s", getStr(SCRMSG_DlgFailed), msg.shortString ) ;
		AfxMessageBox( buf ) ;
		return -1 ;
	}
}

int sDllDialog::DoModal()
{
	CWnd* w = CWnd::GetSafeOwner( m_parent, NULL ) ;
	__try
	{
	    int ret = _DoModal() ;
		if( ret != -1 )
		{
			return ret ;
		}
	}
	CATCH_AND_SAY_SYSTEM_EXCEPTION 

	if( w != NULL )
		w->EnableWindow(TRUE);
	return -1 ;
}

BOOL sDllDialog::Create( ResDllDesc *res_mod, UINT dlg_id, CWnd *pParent )
{
	if( res_mod != NULL )
		m_resMod = res_mod ;
	if( dlg_id != 0 )
		m_dlgId  = dlg_id  ;
	if( pParent != NULL )
		m_parent = pParent ;

	char buf[1024] ;
	try
	{
		_load();
		BOOL rval = CDialog::CreateIndirect( m_hResource, m_parent );
		return rval; 
	}
	catch( Msg &msg )
	{
		sprintf( buf, "%s:\n\n%s", getStr(SCRMSG_DlgFailed), msg.shortString ) ;
	}
	catch( ... )
	{
		strcpy( buf, (char*)getStr(SCRMSG_DlgFailedUnknown) ) ;
	}
	AfxMessageBox( buf ) ;

	return FALSE; 
}


BOOL sDllDialog::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
    UINT         hCtrl= pNMHDR->idFrom;		// idFrom is actually the HWND of the tool

    if (pTTT->uFlags & TTF_IDISHWND)
    {
        UINT nID = ::GetDlgCtrlID((HWND)hCtrl);
        if(nID)
        {
            pTTT->hinst = NULL ;
            pTTT->lpszText = (char*)getToolTipText(nID,_txt) ;
            if( pTTT->lpszText == NULL )
            {
				if( m_resMod == NULL )
				{
					pTTT->lpszText  = MAKEINTRESOURCE(nID);
					pTTT->hinst     = AfxGetResourceHandle() ;
				}
				else
				{
					try
					{
						m_resMod->loadString( nID, _txt, 199 );
						pTTT->lpszText = _txt;
					}
					catch( ... )
						{ return FALSE ; }
				}
            }
            return  TRUE ;
        }
    }
    return  FALSE;
}


BOOL sDllDialog::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
	LPNMHDR lpNMHDR = (LPNMHDR)lParam ;
	if( lpNMHDR->code == TTN_GETDISPINFO )
		::SendMessage( lpNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, MAX_TOOLTIP_WIDTH ) ;

	return CDialog::OnNotify( wParam, lParam, pResult ) ;
}


BOOL sDllDialog::OnHelpInfo( HELPINFO* pHelpInfo )
{
	if( m_resMod == NULL ||
		m_resMod->helpFile() == NULL  ||
		pHelpInfo->iContextType != HELPINFO_WINDOW )
		return FALSE ;

	DWORD  *map = getHelpArray() ;
	if( map == NULL )
		return ::WinHelp( (HWND)pHelpInfo->hItemHandle, m_resMod->helpFile(),
				HELP_CONTEXTPOPUP, pHelpInfo->dwContextId ) ;
	else
		return ::WinHelp( (HWND)pHelpInfo->hItemHandle, m_resMod->helpFile(),
				HELP_WM_HELP, (DWORD)(LPVOID)map ) ;
}


void sDllDialog::OnContextMenu( CWnd* pWnd, CPoint point )
{
	if( m_resMod == NULL ||
		m_resMod->helpFile() == NULL )
		return ;

	DWORD  *hlpMap = getHelpArray() ;
	if( hlpMap == NULL )
	{
		#define MAX_ITEMS	100
		static DWORD map[2*MAX_ITEMS+2] ;
		hlpMap = (DWORD*)map ;

		// generate default help map
		int cnt=0 ;
		CWnd *cur = GetWindow( GW_CHILD ) ;
		while( cur != NULL )
		{
			int   ctrl_id= cur->GetDlgCtrlID( ) ;
			DWORD help_id= cur->GetWindowContextHelpId( ) ;
			if( help_id != 0 )
			{
				map[2*cnt  ] = ctrl_id ;
				map[2*cnt+1] = help_id ;
				if( ++cnt >= MAX_ITEMS )
					break ;
			}
			cur = cur->GetWindow( GW_HWNDNEXT ) ;
		}
		map[2*cnt  ] = 0 ;
		map[2*cnt+1] = 0 ;
	}

	::WinHelp( pWnd->m_hWnd, m_resMod->helpFile(), HELP_CONTEXTMENU, (DWORD)(LPVOID)hlpMap ) ;
}

/*
BOOL sDllDialog::OnHelpInfo( HELPINFO* pHelpInfo )
{
	BOOL ret = FALSE ;
	if( pHelpInfo->iContextType == HELPINFO_WINDOW )
	{
		DWORD *map = m_helpMap ? m_helpMap : getHelpArray() ;
		if( map != NULL )
			ret = ::WinHelp( (HWND)pHelpInfo->hItemHandle,
				AfxGetApp()->m_pszHelpFilePath,
				HELP_WM_HELP,
				(DWORD)(LPVOID)map ) ;
	}
	return ret;
}
void sDllDialog::OnContextMenu( CWnd* pWnd, CPoint point )
{
	if( this == pWnd )
		return ;

	DWORD *map = m_helpMap ? m_helpMap : getHelpArray() ;
	if( map != NULL )
		::WinHelp( pWnd->m_hWnd,
			AfxGetApp()->m_pszHelpFilePath,
			HELP_CONTEXTMENU,
			(DWORD)(LPVOID)map ) ;
}
*/

void sDllDialog::enableAllControls( BOOL enable )
{
	CWnd  *pWndCtl = GetWindow( GW_CHILD);
	while( pWndCtl != NULL )
	{
		int id = pWndCtl->GetDlgCtrlID(); 
		if( id )
			pWndCtl->EnableWindow( enable );
		pWndCtl = pWndCtl->GetWindow( GW_HWNDNEXT);
	}
}
