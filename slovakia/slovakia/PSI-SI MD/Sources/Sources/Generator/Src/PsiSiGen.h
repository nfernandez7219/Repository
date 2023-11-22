// PsiSiGen.h : main header file for the PSISIGEN application
//

#if !defined(AFX_PSISIGEN_H__C83DB575_F1FF_11D4_BDB4_0000B49DBC06__INCLUDED_)
#define AFX_PSISIGEN_H__C83DB575_F1FF_11D4_BDB4_0000B49DBC06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CPsiSiGenApp:
// See PsiSiGen.cpp for the implementation of this class
//

class CPsiSiGenApp : public CWinApp
{
	char	_DataFileName[_MAX_PATH] ;
public:
	CPsiSiGenApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPsiSiGenApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	inline const char  *GetDataFileName( )							{ return _DataFileName ; }
		   void			SetDataFileName( const char *fileName ) ;

public:
	//{{AFX_MSG(CPsiSiGenApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CPsiSiGenApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PSISIGEN_H__C83DB575_F1FF_11D4_BDB4_0000B49DBC06__INCLUDED_)
