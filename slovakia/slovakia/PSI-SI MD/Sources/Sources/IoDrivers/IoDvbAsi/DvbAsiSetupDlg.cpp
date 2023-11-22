#include "tools2.hpp"
#include "loadRes.hpp"
#include "DrvInterface.hpp"
#include "ComDvbAsi.hpp"
#include "resource.h"
#include <process.h>
#include <STDLIB.H>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ResDllDesc *resMod = 0 ;

ResDllDesc *getRcModule( )
{
	if( resMod == NULL )
		resMod = new ResDllDesc( "DVBASI", theApp.m_hInstance ) ;
	return resMod ;
}

void releaseRcModule( )
{
	delete resMod ;
	resMod = NULL ;
}

//------------------------------------------------------------------
//							ReadmeDlg
//------------------------------------------------------------------

class ReadmeDlg : public sDllDialog
{
	BOOL	_isServer ;
	char	*bufer    ;

	virtual	BOOL	OnInitDialog	( ) ;

  public:
	ReadmeDlg( CWnd *parent, BOOL isServer )
		: sDllDialog( getRcModule(), IDD_AsiReadmeDlg, parent ) { _isServer = isServer ;}
	~ReadmeDlg( ) { delete bufer ;}
} ;



BOOL ReadmeDlg::OnInitDialog( )
{
    BOOL ret = sDllDialog::OnInitDialog();

	int			pos			= 0 ;
	CStdioFile	file;

	bufer = new char[12800] ;
	char buf[512], path[512], drv[10], dir[512] ;

	GetModuleFileName( theApp.m_hInstance, path, sizeof(path) ) ;
	_splitpath( path, drv, dir, NULL, NULL ) ;
	_makepath( buf,  drv,  dir, "Doc", NULL ) ;
	_makepath( path, NULL, buf, _isServer ? "ReadmeDvbAsiSrv" : "ReadmeDvbAsiRcv", "txt" ) ;

	if( !file.Open( path, CFile::modeRead ) )
	{
		strcpy( buf, "Can't open the file " ) ;
		strcat( buf, path ) ;
		AfxMessageBox( buf , MB_ICONEXCLAMATION ) ;
		return FALSE ;
	}

	while( file.Read( bufer+pos, 1 )  == 1 ) 
	{	
		if( bufer[pos] == '\n' ) 
		{
			if( bufer[pos-1] != '\r' )
			{
				bufer[pos] = '\r' ;
				pos++			  ;
				bufer[pos] = '\n' ;
			}
		}
		pos++ ;
	}
	bufer[pos] = 0 ;

	SetDlgItemText( IDC_AsiReadmeEdit, bufer ) ;

	return ret ;
}



//------------------------------------------------------------------
//						DvbAsiClientStatDlg
//------------------------------------------------------------------


class DvbAsiClientStatDlg : public sDllDialog
{
  protected:
	virtual	BOOL	OnInitDialog	( ) ;
	virtual void	DoDataExchange	( CDataExchange* pDX ) ;    // DDX/DDV support

  public:
	DvbAsiClientStatDlg(  ) ;
	~DvbAsiClientStatDlg( ) ;
} ;


DvbAsiClientStatDlg::DvbAsiClientStatDlg( )
  : sDllDialog( getRcModule(), IDD_DvbAsiClienStatDlg, NULL )
{
	
}


DvbAsiClientStatDlg::~DvbAsiClientStatDlg( )
{
	releaseRcModule() ;
}

BOOL DvbAsiClientStatDlg::OnInitDialog()
{
    BOOL ret = sDllDialog::OnInitDialog();

	//SetWindowPos( &wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE );

	return ret ;
}

