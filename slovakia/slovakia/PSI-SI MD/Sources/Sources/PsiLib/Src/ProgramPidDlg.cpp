// ProgramPidDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tables.h"
#include "ProgramPidDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ProgramPidDlg dialog


ProgramPidDlg::ProgramPidDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ProgramPidDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(ProgramPidDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ProgramPidDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ProgramPidDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ProgramPidDlg, CDialog)
	//{{AFX_MSG_MAP(ProgramPidDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ProgramPidDlg message handlers
