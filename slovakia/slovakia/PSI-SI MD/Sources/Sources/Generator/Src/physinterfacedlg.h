#if !defined(AFX_PHYSINTERFACEDLG_H__8E0D4874_00AB_11D5_A121_C1FC1375FA29__INCLUDED_)
#define AFX_PHYSINTERFACEDLG_H__8E0D4874_00AB_11D5_A121_C1FC1375FA29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// physinterfacedlg.h : header file
//

#include "..\resource.h"

/////////////////////////////////////////////////////////////////////////////
// CPhysInterfaceDlg dialog

class CPhysInterfaceDlg : public CDialog
{
// Construction
public:
	CPhysInterfaceDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPhysInterfaceDlg)
	enum { IDD = IDD_PhysInterface };
	int		m_Interface;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhysInterfaceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPhysInterfaceDlg)
	afx_msg void OnInterfaceChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHYSINTERFACEDLG_H__8E0D4874_00AB_11D5_A121_C1FC1375FA29__INCLUDED_)
