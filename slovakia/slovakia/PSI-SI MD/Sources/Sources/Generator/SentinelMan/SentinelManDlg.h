// SentinelManDlg.h : header file
//

#if !defined(AFX_SENTINELMANDLG_H__47D83AD9_0CC4_11D5_BDC8_0000B49DBC06__INCLUDED_)
#define AFX_SENTINELMANDLG_H__47D83AD9_0CC4_11D5_BDC8_0000B49DBC06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSentinelManDlg dialog

class CSentinelManDlg : public CDialog
{
// Construction
public:
	CSentinelManDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSentinelManDlg)
	enum { IDD = IDD_SENTINELMAN_DIALOG };
	CComboBox	m_AppList;
	CListBox	m_OperList;
	CString	m_Operator;
	int		m_iApplication;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSentinelManDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	void	LoadCustomers	( const char *prefix ) ;

	// Generated message map functions
	//{{AFX_MSG(CSentinelManDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkOperators();
	afx_msg void OnNewOperator();
	afx_msg void OnDelOperator();
	afx_msg void OnApplicationChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENTINELMANDLG_H__47D83AD9_0CC4_11D5_BDC8_0000B49DBC06__INCLUDED_)
