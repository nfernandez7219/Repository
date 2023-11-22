
#include "stdafx.h"
#include "Descriptors.h"
#include "TableHolders.h"
#include "WizardDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable:4355)

int RunNetworkWizard(TableHolder *tableHolder, CWnd *parent) ;

class TableHolder ;
class NetworkWizardDlg ;

/////////////////////////////////////////////////////////////////////////////
// NetworkWizPage1 dialog

class NetworkWizPage1 : public CDialog
{
// Construction
public:
	NetworkWizPage1(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(NetworkWizPage1)
	enum { IDD = IDD_NewNtwWiz_1 };
	CStatic	m_NetworkIdText;
	CComboBox	m_NetIdList;
	CString	m_NetName;
	CString	m_NetworkId;
	//}}AFX_DATA
	UINT	m_uiNetId ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NetworkWizPage1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NetworkWizPage1)
	virtual BOOL OnInitDialog();
	afx_msg void OnNetworkIdSelected();
	afx_msg void OnNetworkIdEdited();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// NetworkWizPage2 dialog

class NetworkWizPage2 : public CDialog
{
	NetworkWizardDlg	*_pParent ;
// Construction
public:
	NetworkWizPage2(NetworkWizardDlg* pParent ) ;

// Dialog Data
	//{{AFX_DATA(NetworkWizPage2)
	enum { IDD = IDD_NewNtwWiz_2 };
	CStatic	m_OrigNetIdText;
	CComboBox	m_OrigNetId;
	UINT	m_TsId;
	int		m_Delivery;
	CString	m_stOrigNetId;
	//}}AFX_DATA
	UINT	m_uiOrigNetId ;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NetworkWizPage2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NetworkWizPage2)
	virtual BOOL OnInitDialog();
	afx_msg void OnNetworkIdSelected();
	afx_msg void OnDeliveryChanged();
	afx_msg void OnNetworkIdEdited();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// NetworkWizPageSat dialog

class NetworkWizPageSat : public CDialog
{
// Construction
public:
	NetworkWizPageSat(CWnd* pParent = NULL);   // standard constructor

	inline void		SetItenFocus (UINT id)				{ GetDlgItem(id)->SetFocus(); }

// Dialog Data
	//{{AFX_DATA(NetworkWizPageSat)
	enum { IDD = IDD_NewNtwWiz_Sat };
	int		m_Fec;
	double	m_Freq;
	double	m_OrbitPos;
	double	m_SymbRate;
	int		m_WestEast;
	int		m_Polar;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NetworkWizPageSat)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NetworkWizPageSat)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// NetworkWizPageCable dialog

class NetworkWizPageCable : public CDialog
{
// Construction
public:
	NetworkWizPageCable(CWnd* pParent = NULL);   // standard constructor

	inline void		SetItenFocus (UINT id)				{ GetDlgItem(id)->SetFocus(); }

// Dialog Data
	//{{AFX_DATA(NetworkWizPageCable)
	enum { IDD = IDD_NewNtwWiz_Cable };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NetworkWizPageCable)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NetworkWizPageCable)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// NetworkWizPageTer dialog

class NetworkWizPageTer : public CDialog
{
// Construction
public:
	NetworkWizPageTer(CWnd* pParent = NULL);   // standard constructor

	inline void		SetItenFocus (UINT id)				{ GetDlgItem(id)->SetFocus(); }

// Dialog Data
	//{{AFX_DATA(NetworkWizPageTer)
	enum { IDD = IDD_NewNtwWiz_Ter };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NetworkWizPageTer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NetworkWizPageTer)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class NetworkWizardDlg : public WizardDlg
{
	NetworkWizPage1		_page1 ;
	NetworkWizPage2		_page2 ;

	NetworkWizPageCable	_cablePage ;
	NetworkWizPageSat	_satPage ;
	NetworkWizPageTer	_terPage ;

	BOOL				_bWasOnPage2 ;

	TableHolder*		_tableHolder ;

protected:
	virtual	void	OnPageChange ( int newPageIndex ) ;
	virtual BOOL	OnInitDialog ( )  ;
	virtual void	OnOK		 ( );

public:
	NetworkWizardDlg( TableHolder *tableHolder, CWnd *parent=NULL ) ;