void DvbAsiClientStatDlg::DoDataExchange(CDataExchange* pDX)
{
	sDllDialog::DoDataExchange(pDX);

	if( !pDX->m_bSaveAndValidate)
	{
		int MinNumPend = stats.MinNumPend ;
		DDX_Text ( pDX, IDC_StartDma,		 stats.StartDma			) ;
		DDX_Text ( pDX, IDC_MaxDspIntCount,  stats.MaxDspIntCount	) ;
		DDX_Text ( pDX, IDC_NumPciAbts,		 stats.NumPciAbts		) ;
		DDX_Text ( pDX, IDC_MinNumPend,		 MinNumPend				) ;		
		DDX_Text ( pDX, IDC_NumPend,		 stats.NumPend			) ;
		DDX_Text ( pDX, IDC_NumInts,		 stats.NumInts			) ;
		DDX_Text ( pDX, IDC_NumFifoErrs,	 stats.NumFifoErrs		) ;
		DDX_Text ( pDX, IDC_NumQued,		 stats.NumQued			) ;

		DDX_Text ( pDX, IDC_NumExErr,		 stats.NumExErr			) ;
		DDX_Text ( pDX, IDC_NumLost,		 stats.NumLost			) ;
	}
}



//------------------------------------------------------------------
//					DvbAsiSetupDlg
//------------------------------------------------------------------

class DvbAsiSetupDlg : public sDllDialog
{
  protected:
	ConfigClass*	_cfg ;
	BOOL            _controlsChanged;
	BOOL			_readonly ;
	UINT			_id ;


	virtual	BOOL	OnInitDialog( ) ;
	virtual void	OnOK( ) ;
	void setChanged( )
	{ 
		if( !_controlsChanged )
		{
			_controlsChanged = TRUE; 
			GetDlgItem(IDOK)->EnableWindow( TRUE ) ;
		}
	}

  public:
	DvbAsiSetupDlg( int id, CWnd *parent, ConfigClass *cfg ) ;
	~DvbAsiSetupDlg() ;
} ;


DvbAsiSetupDlg::DvbAsiSetupDlg( int id, CWnd *parent, ConfigClass *cfg )
  : sDllDialog( getRcModule(), id, parent )
{
	_controlsChanged	= FALSE    ;
	_cfg				= cfg	   ;
	_id                 = UINT(id) ;

	if( !_setup.load( _cfg ) )
		AfxMessageBox( "Can't open the config file", MB_ICONWARNING );
}

DvbAsiSetupDlg::~DvbAsiSetupDlg( )
{
	releaseRcModule() ;
}

BOOL DvbAsiSetupDlg::OnInitDialog()
{
    BOOL ret = sDllDialog::OnInitDialog();

	SetWindowPos( &wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE );
	GetDlgItem( IDOK )->EnableWindow( FALSE ) ;
	return ret ;
}

void DvbAsiSetupDlg::OnOK( )
{
	if ( !UpdateData( TRUE ) )
		return ;

	if ( _controlsChanged )
	{
		//save setings to config
	if( !_setup.save( _cfg ) )
		AfxMessageBox( "Can't open the config file", MB_ICONWARNING );

	}
	sDllDialog::OnOK() ;
}


//------------------------------------------------------------------
//					ServerSetupDlg
//------------------------------------------------------------------


class ServerSetupDlg : public DvbAsiSetupDlg
{
	virtual	BOOL	OnInitDialog( ) ;
	virtual void	DoDataExchange( CDataExchange* pDX ) ;    // DDX/DDV support

	DECLARE_MESSAGE_MAP( )
	afx_msg void OnControlChanged( )		{ setChanged() ; }
	afx_msg void OnHelp( ) ;

  public:
	ServerSetupDlg( CWnd *parent, ConfigClass *cfg, BOOL readonly )
		: DvbAsiSetupDlg( ID_DvbAsiSetupDlgSrv, parent, cfg )	{ _readonly = readonly ;}
} ;

