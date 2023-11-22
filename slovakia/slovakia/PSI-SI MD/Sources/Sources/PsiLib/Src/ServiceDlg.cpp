// ServiceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tables.h"
#include "ServiceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ServiceDlg dialog


ServiceDlg::ServiceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ServiceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(ServiceDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ServiceDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ServiceDlg, CDialog)
	//{{AFX_MSG_MAP(ServiceDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ServiceDlg message handlers
