#include "tools2.hpp"
#include "except.hpp"
#include <afxole.h>
#include <winsock2.h>

#include "transportClass.hpp"
#include "transportPrivate.hpp"
#include "app_rc.hpp"
#include "..\WinNT\RC\ConnectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SOCKET		UniversalTransportMedium::SocketMedium
#define MSMQ		UniversalTransportMedium::MSMQMedium
#define UNKNOWN		UniversalTransportMedium::UnknownMedium
#define NUMOFMEDIA	2

#define MAX_CONNECT_RETRYS		2

#define DEL(x)	{ delete x ; x = NULL ; }


// ********************************************************************************************
//
//	TransportClassSetupDialog
//
// ********************************************************************************************


//
// Transport media selection dialog
//
class UTMMediumSelectionSheetDialog : public sDllDialog
{
	TransportClass *_transportClass ;
	CComboBox	   *_mediumTypes ;

  protected:
	virtual void	DoDataExchange	( CDataExchange* pDX );

  public:
	UTMMediumSelectionSheetDialog	( TransportClass *t ) ;
	void			setDefaults		( ) ;
} ;

#define setSelectedMedium(name)		_mediumTypes->SelectString( -1, name )


UTMMediumSelectionSheetDialog::UTMMediumSelectionSheetDialog( TransportClass *t )
	: sDllDialog( &applibResModule, sFoxD_UserConnectionMedium )
{
	_transportClass = t ;
	_mediumTypes	= NULL ;
}

void UTMMediumSelectionSheetDialog::DoDataExchange( CDataExchange* pDX )
{
	sDllDialog::DoDataExchange( pDX ) ;
	if( _transportClass->isServer() )
		return ;

	if( pDX->m_bSaveAndValidate )
	{
		char mediumName[128] ;
		_mediumTypes->GetWindowText( mediumName, sizeof(mediumName) ) ;
		_transportClass->setClientMediumName( mediumName ) ;
	}
	else
	{
		const char* mediumName ;
		if( _mediumTypes == NULL )
		{
			// initialize combo box
			_mediumTypes = (CComboBox*)GetDlgItem( sFoxC_ConnectionMedium ) ;

			const sStringPtrArray &names = _transportClass->getMediumNames() ;
			for( int i=0 ; i < names.count() ; i++ )
				_mediumTypes->AddString( names[i] ) ;
		}

		mediumName = _transportClass->getClientMediumName( ) ;
		setSelectedMedium( mediumName ) ;
	}
}

void UTMMediumSelectionSheetDialog::setDefaults()
{
	setSelectedMedium( UniversalTransportMedium::mediumType2String( SOCKET ) ) ;
}


//------------------------------------------------------------------


TransportClassSetupDialog::TransportClassSetupDialog( TransportClass *t, CWnd *par )
	: sSheetDialog( &applibResModule, PagesDlg, sFoxD_UserConnection, sFoxC_ConnectGroupTab, 0, par )
{
	_runAsPopup		= FALSE ;
	_transportClass = t ;
	if( !_transportClass->isServer() )
	{
		_typePage = new UTMMediumSelectionSheetDialog( _transportClass ) ;
		addPage( ((UTMMediumSelectionSheetDialog*)_typePage), "General" );
	}
	else
		_typePage = NULL ;

	const sStringPtrArray &names = _transportClass->getMediumNames() ;

	// add medium specific tabs
	for( int i=0 ; i < names.count() ; i++ )
	{
		const char* mediumName = names[i] ;
		sDllDialog *dlg = _transportClass->getConnectionSetupDlg( mediumName, NULL, TRUE ) ;
		if ( dlg )
			addPage( dlg, mediumName );
	}
}

BOOL TransportClassSetupDialog::DoModal( )
{
	_runAsPopup = TRUE ;
	BOOL ret = sSheetDialog::DoModal() ;
	_runAsPopup = FALSE ;
	return ret ;
}

UINT TransportClassSetupDialog::IDD( )
{
	return sFoxD_UserConnection ;
}

BOOL TransportClassSetupDialog::OnInitDialog()
{
	sSheetDialog::OnInitDialog() ;
	if( !_transportClass->isServer() )
		GetDlgItem(sFoxC_ApplyConnection)->ShowWindow( SW_HIDE ) ;
    createAllPages();
	if( !_runAsPopup )
	{
		GetDlgItem(IDOK    )->ShowWindow( SW_HIDE ) ;
		GetDlgItem(IDCANCEL)->ShowWindow( SW_HIDE ) ;
	}
	else
	{
		SetWindowText( _transportClass->name() ) ;
	}
	return TRUE ;
}

BOOL TransportClassSetupDialog::OnCommand( WPARAM wParam, LPARAM lParam)
{
	UINT id = LOWORD(wParam) ;
	switch ( id )
	{
		case sFoxC_SaveConnection:
			updateData( TRUE ) ;
			_transportClass->save() ;
			return TRUE ;
		case sFoxC_UndoConnection:
			_transportClass->undo() ;
			updateData( FALSE ) ;
			return TRUE ;
		case sFoxC_DefaultConnection:
			setDefaults() ;
			return TRUE ;
		case sFoxC_ApplyConnection :
		{
			updateData( TRUE ) ;
			CWaitCursor wait ;
			_transportClass->saveAndValidate() ;
			return TRUE ;
		}
		case IDCLOSE  :
		case IDCANCEL :
			_transportClass->undo() ;
			break ;

		case IDOK:
		{
			CWaitCursor wait ;
			updateData( TRUE ) ;
			_transportClass->saveAndValidate() ;
			break ;
		}
	}

	return sSheetDialog::OnCommand( wParam, lParam ) ;
}

void TransportClassSetupDialog::setDefaults()
{
	if( !_transportClass->isServer() )
		((UTMMediumSelectionSheetDialog*)_typePage)->setDefaults() ;

	const sStringPtrArray &names = _transportClass->getMediumNames() ;
	for( int i=0 ; i < names.count() ; i++ )
	{
		const char* mediumName = names[i] ;
		UTMConnectionSetupDialog *dlg = (UTMConnectionSetupDialog*)getPageDialog( mediumName ) ;
		if ( dlg != NULL )
			dlg->setDefaults() ;
	}
}


// ********************************************************************************************
//
//	UTMConnectionSetupDialog
//		Base class for medium-specific setup dialog
//
// ********************************************************************************************


UTMConnectionSetupDialog::UTMConnectionSetupDialog( TransportClass *t, UINT dlgID, CWnd *parent, BOOL bUseAsSheetPage )
	: sDllDialog( &applibResModule, dlgID )
{
	_bUseAsSheetPage= bUseAsSheetPage;
	_transportClass = t ;
}

BOOL UTMConnectionSetupDialog::OnInitDialog()
{
	if( _bUseAsSheetPage )
	{
		CWnd *pOK	 = GetDlgItem(IDOK );
		CWnd *pCancel= GetDlgItem(IDCANCEL );
		
		if( pOK )
			pOK->ShowWindow( SW_HIDE ) ;
		if( pCancel )
			pCancel->ShowWindow( SW_HIDE ) ;
	}
	return sDllDialog::OnInitDialog();
}

void UTMConnectionSetupDialog::getConnectString( char *str )
{
	str[0] = 0 ;
}

BOOL UTMConnectionSetupDialog::getMediumAllowed()
{
	if( _transportClass->isServer() )
	{
		CButton *btn = (CButton*)GetDlgItem( sFoxC_MediumAllowed ) ;
		if ( btn )
			return btn->GetCheck() != 0 ;
	}
	return FALSE ;
}

void UTMConnectionSetupDialog::setMediumAllowed( BOOL a )
{
	if( _transportClass->isServer() )
	{
		CButton *btn = (CButton*)GetDlgItem( sFoxC_MediumAllowed ) ;
		if ( btn )
			btn->SetCheck( a ? 1 : 0 ) ;
	}
}

void UTMConnectionSetupDialog::setDefaults()
{
	setMediumAllowed( FALSE ) ;
}

