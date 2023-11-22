#if !defined(AFX_DESCRIPTORDLG_H__D7D6A4F2_F5E0_11D4_A116_FF8EA467E529__INCLUDED_)
#define AFX_DESCRIPTORDLG_H__D7D6A4F2_F5E0_11D4_A116_FF8EA467E529__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DescriptorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DescriptorDlg dialog

class DescriptorDlg : public CDialog
{
	int			_numDesc ;
	uchar*		_descIds ;
	CDialog*	_descDlg ;

// Construction
public:
	DescriptorDlg( int numDesc, uchar *descIds, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DescriptorDlg)
	enum { IDD = IDD_AddDescriptor };
	CListBox	_descList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DescriptorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	UINT GetDescDlgID( uchar descTag ) ;

	// Generated message map functions
	//{{AFX_MSG(DescriptorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDescrTypeChanged();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DESCRIPTORDLG_H__D7D6A4F2_F5E0_11D4_A116_FF8EA467E529__INCLUDED_)
