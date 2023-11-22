#if !defined(AFX_PATPMTDLG_H__D7D6A4F1_F5E0_11D4_A116_FF8EA467E529__INCLUDED_)
#define AFX_PATPMTDLG_H__D7D6A4F1_F5E0_11D4_A116_FF8EA467E529__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatPmtDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PatPmtDlg dialog

class PatPmtDlg : public CDialog
{
// Construction
public:
	PatPmtDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PatPmtDlg)
	enum { IDD = IDD_PAT_PMT };
	CListCtrl	m_StreamList;
	CListCtrl	m_ProgramList;
	BOOL	m_ProgramMapping;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PatPmtDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PatPmtDlg)
	afx_msg void OnAddNetworkDesc();
	afx_msg void OnProgramMapping();
	virtual BOOL OnInitDialog();
	afx_msg void OnAddStreamDesc();
	afx_msg void OnAddProgram();
	afx_msg void OnSelchangeCBStreamType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATPMTDLG_H__D7D6A4F1_F5E0_11D4_A116_FF8EA467E529__INCLUDED_)