BOOL UTMConnectionSetupDialog::OnCommand( WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == IDOK && _bUseAsSheetPage==FALSE )
		if(!UpdateData() )
			return 0;
	return sDllDialog::OnCommand(wParam, lParam);
}

// ********************************************************************************************
//
//	SocketConnectionSetupSheetDialog
//		Socket implementation of UTMConnectionSetupDialog
//
// ********************************************************************************************


class SocketConnectionSetupSheetDialog : public UTMConnectionSetupDialog
{
  protected:
	virtual	BOOL	OnCommand		( WPARAM wParam, LPARAM lParam ) ;
	virtual void	DoDataExchange	( CDataExchange* pDX );

	void	decomposeConnectString	( const char *connectString ) ;

  private:
	CIPAddressCtrl	_ipAddressCtrl ;		// the IP address control
	USHORT			_port ;
	BOOL			_byIPAddress ;			// TRUE if ipAddress is valid
	ULONG			_ipAddress ;
	char			_hostName[_MAX_PATH] ;

	void	enableDisable			( ) ;
	void	onBrowse				( ) ;
	void	onTest					( ) ;
	void	onIP					( ) ;

  public:
	SocketConnectionSetupSheetDialog	( TransportClass *t, CWnd *parent, BOOL bUseAsSheetPage=TRUE ) ;

	virtual	void	getConnectString	( char *buffer ) ;
	virtual	void	setDefaults			( ) ;
} ;

SocketConnectionSetupSheetDialog::SocketConnectionSetupSheetDialog( TransportClass *t, CWnd *parent, BOOL bUseAsSheetPage )
	: UTMConnectionSetupDialog( t,
		t->isServer() ? sFoxD_UserConnectionSOCKET_Server : sFoxD_UserConnectionSOCKET_Client,
		parent, bUseAsSheetPage )
{
	char connectStr[256] ;
	_transportClass->getMediumConnectString( SOCKET, connectStr ) ;
	decomposeConnectString( connectStr ) ;
}

void SocketConnectionSetupSheetDialog::getConnectString( char *buffer )
{
	// build open string
	if ( _byIPAddress )
	{
		DWORD ipAddr = htonl( _ipAddress ) ;
		char *ipAddrStr = inet_ntoa( *(in_addr*)&ipAddr ) ;
		sprintf( buffer, "%s:%d", ipAddrStr ? ipAddrStr : "", _port ) ;
	}
	else
		sprintf( buffer, "%s:%d", _hostName, _port ) ;
}

void SocketConnectionSetupSheetDialog::setDefaults()
{
	_transportClass->setDefaults( SOCKET ) ;
	UpdateData( FALSE ) ;
}

void SocketConnectionSetupSheetDialog::decomposeConnectString( const char *connectString )
{
	// decompose open string
	char buff[512] ;
	strcpy( buff, connectString ) ;

	char *port = strchr( buff, ':' ) ;
	if ( port == NULL )
		_port = 0 ;
	else
	{
		*port = 0 ;
		port++ ;
		_port = atoi( port ) ;
	}

	// the host name or IP address
	_ipAddress = inet_addr( buff ) ;
	if ( _ipAddress != INADDR_NONE )
	{
		_byIPAddress	= 1 ;
		_ipAddress		= ntohl( _ipAddress ) ;
		_hostName[0]	= 0 ;
	}
	else
	{
		_byIPAddress	= 0 ;
		_ipAddress		= 0 ;
		strcpy( _hostName, buff ) ;
	}
}

BOOL SocketConnectionSetupSheetDialog::OnCommand( WPARAM wParam, LPARAM lParam )
{
    UINT id = LOWORD(wParam) ;
	switch ( id )
	{
		case sFoxC_Browse:
			onBrowse() ;
			return TRUE ;
		case sFoxC_TCPTest:
			onTest() ;
			return TRUE ;
		case sFoxC_UseIP:
			onIP() ;
			return TRUE ;
		default:
			return sDllDialog::OnCommand( wParam, lParam ) ;
	}
}

void SocketConnectionSetupSheetDialog::enableDisable()
{
	GetDlgItem( sFoxC_IP )->EnableWindow( _byIPAddress ) ;
	GetDlgItem( sFoxC_Browse )->EnableWindow( !_byIPAddress ) ;
}

void SocketConnectionSetupSheetDialog::DoDataExchange( CDataExchange* pDX )
{
	sDllDialog::DoDataExchange( pDX ) ;

	char connectString[256] ;
	if( !_transportClass->isServer() )
		DDX_Control( pDX, sFoxC_IP, _ipAddressCtrl ) ;

	// _transportClass -> dialog data
	if( !pDX->m_bSaveAndValidate )
	{
		_transportClass->getMediumConnectString( SOCKET, connectString ) ;

		decomposeConnectString( connectString ) ;
		if( !_transportClass->isServer() )
		{
			_ipAddressCtrl.SetAddress( _ipAddress ) ;
			enableDisable() ;
		}
		else
		{
			BOOL allowed = _transportClass->isServerMediumAllowed( SOCKET ) ;
			setMediumAllowed( allowed ) ;
		}
	}

	// dialog data <-> Windows
	UINT ui = _port ;
	DDX_Text( pDX, sFoxC_TCPPort, ui ) ;
	_port = ui ;

	if( !_transportClass->isServer() )
	{
		CString name ;
		if ( !pDX->m_bSaveAndValidate )
		{
			if ( _byIPAddress )
				name = "" ;
			else
				name = _hostName ;
		}

		DDX_Check( pDX, sFoxC_UseIP, _byIPAddress ) ;
		DDX_Text( pDX, sFoxC_Machine, name ) ;		// set only

		if ( _byIPAddress )
			_ipAddressCtrl.GetAddress( _ipAddress ) ;
	}

	// dialog data -> _transportClass
	if( pDX->m_bSaveAndValidate )
	{
		getConnectString( connectString ) ;
		_transportClass->setMediumConnectString( SOCKET, connectString ) ;

		if( _transportClass->isServer() )
		{
			BOOL allowed = getMediumAllowed() ;
			_transportClass->setServerMediumAllowed( SOCKET, allowed ) ;
		}
	}
}

static char *browseMachineName( CWnd *wnd, char *buf )	// buf must be MAX_PATH sized
{
	// get list of all reachable computers
	ITEMIDLIST *pidl ;
	SHGetSpecialFolderLocation( NULL, CSIDL_NETWORK, &pidl ) ;

	// show the dialog to get the computer name
	BROWSEINFO bi ;
	bi.hwndOwner		= wnd->m_hWnd ;
	bi.pidlRoot			= pidl ;
	bi.pszDisplayName	= buf ;
	bi.lpszTitle		= "Select computer" ;
	bi.ulFlags			= BIF_BROWSEFORCOMPUTER ;
	bi.lpfn				= NULL ;
	bi.lParam			= NULL ;
	bi.iImage			= 0 ;
	ITEMIDLIST *res = SHBrowseForFolder( &bi ) ;
	if ( res != NULL )
		return buf;
	else
		return NULL;
}

void SocketConnectionSetupSheetDialog::onBrowse()
{
	UpdateData() ;

	char compName[MAX_PATH] ; 
	memset( compName, 0, MAX_PATH ) ;
	if ( browseMachineName( this, compName ) )
	{
		strcpy( _hostName, compName ) ;
		GetDlgItem(sFoxC_Machine)->SetWindowText( _hostName ) ;
		//UpdateData( FALSE ) ;
	}
}

void SocketConnectionSetupSheetDialog::onTest()
{
	UpdateData() ;

	char reason[1024] ;
	char connectString[256] ;
	getConnectString( connectString ) ;

	CWaitCursor wait ;

	SocketTransport transport ;
	transport.setHandshake(
		_transportClass->handshakeFunction(),
		_transportClass->handshakeParam(),
		(TransportMediumMsg*)_transportClass->handshakeMessage() ) ;

	if( transport.testConnection( connectString, reason) )
	{
		char msg[1024] ;
		strcpy( msg, "Server application contacted successfully" ) ;
		if( _transportClass->getClientMediumType() != SOCKET )
		{
			int len = strlen( msg ) ;
			msg[len++] = '\n' ;
			msg[len++] = '\n' ;
			strcpy( msg+len, "Warning:\nTcp medium is not selected for the communication.\nYou will not be able to send any data." ) ;
		}
		AfxMessageBox( msg, MB_ICONINFORMATION ) ;
	}
	else
	{
		char buf[1024] ;
		sprintf( buf, "%s:\n\n%s", "Failed to contact the server", reason ) ;
		AfxMessageBox( buf ) ;
	}
}