	void	SetDeliveryPage ( int delivery ) ;
} ;


/////////////////////////////////////////////////////////////////////////////
// NetworkWizPage1 dialog

NetworkWizPage1::NetworkWizPage1(CWnd* pParent /*=NULL*/)
	: CDialog(NetworkWizPage1::IDD, pParent)
{
	//{{AFX_DATA_INIT(NetworkWizPage1)
	m_NetName = _T("");
	m_NetworkId = _T("");
	//}}AFX_DATA_INIT
}

static void addNetworkIdsToCombo( CComboBox *combo )
{
	int num = 0 ;
	for ( int i = 1; i <= 0xFF00; i++ )
	{
		char buf[256] ;
		networkIdAsText(i, buf) ;
		if (buf[0]!='\x0')
		{
			num++ ;
			int index = combo->AddString(buf) ;
			combo->SetItemData(index, i) ;
		}
	}
}

BOOL NetworkWizPage1::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	addNetworkIdsToCombo( &m_NetIdList ) ;
	
	return TRUE;
}

static BOOL checkNetworkId ( const char *id, UINT &val )
{
	char *end ;
	val = strtol(id, &end, 16) ;
	if (*end != '\x0')
	{
		AfxGetMainWnd()->MessageBox( "Enter hexadecimal value.", "Error", MB_OK|MB_ICONERROR ) ;
		return FALSE ;
	}

	if (val < 1 || val > 0xFFFF)
	{
		AfxGetMainWnd()->MessageBox( "Enter hexadecimal value from interval 0x0001-0xFFFF.", "Error", MB_OK|MB_ICONERROR ) ;
		return FALSE ;
	}
	
	return TRUE ;
}

void NetworkWizPage1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NetworkWizPage1)
	DDX_Control(pDX, IDC_NetwIdAsText, m_NetworkIdText);
	DDX_Text(pDX, IDC_NetworkName, m_NetName);
	DDV_MaxChars(pDX, m_NetName, 250);
	DDX_CBString(pDX, IDC_cbNetworkId, m_NetworkId);
	DDX_Control(pDX, IDC_cbNetworkId, m_NetIdList);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate && !checkNetworkId(m_NetworkId, m_uiNetId ) )
		pDX->Fail() ;
}

BOOL NetworkWizPage1::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (wParam==ID_RefreshValueList)
	{
		int val = (int)lParam ;
		char buf[256] ;
		networkIdAsText(val, buf) ;
		m_NetworkIdText.SetWindowText(buf) ;

		sprintf(buf, "0x%x", val) ;
		strupr(buf+2) ;
		m_NetIdList.SetWindowText(buf) ;
		return TRUE ;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

void NetworkWizPage1::OnNetworkIdSelected() 
{
	int i = m_NetIdList.GetCurSel() ;
	if (i==CB_ERR)
		return ;

	int val = m_NetIdList.GetItemData(i) ;
	PostMessage(WM_COMMAND, ID_RefreshValueList, (LPARAM)val) ;
}

void NetworkWizPage1::OnNetworkIdEdited() 
{
	char buf[256], *end ;
	m_NetIdList.GetWindowText(buf, 256) ;
	
	long val = strtol(buf, &end, 16) ;
	if (*end != '\x0')
		strcpy(buf, "Error: Not hexadecimal number") ;
	else
	{
		networkIdAsText(val, buf) ;
		if (buf[0] == '\x0')
			strcpy(buf, "Error: Invalid value") ;
	}
	m_NetworkIdText.SetWindowText(buf) ;
}

BEGIN_MESSAGE_MAP(NetworkWizPage1, CDialog)
	//{{AFX_MSG_MAP(NetworkWizPage1)
	ON_CBN_SELCHANGE(IDC_cbNetworkId, OnNetworkIdSelected)
	ON_CBN_EDITCHANGE(IDC_cbNetworkId, OnNetworkIdEdited)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// NetworkWizPage2 dialog

NetworkWizPage2::NetworkWizPage2(NetworkWizardDlg* pParent)
	: CDialog(NetworkWizPage2::IDD, pParent)
{
	//{{AFX_DATA_INIT(NetworkWizPage2)
	m_TsId = 0;
	m_Delivery = 1;
	m_stOrigNetId = _T("");
	//}}AFX_DATA_INIT

	_pParent = pParent ;
}

BOOL NetworkWizPage2::OnInitDialog() 
{
	CDialog::OnInitDialog();

	addNetworkIdsToCombo( &m_OrigNetId ) ;
	_pParent->SetDeliveryPage(m_Delivery) ;
	
	return TRUE;
}

void NetworkWizPage2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NetworkWizPage2)
	DDX_Control(pDX, IDC_OrigNetwIdAsText, m_OrigNetIdText);
	DDX_Control(pDX, IDC_cbOrigNetworkId, m_OrigNetId);
	DDX_Text(pDX, IDC_TsId, m_TsId);
	DDV_MinMaxUInt(pDX, m_TsId, 1, 65535);
	DDX_Radio(pDX, IDC_Cable, m_Delivery);
	DDX_CBString(pDX, IDC_cbOrigNetworkId, m_stOrigNetId);
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate && !checkNetworkId(m_stOrigNetId, m_uiOrigNetId) )
		pDX->Fail() ;
}

