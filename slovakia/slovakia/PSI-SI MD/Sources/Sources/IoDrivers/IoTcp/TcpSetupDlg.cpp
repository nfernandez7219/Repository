#include "tools2.hpp"
#include "loadRes.hpp"
#include "DrvInterface.hpp"
#include "tcp.hpp"
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

/////////////////////////////////////////////////////////////////////////////
// TcpSetupDlg dialog

class TcpSetupDlg : public sDllDialog
{
	ConfigClass*	_cfg ;
	BOOL			_bServer ;
	BOOL			_bReadOnly ;
// Construction
public:
	TcpSetupDlg	(CWnd* pParent, ConfigClass *cfg, BOOL bServer, BOOL bReadOnly );  

	virtual ~TcpSetupDlg( ) ;

// Dialog Data
	//{{AFX_DATA(TcpSetupDlg)
	enum { IDD = IDD_TcpSetup };
	CIPAddressCtrl	m_IP;
	UINT	m_Port;
	int		m_Protocol;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TcpSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TcpSetupDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// TcpSetupDlg dialog


TcpSetupDlg::TcpSetupDlg(CWnd* pParent, ConfigClass *cfg, BOOL bServer, BOOL bReadOnly )
  : sDllDialog( getRcModule(), TcpSetupDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(TcpSetupDlg)
	m_Port = 0;
	m_Protocol = 0;
	//}}AFX_DATA_INIT

	_cfg		= cfg	;
	_bServer	= bServer ;
	_bReadOnly	= bReadOnly ;
}

TcpSetupDlg::~TcpSetupDlg( )
{
	releaseRcModule() ;
}

void TcpSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	sDllDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TcpSetupDlg)
	DDX_Control(pDX, IDC_edtIpAddress, m_IP);
	DDX_Text(pDX, IDC_edtIpPort, m_Port);
	DDV_MinMaxUInt(pDX, m_Port, 0, 65535);
	DDX_Radio(pDX, IDC_btnUdp, m_Protocol);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TcpSetupDlg, sDllDialog)
	//{{AFX_MSG_MAP(TcpSetupDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TcpSetupDlg message handlers

void TcpSetupDlg::OnOK() 
{
	if (!UpdateData(TRUE))
		return ;

	BYTE b1,b2,b3,b4 ;
	m_IP.GetAddress(b1,b2,b3,b4) ;

	char st[256] ;
	if (_bServer)
	{
		sprintf( st, "%d.%d.%d.%d", b1,b2,b3,b4 ) ;
		_cfg->set(CFGSECT, "Host", st ) ;
	}
	_cfg->set(CFGSECT, "Protocol", m_Protocol?"tcp":"udp" ) ;
	itoa(m_Port, st,10) ;
	_cfg->set(CFGSECT, "Port", st ) ;

	sDllDialog::OnOK();
}

BOOL TcpSetupDlg::OnInitDialog() 
{
	ULONG ip = 0 ;
	if (_bServer)
	{
		char *host = _cfg->get(CFGSECT, "Host" ) ;
		ip = inet_addr(host) ;
		if (ip==INADDR_NONE)
			ip = 0 ;
	}

	char *protocol = _cfg->get(CFGSECT, "Protocol" ) ;
	if (protocol && strupr(protocol) && strstr(protocol,"TCP")!=0)
		m_Protocol = 1 ;

	char *port = _cfg->get(CFGSECT, "Port" ) ;
	m_Port = port?atoi(port):4321 ;

	sDllDialog::OnInitDialog();

	BYTE *b = (BYTE*)&ip ;
	m_IP.SetAddress(b[0],b[1],b[2],b[3]) ;

	if (_bReadOnly)
	{
		GetDlgItem(IDC_edtIpPort	)->EnableWindow(FALSE) ;
		GetDlgItem(IDC_edtIpAddress	)->EnableWindow(FALSE) ;
		GetDlgItem(IDC_btnUdp		)->EnableWindow(FALSE) ;
		GetDlgItem(IDC_btnTcp		)->EnableWindow(FALSE) ;
		GetDlgItem(IDOK				)->EnableWindow(FALSE) ;
	}

	if (!_bServer)
		GetDlgItem(IDC_edtIpAddress	)->EnableWindow(FALSE) ;
	
	return TRUE;
}

static BOOL clientDialogRunning = FALSE ;
static BOOL serverDialogRunning = FALSE ;

void TcpSetupDialog( CWnd* wnd, ConfigClass *cfg, BOOL asServer )
{
	BOOL *runningFlag = (asServer?&serverDialogRunning:&clientDialogRunning) ;
	if( !(*runningFlag) )
	{
		*runningFlag = TRUE ;
		TcpSetupDlg dlg( wnd, cfg, asServer, bOutputOpened ) ;
		dlg.DoModal() ;
		*runningFlag = FALSE ;
	}
}