void SocketConnectionSetupSheetDialog::onIP()
{
	CButton *btn = (CButton*)GetDlgItem( sFoxC_UseIP ) ;
	_byIPAddress = btn->GetCheck() ;

	enableDisable() ;
}

// ********************************************************************************************
//	class MSMQLocationsDlg 
// ********************************************************************************************

class MSMQLocationsDlg : public sDllDialog
{
	int				 _current;
	char			*_label;

	MQSpecificationPtrArray *_locations;

	virtual	BOOL	OnCommand		( WPARAM wParam, LPARAM lParam ) ;
	virtual BOOL	OnInitDialog	( ) ;
	
	//{{AFX_MSG(MSMQLocationsDlg)
	afx_msg void OnListBoxDblClick();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  public:
	MSMQLocationsDlg	( CWnd *parent, MQSpecificationPtrArray *locations, char *label );
	int		getCurrent	( )				{ return _current; }
};

BEGIN_MESSAGE_MAP(MSMQLocationsDlg, sDllDialog)
	//{{AFX_MSG_MAP(MSMQLocationsDlg)
	ON_LBN_DBLCLK(sFoxC_MSMQFoundQueues, OnListBoxDblClick )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

MSMQLocationsDlg::MSMQLocationsDlg( CWnd *parent, MQSpecificationPtrArray *locations, char *label )
	: sDllDialog( &applibResModule, sFoxD_UserConnectionMSMQFoundLocations, parent)
{
	_current	= -1;
	_label		= label;
	_locations	= locations;
}

void MSMQLocationsDlg::OnListBoxDblClick()
{
	OnCommand( IDOK, 0 );
}

BOOL MSMQLocationsDlg::OnCommand( WPARAM wParam, LPARAM lParam )
{
	if ( LOWORD(wParam) == IDOK )
	{
		CListBox *list = (CListBox*)GetDlgItem(sFoxC_MSMQFoundQueues);
		int pos = list->GetCurSel();
		_current = list->GetItemData(pos);
	}
	return sDllDialog::OnCommand( wParam, lParam );
}

BOOL MSMQLocationsDlg::OnInitDialog( )
{
	if ( !_locations )
		return FALSE;

	char buf[128];
	strcpy(buf, "MSMQ - queue locations labeled \"");
	strcat(buf, _label);
	strcat(buf, "\"");
	SetWindowText(buf);
	CListBox *list = (CListBox*)GetDlgItem(sFoxC_MSMQFoundQueues);
	for ( int i = 0; i < _locations->count(); i++ )
	{
		int pos = list->AddString(_locations->item(i)->szPathName);
		list->SetItemData(pos, i);
	}
	list->SetCurSel(0);
	
	return sDllDialog::OnInitDialog();
}

// ********************************************************************************************
//	class MSMQTestQueueDlg 
// ********************************************************************************************

class MSMQTestQueueDlg : public sDllDialog
{
	friend UINT MSMQTestQueueThreadFn( LPVOID pParam );

	char				*_path;
	char				 _txt[512];
	CWinThread			*_thread;
	CRITICAL_SECTION	 _lock;
	HANDLE				 _eventEnd;
	char				 _formatName[MAX_Q_FORMATNAME_LEN];
	BOOL				 _succeed;

	virtual	BOOL	OnCommand		( WPARAM wParam, LPARAM lParam ) ;
	virtual BOOL	OnInitDialog	( ) ;
	
  public:
	MSMQTestQueueDlg	( CWnd *parent, char *path );
	~MSMQTestQueueDlg	( );

	const char *getFormatName( )	{ return _succeed ? _formatName : NULL; }
};

#define TEST_THREAD_CANCEL											\
	if( ::WaitForSingleObject( _eventEnd, 0 ) == WAIT_OBJECT_0 )	\
	{																\
		::CloseHandle( _eventEnd );									\
		return 0;													\
	}																\

static UINT MSMQTestQueueThreadFn( LPVOID pParam )
{
	MSMQTestQueueDlg *dlg		= (MSMQTestQueueDlg*)pParam;
	CRITICAL_SECTION *lock		= &dlg->_lock;
	HANDLE			 _eventEnd	= dlg->_eventEnd;

	char *txt	 = dlg->_txt;
	char *path	 = dlg->_path;
	char *format = dlg->_formatName;
	
	int mqResult;

	EnterCriticalSection( lock );
	strcpy(txt,"\r\n Locating message queue ... ");
	LeaveCriticalSection( lock );
	dlg->PostMessage(WM_COMMAND, jFoxC_MSMQTestOutputUpdate);

	// locating message queue
	try
	{
		mqResult = MessageQueue::findFormatName(format,MAX_Q_FORMATNAME_LEN,path);
	} catch(...) 
	{ 
		mqResult = -1; 
	}

	TEST_THREAD_CANCEL;

	// update dialog content
	EnterCriticalSection( lock );
	switch ( mqResult )
	{
	case 0:
		strcat(txt,"failed");
		strcat(txt,"\r\n\r\n ");
		strcat(txt,"Message queue not found.");
		break;
	case -1:
		strcat(txt,"failed");
		strcat(txt,"\r\n\r\n ");
		strcat(txt,"Message queue error.\r\n MSMQ probably not installed.");
		break;
	default:
		dlg->_succeed = TRUE;
		strcat(txt,"found");
		strcat(txt,"\r\n\r\n");
		strcat(txt,"Message queue ID : \r\n\t");
		strcat(txt,format);
		strcat(txt,"\r\n\r\n Examining send access ... ");
	}
	LeaveCriticalSection( lock );
	dlg->PostMessage(WM_COMMAND, jFoxC_MSMQTestOutputUpdate);
	
	if ( mqResult <= 0 )	// message queue not found
	{
		dlg->PostMessage(WM_COMMAND, jFoxC_MSMQThreadFinished);
		return 0;
	}

	// testing send access
	BOOL sendAccess;
	HRESULT errCode;
	try
	{
		MessageQueue mq;
		sendAccess = mq._open(format);
		errCode = mq.getLastResult();
		mq.close();
	} catch(...) 
	{
		sendAccess = FALSE;
	}
	
	TEST_THREAD_CANCEL;

	// update dialog content
	EnterCriticalSection( lock );
	if ( sendAccess )
		strcat(txt,"OK");
	else
	{
		strcat(txt,"failed (");
		char errTxt[256];
		HRESULTasText( errCode, errTxt ) ;
		strcat(txt, errTxt);
		strcat(txt,")\r\n");
	}
	strcat(txt,"\r\n Examining receive access ... ");
	LeaveCriticalSection( lock );
	dlg->PostMessage(WM_COMMAND, jFoxC_MSMQTestOutputUpdate);

	// testing receive access
	BOOL recAccess;
	try
	{
		MessageQueue mq;
		recAccess = mq._open(format, MessageQueue::ReceiveAccess);
		errCode = mq.getLastResult();
		mq.close();
	} catch(...) 
	{
		sendAccess = FALSE; 
	}

	TEST_THREAD_CANCEL;

	// update dialog content
	EnterCriticalSection( lock );
	if ( recAccess )
		strcat(txt,"OK");
	else
	{
		strcat(txt,"failed (");
		char errTxt[256];
		HRESULTasText( errCode, errTxt ) ;
		strcat(txt, errTxt);
		strcat(txt,")\r\n");
	}
	strcat(txt,"\r\n\r\n Test completed.");
	LeaveCriticalSection( lock );
	dlg->PostMessage(WM_COMMAND, jFoxC_MSMQThreadFinished);

	::CloseHandle( _eventEnd );
	return 1;
}


