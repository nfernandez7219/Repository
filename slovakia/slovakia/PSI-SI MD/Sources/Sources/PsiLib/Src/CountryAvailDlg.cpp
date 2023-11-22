
#include "stdafx.h"
#include "descriptors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CountryAvailDlg dialog

class CountryAvailDlg : public CDialog
{
	CountryAvailabilityDescriptor *_desc1, *_desc2 ;
// Construction
public:
	CountryAvailDlg( CountryAvailabilityDescriptor *desc1, CountryAvailabilityDescriptor *desc2 ); 

// Dialog Data
	//{{AFX_DATA(CountryAvailDlg)
	enum { IDD = IDD_CountryAvail };
	CListBox	m_NotAvailList;
	CListBox	m_CountriesList;
	CListBox	m_AvailList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CountryAvailDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CountryAvailDlg)
	afx_msg void OnAddAvail();
	afx_msg void OnAddNotAvail();
	afx_msg void OnRemoveAvail();
	afx_msg void OnRemoveNotAvail();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CountryAvailDlg dialog


CountryAvailDlg::CountryAvailDlg(CountryAvailabilityDescriptor *desc1, CountryAvailabilityDescriptor *desc2)
	: CDialog(CountryAvailDlg::IDD, AfxGetMainWnd())
{
	//{{AFX_DATA_INIT(CountryAvailDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	_desc1 = desc1->countriesAvailable()?desc1:desc2 ; 
	_desc2 = desc1->countriesAvailable()?desc2:desc1 ; 
}


void CountryAvailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CountryAvailDlg)
	DDX_Control(pDX, IDC_lstNotAvailable, m_NotAvailList);
	DDX_Control(pDX, IDC_lstCountries, m_CountriesList);
	DDX_Control(pDX, IDC_lstAvailable, m_AvailList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CountryAvailDlg, CDialog)
	//{{AFX_MSG_MAP(CountryAvailDlg)
	ON_BN_CLICKED(IDC_AddAvail, OnAddAvail)
	ON_BN_CLICKED(IDC_AddNotAvail, OnAddNotAvail)
	ON_BN_CLICKED(IDC_RemoveAvail, OnRemoveAvail)
	ON_BN_CLICKED(IDC_RemoveNotAvail, OnRemoveNotAvail)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CountryAvailDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int nCountries = (int)COUNTRIES_NUMBER;

	for ( int i = 0; i < nCountries; i++ )
	{
		if (_desc1->hasCountry(s_stCountries[i]))
			m_AvailList.AddString(s_stCountries[i]);
		else
		if (_desc2->hasCountry(s_stCountries[i]))
			m_NotAvailList.AddString(s_stCountries[i]);
		else
			m_CountriesList.AddString(s_stCountries[i]);
	}
	
	return TRUE;
}

static void MoveString( CListBox &listFrom, CListBox &listTo)
{
	int index = listFrom.GetCurSel() ;
	if (index!=LB_ERR)
	{
		char buf[256] ;
		listFrom.GetText(index, buf) ;
		listFrom.DeleteString(index) ;
		index = listTo.AddString(buf) ;
		listTo.SetCurSel(index) ;
	}
}

void CountryAvailDlg::OnAddAvail() 
{
	MoveString(m_CountriesList, m_AvailList) ;
}

void CountryAvailDlg::OnAddNotAvail() 
{
	MoveString(m_CountriesList, m_NotAvailList) ;
}

void CountryAvailDlg::OnRemoveAvail() 
{
	MoveString(m_AvailList, m_CountriesList ) ;
}

void CountryAvailDlg::OnRemoveNotAvail() 
{
	MoveString(m_NotAvailList, m_CountriesList ) ;
}

static void SaveDesc( CountryAvailabilityDescriptor *desc, CListBox &list)
{
	CountryStruct *country = desc->countries ;
	char buf[256] ;

	int nItems = list.GetCount() ;
	for (int i = 0; i < nItems; i++)
	{
		list.GetText(i, buf) ;
		memcpy( country[i].country_id, buf, 3) ;
	}
	desc->descriptor_length = 1 + nItems*3 ;
}

void CountryAvailDlg::OnOK() 
{
	SaveDesc(_desc1, m_AvailList) ;
	SaveDesc(_desc2, m_NotAvailList) ;

	CDialog::OnOK();
}

BOOL RunCountryAvailDlg(CountryAvailabilityDescriptor *desc1, CountryAvailabilityDescriptor *desc2)
{
	CountryAvailDlg dlg(desc1,desc2) ;
	return dlg.DoModal()==IDOK ;
}
