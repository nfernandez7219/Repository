// SentinelManDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SentinelMan.h"
#include "SentinelManDlg.h"
#include "..\..\server\DvbSentinel\H\DVBSentinel.h"
#include "Heap.h"
#include "scrtools.hpp"
#include "env.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OperNameDlg dialog

class OperNameDlg : public CDialog
{
// Construction
public:
	OperNameDlg(const char *initSt, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(OperNameDlg)
	enum { IDD = IDD_OperatorName };
	CString	m_OperName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OperNameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
};
/////////////////////////////////////////////////////////////////////////////
// OperNameDlg dialog


OperNameDlg::OperNameDlg(const char *initSt, CWnd* pParent /*=NULL*/)
	: CDialog(OperNameDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(OperNameDlg)
	m_OperName = _T(initSt);
	//}}AFX_DATA_INIT
}

void OperNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OperNameDlg)
	DDX_Text(pDX, IDC_edtOperName, m_OperName);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CSentinelManDlg dialog

CSentinelManDlg::CSentinelManDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSentinelManDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSentinelManDlg)
	m_Operator = _T("");
	m_iApplication = -1;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSentinelManDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSentinelManDlg)
	DDX_Control(pDX, IDC_cbApplication, m_AppList);
	DDX_Control(pDX, IDC_lstOperators, m_OperList);
	DDX_LBString(pDX, IDC_lstOperators, m_Operator);
	DDX_CBIndex(pDX, IDC_cbApplication, m_iApplication);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSentinelManDlg, CDialog)
	//{{AFX_MSG_MAP(CSentinelManDlg)
	ON_LBN_DBLCLK(IDC_lstOperators, OnDblclkOperators)
	ON_BN_CLICKED(IDC_NewOperator, OnNewOperator)
	ON_BN_CLICKED(IDC_DelOperator, OnDelOperator)
	ON_CBN_SELCHANGE(IDC_cbApplication, OnApplicationChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSentinelManDlg message handlers

static LPCSTR s_AppPrefix[] =
{
	"PSI", "DVB",
} ;

void CSentinelManDlg::LoadCustomers( const char *prefix )
{
	ConfigClass cfg("setupHWkey.cfg") ;
	cfg.open() ;

	m_OperList.ResetContent() ;

	sStringPtrArray *sectList = cfg.sectionList( ) ;
	int nItems = sectList->count() ;
	for (int i = 0; i < nItems; i++)
	{
		char name[256] ;
		strcpy( name ,sectList->item(i) ) ;
		if ( strncmp( name, prefix, 3 )==0 )
			m_OperList.AddString(name+4) ;
	}
}

BOOL CSentinelManDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_AppList.AddString("PSI/SI Generator") ;
	m_AppList.AddString("DVB Server") ;
	m_AppList.SetCurSel(0) ;

	LoadCustomers(s_AppPrefix[0]) ;
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSentinelManDlg::OnDblclkOperators() 
{
	UpdateData(TRUE) ;
	char op[256] ;
	sprintf( op, "%s_%s", s_AppPrefix[m_iApplication], LPCTSTR(m_Operator) ) ;

	switch(m_iApplication)
	{
		case 0 :
		{
			PsiSentinel senti ;
			senti.openSetupDialog(op, this) ;
			break ;
		}
		case 1 :
		{
			DVBSentinel senti ;
			senti.openSetupDialog(op, this) ;
			break ;
		}
	} ;
}


void CSentinelManDlg::OnNewOperator() 
{
	OperNameDlg dlg("", this) ;
	if (dlg.DoModal() == IDOK)
	{
		const char *name = dlg.m_OperName ;
		int index = m_OperList.AddString(name) ;
		m_OperList.SetCurSel(index) ;
	}
}

void CSentinelManDlg::OnDelOperator() 
{
	int res = MessageBox("Are you sure to delete selected operator ?", "Question", MB_YESNO|MB_ICONQUESTION ) ;
	if (res != IDYES)
		return ;

	UpdateData(TRUE) ;
	char op[256] ;
	sprintf( op, "%s_%s", s_AppPrefix[m_iApplication], LPCTSTR(m_Operator) ) ;

	int sel = m_OperList.GetCurSel() ;
	if (sel!=LB_ERR )
		m_OperList.DeleteString(sel) ;	

	ConfigClass cfg("setupHWkey.cfg") ;
	cfg.open() ;
	cfg.delSection(op) ;
	cfg.save(FALSE) ;
}

void CSentinelManDlg::OnApplicationChanged() 
{
	int sel = m_AppList.GetCurSel() ;
	LoadCustomers(s_AppPrefix[sel]) ;
}
