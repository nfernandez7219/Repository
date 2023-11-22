#ifndef         __CTRLTOOL_HPP__
#define         __CTRLTOOL_HPP__


//
// Bitmap button displaying and icon and optional text.
// The button may act as push button or 2-state check button.
//
// Text (if given) is displayed just above the bottom border of the button and as the font
// standard system font is taken. This should be taken into account when designing dialog
// resource. (I.e. push button defined in the dialog resource template should be large
// enough to cover both icon and text.)
//
// Button initialization (in OnInitDialog()):
//	- call SubclassDlgItem() supplying icon and/or text to be used.
//	- for 2-state button call MakeCheckButton() supplying icon/text for pressed state
//
class sIconButton : public CButton
{
	BOOL	bExist;
	BOOL	bIsCheck;
	UINT	iIcoW;
	UINT	iIcoH;
	BOOL	bCheckBtn;
	HICON	hUpBmp;
	HICON	hDownBmp;
	CString	sUpTxt;
	CString	sDownTxt;

	void	DrawItem	( LPDRAWITEMSTRUCT lpDrawItemStruct );
	LRESULT WindowProc	( UINT message, WPARAM wParam, LPARAM lParam );

  public:
	void SubclassDlgItem(
		UINT uCtrlID,		// resource id of the push button control which is to be modified
		CWnd *pPar,			// parent dialog
		UINT uBmpUpID,		// icon resource id
		LPCTSTR upText=0,	// text to be displayed in the button bottom part
		HMODULE mod=0		// resource module containing icon resource (0 for current module)
		);

	// Support for 2-state buttons
	void MakeCheckButton(
		UINT uBmpDownID,	// icon resource id
		LPCTSTR dwnText=0,	// text used in the pressed state (for NULL "up" text will be used)
		HMODULE mod=0		// resource module containing icon resource (0 for current module)
		);
	void SetCheck		( int nCheck ) ;
	BOOL GetCheck		()					{ return bIsCheck; }

	BOOL EnableWindow	( BOOL bEnable ) ;

	sIconButton();
   ~sIconButton();
};


/****************************  sColorButton  *******************************/
/*
#LButton displaing color
#BRemarks:
  #NThis class is used for displaing color
  Color is specified by RGB.  
  After pushing button Color dialog is displaing


*/
class sColorButton : public CButton
{
    BOOL        stayDown;
    COLORREF    col     ;
	BOOL		displayColDLG;

  protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);   //#*
	afx_msg void OnBNClicked();

  public:
    operator COLORREF()                         { return col; };          //Return COLORREF which contains actual color
    COLORREF GetColor           ( void       )  { return col; };          //Return COLORREF which contains actual color
    void     SetColor           ( COLORREF c )  { col=c; RedrawWindow();};//Set actual color which will displaing in the button
    void     DisplayColorDialog ();                                       //Display CColorDialog and update button color width selected color     
    
    BOOL     AttachButton       (int nCtlID, CWnd* pParentWnd, BOOL dispColDLG=TRUE );           //Attach this control to control
    BOOL     Create             ( LPCTSTR lpszCaption, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, int nID); //*#

    sColorButton()              { stayDown = FALSE; col = RGB(255,255,255); displayColDLG=TRUE; }; //Construct color button class
   	DECLARE_MESSAGE_MAP()
   	DECLARE_DYNAMIC( sColorButton );
};

// DDX functions for use with the sColorButton
void DDX_sColorButton( CDataExchange* pDX, sColorButton &btn, COLORREF& color);


/****************************  sFontButton  *******************************/
/*
#LButton displaing font and his color
#BRemarks:
  #NThis class is used for displaing specified font
  Font is specified by LOGFONT.
  Font color is specified via COLORREF
  After pushing button Font dialog is displaing


*/
class sFontButton : public CButton
{
    BOOL        stayDown;
    COLORREF    col     ;
    LOGFONT     font    ;

  protected:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);//#*
	afx_msg void OnBNClicked();

  public:
    CString  m_fontText;

    operator COLORREF()                   { return col;  }; //Return COLORREF which contains actual font color
    operator LOGFONT ()                   { return font; }; //Return LOGFONT which contains actual font
    COLORREF GetColor( void       )       { return col;  }; //Return COLORREF which contains actual font color
    LOGFONT  GetFont ( void       )       { return font; }; //Return LOGFONT which contains actual font
    void     SetFont ( LOGFONT f, COLORREF c)   { col=c, font=f; RedrawWindow(); }; //Set actual font and his color, This one will displaing in the button
    void     DisplayFontDialog();                           //Display CFontDialog and update button color width selected font     
    
    BOOL     AttachButton(int nCtlID, CWnd* pParentWnd);           //Attach this control to control
    BOOL     Create      ( LPCTSTR lpszCaption, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, int nID); //*#

    sFontButton();                                          //Construct font button class
  	DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC( sFontButton );
};

