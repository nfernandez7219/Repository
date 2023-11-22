#if !defined(AFX_ADDPRGWIZARD_H__73819496_07D9_11D5_BDC3_0000B49DBC06__INCLUDED_)
#define AFX_ADDPRGWIZARD_H__73819496_07D9_11D5_BDC3_0000B49DBC06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddPrgWizard.h : header file
//

#include "WizardDlg.h"

class AddProgramWizardDlg ;

/////////////////////////////////////////////////////////////////////////////
// AddPrgWizardPage1 dialog

class AddPrgWizardPage1 : public CDialog
{
	AddProgramWizardDlg*		_pParent ;
// Construction
public:
	AddPrgWizardPage1(AddProgramWizardDlg* pParent);   // standard constructor

	int	GetPrgType	( ) ;

// Dialog Data
	//{{AFX_DATA(AddPrgWizardPage1)
	enum { IDD = IDD_ADDPRGWIZ_1 };
	CListCtrl	m_lstPrgType;
	UINT	m_PmtPid;
	UINT	m_PrgNum;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AddPrgWizardPage1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AddPrgWizardPage1)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// AddPrgWizardPage2 dialog

class AddPrgWizardPage2 : public CDialog
{
// Construction
public:
	AddPrgWizardPage2(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(AddPrgWizardPage2)
	enum { IDD = IDD_ADDPRGWIZ_2 };
	CString	m_ProvName;
	CString	m_ServName;
	int		m_ServType;
	UINT	m_ServId;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AddPrgWizardPage2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AddPrgWizardPage2)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// AddPrgWizardPage3 dialog

class PmtStreamHolderArray ;

class AddPrgWizardPage3 : public CDialog
{
	int		m_SelectedStreamIndex ;
// Construction
public:
	AddPrgWizardPage3(CWnd* pParent = NULL);   // standard constructor
	virtual ~AddPrgWizardPage3() ;

	void	enablePcrPidCtrl	( BOOL enable=TRUE ) ;
	void	updateStreamList	( ) ;
	void	selectStream		(int index) ;
	void	saveStream			( ) ;

// Dialog Data
	//{{AFX_DATA(AddPrgWizardPage3)
	enum { IDD = IDD_ADDPRGWIZ_3 };
	CListCtrl	m_StreamList;
	BOOL	m_PcrEnabled;
	int		m_DataBroadId;
	int		m_StreamType;
	UINT	m_ElemPid;
	UINT	m_PcrPid;
	//}}AFX_DATA

	PmtStreamHolderArray	*m_Streams ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AddPrgWizardPage3)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void	updatePcrPidCtrl	( ) ;

	// Generated message map functions
	//{{AFX_MSG(AddPrgWizardPage3)
	virtual BOOL OnInitDialog();
	afx_msg void OnPcrPidEnable();
	afx_msg void OnStreamListClick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class TableHolder ;

class AddProgramWizardDlg : public WizardDlg
{
	AddPrgWizardPage1	_page1 ;
	AddPrgWizardPage2	_page2 ;
	AddPrgWizardPage3	_page3 ;

	BOOL				_bWasOnPage2 ;
	BOOL				_bWasOnPage3 ;

	TableHolder*		_tableHolder ;

protected:
	virtual	void	OnPageChange ( int newPageIndex ) ;
	virtual BOOL	OnInitDialog ( )  ;
	virtual void	OnOK		 ( );

public:
	AddProgramWizardDlg( TableHolder *tableHolder, CWnd *parent=NULL ) ;

	BOOL	programExists	( int prgNum ) ;
} ;

int RunNetworkWizard(TableHolder *tableHolder, CWnd *parent) ;

//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_ADDPRGWIZARD_H__73819496_07D9_11D5_BDC3_0000B49DBC06__INCLUDED_)
