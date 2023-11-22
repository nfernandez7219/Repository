#if !defined(AFX_TABLETREE_H__338B8543_F777_11D4_BDB7_0000B49DBC06__INCLUDED_)
#define AFX_TABLETREE_H__338B8543_F777_11D4_BDB7_0000B49DBC06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TableTree.h : header file
//

#include <afxcview.h>

class BaseHolder ;
class TableHolder ;
class NIT_Holder ;
class PAT_Holder ; 
class PMT_Holder ;
class SDT_Holder ;
class PMT_HolderArray ;
class DescriptorArray ;

/////////////////////////////////////////////////////////////////////////////
// TableTree view

class TableTree : public CTreeView
{
	TableHolder		*_TableHolder ;
	BOOL			 _bViewMode ;
public:
//protected:
	TableTree( ) ;
	
	DECLARE_DYNCREATE(TableTree)

// Attributes
public:
//
// Operations
public:
	void CreateTreeItems	( TableHolder *tableHolder, BOOL bViewMode = FALSE ) ;
	void RefreshItem		( ) ;
	void DeleteAllItems		( )								{ GetTreeCtrl().DeleteAllItems( ) ; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TableTree)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~TableTree();

	HTREEITEM	InsertItem		( const char *name, BaseHolder *itemHolder=NULL, HTREEITEM parent=TVI_ROOT, BOOL bold=TRUE ) ;

	void	CreateSubtreeNIT	( NIT_Holder *nitHolder ) ;
	void	CreateSubtreePAT	( PAT_Holder *patHolder, PMT_HolderArray *pmtHolders ) ;
	void	CreateSubtreePMT	( PMT_Holder *pmtHolder, HTREEITEM parent ) ;
	void	CreateSubtreeSDT	( SDT_Holder *sdtHolder) ;
	void	CreateSubtreeCAT	( CAT_Holder *catHolder) ;

	void	  CreateDescriptorSubtree		( DescriptorArray *descArr, int nDesc, uchar *descIds, HTREEITEM parent ) ;
	HTREEITEM CreateProgramStreamSubtree	( PmtStreamHolder *streamHolder, HTREEITEM parent ) ;

	// Generated message map functions
protected:
	//{{AFX_MSG(TableTree)
	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABLETREE_H__338B8543_F777_11D4_BDB7_0000B49DBC06__INCLUDED_)