// DDX functions for use with sFontButton
void DDX_sFontButton( CDataExchange* pDX, sFontButton &btn, LOGFONT&  font, COLORREF& color );


/****************************  sDrawControl  *******************************/
typedef void (CWnd::*sDrawControlFun)( CDC *, int, int ) ; 

/*
#LCreate control to which is possible to draw
#BRemarks:
#NCreate control to which is possible to draw. Drawing is realised via 
  draw function which is registred in the time of the construction.
  Type of the draw function is:
      #Itypedef void (CWnd::*sDrawControlFun)( CDC *dc, int width, int height ) ; 


*/
class sDrawControl: public CWnd
{
  protected:
    sDrawControlFun  m_drawFnc;
    COLORREF         m_color  ;
    CWnd            *m_drawObj;

    afx_msg BOOL OnEraseBkgnd(CDC* pDC); //#*
    afx_msg void OnPaint( )                          { CPaintDC dc(this); OnDraw( &dc );};   //#*
    virtual void OnDraw        ( CDC *dc );

  public:
    COLORREF    backgroundColor( void       )       { return m_color;}; //Return or set Cbrush used for the background filling
    void        backgroundColor( COLORREF c )       { m_color = c;   };

    void        SetDrawFnc     ( sDrawControlFun fnc, CWnd *obj=NULL ) ;
    BOOL        AttachPane     ( int nCtlID, CWnd* pParentWnd, sDrawControlFun f, CWnd *obj=NULL );
    BOOL        Create         ( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, int nID); //*#
    void        DDX_DrawControl( CDataExchange* pDX, UINT id, sDrawControlFun f );

    sDrawControl( ) : CWnd()                        { m_drawFnc = NULL, m_drawObj = NULL; m_color = RGB(255, 255, 255); };  //Construct class (no create control)
   	DECLARE_MESSAGE_MAP()
    DECLARE_DYNAMIC( sDrawControl );
} ;

/*******************************************************************************************************/
/*                                           EXTERN FUNCTIONS                                          */
/*******************************************************************************************************/
float sReadEditedFloat  ( CWnd *edt,				   float from=LONG_MIN, float to=LONG_MAX );
float sReadEditedFloat	( CDataExchange* pDX, UINT id, float from=LONG_MIN, float to=LONG_MAX );
int   sReadEditedInt    ( CWnd *edt,				   int   from=INT_MIN,  int   to=INT_MAX  );
int	  sReadEditedInt	( CDataExchange* pDX, UINT id, int   from=INT_MIN,  int   to=INT_MAX  );
void  sWriteEditedFloat ( CWnd *edt, float num, const char *fmt="%.2f" );
void  sWriteEditedInt   ( CWnd *edt, int num,   const char *fmt="%d"   );

/*******************************************************************************************************/
/*                                           sDialogConfigFile                                         */
/*******************************************************************************************************/
/*
#LFile to which will saved settings of the any dialogs and controls. 
#BRemark:
  #NThis class is used for saving of the settings for any dialogs and controls. 
  This class work width file \"SystemDir\\PrgName.cfg\".   


*/
class sDialogConfigFile
{
    ConfigClass *cfg;

  public:
    ConfigClass *config ( ) ;
	void SaveWndState   ( CWnd *w, const char *sect );
	BOOL LoadWndState   ( CWnd *w, const char *sect, WINDOWPLACEMENT *p=NULL );
    void closeConfig    ( void );

    sDialogConfigFile() { cfg = NULL ;  } 
   ~sDialogConfigFile() { closeConfig();}; 
};


/********************************************************************************/
/*					                sHistoryBox      							*/
/********************************************************************************/
typedef BOOL (CWnd::*FIND_PROC)( const char*, long );
/*
#LClass create history combo box
#BRemark:
  #NClass create history combo box (tool useful for the searching proc.)
  This combo box memorize specified number of the strings.
    

*/
class sHistoryBox : public CComboBox
{
    FIND_PROC    _proc      ;
    CWnd        *_obj       ;
    UINT         _parId     ;

  protected:
    virtual BOOL PreTranslateMessage(MSG* pMsg);  //#*

  public:
    uchar m_StringCount;

