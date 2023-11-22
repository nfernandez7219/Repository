
#include "stdafx.h"
#include "slist.h"
#include "descriptors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void CStringDelFunc( CString **st)
{
	delete *st ;
	*st = NULL ;
}

class CStringPtrArray : public sTemplateArray<CString*>
{
public:
	CStringPtrArray()
	{
		setDelFunc(CStringDelFunc) ;
	}
} ;

BOOL chooseLanguage ( uchar *lang, CWnd *pParent ) ;

/////////////////////////////////////////////////////////////////////////////
// MultiLingNameDlg dialog

class MultiLingNameDlg : public CDialog
{
	BOOL		_bProviderPresent ;
	char*		_data ;
	int*		_size ;

	CStringPtrArray		_names ;
	CStringPtrArray		_providers ;

	int			_selected ;

// Construction
public:
	MultiLingNameDlg(BOOL bProvider, char *data, int *size );

// Dialog Data
	//{{AFX_DATA(MultiLingNameDlg)
	enum { IDD = IDD_MultiLingName };
	CListBox	m_LangList;
	CString	m_Name;
	CString	m_Provider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MultiLingNameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MultiLingNameDlg)
	afx_msg void OnDelLang();
	afx_msg void OnAddLang();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelLanguage();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// MultiLingNameDlg dialog


MultiLingNameDlg::MultiLingNameDlg( BOOL bProvider, char *data, int *size )
	: CDialog(MultiLingNameDlg::IDD, AfxGetMainWnd())
{
	//{{AFX_DATA_INIT(MultiLingNameDlg)
	m_Name = _T("");
	m_Provider = _T("");
	//}}AFX_DATA_INIT
	_bProviderPresent = bProvider ;
	_data = data ;
	_size = size ;
	_selected = -1;
}

BOOL MultiLingNameDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	char *p = _data ;
	char *end = _data + *_size ;

	while ( p < end )
	{
		char buf[256] ;
		LanguageDescriptor::langAsText((uchar*)p, buf) ;
		m_LangList.AddString(buf) ;
		p += 3 ;

		if (_bProviderPresent)
		{
			uchar provLen = (uchar)*p ;
			p++ ;
			memcpy(buf, p, provLen) ;
			buf[provLen] = '\x0' ;
			p += provLen ;
			_providers.add(new CString(buf)) ;
		}

		uchar nameLen = (uchar)*p ;
		p++ ;
		memcpy(buf, p, nameLen) ;
		buf[nameLen] = '\x0' ;
		p += nameLen ;
		_names.add(new CString(buf)) ;
	}

	if (!_bProviderPresent)
	{
		GetDlgItem(IDC_ProvLabel)->ShowWindow(SW_HIDE) ;
		GetDlgItem(IDC_edtProvider)->ShowWindow(SW_HIDE) ;
	}

	m_LangList.SetCurSel(0) ;
	OnSelLanguage() ;
	
	return TRUE;
}

void MultiLingNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(MultiLingNameDlg)
	DDX_Control(pDX, IDC_lstLanguages, m_LangList);
	//}}AFX_DATA_MAP

	if (_selected>=0)
	{
		DDX_Text(pDX, IDC_edtName, *(_names[_selected]));
		if (_bProviderPresent)
			DDX_Text(pDX, IDC_edtProvider, *(_providers[_selected]));
	}
}


BEGIN_MESSAGE_MAP(MultiLingNameDlg, CDialog)
	//{{AFX_MSG_MAP(MultiLingNameDlg)
	ON_BN_CLICKED(IDC_btnDelLang, OnDelLang)
	ON_BN_CLICKED(IDC_btnAddLang, OnAddLang)
	ON_LBN_SELCHANGE(IDC_lstLanguages, OnSelLanguage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void MultiLingNameDlg::OnSelLanguage() 
{
	if (_selected>=0)
		UpdateData(TRUE) ;

	int i = m_LangList.GetCurSel() ;
	if (i != LB_ERR)
	{
		_selected = i ;
		UpdateData(FALSE) ;
	}
}

void MultiLingNameDlg::OnAddLang() 
{
	uchar lang[3] ;
	if ( chooseLanguage((uchar*)lang, this) )
	{
		char buf[256] ;
		LanguageDescriptor::langAsText(lang, buf) ;
		int index = m_LangList.AddString(buf) ;
		_names.add(new CString);
		_providers.add(new CString);
		m_LangList.SetCurSel(index) ;
		OnSelLanguage() ;
	}
}

void MultiLingNameDlg::OnDelLang() 
{
	int sel = m_LangList.GetCurSel() ;
	if (sel == LB_ERR)
		return ;

	m_LangList.DeleteString(sel) ;
	_names.del(sel) ;
	_providers.del(sel) ;

	if (sel==0)
		sel++ ;

	m_LangList.SetCurSel(sel-1) ;
}

void MultiLingNameDlg::OnOK() 
{
	OnSelLanguage() ;

	char buffer[2000] ;
	char *p = buffer ;
	int nLangs = _names.count() ;

	for ( int i = 0; i < nLangs; i++ )
	{
		char buf[256] ;
		m_LangList.GetText(i,buf) ;
		memcpy(p, buf, 3) ;
		p += 3 ;

		if (_bProviderPresent)
		{
			CString *prov =_providers[i] ;
			uchar provLen = prov->GetLength() ;
			*((uchar*)p) = provLen ;
			p++ ;
			memcpy(p, LPCTSTR(*prov), provLen) ;
			p += provLen ;
		}

		CString *name =_names[i] ;
		uchar nameLen = name->GetLength() ;
		*((uchar*)p) = nameLen ;
		p++ ;
		memcpy(p, LPCTSTR(*name), nameLen) ;
		p += nameLen ;
	}

	*_size = int(p-buffer) ;

	if (*_size > 253)
	{
		MessageBox("Too many languages", "Error", MB_OK|MB_ICONERROR);
		return ;
	}

	memcpy ( _data, buffer, *_size ) ;

	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL RunMultiLingNameDlg(BOOL bProvider, char *data, int *size )
{
	MultiLingNameDlg dlg(bProvider, data, size ) ;

	return dlg.DoModal() == IDOK ;
}