BEGIN_MESSAGE_MAP( ServerSetupDlg, DvbAsiSetupDlg )
	//{{AFX_MSG_MAP(ServerSetupDlg)
	ON_EN_CHANGE( IDC_transSpeed, OnControlChanged )
	ON_BN_CLICKED( IDC_204bytes,  OnControlChanged )
	ON_BN_CLICKED( IDC_NoIb,	  OnControlChanged )
	ON_BN_CLICKED( IDC_AsiServerHelp,	  OnHelp   )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL ServerSetupDlg::OnInitDialog( )
{
    BOOL ret = DvbAsiSetupDlg::OnInitDialog();

	char title[64] ;
	char ver[64] ;
	char version[64] ;
	char path[128] ;

	GetModuleFileName( theApp.m_hInstance, path, sizeof(path) ) ;
	getFileVersion( path, ver, UPPER_VERSION );
	strncpy( version, ver, sizeof(version)-1 ) ;
	version[ sizeof(version)-1] = 0 ;
	strcpy( title, "Dvb Asi Setup - version " ) ;
	strcat( title, version ) ;
	SetWindowText( title ) ;
	
	if( _readonly )
	{
		CEdit *speedEdit   = (CEdit*)GetDlgItem( IDC_transSpeed) ;
		CWnd  *check204    = (CEdit*)GetDlgItem( IDC_204bytes ) ;
		CWnd  *checkNoIb   = (CEdit*)GetDlgItem( IDC_NoIb ) ;
		speedEdit->EnableWindow( FALSE ) ;
		check204->EnableWindow(  FALSE ) ;
		checkNoIb->EnableWindow( FALSE ) ;
	}
	return ret ;
}

void ServerSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	DvbAsiSetupDlg::DoDataExchange(pDX);

	DDX_Text ( pDX, IDC_transSpeed,	 _setup._speed ) ;
	DDV_MinMaxFloat( pDX, _setup._speed, 0, 270 ) ;
	DDX_Check( pDX, IDC_204bytes  ,  _setup._204Bytes ) ;
	DDX_Check( pDX, IDC_NoIb  ,  _setup._NoIbStuffing ) ;
}

void ServerSetupDlg::OnHelp()
{
	char buf[512], path[512], drv[10], dir[512] ;

	GetModuleFileName( theApp.m_hInstance, path, sizeof(path) ) ;
	_splitpath( path, drv, dir, NULL, NULL ) ;
	_makepath( buf, drv, dir, "Doc", NULL ) ;
	_makepath( path, NULL, buf,"ReadmeDvbAsiSrv", "txt" ) ;
	sprintf( buf, "notepad.exe %s", path );

	int ret = WinExec( buf, SW_SHOW);		// call notepad to show the help file
	if( ret < 32 )
	{
		if( ret == 0 )
			AfxMessageBox( "Not enough memory to run Notepad.exe", MB_ICONEXCLAMATION ) ;
		else
			AfxMessageBox( "Can't open file Notepad.exe", MB_ICONEXCLAMATION ) ;
	}

}


//------------------------------------------------------------------
//						ClientSetupDlg
//------------------------------------------------------------------


class ClientSetupDlg : public DvbAsiSetupDlg
{
	virtual	BOOL	OnInitDialog( ) ;
	virtual void	DoDataExchange( CDataExchange* pDX ) ;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnControlChanged( )		{ setChanged() ; }
	afx_msg void OnHelp( ) ;

  public:
	ClientSetupDlg( CWnd *parent, ConfigClass *cfg, BOOL readonly )
		: DvbAsiSetupDlg( ID_DvbAsiSetupDlgRcv, parent, cfg )	{ _readonly = readonly; }
} ;

