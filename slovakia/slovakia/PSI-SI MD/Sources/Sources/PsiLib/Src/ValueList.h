#if !defined(AFX_VALUELIST_H__91B3BE93_F835_11D4_BDB8_0000B49DBC06__INCLUDED_)
#define AFX_VALUELIST_H__91B3BE93_F835_11D4_BDB8_0000B49DBC06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ValueList.h : header file
//

#include <afxcview.h>

class BaseHolder ;

/////////////////////////////////////////////////////////////////////////////
// ValueList view

class ValueList : public CListView
{
	BOOL		_bViewMode ;
protected:
public:
	ValueList();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(ValueList)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ValueList)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~ValueList();

	// Generated message map functions
protected:
	BaseHolder* m_pHolder;

	//{{AFX_MSG(ValueList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void Refresh		( BaseHolder *holder );
	void SetViewMode	( BOOL bViewMode )					{ _bViewMode = bViewMode ; }
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VALUELIST_H__91B3BE93_F835_11D4_BDB8_0000B49DBC06__INCLUDED_)