MSMQTestQueueDlg::MSMQTestQueueDlg( CWnd *parent, char *path )
	: sDllDialog( &applibResModule, sFoxD_UserConnectionMSMQTestQueue, parent)
{
	_path = path;
	_txt[0] = 0;
	_thread = NULL;
	_succeed = FALSE;
	_eventEnd = ::CreateEvent(NULL, TRUE, FALSE, NULL);
}

MSMQTestQueueDlg::~MSMQTestQueueDlg	( )
{
	DeleteCriticalSection( &_lock );
}

BOOL MSMQTestQueueDlg::OnCommand( WPARAM wParam, LPARAM lParam )
{
	switch( LOWORD(wParam) )
	{
	case IDOK :
		::SetEvent( _eventEnd );
		break;
	case jFoxC_MSMQThreadFinished:
		SetDlgItemText(IDOK, "OK");
	case jFoxC_MSMQTestOutputUpdate:
		EnterCriticalSection( &_lock );
		SetDlgItemText( jFoxC_MSMQTestOutput, _txt );
		LeaveCriticalSection( &_lock );
		return TRUE;
	}
	return sDllDialog::OnCommand( wParam, lParam );
}

BOOL MSMQTestQueueDlg::OnInitDialog( )
{
	char buf[MAX_PATH];
	strcpy(buf, "Testing queue \"");
	strcat(buf, _path);
	strcat(buf, "\"");
	SetWindowText(buf);
	InitializeCriticalSection( &_lock );
	SetDlgItemText( jFoxC_MSMQTestOutput, _txt );
	
	// start thread
	_thread = AfxBeginThread(MSMQTestQueueThreadFn, this );
	
	return sDllDialog::OnInitDialog();
}


// ********************************************************************************************
//	class MSMQWaitThreadDlg
// ********************************************************************************************
//
// - thread execution dialog
// - thread is not killed in case of cancel


#define MSMQThreadCanceled 0xFF
#define MSMQThreadFinished 0

class MSMQWaitThreadDlg : public sDllDialog
{
	CWinThread			*_thread;
	AFX_THREADPROC		 _threadFn;
	char				*_caption;

	virtual	BOOL	OnCommand		( WPARAM wParam, LPARAM lParam ) ;
	virtual BOOL	OnInitDialog	( ) ;
	
  public:
	LPVOID				 _threadParam;

	MSMQWaitThreadDlg	( CWnd *parent, char *caption, AFX_THREADPROC threadFn, LPVOID pParam );
};

MSMQWaitThreadDlg::MSMQWaitThreadDlg( CWnd *parent, char *caption, AFX_THREADPROC threadFn, LPVOID pParam )
	: sDllDialog( &applibResModule, sFoxD_UserConnectionMSMQWaitThread, parent)
{
	_threadFn = threadFn;
	_threadParam = pParam;
	_caption = caption;
}

BOOL MSMQWaitThreadDlg::OnInitDialog( )
{
	SetWindowText(_caption);
	// start thread
	_thread = AfxBeginThread( _threadFn, this );
	
	return sDllDialog::OnInitDialog();
}

BOOL MSMQWaitThreadDlg::OnCommand( WPARAM wParam, LPARAM lParam )
{
	switch( LOWORD(wParam) )
	{
	case IDCANCEL :
		// _thread->kill
		EndDialog( MSMQThreadCanceled );
		return TRUE;
	case jFoxC_MSMQThreadFinished:
		EndDialog( MSMQThreadFinished );
		return TRUE;
	}
	return sDllDialog::OnCommand( wParam, lParam );
}

// ********************************************************************************************
//
//	MSMQConnectionSetupSheetDialog
//		MSMQ implementation of UTMConnectionSetupDialog
//
// ********************************************************************************************


class MSMQConnectionSetupSheetDialog : public UTMConnectionSetupDialog
{
  protected:
	virtual	BOOL	OnCommand		( WPARAM wParam, LPARAM lParam ) ;
	virtual BOOL	OnInitDialog	( ) ;
	virtual void	DoDataExchange	( CDataExchange* pDX );

	void	decomposeConnectString	( const char *connectString ) ;
	void	updatePath				( ) ;

  private:
	char	 _path[MAX_Q_PATHNAME_LEN];
	char	 _formatName[MAX_Q_FORMATNAME_LEN];
	char	 _defaultQueueName[MAX_Q_PATHNAME_LEN];
	BOOL	 _validFormatName;

	void	findLabelLocations		( ) ;
	BOOL	TestQueue				( )	;
	void	onBrowse				( ) ;
	void	enableDisable			( ) ;

	virtual BOOL	getMediumAllowed		( ) ;
	virtual void	setMediumAllowed		( BOOL a ) ;

  public:
	MSMQConnectionSetupSheetDialog( TransportClass *t, CWnd *parent, const char *defaultQueueName, BOOL bUseAsSheetPage=TRUE ) ;

	virtual	void	getConnectString	( char *buffer ) ;
	virtual	void	setDefaults			( ) ;
} ;

MSMQConnectionSetupSheetDialog::MSMQConnectionSetupSheetDialog( TransportClass *t, CWnd *parent, const char *defaultQueueName, BOOL bUseAsSheetPage )
	: UTMConnectionSetupDialog( t,
		sFoxD_UserConnectionMSMQ,
		parent, bUseAsSheetPage )
{
	_path[0] = 0;
	_formatName[0] = 0;
	strcpy( _defaultQueueName, defaultQueueName );
}

BOOL MSMQConnectionSetupSheetDialog::OnInitDialog()
{
	UTMConnectionSetupDialog::OnInitDialog();
	SetDlgItemText(jFoxC_MSMQLabel, _transportClass->name() );
	if ( ! _transportClass->isServer() )
		GetDlgItem( jFoxC_MSMQMediumAllowed )->ModifyStyle(WS_VISIBLE,0); // remove checkbox from client setup dialog
	return TRUE ;
}

void MSMQConnectionSetupSheetDialog::setDefaults()
{
	_transportClass->setDefaults( MSMQ ) ;
	UpdateData( FALSE ) ;
}

void MSMQConnectionSetupSheetDialog::decomposeConnectString( const char *connectString )
{
	MessageQueue::decomposeConnectString( _formatName, _path, connectString );
}

void MSMQConnectionSetupSheetDialog::getConnectString( char *buffer )
{
	// build open string
	MessageQueue::composeConnectString(buffer, _formatName, _path);
}

void MSMQConnectionSetupSheetDialog::DoDataExchange( CDataExchange* pDX )
{
	sDllDialog::DoDataExchange( pDX ) ;

	char connectString[256] ;

	// _transportClass -> dialog data
	if( !pDX->m_bSaveAndValidate )
	{
		_transportClass->getMediumConnectString( MSMQ, connectString ) ;
		decomposeConnectString( connectString ) ;
		((CButton*)GetDlgItem(jFoxC_MSMQByPath))->SetCheck(1);
		((CButton*)GetDlgItem(jFoxC_MSMQByMachine))->SetCheck(0);
		enableDisable( );
		SetDlgItemText(jFoxC_MSMQPath, _path);
		if ( _transportClass->isServer() )
		{
			BOOL allowed = _transportClass->isServerMediumAllowed( MSMQ ) ;
			setMediumAllowed( allowed ) ;
		}
	}

	// dialog data -> _transportClass
	if( pDX->m_bSaveAndValidate )
	{
		updatePath( );
		getConnectString( connectString ) ;
		_transportClass->setMediumConnectString( MSMQ, connectString ) ;
		if( _transportClass->isServer() )
		{
			BOOL allowed = getMediumAllowed() ;
			_transportClass->setServerMediumAllowed( MSMQ, allowed ) ;
		}
	}
}


//-------------------------------------------------------------------