BEGIN_MESSAGE_MAP(ClientSetupDlg, DvbAsiSetupDlg)
	//{{AFX_MSG_MAP(ClientSetupDlg)
	ON_BN_CLICKED( IDC_RFHigh , OnControlChanged )
	ON_BN_CLICKED( IDC_PacSync, OnControlChanged )
	ON_BN_CLICKED( IDC_AsiClientHelp,	  OnHelp )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL ClientSetupDlg::OnInitDialog( )
{
    BOOL ret = DvbAsiSetupDlg::OnInitDialog();
	char title[64] ;
	char ver[64] ;
	char version[64] ;
	char path[128] ;

	GetModuleFileName( theApp.m_hInstance, path, sizeof(path) ) ;
	getFileVersion( path, ver, UPPER_VERSION );
	strncpy( version, ver, sizeof(version)-1 ) ;
	version[ sizeof(version)-1] = 0 ;
	strcpy( title, "Dvb Asi Setup - version " ) ;
	strcat( title, version ) ;
	SetWindowText( title ) ;

	if( _readonly )
	{
		CWnd *pacSync = (CEdit*)GetDlgItem( IDC_PacSync ) ;
		CWnd *rfHigh = (CEdit*)GetDlgItem( IDC_RFHigh ) ;
		pacSync->EnableWindow( FALSE ) ;
		rfHigh->EnableWindow(  FALSE ) ;
	}
	return ret ;
}

void ClientSetupDlg::DoDataExchange( CDataExchange* pDX )
{
	DvbAsiSetupDlg::DoDataExchange(pDX);

	DDX_Check( pDX, IDC_PacSync,   _setup._pacSynth )	;
	DDX_Check( pDX, IDC_RFHigh,	   _setup._rfHigh )		;
}

void ClientSetupDlg::OnHelp()
{
	char buf[512], path[512], drv[10], dir[512] ;

	GetModuleFileName( theApp.m_hInstance, path, sizeof(path) ) ;
	_splitpath( path, drv, dir, NULL, NULL ) ;
	_makepath( buf,  drv,  dir, "Doc",			   NULL  ) ;
	_makepath( path, NULL, buf, "ReadmeDvbAsiRcv", "txt" ) ;
	sprintf( buf, "notepad.exe %s", path );

	int ret = WinExec( buf, SW_SHOW);		// call notepad to show the help file
	if( ret < 32 )
	{
		if( ret == 0 )
			AfxMessageBox( "Not enough memory to run Notepad.exe", MB_ICONEXCLAMATION ) ;
		else
			AfxMessageBox( "Can't open file Notepad.exe", MB_ICONEXCLAMATION ) ;
	}
}

//------------------------------------------------------------------
//						DvbAsiInp statistics methods
//------------------------------------------------------------------

void ComInpDvbAsi::clearStatistics( )
{
	stats.clear() ;
}

BOOL ComInpDvbAsi::getStatisticsLog( char *buf )
{
	return stats.report( buf ) ;
}

sDllDialog *ComInpDvbAsi::createStatisticsPage( )
{
	return new DvbAsiClientStatDlg ;
}


//------------------------------------------------------------------
//							Exports
//------------------------------------------------------------------

#define ENOMEM 12   // not enough memory
// Monitor might need to run server dialog under client and vice versa.
static BOOL clientDialogRunning = FALSE ;
static BOOL serverDialogRunning = FALSE ;

void DvbAsiSetupDialog( CWnd *wnd, ConfigClass *cfg, BOOL asServer ) 
{
	if( asServer )
	{
		if( !serverDialogRunning )
		{
			serverDialogRunning = TRUE ;
			ServerSetupDlg dlg( wnd, cfg, comOut != NULL ) ;
			dlg.DoModal() ;
			serverDialogRunning = FALSE ;
		}
	}
	else
	{
		if( !clientDialogRunning )
		{
			clientDialogRunning = TRUE ;
			ClientSetupDlg dlg( wnd, cfg, comInp != NULL ) ;
			dlg.DoModal() ;
			clientDialogRunning = FALSE ;
		}
	}
}


