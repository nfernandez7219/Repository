#if !defined(AFX_WIZARDDLG_H__73819495_07D9_11D5_BDC3_0000B49DBC06__INCLUDED_)
#define AFX_WIZARDDLG_H__73819495_07D9_11D5_BDC3_0000B49DBC06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WizardDlg.h : header file
//

typedef CDialog *PDialog ;

/////////////////////////////////////////////////////////////////////////////
// WizardDlg dialog

class WizardDlg : public CDialog
{
	int			_activeDlg ;
	PDialog		_dialogs[5] ;

// Construction
public:
	WizardDlg(CWnd* pParent, PDialog dlg1, PDialog dlg2=NULL, PDialog dlg3=NULL, PDialog dlg4=NULL );   // standard constructor

	void	SetPage		( int index, PDialog dlg ) ;
// Dialog Data
	//{{AFX_DATA(WizardDlg)
	enum { IDD = IDD_Wizard };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WizardDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void	SetDlgActive ( int index ) ;

	virtual	void	OnPageChange ( int newPageIndex )		{ }

	// Generated message map functions
	//{{AFX_MSG(WizardDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPrevPage();
	afx_msg void OnNextPage();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WIZARDDLG_H__73819495_07D9_11D5_BDC3_0000B49DBC06__INCLUDED_)