BOOL NetworkWizPage2::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (wParam==ID_RefreshValueList)
	{
		int val = (int)lParam ;
		char buf[256] ;
		networkIdAsText(val, buf) ;
		m_OrigNetIdText.SetWindowText(buf) ;

		sprintf(buf, "0x%x", val) ;
		strupr(buf+2) ;
		m_OrigNetId.SetWindowText(buf) ;
		return TRUE ;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

void NetworkWizPage2::OnNetworkIdEdited() 
{
	char buf[256], *end ;
	m_OrigNetId.GetWindowText(buf, 256) ;
	
	long val = strtol(buf, &end, 16) ;
	if (*end != '\x0')
		strcpy(buf, "Error: Not hexadecimal number") ;
	else
	{
		networkIdAsText(val, buf) ;
		if (buf[0] == '\x0')
			strcpy(buf, "Error: Invalid value") ;
	}
	m_OrigNetIdText.SetWindowText(buf) ;
}

static BOOL isRadioSelected( CDialog *dlg, UINT radioId )
{
	CButton *radio = (CButton*)dlg->GetDlgItem(radioId) ;
	return radio->GetCheck()!=0 ;
}

void NetworkWizPage2::OnDeliveryChanged() 
{
	int delivery ;

		 if ( isRadioSelected(this, IDC_Cable) )		delivery = 0 ;
	else if ( isRadioSelected(this, IDC_Satellite) )	delivery = 1 ;
	else if ( isRadioSelected(this, IDC_Terrestrial) )	delivery = 2 ;
	else return ;

	_pParent->SetDeliveryPage(delivery) ;
}

void NetworkWizPage2::OnNetworkIdSelected() 
{
	int i = m_OrigNetId.GetCurSel() ;
	if (i==CB_ERR)
		return ;

	int val = m_OrigNetId.GetItemData(i) ;
	PostMessage(WM_COMMAND, ID_RefreshValueList, (LPARAM)val) ;
}

BEGIN_MESSAGE_MAP(NetworkWizPage2, CDialog)
	//{{AFX_MSG_MAP(NetworkWizPage2)
	ON_CBN_SELCHANGE(IDC_cbOrigNetworkId, OnNetworkIdSelected)
	ON_BN_CLICKED(IDC_Cable, OnDeliveryChanged)
	ON_BN_CLICKED(IDC_Satellite, OnDeliveryChanged)
	ON_BN_CLICKED(IDC_Terrestrial, OnDeliveryChanged)
	ON_CBN_EDITCHANGE(IDC_cbOrigNetworkId, OnNetworkIdEdited)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// NetworkWizPageSat dialog


NetworkWizPageSat::NetworkWizPageSat(CWnd* pParent /*=NULL*/)
	: CDialog(NetworkWizPageSat::IDD, pParent)
{
	//{{AFX_DATA_INIT(NetworkWizPageSat)
	m_Fec = 3;
	m_Freq = 0.0;
	m_OrbitPos = 0.0;
	m_SymbRate = 0.0;
	m_WestEast = 0;
	m_Polar = 0;
	//}}AFX_DATA_INIT
}


void NetworkWizPageSat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NetworkWizPageSat)
	DDX_CBIndex(pDX, IDC_cbFec, m_Fec);
	DDX_Text(pDX, IDC_SatFreq, m_Freq);
	DDV_MinMaxDouble(pDX, m_Freq, 0., 200.);
	DDX_Text(pDX, IDC_SatOrbitPos, m_OrbitPos);
	DDV_MinMaxDouble(pDX, m_OrbitPos, 0., 180.);
	DDX_Text(pDX, IDC_SatSymbRate, m_SymbRate);
	DDV_MinMaxDouble(pDX, m_SymbRate, 1.e-004, 200.);
	DDX_Radio(pDX, IDC_SatWest, m_WestEast);
	DDX_CBIndex(pDX, IDC_cbSatPolar, m_Polar);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NetworkWizPageSat, CDialog)
	//{{AFX_MSG_MAP(NetworkWizPageSat)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// NetworkWizPageCable dialog