void installDialog( HWND parent, ConfigClass *cfg, BOOL bServer ) 
{
	const char	*title = "DVB ASI" ;
	char		path[512], buf[512], drv[128], dir[128] ;
		
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	::MessageBox( parent,
		"Installation Program will now run Setup for DVB ASI system driver.\n\n"
		"This dialog defines DVB ASI devices.\n\n"
		"PLEASE MAKE SURE THAT DEVICE Dvb1 IS DEFINED.\n"
		"(Upperleft combo box)\n\n"
		"If yes, then you can quit the dialog.\n"
		"IF NOT, then PRESS ADD button first.\n\n"
		"You can later change this setup running \"DVBcfg.exe\" from\n"
		"subdirectory FirmWare of your installation path ",
		title, MB_OK | MB_ICONINFORMATION ) ;

//		"If there is no \"DVB Device Name\" displayed in the following\n"
//		"window, be sure you add a device by click on \"Add\" button\n"
//		"and apply the changes by click on \"Apply\" button ! ! !\n\n"
//		"Don't set the \"Rx - Max Number of Buffers\" and\n"
//		"\"Tx - Max Number of Buffers\" under value = 2 !",

	
	GetModuleFileName( theApp.m_hInstance, path, sizeof(path) ) ;
	_splitpath( path, drv , dir, NULL,       NULL )  ;
	_makepath ( buf , drv , dir, "FirmWare", NULL )  ;
	_makepath ( path, NULL, buf, "DVBcfg",   "exe" ) ;
	
	if( _spawnl( _P_WAIT, path, " ", NULL ) <0 )
	{
		if( errno == ENOMEM )
			::MessageBox( parent,"Not enough memory to run DVBCfg.exe",title, MB_ICONEXCLAMATION ) ;
		else
			::MessageBox( parent,"Can't open file DVBCfg.exe",title, MB_ICONEXCLAMATION ) ;
	}

	if( ::MessageBox( parent,
			"Installation succeeded.\n\n"
			"DVB ASI Card needs to be configured.\n"
			"You can do it now if you press 'Yes' button, or later\n"
			"from the application Setup menu.\n\n"
			"Do you wish to configure the card now?",
			title, MB_YESNO|MB_ICONQUESTION) == IDYES )
	{
		if( bServer )
		{
			ServerSetupDlg dlg( NULL, cfg , FALSE ) ;
			dlg.DoModal() ;
		}
		else 
		{
			ClientSetupDlg dlg( NULL, cfg , FALSE ) ;
			dlg.DoModal() ;
		}
	}
	else
	{
		::MessageBox( parent,
			"Configuration skipped.\n\n"
			"You can configure DVB ASI card any time from the application Setup menu.",
			title, MB_OK|MB_ICONINFORMATION ) ;
	}
}



void uninstallDialog( HWND parent, ConfigClass *cfg, BOOL bServer ) 
{
	if( ::MessageBox( parent, "Are you sure to uninstall your DVB ASI Card driver?", "DvbAsi Uninstall", MB_YESNO | MB_ICONQUESTION) != IDYES )
		return ;
	
	SERVICE_STATUS	drvStatus;
	SC_HANDLE		srvControlMnrg = 0 ;
	SC_HANDLE		dvbDriverHandle= 0 ;
	
	srvControlMnrg = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( srvControlMnrg )
	{
		dvbDriverHandle = OpenService( srvControlMnrg, "DvbLink", DELETE | SERVICE_STOP );
		if ( dvbDriverHandle )
		{
			ControlService( dvbDriverHandle, SERVICE_CONTROL_STOP, &drvStatus ) ;
			DeleteService( dvbDriverHandle ) ;
		}
	}

	if( dvbDriverHandle )
		CloseServiceHandle( dvbDriverHandle );
	if( srvControlMnrg )
		CloseServiceHandle( srvControlMnrg );

	::MessageBox( parent, "For complet uninstalation of DVB ASI card driver you must to restart your computer.", "DvbAsi Uninstall", MB_OK | MB_ICONINFORMATION ) ;

	return ;
}			

