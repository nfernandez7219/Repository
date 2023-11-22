#if !defined(AFX_NITSDTDLG_H__D7D6A4F5_F5E0_11D4_A116_FF8EA467E529__INCLUDED_)
#define AFX_NITSDTDLG_H__D7D6A4F5_F5E0_11D4_A116_FF8EA467E529__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NitSdtDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// NitSdtDlg dialog

class NitSdtDlg : public CDialog
{
// Construction
public:
	NitSdtDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(NitSdtDlg)
	enum { IDD = IDD_NIT_SDT };
	CListCtrl	m_StreamList;
	CListCtrl	m_ServiceList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NitSdtDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NitSdtDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddService();
	afx_msg void OnAddNtwDesc();
	afx_msg void OnAddTSDesc();
	afx_msg void OnAddServDesc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NITSDTDLG_H__D7D6A4F5_F5E0_11D4_A116_FF8EA467E529__INCLUDED_)
