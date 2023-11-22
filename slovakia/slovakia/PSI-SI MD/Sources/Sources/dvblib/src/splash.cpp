/*
 *	Filename:		splash.cpp
 *
 *	Version:		1.00
 *
 *	Description: implements spash screen at the program startup.
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"

#include "splash.hpp"


//------------------------------------------------------------------------------------------
//	sSplashWnd
//------------------------------------------------------------------------------------------


BOOL		sSplashWnd::c_bShowSplashWnd;
sSplashWnd* sSplashWnd::c_pSplashWnd;


sSplashWnd::~sSplashWnd()
{
	ASSERT(c_pSplashWnd == this);
	c_pSplashWnd = NULL;
}

BEGIN_MESSAGE_MAP(sSplashWnd, CWnd)
	//{{AFX_MSG_MAP(sSplashWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void sSplashWnd::EnableSplashScreen(BOOL bEnable /*= TRUE*/)
{
	c_bShowSplashWnd = bEnable;
}

void sSplashWnd::ShowSplashScreen( UINT bitmapId, CWnd* pParentWnd/*=NULL*/)
{
	if (!c_bShowSplashWnd || c_pSplashWnd != NULL)
		return;

	// Allocate a new splash screen, and create the window.
	c_pSplashWnd = new sSplashWnd;
	if (!c_pSplashWnd->Create(bitmapId,pParentWnd))
		delete c_pSplashWnd;
	else
		c_pSplashWnd->UpdateWindow();
}

BOOL sSplashWnd::PreTranslateAppMessage(MSG* pMsg)
{
	if (c_pSplashWnd == NULL)
		return FALSE;

	// If we get a keyboard or mouse message, hide the splash screen.
	if (pMsg->message == WM_KEYDOWN ||
	    pMsg->message == WM_SYSKEYDOWN ||
	    pMsg->message == WM_LBUTTONDOWN ||
	    pMsg->message == WM_RBUTTONDOWN ||
	    pMsg->message == WM_MBUTTONDOWN ||
	    pMsg->message == WM_NCLBUTTONDOWN ||
	    pMsg->message == WM_NCRBUTTONDOWN ||
	    pMsg->message == WM_NCMBUTTONDOWN)
	{
		c_pSplashWnd->HideSplashScreen();
		return TRUE;	// message handled here
	}
	return FALSE;	// message not handled
}

BOOL sSplashWnd::Create( UINT bitmapId, CWnd* pParentWnd /*= NULL*/)
{
	if (!m_bitmap.LoadBitmap(bitmapId))
		return FALSE;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);

	return CreateEx(0,
		AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
		NULL, WS_POPUP | WS_VISIBLE, 0, 0, bm.bmWidth, bm.bmHeight, pParentWnd->GetSafeHwnd(), NULL);
}

int sSplashWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	CenterWindow();
	SetTimer(1, 2000, NULL);
	return 0;
}

void sSplashWnd::OnPaint()
{
	CPaintDC dc(this);

	CDC dcImage;
	if (!dcImage.CreateCompatibleDC(&dc))
		return;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);
	CBitmap* pOldBitmap = dcImage.SelectObject(&m_bitmap);
	dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcImage, 0, 0, SRCCOPY);
	dcImage.SelectObject(pOldBitmap);
}
