#ifndef __INC_SPLASH_HPP__
#define __INC_SPLASH_HPP__

class sSplashWnd : public CWnd
{
  protected:
	static	BOOL				c_bShowSplashWnd;
	static	sSplashWnd*	c_pSplashWnd;
			CBitmap				m_bitmap;

	BOOL	Create			( UINT bitmapId, CWnd* pParentWnd = NULL);
	void	HideSplashScreen()			{ DestroyWindow();AfxGetMainWnd()->UpdateWindow();};

	//{{AFX_MSG(sSplashWnd)
	afx_msg int  OnCreate	( LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer	( UINT nIDEvent)	{ 	HideSplashScreen(); };
	afx_msg void OnPaint	();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  public:
	static void  EnableSplashScreen		( BOOL bEnable	   = TRUE );
	static void  ShowSplashScreen		( UINT bitmapId, CWnd* pParentWnd = NULL );
	static BOOL	 PreTranslateAppMessage	( MSG* pMsg);
	virtual void PostNcDestroy()		{ delete this; };

	~sSplashWnd();
};

#endif
