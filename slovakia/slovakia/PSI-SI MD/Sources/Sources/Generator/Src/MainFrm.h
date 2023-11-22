// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__C83DB579_F1FF_11D4_BDB4_0000B49DBC06__INCLUDED_)
#define AFX_MAINFRM_H__C83DB579_F1FF_11D4_BDB4_0000B49DBC06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class TableHolder ;
class PsiDataArray ;
class BroadcastManager ;
class TableTree ;
class ValueList ;

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
public:

	void	UpdateTitle ( ) ;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();

protected:  // control bar embedded members
	CToolBar    m_wndToolBar;
	CReBar      m_wndReBar;
//	CDialogBar  m_wndDlgBar;
	TableTree  *m_TableTree;
	ValueList  *m_valueListView ;

	TableHolder			*m_TableHolder ;
	TableHolder			*m_RunningHolder ;
	PsiDataArray		*m_PsiDataArray ;
	BroadcastManager	*m_BroadcastManager ;

	int					 m_ViewMode ;
	int					 m_DevMode ;

	void	SetDevMode	( int mode ) ;

	enum ViewMode
	{
		Edit, ViewEdited, ViewRunning, ViewModifications

	} ;

// Generated message map functions
protected:
	BOOL RunOpenSaveAsDlg	( BOOL bSaveDlg, CString &path ) ;

	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnStart();
	afx_msg void OnStop();
	afx_msg void OnUpdateUI(CCmdUI* pCmdUI);
	afx_msg void OnFileOpen();
	afx_msg void OnOpenDontApply();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveDontApply();
	afx_msg void OnDestroy();
	afx_msg void OnViewModeChanged(UINT nID);
	afx_msg void OnTablesFreq();
	afx_msg void OnPhysInterfaceSetup();
	afx_msg void OnProgramWizard();
	afx_msg void OnNewNetworkWizard();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__C83DB579_F1FF_11D4_BDB4_0000B49DBC06__INCLUDED_)
