// physinterfacedlg.cpp : implementation file
//

#include "stdafx.h"
#include "physinterfacedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPhysInterfaceDlg dialog


CPhysInterfaceDlg::CPhysInterfaceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPhysInterfaceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPhysInterfaceDlg)
	m_Interface = 0;
	//}}AFX_DATA_INIT
}


void CPhysInterfaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPhysInterfaceDlg)
	DDX_Radio(pDX, IDC_btnDvbAsi, m_Interface);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPhysInterfaceDlg, CDialog)
	//{{AFX_MSG_MAP(CPhysInterfaceDlg)
	ON_BN_CLICKED(IDC_btnDvbAsi, OnInterfaceChanged)
	ON_BN_CLICKED(IDC_btnRs422, OnInterfaceChanged)
	ON_BN_CLICKED(IDC_btnEthernet, OnInterfaceChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhysInterfaceDlg message handlers

void CPhysInterfaceDlg::OnInterfaceChanged() 
{
	UpdateData(TRUE) ;

	GetDlgItem(IDC_edtStuffing)->EnableWindow(m_Interface==0) ;	
	GetDlgItem(IDC_chkNoIbStuffing)->EnableWindow(m_Interface==0) ;	

	GetDlgItem(IDC_edtComergonTest)->EnableWindow(m_Interface==1) ;	
	GetDlgItem(IDC_edtComergonPort)->EnableWindow(m_Interface==1) ;	
	GetDlgItem(IDC_edtComergonIrq)->EnableWindow(m_Interface==1) ;	

	GetDlgItem(IDC_edtIpAddress)->EnableWindow(m_Interface==2) ;	
	GetDlgItem(IDC_edtIpPort)->EnableWindow(m_Interface==2) ;	
	GetDlgItem(IDC_btnUdp)->EnableWindow(m_Interface==2) ;	
	GetDlgItem(IDC_btnTcp)->EnableWindow(m_Interface==2) ;	
}
