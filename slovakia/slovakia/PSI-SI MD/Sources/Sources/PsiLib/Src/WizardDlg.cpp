// WizardDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WizardDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// WizardDlg dialog


WizardDlg::WizardDlg(CWnd* pParent, PDialog dlg1, PDialog dlg2, PDialog dlg3, PDialog dlg4)
	: CDialog(WizardDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(WizardDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	_dialogs[0] = dlg1 ;
	_dialogs[1] = dlg2 ;
	_dialogs[2] = dlg3 ;
	_dialogs[3] = dlg4 ;
	_dialogs[4] = NULL ;

	_activeDlg = 0 ;
}


void WizardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(WizardDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

void WizardDlg::SetPage( int index, PDialog dlg )
{
	if ( index > 3 || index < 0 || (index>0 && _dialogs[index-1]==NULL) )
		return ;

	if ( ! ::IsWindow(dlg->m_hWnd) )
	{
		UINT id = UINT( ((WizardDlg*)dlg)->m_lpszTemplateName ) ;
		dlg->Create(id, this) ;
	}

	BOOL bNewPage = _dialogs[index]==NULL ;
	_dialogs[index] = dlg ;
	if ( bNewPage && _activeDlg==index-1 )
		GetDlgItem(ID_WIZNEXT)->EnableWindow(TRUE) ;
}

BEGIN_MESSAGE_MAP(WizardDlg, CDialog)
	//{{AFX_MSG_MAP(WizardDlg)
	ON_BN_CLICKED(ID_WIZBACK, OnPrevPage)
	ON_BN_CLICKED(ID_WIZNEXT, OnNextPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WizardDlg message handlers

BOOL WizardDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int i = 0 ;
	while (_dialogs[i] != NULL)
	{
		CDialog *dlg = _dialogs[i] ;
	
		if ( ! ::IsWindow(dlg->m_hWnd) )
		{
			// maly ofaj
			UINT id = UINT( ((WizardDlg*)dlg)->m_lpszTemplateName ) ;
			dlg->Create(id, this) ;
		}

		i++ ;
	}
	
	SetDlgActive(0) ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void WizardDlg::SetDlgActive ( int index )
{
	_dialogs[_activeDlg]->ShowWindow(SW_HIDE) ;
	_activeDlg = index ;
	_dialogs[_activeDlg]->ShowWindow(SW_SHOW) ;
	
	GetDlgItem(ID_WIZBACK)->EnableWindow(index>0) ;
	GetDlgItem(ID_WIZNEXT)->EnableWindow(_dialogs[index+1]!=NULL) ;

	OnPageChange(index) ;
}

void WizardDlg::OnPrevPage() 
{
	if (_activeDlg <= 0)
		return ;

	SetDlgActive (_activeDlg-1) ;
}

void WizardDlg::OnNextPage() 
{
	if (_dialogs[_activeDlg+1]==NULL)
		return ;

	BOOL res = _dialogs[_activeDlg]->UpdateData(TRUE) ;

	if (res)
		SetDlgActive (_activeDlg+1) ;
}

void WizardDlg::OnOK() 
{
	int i = 0 ;
	while (_dialogs[i] != NULL)
	{
		BOOL res = _dialogs[i]->UpdateData(TRUE) ;
		if (!res)
		{
			SetDlgActive (i) ;
			return ;
		}
		i++ ;
	}
	
	CDialog::OnOK();
}