/*
struct MSMQFindQueueRec
{
	char *_path;
	BOOL  _canceled;
	BOOL  _found;

	MSMQFindQueueRec(char *path)
	{
		_path = path;
		_canceled = _found = FALSE;
	}
};

static UINT MSMQFindQueueThreadFn( LPVOID pParam )
{
	MSMQWaitThreadDlg *dlg = (MSMQWaitThreadDlg*)pParam;
	MSMQFindQueueRec *rec = (MSMQFindQueueRec*)dlg->_threadParam;
	char buf[MAX_PATH];
	strcpy(buf, rec->_path);
	BOOL bFound = MessageQueue::validateConnectString(buf) ;
	if ( rec->_canceled )
	{
		delete rec;
		return 0;
	}
	rec->_found = bFound;
	strcpy(rec->_path, buf);
	dlg->PostMessage(WM_COMMAND, jFoxC_MSMQThreadFinished, 0);
	return 0;
}

BOOL MSMQConnectionSetupSheetDialog::validate( )
{
	if ( _transportClass->isServer() && getMediumAllowed( ) || _transportClass->getClientMediumType() == MSMQ )
	{
		char buf[MAX_PATH];
		updatePath( );
		getConnectString( buf ) ;
		MSMQFindQueueRec *rec = new MSMQFindQueueRec(buf);

		// invoke thread
		MSMQWaitThreadDlg dlg( NULL,"Looking for specified queue ...", MSMQFindQueueThreadFn, rec );
		int retVal = dlg.DoModal();

		if ( retVal == MSMQThreadCanceled )
		{
			rec->_canceled = TRUE;
			return FALSE;
		}

		BOOL bValid = rec->_found;
		delete rec;
	
		if (bValid)
			decomposeConnectString( buf ) ;
		return bValid;
	}
	return TRUE;
}
*/


//-------------------------------------------------------------------


BOOL MSMQConnectionSetupSheetDialog::getMediumAllowed( )
{
	if ( _transportClass->isServer() )
	{
		CButton *btn = (CButton*)GetDlgItem( jFoxC_MSMQMediumAllowed ) ;
		if ( btn )
			return btn->GetCheck() != 0 ;
	}
	return FALSE ;
}

void MSMQConnectionSetupSheetDialog::setMediumAllowed( BOOL a )
{
	if ( _transportClass->isServer() )
	{
		CButton *btn = (CButton*)GetDlgItem( jFoxC_MSMQMediumAllowed ) ;
		if ( btn )
			btn->SetCheck( a ? 1 : 0 ) ;
	}
}

void MSMQConnectionSetupSheetDialog::enableDisable( )
{
	BOOL bMachine, bPath;
	if ( ((CButton*)GetDlgItem(jFoxC_MSMQByMachine))->GetCheck() )
	{
		bMachine = TRUE;
		bPath = FALSE;
	}
	else
	{
		bMachine = FALSE;
		bPath = TRUE;
	}

	GetDlgItem(jFoxC_MSMQMachine)->EnableWindow(bMachine);
	GetDlgItem(jFoxC_MSMQBrowseMachine)->EnableWindow(bMachine);
	GetDlgItem(jFoxC_MSMQPath)->EnableWindow(bPath);
}

BOOL MSMQConnectionSetupSheetDialog::OnCommand( WPARAM wParam, LPARAM lParam )
{
    UINT id = LOWORD(wParam) ;
	switch ( id )
	{
		case jFoxC_MSMQBrowseMachine:
			onBrowse();
			return TRUE;
		case jFoxC_MSMQLocateLabel:
			findLabelLocations( );
			return TRUE;
		case jFoxC_MSMQTestQueue:
			TestQueue();
			return TRUE;
		case jFoxC_MSMQByMachine:
		case jFoxC_MSMQByPath:
			enableDisable();
			return TRUE;
		case IDOK:
		default:
			return sDllDialog::OnCommand( wParam, lParam ) ;
	}
	return TRUE ;
}

void MSMQConnectionSetupSheetDialog::onBrowse()
{
	char compName[MAX_PATH] ; 
	memset( compName, 0, MAX_PATH ) ;
	if ( browseMachineName( this, compName ) )
	{
		GetDlgItem(jFoxC_MSMQMachine)->SetWindowText( compName ) ;
		updatePath( ) ;
	}
}

class MSMQLocateRec
{
  public:
	MQSpecificationPtrArray *_locations;
	char					*_label;
	BOOL					 _canceled;

	MSMQLocateRec( MQSpecificationPtrArray *locations, char *label )
	{
		_locations = locations;
		_label = label;
		_canceled = FALSE;
	}
};

static UINT MSMQLocateQueuesThreadFn( LPVOID pParam )
{
	MSMQWaitThreadDlg *dlg = (MSMQWaitThreadDlg*)pParam;
	MSMQLocateRec *rec = (MSMQLocateRec*)dlg->_threadParam;
	MQSpecificationPtrArray locations;
	char label[MAX_Q_PATHNAME_LEN];
	strcpy(label, rec->_label);
	try
	{
		MessageQueue::locateQueue(label, locations);
	}
	catch(...) { }
	
	if ( ! rec->_canceled )
	{
		// loading result
		MQSpecificationPtrArray *result = rec->_locations;
		int nItems = locations.count();
		for ( int i = 0; i < nItems; i++ )
		{
			MQSpecification *newItem = new MQSpecification( *locations.item(i) );
			result->add(newItem);
		}
		// send finish message to wait dialog
		dlg->PostMessage(WM_COMMAND, jFoxC_MSMQThreadFinished, 0);
	}
	else
		delete rec;		// label location was canceled without deleting rec

	return 0;
}

void MSMQConnectionSetupSheetDialog::findLabelLocations( )
{
	MQSpecificationPtrArray locations;
	char label[MAX_Q_PATHNAME_LEN];
	GetDlgItemText(jFoxC_MSMQLabel, label, MAX_Q_PATHNAME_LEN);
	// creating thread exchange record
	MSMQLocateRec *rec = new MSMQLocateRec( &locations, label );	// dynamic allocation needed by thread (thread can run even when function returns (when dialog is canceled))

	// creating and invoking dialog waiting for thread
	MSMQWaitThreadDlg dlg( this,"Locating queues ...", MSMQLocateQueuesThreadFn, rec );
	int retVal = dlg.DoModal();

	if ( retVal == MSMQThreadCanceled )
	{
		rec->_canceled = TRUE;
		return;
	}

	delete rec;

	int nLocs = locations.count();
	MQSpecification *chosenLoc;
	switch ( nLocs )
	{
		case 0:
			AfxMessageBox("No message queue with specified label was found.");
			return;
		case 1:
			chosenLoc = locations.item(0);
			break;
		default:
			{
				MSMQLocationsDlg dlg(this, &locations, label);
				int ret = dlg.DoModal();
				if (ret != IDOK)
					return;

				int pos = dlg.getCurrent();
				if ( pos < 0 || pos >= locations.count() )
					return;
				chosenLoc = locations.item(pos);
			}
	}

	// store chosen location to class variables
	strcpy(_path, chosenLoc->szPathName);
	strcpy (_formatName, chosenLoc->szFormatName);

	// update dialog controls
	SetDlgItemText( jFoxC_MSMQPath, _path );
	((CButton*)GetDlgItem( jFoxC_MSMQByPath ))->SetCheck(1);
	((CButton*)GetDlgItem(jFoxC_MSMQByMachine))->SetCheck(0);
	enableDisable();
}

void MSMQConnectionSetupSheetDialog::updatePath( )
{
	char buf[MAX_Q_PATHNAME_LEN];
	if ( ((CButton*)GetDlgItem(jFoxC_MSMQByMachine))->GetCheck() )
	{
		GetDlgItemText(jFoxC_MSMQMachine, buf, MAX_Q_PATHNAME_LEN);
		strncat(buf, "\\", MAX_Q_PATHNAME_LEN );
		strncat(buf, _defaultQueueName, MAX_Q_PATHNAME_LEN );
		SetDlgItemText(jFoxC_MSMQPath, buf );
	}
	else
		GetDlgItemText(jFoxC_MSMQPath, buf, MAX_Q_PATHNAME_LEN);

	if ( _stricmp(buf, _path) != 0 )
	{
		strcpy(_path, buf);
		_formatName[0] = 0;
	}
}

