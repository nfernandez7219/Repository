// SentinelMan.h : main header file for the SENTINELMAN application
//

#if !defined(AFX_SENTINELMAN_H__47D83AD7_0CC4_11D5_BDC8_0000B49DBC06__INCLUDED_)
#define AFX_SENTINELMAN_H__47D83AD7_0CC4_11D5_BDC8_0000B49DBC06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSentinelManApp:
// See SentinelMan.cpp for the implementation of this class
//

class CSentinelManApp : public CWinApp
{
public:
	CSentinelManApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSentinelManApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSentinelManApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENTINELMAN_H__47D83AD7_0CC4_11D5_BDC8_0000B49DBC06__INCLUDED_)