    virtual void Load( ConfigClass *cfg);                                               //Load history list from the stream
    virtual void Save( ConfigClass *cfg);                                               //Save history list to the stream
    virtual BOOL FindString ( long lValue );                                            //Call registred proc. for finding string 
    void         RegFindProc( CWnd *obj, FIND_PROC proc, UINT parId=0 ) { _obj=obj; _proc=proc; _parId=parId; };  //Register object and procedure which will to find string. If parId is defined -> GetParent( parId ) is send on ENTER 
    int          AddString  ( LPCTSTR lpszString);                                      //Add new string to the history list
    CString      GetFindString();                                                       //Return string which have to be found 
    
	sHistoryBox( uchar strCount = 20 )  { m_StringCount = strCount; _parId=0;_obj=NULL; _proc=NULL;};   //Construct sHistoryBox class
    virtual ~sHistoryBox()              {};
};

/**********************************************************************************/
/*                               sFontSizeInfo                                    */
/**********************************************************************************/
//#*
class sFontSizeInfo
{
  private:
	int    _charHeight;
	int     charWidths[256];

public:

    void        refreshFontSizes( CDC  *dp);
    inline void refreshFontSizes( CWnd *dp) { CClientDC  dc( dp ); refreshFontSizes( &dc  ); };

    inline int  operator[](int i) const     { return charWidths[i]; }
    inline int  charHeight() const          { return _charHeight;   }
	int         length(char *, int length);

    inline      sFontSizeInfo()             {}
    inline      sFontSizeInfo( CDC *dp)     { refreshFontSizes( dp);}
};

/*************************************************************************************************/
/*                                           sListCtrl                                           */
/*************************************************************************************************/
/*
#LCreate extention list control
#BRemarks:
  #NThis control is used like a multi column list box.
  Mechanism of the class allow to specified sort functions,
  tooltips for all cells, ats.
  This class have also overloaded draw function for drawinf of the all cells.
  

*/
class sListCtrl : public CListCtrl
{
    int     _row, _selRow;

    LV_ITEM *GetItem      ( int index, int subItem=0 );
    void     SetItemStruct( LV_ITEM &lvitem,int col, int item);
    void     DrawItem     ( LPDRAWITEMSTRUCT lpDrawItemStruct);
    void     RepaintSelectedItems();
    void     RepaintItems( CRect &r, int row );

  protected:
    afx_msg void     OnSort       ( LPNMHDR pnmhdr, LRESULT *pResult );
    afx_msg void     OnItemChanged( NMHDR  *pNMHDR, LRESULT *pResult );

			LPCTSTR         MakeShortString( CDC* pDC,  CRect r, LPCTSTR lpszLong ); //#*
    virtual void            DrawCell    ( CDC *dc, CRect r,              int col, long item ) {}; //Function allow draw cell by no statndard style
	virtual void            DrawText    ( CDC *dc, CRect r, CString str, int col, long item );    //Function allow draw text by no statndard style 
    virtual char           *GetText     ( int col,long item){ return "";    };  //Return text which have to be displaing in the spacified cell
    virtual PFNLVCOMPARE    GetSortFun  ( int col )         { return NULL;  };  //Return sort function for the each column    
    virtual UINT            GetBitmap   ( long item)        { return 0;     };  //return resource ID for bitmap on specified row
    virtual int             GetColWidth ( int col )         { return 80;    };  //Width of the specified column
    virtual int             GetNCol     ()                  { return 1;     };  //Count of the columns
    virtual int             GetNRow     ()                  { return 0;     };  //Count of the rows
    virtual long            GetLValue   ( int row )         { return row;   };  //32-Bite value associated width specified row
    
  public:

	BOOL    InitControl ();                                                     //Call in the time of the initialisation    
    BOOL    DelRow      ( long lValue );                                        //Delete item width specified long
    BOOL    AddRow      ( int row );                                            //New item at the end of the list
    BOOL    UpdateItem  ( int row );                                            //Update item in specified row
    BOOL    UpdateItems ();                                                     //Update all items via data in DrawText, GetNCol, ...
    int     GetCurSel   ();                                                     //Return current item index
    long    GetCurLValue()              { int cur = GetCurSel(); return cur != -1 ? GetItemData( cur ) : 0L;}   //Return long associated width current item
	long	GetLValue	( CPoint &p, UINT supportedFlags=LVHT_ONITEM );         //Return item in point p; flgs - see help HitTest
	int		FindByParam	( long lParam );										//Return index of item with param value 
    void    SetCurSel   ( int i );                                              //Set new item like a current    

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE( sListCtrl )
};

#endif