BOOL MSMQConnectionSetupSheetDialog::TestQueue( )
{
	updatePath( );
	MSMQTestQueueDlg *dlg = new MSMQTestQueueDlg(this,_path);
	dlg->DoModal();
	const char *fName = dlg->getFormatName();
	if (fName)
		strcpy(_formatName, fName);

	delete dlg;
	return TRUE;
}

// ********************************************************************************************
//
//	TransportClass
//
// ********************************************************************************************


#define CFG_OpenString_Client	"OpenString_Client"
#define CFG_OpenString_Server	"OpenString_Server"
#define CFG_Allowed				"AllowedInServer"
#define CFG_DefaultMediumType	"DefaultClientMedium"

static char *defaults[] = {
	"[TransportMedium_TCPIP]", 0		, 0,	// 0
	CFG_OpenString_Server	, 0			, 0,	// 3
	CFG_OpenString_Client	, 0			, 0,	// 6
	CFG_Allowed				, "1"		, 0,	// 9
	"[TransportMedium_MSMQ]", 0			, 0,	// 12
	CFG_OpenString_Server	, 0			, 0,	// 15
	CFG_OpenString_Client	, 0			, 0,	// 18
	CFG_Allowed				, "0"		, 0,	// 21
	"[TransportMedium]"		, 0			, 0,	// 24
	CFG_DefaultMediumType	, "TCPIP"	, 0,	// 
} ;

#define n_defaults (sizeof(defaults)/sizeof(char*)/3)

TransportClass::TransportClass( const char *nam, const char *key, BOOL is_server, USHORT defTcpPort, int mediumMask )
{
	ASSERT( nam != NULL  &&  key != NULL ) ;

	char errBuf[1024] ;
	if( mediumMask&mTCP && !UniversalTransportMedium::socketInitialize( errBuf) )
		stdErrorDialog( "WinSock initialization failed:\n%s", errBuf ) ;

	memset( serverConnection, 0, sizeof(serverConnection) ) ;
	_name			 = nam ? strdup(nam) : NULL ;
	_key			 = key ? strdup(key) : NULL ;
	_cfg			 = NULL ;
	clientReferences = 0 ;
	clientConnection = 0 ;
	_isServer		 = is_server;
	_srvRcvFnc		 = NULL ;
	_srvParam		 = NULL ;
	_wnd			 = NULL ;
	_blocked		 = FALSE ;
	_defTcpPort		 = defTcpPort ? defTcpPort : 12346 ;
	_handshakeFnc	 = 0 ;
	_handshakeParam  = 0 ;

	_mediumModuleInstalled[SOCKET] = (mediumMask&mTCP)  != 0;
	_mediumModuleInstalled[MSMQ]   = (mediumMask&mMSMQ) != 0;

	char path[512], drv[10], dir[512] ;
	GetModuleFileName( NULL, path, sizeof(path) ) ;
	_splitpath( path, drv, dir, NULL, NULL ) ;

	char configFile[512] ;
	_makepath( configFile, drv, dir, "Cfg", NULL ) ;
	sprintf( configFile+strlen(configFile), "\\Connection_%s.cfg", _key ) ;

	_cfg = new ConfigClass( configFile ) ;
	_cfg->open( NULL, 0 ) ;
	try
	{
		// Install defaults into config file
		char defServerTcpOpenString[100] ;
		char defClientTcpOpenString[100] ;
		char defMSMQopenString[100] ;

		// ":12346"
		sprintf( defServerTcpOpenString, ":%d", _defTcpPort ) ;
		defaults[4] = defServerTcpOpenString ;

		DWORD size = sizeof(defMSMQopenString) - strlen(_key) ;
		char *s ;
		if( GetComputerName( defMSMQopenString, &size) )
		{
			sprintf( defClientTcpOpenString, "%s:%d", defMSMQopenString, _defTcpPort ) ;
			defaults[7] = defClientTcpOpenString ;	// "thisComputer:12346"
			s = defMSMQopenString + size ;
		}
		else
		{
			s = defMSMQopenString ;
			defaults[7] = 0 ;
		}

		defaults[16] = defMSMQopenString ;
		defaults[19] = defMSMQopenString ;
		sprintf( s, "\\%s[]", _key ) ;

		_cfg->install( (const char**)defaults, n_defaults ) ;
	}
	catch( ... ) {}

	if (_mediumModuleInstalled[SOCKET])
		_mediumNames.add( "TCPIP") ;

	if (_mediumModuleInstalled[MSMQ])
		_mediumNames.add( "MSMQ" ) ;
}

TransportClass::~TransportClass()
{
	delete _wnd ;
	_cfg->save( 1 ) ;
	delete _cfg ;
	if( _name != NULL )
		free( (void*)_name ) ;
	if( _key != NULL )
		free( (void*)_key ) ;

	UniversalTransportMedium::socketDeinitialize() ;
}

void TransportClass::setClientHandshake( TransportMediumHandshakeFun f, LPARAM param, TransportMediumMsg *packet )
{
	_handshakeFnc		= f ;
	_handshakeParam		= param ;
	_handshakeSendMsg	= *packet ;
}


//====================================================================

BOOL TransportClass::setupDialog( CWnd *parent )
{
	TransportClassSetupDialog dlg( this, parent ) ;
	return dlg.DoModal() == IDOK ;
}

// show dialog box to retrieve user settings, build the connect string
BOOL TransportClass::askUserConnectString( UTM_TYPE medium, char *connStr, CWnd *parent )
{
	UTMConnectionSetupDialog	*setupDlg ;

	// return dialog page
	setupDlg = getConnectionSetupDlg( medium, parent, FALSE ) ;
	if( setupDlg==NULL || setupDlg->DoModal() != IDOK )
	{
		delete setupDlg;
		return FALSE ;
	}

	// Return connection string
	setupDlg->getConnectString( connStr ) ;
	delete setupDlg;
	return TRUE ;
}


UTMConnectionSetupDialog *TransportClass::getConnectionSetupDlg( UTM_TYPE medium, CWnd *parent, BOOL bShowAsSheetPage )
{
	switch ( medium )
	{
		case SOCKET :
			return new SocketConnectionSetupSheetDialog( this, parent, bShowAsSheetPage ) ;
		case MSMQ:
			return new MSMQConnectionSetupSheetDialog( this, parent, _key, bShowAsSheetPage ) ;
	}
	return NULL ;
}


UTMConnectionSetupDialog *TransportClass::getConnectionSetupDlg( const char *mediumName, CWnd *parent, BOOL bShowAsSheetPage )
{
	return getConnectionSetupDlg( UniversalTransportMedium::mediumName2Type( mediumName), parent, bShowAsSheetPage ) ;
}


UTM_TYPE TransportClass::getClientMediumType( )
{
	char *mediumName = _cfg->get( "#TransportMedium", CFG_DefaultMediumType ) ;
	if ( mediumName != NULL )
		return UniversalTransportMedium::mediumName2Type( mediumName ) ;
	return UNKNOWN ;
}

void TransportClass::setClientMediumType( UTM_TYPE medium )
{
	const char *mediumName = UniversalTransportMedium::mediumType2String( medium ) ;
	if ( mediumName != NULL )
		_cfg->set( "#TransportMedium", CFG_DefaultMediumType, mediumName ) ;
}

const char *TransportClass::getClientMediumName( )
{
	return _cfg->get( "#TransportMedium", CFG_DefaultMediumType ) ;
}

void TransportClass::setClientMediumName( const char *mediumName )
{
	_cfg->set( "#TransportMedium", CFG_DefaultMediumType, mediumName ) ;
}

inline void getMediumSettingsSection( UTM_TYPE medium, char *sect )
{
	// # means absolute section name (no user decoration)
	sprintf( sect, "#TransportMedium_%s", UniversalTransportMedium::mediumType2String( medium ) ) ;
}

const char *TransportClass::getClientConnectString( char *buf )
{
	UTM_TYPE typ = getClientMediumType( ) ;
	if( typ == UNKNOWN )
	{
		buf[0] = 0 ;
		return NULL ;
	}
	getMediumConnectString( typ, buf ) ;
	return buf ;
}