NetworkWizPageCable::NetworkWizPageCable(CWnd* pParent /*=NULL*/)
	: CDialog(NetworkWizPageCable::IDD, pParent)
{
	//{{AFX_DATA_INIT(NetworkWizPageCable)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void NetworkWizPageCable::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NetworkWizPageCable)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NetworkWizPageCable, CDialog)
	//{{AFX_MSG_MAP(NetworkWizPageCable)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// NetworkWizPageTer dialog

NetworkWizPageTer::NetworkWizPageTer(CWnd* pParent /*=NULL*/)
	: CDialog(NetworkWizPageTer::IDD, pParent)
{
	//{{AFX_DATA_INIT(NetworkWizPageTer)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void NetworkWizPageTer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NetworkWizPageTer)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(NetworkWizPageTer, CDialog)
	//{{AFX_MSG_MAP(NetworkWizPageTer)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//	NetworkWizard
/////////////////////////////////////////////////////////////////////////////

NetworkWizardDlg::NetworkWizardDlg( TableHolder *tableHolder, CWnd *parent )
 :
	_page1(this),
	_page2(this),
	_cablePage(this),
	_satPage(this),
	_terPage(this),
	WizardDlg(parent, &_page1, &_page2) 
{
	_tableHolder = tableHolder ;

	_bWasOnPage2 =  FALSE ;
}

BOOL NetworkWizardDlg::OnInitDialog() 
{
	WizardDlg::OnInitDialog();

	SetWindowText("New Network Wizard") ;

	return TRUE ;
}

void NetworkWizardDlg::SetDeliveryPage ( int delivery )
{
	switch (delivery)
	{
		case 0 : SetPage(2, &_cablePage) ; break ;
		case 1 : SetPage(2, &_satPage) ; break ;
		case 2 : SetPage(2, &_terPage) ; break ;
	}
}

void NetworkWizardDlg::OnPageChange( int newPageIndex )
{
}

void NetworkWizardDlg::OnOK() 
{
	if (!_page1.UpdateData(TRUE) )
	{
		SetDlgActive(0) ;
		return ;
	}
	if (!_page2.UpdateData(TRUE) )
	{
		SetDlgActive(1) ;
		return ;
	}

	int delivery = _page2.m_Delivery ;
	CDialog *delivPage = NULL ;
	switch (delivery)
	{
		case 0: delivPage = &_cablePage; break ;
		case 1: delivPage = &_satPage; break ;
		case 2: delivPage = &_terPage; break ;
	} ;
	
	if ( delivPage && !delivPage->UpdateData(TRUE) )
	{
		SetDlgActive(2) ;
		return ;
	}

	// update table holder

	// NIT - Network ID
	_tableHolder->setNITnetId(_page1.m_uiNetId) ;

	_tableHolder->setSDT_TS(_page2.m_TsId, _page2.m_uiOrigNetId) ;

	// NIT - Network Name Descriptor
	NetworkNameDescHolder *descHolder = new NetworkNameDescHolder ;
	NetworkNameDescriptor *netNameDesc = descHolder->getDesc() ;
	netNameDesc->create(_page1.m_NetName) ;
	DescriptorArray *nitDescArr = _tableHolder->NIT_descriptors() ;
	nitDescArr->clearList() ;
	nitDescArr->add(descHolder) ;

	// NIT - Transport Stream
	NitTSHolderArray *trStreams = _tableHolder->NIT_transpStreams() ;
	trStreams->clearList() ;
	NitTransportStreamHolder *tsHolder = new NitTransportStreamHolder(_page2.m_TsId, _page2.m_uiOrigNetId) ;
	trStreams->add(tsHolder) ;

	// NIT - Transport Stream - Delivery system descriptor
	DescriptorArray *tsDescArr = tsHolder->descriptors() ;
	switch (delivery)
	{
		case 0: break ;
		case 1: 
		{
			SatelliteDeliveryDescHolder *satHolder = new SatelliteDeliveryDescHolder ;
			SatelliteDeliveryDescriptor *satDesc = satHolder->getDesc() ;
			satDesc->create(
						_satPage.m_Freq,			// freq
						_satPage.m_OrbitPos,		// orbital pos
						_satPage.m_WestEast,		// westEast
						_satPage.m_Polar,			// polarization
						_satPage.m_SymbRate,		// symbol rate
						_satPage.m_Fec				// FEC rate
			) ;
			tsDescArr->add(satHolder) ;
			break ;
		}
		case 2: break ;
	} ;

	CDialog::OnOK() ;
}

int RunNetworkWizard(TableHolder *tableHolder, CWnd *parent)
{
	NetworkWizardDlg dlg(tableHolder, parent) ;
	return dlg.DoModal() ;
}

/////////////////////////////////////////////////////////////////////////////
// SatDelivEdit dialog

class SatDelivEdit : public CDialog
{
	NetworkWizPageSat				_satPage ;
	SatelliteDeliveryDescriptor*	_satDesc ;
	int								_itemIndex ;
// Construction
public:
	SatDelivEdit(SatelliteDeliveryDescriptor *satDesc, int itemIndex, CWnd* pParent = NULL);

	//{{AFX_DATA(SatDelivEdit)
	enum { IDD = IDD_DeliveryEdit };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

protected:

	//{{AFX_MSG(SatDelivEdit)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
};
/////////////////////////////////////////////////////////////////////////////
// SatDelivEdit dialog


SatDelivEdit::SatDelivEdit(SatelliteDeliveryDescriptor *satDesc, int itemIndex, CWnd* pParent)
	: CDialog(SatDelivEdit::IDD, pParent),
	_satPage(this) 
{
	_satDesc	= satDesc;
	_itemIndex	= itemIndex;

	_satPage.m_Fec = satDesc->getFEC();
	_satPage.m_Freq = satDesc->getFrequency();
	_satPage.m_OrbitPos = satDesc->getOrbitalPos();
	_satPage.m_SymbRate = satDesc->getSymbolRate();
	_satPage.m_WestEast = satDesc->getWestEast();
	_satPage.m_Polar = satDesc->getPolarization();
}


void SatDelivEdit::OnOK() 
{
	if (!_satPage.UpdateData())
		return ;

	_satDesc->create(
				_satPage.m_Freq,			// freq
				_satPage.m_OrbitPos,		// orbital pos
				_satPage.m_WestEast,		// westEast
				_satPage.m_Polar,			// polarization
				_satPage.m_SymbRate,		// symbol rate
				_satPage.m_Fec				// FEC rate
	) ;

	CDialog::OnOK();
}

BOOL SatDelivEdit::OnInitDialog() 
{
	static UINT dlgIds[] = 
	{
		IDC_SatFreq         ,
		IDC_SatOrbitPos     ,
		IDC_SatWest         ,
		IDC_cbSatPolar      ,
		IDC_SatFreq         ,
		IDC_SatSymbRate     ,
		IDC_cbFec           ,
	} ;

	CDialog::OnInitDialog();
	_satPage.Create(IDD_NewNtwWiz_Sat, this) ;
	_satPage.ShowWindow(SW_SHOW);
	SetWindowText("Edit Satellite descriptor");
	
	_satPage.SetFocus() ;
	_satPage.SetItenFocus(dlgIds[_itemIndex]) ;
	
	return TRUE;
}

BOOL RunSatDelivEdit(SatelliteDeliveryDescriptor *satDesc, int itemIndex)
{
	SatDelivEdit dlg (satDesc, itemIndex) ;
	return dlg.DoModal() == IDOK ;
}
