#if !defined(AFX_SERVICEDLG_H__D7D6A4F6_F5E0_11D4_A116_FF8EA467E529__INCLUDED_)
#define AFX_SERVICEDLG_H__D7D6A4F6_F5E0_11D4_A116_FF8EA467E529__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ServiceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ServiceDlg dialog

class ServiceDlg : public CDialog
{
// Construction
public:
	ServiceDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ServiceDlg)
	enum { IDD = IDD_Service };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ServiceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ServiceDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVICEDLG_H__D7D6A4F6_F5E0_11D4_A116_FF8EA467E529__INCLUDED_)