const char *TransportClass::getMediumConnectString( UTM_TYPE medium, char *buffer )
{
	char sect[256] ;
	getMediumSettingsSection( medium, sect ) ;

	buffer[0] = 0 ;
	const char *ostr ;
	if ( _isServer )
		ostr = _cfg->get( sect, CFG_OpenString_Server ) ;
	else
		ostr = _cfg->get( sect, CFG_OpenString_Client ) ;
	if ( ostr )
		strcpy( buffer, ostr ) ;
	return buffer ;
}

void TransportClass::setMediumConnectString( UTM_TYPE medium, const char *buffer )
{
	char sect[256] ;
	getMediumSettingsSection( medium, sect ) ;

	const char *var = _isServer ? CFG_OpenString_Server : CFG_OpenString_Client ;
	_cfg->set( sect, var, buffer ) ;
}

void TransportClass::getPeerLocation( char *locationBuffer, UTM_TYPE medium ) 
{
	if (!locationBuffer)
		return ;

	UniversalTransportMedium *utm = NULL ;
	
	if ( _isServer )
	{
		if ( medium!=UNKNOWN )
			utm = serverConnection[medium] ;
	}
	else
		utm = clientConnection ;

	if ( utm )
		utm->getPeerLocation(locationBuffer) ;
	else
		locationBuffer[0] = '\x0' ;
}

BOOL TransportClass::isServerMediumAllowed( UTM_TYPE medium )
{
	char sect[256] ;
	getMediumSettingsSection( medium, sect ) ;

	int a = 0 ;
	_cfg->getInt( sect, CFG_Allowed, &a ) ;
	return a ;
}

void TransportClass::setServerMediumAllowed( UTM_TYPE medium, BOOL allow )
{
	char sect[256] ;
	getMediumSettingsSection( medium, sect ) ;

	_cfg->setInt( sect, CFG_Allowed, allow ? 1 : 0 ) ;
}

BOOL TransportClass::undo( )
{
	try
	{
		_cfg->reload() ;
		return TRUE ;
	}
	catch( ... ) {}
	return FALSE ;
}

void TransportClass::setDefaults( UTM_TYPE type )
{
	char str[256], *s ;
	getMediumConnectString( type, str ) ;
	switch( type )
	{
		case SOCKET :
			// set standard port while preserving the machine
			s = strchr( str, ':' ) ;
			sprintf( s ? s : str, ":%d", _defTcpPort ) ;
			setServerMediumAllowed( type, TRUE ) ;
			break ;
		case MSMQ :
			// set standard queue label while preserving the machine, eg. "...\\amaqueue[]"
			s = strchr( str, '\\' ) ;
			if( s == NULL )
				s = str ;
			*s++ = '\\' ;
			strcpy( s, _key ) ;
			strcat( s, "[]" ) ;
			setServerMediumAllowed( type, FALSE ) ;
			break ;
	}

	setMediumConnectString( type, str ) ;
}

void TransportClass::saveAndValidate( )
{
	save() ;
	if( _isServer )
	{
		char reason[1024] ;
		if( !resetServerConnection( reason)  ||  *reason != 0 )
			AfxMessageBox( reason ) ;
	}
	else
	{
		resetClientConnection() ;
	}
}

//--------------------------------------------------------------------
//	client
//--------------------------------------------------------------------


BOOL TransportClass::isLocalClientConnection( )
{
	UTM_TYPE mediumType = getClientMediumType( ) ;
	if( mediumType != SOCKET )
		return FALSE ;

	char connectStr[512] ;
	getClientConnectString( connectStr ) ;
	return SocketTransport::isLocalConnection( connectStr ) ;
}

BOOL TransportClass::isClientConnected( )
{
	if( clientConnection == NULL )
		return FALSE ;
	return clientConnection->isConnected() ;
}

class ConnectFailedDlg : public sDllDialog
{
	const char *_errReason ;
	virtual BOOL OnInitDialog( )
	{
		sDllDialog::OnInitDialog() ;
		CWnd *w = GetDlgItem(jFoxC_ConnErrorDesc) ;
		if( w != NULL )
			w->SetWindowText( _errReason ) ;
		return TRUE ;
	}
	
  public:
	ConnectFailedDlg( const char *caption, const char *errReason ) :
		sDllDialog( &applibResModule, sFoxD_ConnectionFailedDlg, AfxGetMainWnd(), caption )
			{ _errReason = errReason ; }
} ;

BOOL TransportClass::openAsClient( UTM_TYPE mediumType, char *reason, BOOL interactivelly )
{
	char openString[256] ;
	getMediumConnectString( mediumType, openString ) ;

	if( !interactivelly )
	{
		BOOL ret = FALSE ;
		try
		{
			UniversalTransportMedium *conn = new UniversalTransportMedium() ;
			if( conn->openAsClient( mediumType, openString, reason,
				_handshakeFnc, _handshakeParam, &_handshakeSendMsg) )
			{
				ret = TRUE ;
				clientConnection = conn ;
			}
			else
				delete conn ;
		}
		catch( ... ) {}

		return ret ;
	}

	clientConnection = new UniversalTransportMedium() ;
	BOOL needRetrieveUserSettings = (*openString == 0) ;

	for( int tryCount=0 ; tryCount <= MAX_CONNECT_RETRYS ; ++tryCount )
	{
		if ( reason )
			*reason = 0 ;
		if ( needRetrieveUserSettings )
		{
			if( !askUserConnectString( mediumType, openString, AfxGetMainWnd()) )
				goto labelCancel ;
		}

		if( clientConnection->openAsClient( mediumType, openString, reason,
			_handshakeFnc, _handshakeParam, &_handshakeSendMsg) )
		{
			setMediumConnectString( mediumType, openString ) ;
			save() ;
			return TRUE ;
		}

		if ( tryCount < MAX_CONNECT_RETRYS )
		{
			ConnectFailedDlg dlg( _name, reason ) ;
			if ( dlg.DoModal() != IDOK )
				goto labelCancel ;
		}
		needRetrieveUserSettings = TRUE ;
	}

  labelCancel:
	if ( reason )
		strcpy( reason, "Connection cancelled" ) ;
	DEL( clientConnection ) ;		// fatal error, force reopen
	return FALSE ;
}

void TransportClass::registerClientConnection()
{
	clientReferences++ ;
}

void TransportClass::unregisterClientConnection()
{
	clientReferences-- ;
	if ( clientReferences == 0 )
	{
		try
		{
			DEL( clientConnection ) ;
		}
		catch( ... )
		{
			clientConnection = NULL ;
		}
	}
}

void TransportClass::resetClientConnection()
{
	// Delete client connection if the setup was changed.
	// (The connection will be re-created on as needed basis.)

	if( clientConnection == NULL )
		return ;

	UTM_TYPE mediumType = getClientMediumType( ) ;
	if(  clientConnection->mediumType() == mediumType )
	{
		char connectString[256] ;
		getMediumConnectString( mediumType, connectString ) ;
		if( clientConnection->connectStringEqual( connectString) )
			// parameters did not change
			return ;
	}
	DEL( clientConnection ) ;
}

BOOL TransportClass::connectClient( char *reason, BOOL interactivelly )
{
	try
	{
		if ( reason )
			*reason = 0 ;
		if( clientConnection != NULL )
			return TRUE ;

		UTM_TYPE mediumType = getClientMediumType( ) ;
		if( mediumType == UNKNOWN )
		{
			if ( reason )
				strcpy( reason, "No transport medium selected" ) ;
			return FALSE ;
		}

		if (!_mediumModuleInstalled[mediumType])
		{
			if ( reason )
				strcpy( reason, "Selected transport medium not installed." ) ;
			return FALSE ;
		}

		if( openAsClient( mediumType, reason, interactivelly) )
			return TRUE ;
	}
	catch( ... )
	{
		if( reason )
			strcpy( reason, "Unknown error" ) ;
	}
	return FALSE ;
}

