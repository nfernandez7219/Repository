// TablesFreqDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TableHolders.h"
#include "TablesFreqDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COUNT_SPEED(table)	m_##table##Speed = (m_##table##Freq?(_##table##Size*8/m_##table##Freq):0) ;

/////////////////////////////////////////////////////////////////////////////
// CTablesFreqDlg dialog


CTablesFreqDlg::CTablesFreqDlg(TableHolder *tableHolder, UINT patSize, UINT pmtSize, UINT catSize, UINT sdtSize, UINT nitSize, CWnd* pParent /*=NULL*/)
	: CDialog(CTablesFreqDlg::IDD, pParent)
{
	_TableHolder = tableHolder ;

	_PatSize=patSize ;
	_PmtSize=pmtSize ;
	_CatSize=catSize ;
	_SdtSize=sdtSize ;
	_NitSize=nitSize ;

	m_CatFreq = tableHolder->_catFreq;
	m_NitFreq = tableHolder->_nitFreq;
	m_PatFreq = tableHolder->_patFreq;
	m_PmtFreq = tableHolder->_pmtFreq;
	m_SdtFreq = tableHolder->_sdtFreq;

	//{{AFX_DATA_INIT(CTablesFreqDlg)
	m_CatEnabled = m_CatFreq!=0;
	m_NitEnabled = m_NitFreq!=0;
	m_PatEnabled = m_PatFreq!=0;
	m_PmtEnabled = m_PmtFreq!=0;
	m_SdtEnabled = m_SdtFreq!=0;
	//}}AFX_DATA_INIT
	
	m_Finishing=FALSE;

	COUNT_SPEED(Pat);
	COUNT_SPEED(Pmt);
	COUNT_SPEED(Sdt);
	COUNT_SPEED(Nit);
	COUNT_SPEED(Cat);

	m_TotalSpeed = m_PatSpeed+m_PmtSpeed+m_SdtSpeed+m_NitSpeed+m_CatSpeed ;
}

BOOL CTablesFreqDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	GetDlgItem(IDC_PatFreq)->EnableWindow(m_PatEnabled) ;
	GetDlgItem(IDC_PmtFreq)->EnableWindow(m_PmtEnabled) ;
	GetDlgItem(IDC_SdtFreq)->EnableWindow(m_SdtEnabled) ;
	GetDlgItem(IDC_NitFreq)->EnableWindow(m_NitEnabled) ;
	GetDlgItem(IDC_CatFreq)->EnableWindow(m_CatEnabled) ;
	
	return TRUE;
}

#define DATA_EX_TEST(table,min,max)								\
	if (m_##table##Freq==0 && !pDX->m_bSaveAndValidate)			\
	{															\
		DDX_Text(pDX, IDC_##table##Freq, CString("") );			\
		DDX_Text(pDX, IDC_##table##Speed, CString("") );		\
	}															\
	else														\
	{															\
		DDX_Text(pDX, IDC_##table##Freq, m_##table##Freq);		\
		DDV_MinMaxUInt(pDX, m_##table##Freq, min, max);			\
		DDX_Text(pDX, IDC_##table##Speed, m_##table##Speed);	\
	}

void CTablesFreqDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CTablesFreqDlg)
	DDX_Check(pDX, IDC_chkCAT, m_CatEnabled);
	DDX_Check(pDX, IDC_chkNIT, m_NitEnabled);
	DDX_Check(pDX, IDC_chkPAT, m_PatEnabled);
	DDX_Check(pDX, IDC_chkPMT, m_PmtEnabled);
	DDX_Check(pDX, IDC_chkSDT, m_SdtEnabled);
	//}}AFX_DATA_MAP

	if (m_Finishing || !pDX->m_bSaveAndValidate)
	{
		UINT min=25, max=2000 ;

		if (m_PatEnabled) DATA_EX_TEST(Pat,25,2000) ; 
		if (m_PmtEnabled) DATA_EX_TEST(Pmt,25,2000) ;
		if (m_SdtEnabled) DATA_EX_TEST(Sdt,25,10000) ;
		if (m_NitEnabled) DATA_EX_TEST(Nit,25,10000) ;
		if (m_CatEnabled) DATA_EX_TEST(Cat,25,2000) ;

		DDX_Text(pDX, IDC_TotalSpeed, m_TotalSpeed);
	}
	else
	{
		CString val ;

		DDX_Text(pDX, IDC_CatFreq, val);
		m_CatFreq=atoi(LPCTSTR(val));
		DDX_Text(pDX, IDC_NitFreq, val);
		m_NitFreq=atoi(LPCTSTR(val));
		DDX_Text(pDX, IDC_PatFreq, val);
		m_PatFreq=atoi(LPCTSTR(val));
		DDX_Text(pDX, IDC_PmtFreq, val);
		m_PmtFreq=atoi(LPCTSTR(val));
		DDX_Text(pDX, IDC_SdtFreq, val);
		m_SdtFreq=atoi(LPCTSTR(val));
	}
}


BEGIN_MESSAGE_MAP(CTablesFreqDlg, CDialog)
	//{{AFX_MSG_MAP(CTablesFreqDlg)
	ON_EN_CHANGE(IDC_PatFreq, OnChangeFreq)
	ON_EN_CHANGE(IDC_PmtFreq, OnChangeFreq)
	ON_EN_CHANGE(IDC_SdtFreq, OnChangeFreq)
	ON_EN_CHANGE(IDC_NitFreq, OnChangeFreq)
	ON_EN_CHANGE(IDC_CatFreq, OnChangeFreq)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(IDC_chkPAT,IDC_chkCAT, OnEnableDisable)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTablesFreqDlg message handlers

void CTablesFreqDlg::OnEnableDisable(UINT nID)
{
	UpdateData(TRUE) ;
	switch (nID)
	{
		case IDC_chkPAT: GetDlgItem(IDC_PatFreq)->EnableWindow(m_PatEnabled) ; break ;
		case IDC_chkPMT: GetDlgItem(IDC_PmtFreq)->EnableWindow(m_PmtEnabled) ; break ;
		case IDC_chkSDT: GetDlgItem(IDC_SdtFreq)->EnableWindow(m_SdtEnabled) ; break ;
		case IDC_chkNIT: GetDlgItem(IDC_NitFreq)->EnableWindow(m_NitEnabled) ; break ;
		case IDC_chkCAT: GetDlgItem(IDC_CatFreq)->EnableWindow(m_CatEnabled) ; break ;
	}
}

void CTablesFreqDlg::OnChangeFreq() 
{
	UpdateData(TRUE) ;
	
	COUNT_SPEED(Pat);
	COUNT_SPEED(Pmt);
	COUNT_SPEED(Sdt);
	COUNT_SPEED(Nit);
	COUNT_SPEED(Cat);

	m_TotalSpeed = m_PatSpeed+m_PmtSpeed+m_SdtSpeed+m_NitSpeed+m_CatSpeed ;

	UpdateData(FALSE) ;
}

void CTablesFreqDlg::OnOK() 
{
	m_Finishing = TRUE;	
	CDialog::OnOK();
	m_Finishing = FALSE;	

	_TableHolder->_catFreq	= m_CatEnabled?m_CatFreq:0;
	_TableHolder->_nitFreq	= m_NitEnabled?m_NitFreq:0;
	_TableHolder->_patFreq	= m_PatEnabled?m_PatFreq:0;
	_TableHolder->_pmtFreq	= m_PmtEnabled?m_PmtFreq:0;
	_TableHolder->_sdtFreq	= m_SdtEnabled?m_SdtFreq:0;
}
