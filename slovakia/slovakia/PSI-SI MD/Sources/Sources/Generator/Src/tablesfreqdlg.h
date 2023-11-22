#if !defined(AFX_TABLESFREQDLG_H__8E0D4873_00AB_11D5_A121_C1FC1375FA29__INCLUDED_)
#define AFX_TABLESFREQDLG_H__8E0D4873_00AB_11D5_A121_C1FC1375FA29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TablesFreqDlg.h : header file
//

#include "..\resource.h"

/////////////////////////////////////////////////////////////////////////////
// CTablesFreqDlg dialog

class TableHolder ;

class CTablesFreqDlg : public CDialog
{
	TableHolder*		_TableHolder ;
	UINT				_PatSize, _PmtSize, _CatSize, _SdtSize, _NitSize ;
// Construction
public:
	CTablesFreqDlg(TableHolder *tableHolder, UINT patSize, UINT pmtSize, UINT catSize, UINT sdtSize, UINT nitSize, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTablesFreqDlg)
	enum { IDD = IDD_TablesFreq };
	UINT	m_CatFreq;
	UINT	m_CatSpeed;
	UINT	m_NitFreq;
	UINT	m_NitSpeed;
	UINT	m_PatFreq;
	UINT	m_PatSpeed;
	UINT	m_PmtFreq;
	UINT	m_PmtSpeed;
	UINT	m_SdtFreq;
	UINT	m_SdtSpeed;
	UINT	m_TotalSpeed;
	BOOL	m_CatEnabled;
	BOOL	m_NitEnabled;
	BOOL	m_PatEnabled;
	BOOL	m_PmtEnabled;
	BOOL	m_SdtEnabled;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTablesFreqDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTablesFreqDlg)
	afx_msg void OnChangeFreq();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	afx_msg void OnEnableDisable(UINT nID) ;

	DECLARE_MESSAGE_MAP()
private:
	BOOL m_Finishing;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABLESFREQDLG_H__8E0D4873_00AB_11D5_A121_C1FC1375FA29__INCLUDED_)