BOOL TransportClass::sendClientData( const TransportMediumMsg &userData, char *reason, BOOL interactivelly )
{
	_lock.Lock() ;
	BOOL ret = FALSE ;
	try
	{
		if( connectClient( reason, interactivelly) )
		{
			if( clientConnection->sendData( &userData) )
				ret = TRUE ;
			else
			{
				if ( reason )
					clientConnection->errorString( reason ) ;

				if( !clientConnection->hasTimeoutError() )
					DEL( clientConnection ) ;		// fatal error, force reopen
			}
		}
	}
	catch( ... )
	{
		if( reason )
			strcpy( reason, "Unknown error" ) ;
	}
	_lock.Unlock() ;
	return ret ;
}

BOOL TransportClass::canDoReply( )
{
	if( clientConnection != NULL )
		return  clientConnection->isConnectionOriented() ;

	// Connection not established yet
	UTM_TYPE mediumType = getClientMediumType( ) ;
	return mediumType == SOCKET ;
}

// receive reply buffer[maxLength] for packet <sentPacketId>
BOOL TransportClass::getClientReply( DWORD sentPacketId, TransportMediumMsg &reply, char *reason )
{
	_lock.Lock() ;
	try
	{
		if( !isClientConnected() )
		{
			if( reason )
				strcpy( reason, "Connection closed." ) ;
		}
		else
		if( !canDoReply() )
			ASSERT( 0 ) ;
		else
		{
			for( int tryCount=0 ; tryCount < MAX_CONNECT_RETRYS ; tryCount++ )
			{
				if( clientConnection->receiveData( &reply) )
				{
					if( reply.packetId() == sentPacketId )
					{
						_lock.Unlock() ;
						return TRUE ;
					}

					if ( reply.packetId() > sentPacketId )
						break ;
				}
			}
			if( clientConnection->hasTimeoutError() )
			{
				if( reason )
					strcpy( reason, "Timeout exceeded." ) ;
			}
			else
			if( !clientConnection->hasError() )
			{
				if( reason )
					strcpy( reason, "No reply received." ) ;
			}
			else
			{
				if( reason )
					clientConnection->errorString( reason ) ;
				DEL( clientConnection ) ;		// fatal error, force reopen
			}
		}
	}
	catch( ... )
	{
		if( reason )
			strcpy( reason, "Unknown error" ) ;
	}
	_lock.Unlock() ;
	return FALSE ;
}


//--------------------------------------------------------------------
//	server
//--------------------------------------------------------------------


BOOL TransportClass::startReceivingMediumData( UTM_TYPE mediumType, char *reason )
{
	ASSERT( serverConnection[mediumType] == NULL ) ;

	// retrieve the open string
	char openString[256] ;
	getMediumConnectString( mediumType, openString ) ;

	char errStr[1024] ;
	BOOL bModuleInstalled ;
	serverConnection[mediumType] = new UniversalTransportMedium() ;
	if( serverConnection[mediumType]->openAsServer( mediumType, openString, _srvRcvFnc, _srvParam, errStr, &bModuleInstalled ) )
	{
		onServerMediumConnect( mediumType, TRUE ) ;
		if( reason )
			*reason = 0 ;
		_mediumModuleInstalled[mediumType] = TRUE;
		return TRUE ;
	}

	if( reason )
		sprintf( reason, "%s - %s:\n%s", UniversalTransportMedium::mediumType2String(mediumType),
				 "Failed to open the connection", errStr ) ;

	_mediumModuleInstalled[mediumType] = bModuleInstalled;
	if ( ! bModuleInstalled )
		setServerMediumAllowed( mediumType, FALSE ) ;

	// force reopen and reconnect
	delete serverConnection[mediumType] ;
	serverConnection[mediumType] = NULL ;
	return FALSE ;
}

BOOL TransportClass::startServerReceivingData( TransportMediumRcvFun rcvFnc, LPARAM param, char *reason )
{
	ASSERT( rcvFnc != NULL ) ;
	_srvRcvFnc	= rcvFnc ;
	_srvParam	= param ;

	BOOL ret = FALSE ;
	int errorCnt = 0 ;
	if( reason )
		*reason = 0 ;

	int cntAllowed=0 ;
	for( int i=NUMOFMEDIA-1 ; i >= 0  ; i-- )
	{
		char subReason[256] ;
		UTM_TYPE mediumType = (UTM_TYPE)i ;
		if( ! ( _mediumModuleInstalled[mediumType] && isServerMediumAllowed( mediumType) ) )
			continue ;

		cntAllowed++ ;
		if( serverConnection[mediumType] != NULL )
		{
			// already constructed; happens in resetServerConnection()
			ret = TRUE ;
			continue ;
		}

		if( !startReceivingMediumData( mediumType, subReason) )
		{
			errorCnt++ ;
			if( errorCnt < 4 )
			{
				if( reason )
				{
					if( errorCnt > 1 )
						strcat( reason, "\n\n" ) ;
					strcat( reason, subReason ) ;
				}
			}
			else
			if( errorCnt == 4 )
			{
				if( reason )
					strcat( reason, "\n..." ) ;
			}
		}
		else
			ret = TRUE ;
	}
	if( cntAllowed == 0 )
		if( reason )
			strcpy( reason, "No transport medium allowed." ) ;

	if( ret )
		startTimer() ;
	return ret ;
}

void TransportClass::stopReceivingMediumData( UTM_TYPE mediumType )
{
	if( serverConnection[mediumType] != NULL )
	{
		delete serverConnection[mediumType] ;
		serverConnection[mediumType] = NULL ;
		onServerMediumConnect( mediumType, FALSE ) ;
	}
}

void TransportClass::stopServerReceivingData()
{
	stopTimer() ;
	for( int i=NUMOFMEDIA-1 ; i >= 0  ; i-- )
		stopReceivingMediumData( (UTM_TYPE)i ) ;
}

BOOL TransportClass::resetServerConnection( char *reason )
{
	if( isBlocked() )
		return FALSE ;
	BOOL ret=FALSE ;

	try
	{
		setBlocked( TRUE ) ;
		for( int i=NUMOFMEDIA-1 ; i >= 0  ; i-- )
		{
			UTM_TYPE mediumType = (UTM_TYPE)i ;
			if( !isServerMediumAllowed( mediumType) )
			{
				stopReceivingMediumData( mediumType ) ;
				continue ;
			}

			UniversalTransportMedium *medium = serverConnection[mediumType] ;
			if( medium != NULL )
			{
				// retrieve the open string
				char newConnectString[256] ;
				getMediumConnectString( mediumType, newConnectString ) ;
				if( medium->connectStringEqual( newConnectString) )
					continue ;
				stopReceivingMediumData( mediumType ) ;
			}
		}
		ret = startServerReceivingData( _srvRcvFnc, _srvParam, reason ) ;
	}
	catch( ... )
	{
	}
	setBlocked( FALSE ) ;
	return ret ;
}


//--------------------------------------------------------------------
//	timer
//	Sends periodically alive signal, which is used to refresh opened channels.
//--------------------------------------------------------------------


class TransportWnd : public CWnd
{
	TransportClass *_transport ;
  public:
	TransportWnd( TransportClass *transport ) 
	{
		_transport = transport ;
	}

    DECLARE_MESSAGE_MAP()

	afx_msg void OnTimer(UINT nIDEvent)
	{
		_transport->sendAliveSignal() ;
	}
} ;

BEGIN_MESSAGE_MAP(TransportWnd, CWnd)
    ON_WM_TIMER()
END_MESSAGE_MAP()


void TransportClass::startTimer()
{
	if( _wnd != NULL )
		return ;
	_wnd = new TransportWnd( this ) ;
	_wnd->CreateEx(  WS_EX_TOOLWINDOW, AfxRegisterWndClass(0), "TransportWindow", 0, 0, 0, 10, 10, NULL, NULL );
	_wnd->SetTimer(1, 2000, NULL );
}

void TransportClass::stopTimer()
{
	delete _wnd ;
	_wnd = NULL ;
}

void TransportClass::sendAliveSignal( )
{
	for( int i=NUMOFMEDIA-1 ; i >= 0  ; i-- )
	if( serverConnection[i] != NULL )
	{
		if( !serverConnection[i]->refresh() )
			stopReceivingMediumData( (UTM_TYPE)i ) ;
	}
}
