#if !defined(AFX_PROGRAMPIDDLG_H__D7D6A4F7_F5E0_11D4_A116_FF8EA467E529__INCLUDED_)
#define AFX_PROGRAMPIDDLG_H__D7D6A4F7_F5E0_11D4_A116_FF8EA467E529__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgramPidDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ProgramPidDlg dialog

class ProgramPidDlg : public CDialog
{
// Construction
public:
	ProgramPidDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ProgramPidDlg)
	enum { IDD = IDD_ProgramPid };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ProgramPidDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ProgramPidDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRAMPIDDLG_H__D7D6A4F7_F5E0_11D4_A116_FF8EA467E529__INCLUDED_)
