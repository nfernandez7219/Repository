#include "tools2.hpp"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <float.h>
#include "CtrlTool.hpp"


/********************************************************************************/
/*                                 sIconButton	                                */
/********************************************************************************/


sIconButton::sIconButton()
{
	bCheckBtn=FALSE;	
	hUpBmp	= NULL;
	hDownBmp= NULL;
	iIcoW	= 32;
	iIcoH	= 32;
	bIsCheck= FALSE ;
	bExist	= FALSE ;
}

sIconButton::~sIconButton( )
{
	if( hUpBmp	)
		DestroyIcon( hUpBmp  );
	if( hDownBmp)
		DestroyIcon( hDownBmp);
}

//
// Parameters:
//  UINT	uCtrlID	- button ID
//	CWnd*	pPar	- parent window
//	UINT	uBmpUpID- buttons icon ID
//	LPCTSTR lpUpStr - button text
//
// Description:
//  Subclass and setup bitmap icon
//
void sIconButton::SubclassDlgItem( UINT uCtrlID, CWnd *pPar, UINT uBmpUpID, LPCTSTR lpUpStr, HMODULE mod )
{
	if( !pPar->GetDlgItem(uCtrlID) )
		return ;
	ICONINFO cInfo;

	sUpTxt	= lpUpStr ? lpUpStr : "";
	hUpBmp	= (HICON)LoadImage( mod, MAKEINTRESOURCE(uBmpUpID),	IMAGE_ICON, 0, 0, LR_LOADMAP3DCOLORS  );
	if( hUpBmp && GetIconInfo( hUpBmp, &cInfo ) )
	{
		iIcoW = cInfo.xHotspot*2;
		iIcoH = cInfo.yHotspot*2;
	}
	CButton::SubclassDlgItem( uCtrlID, pPar );
	ModifyStyle( BS_ICON|BS_BITMAP, BS_OWNERDRAW );
	bExist = TRUE ;
}

BOOL sIconButton::EnableWindow( BOOL bEnable )
{
	if( !bExist )
		return FALSE ;
	return CButton::EnableWindow( bEnable ) ;
}

void sIconButton::SetCheck( int nCheck )
{
	if( !bExist )
		return ;
	if( bCheckBtn && nCheck != bIsCheck )
	{
		bIsCheck = nCheck;
		RedrawWindow();
	}
}


//
// Parameters:
//  UINT	uBmpDownID - bitmap for checked state
//	LPCTSTR lpDownText - text for checked state
//
// Description:
//  Make the button simulate 2 states -> pressed and unpressed.
//
void sIconButton::MakeCheckButton( UINT uBmpDownID, LPCTSTR lpDownText, HMODULE mod )
{
	if( !bExist )
		return ;
	bCheckBtn= TRUE;
	sDownTxt = lpDownText ? lpDownText : "";
	if( uBmpDownID )
		hDownBmp = (HICON)LoadImage( mod, MAKEINTRESOURCE(uBmpDownID), IMAGE_ICON, 0, 0, LR_LOADMAP3DCOLORS );
}

void sIconButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	COLORREF crOldColor;
	RECT&	r		= lpDrawItemStruct->rcItem;
	HDC		hDC		= lpDrawItemStruct->hDC;
	HBRUSH	hBkg	= ::GetSysColorBrush( COLOR_3DFACE );
	HICON	hIcon	= hUpBmp;
	LPCTSTR	lpText	= sUpTxt;
	UINT	menuH	= GetSystemMetrics( SM_CYMENU )-1;
	UINT	x		= ((r.right -r.left)-iIcoW)/2;
	UINT	y		= ((r.bottom-r.top )-iIcoH)/2 - (lpText ? menuH/3 : 0);

	if( y<= 2 )
		y = 2;
	if( bIsCheck || (lpDrawItemStruct->itemState & ODS_SELECTED) )
	{
		::DrawFrameControl( hDC, &r, DFC_BUTTON, DFCS_BUTTONPUSH|DFCS_PUSHED );
		hIcon = hDownBmp ? hDownBmp : hUpBmp;
		lpText = sDownTxt.IsEmpty() ? sUpTxt : sDownTxt;
	}
	else
		::DrawFrameControl( hDC, &r, DFC_BUTTON, DFCS_BUTTONPUSH );

	if( lpDrawItemStruct->itemState & ODS_DISABLED )
	{
		crOldColor	= ::SetTextColor( hDC, RGB(120,120,120));
		::DrawState( hDC, NULL, NULL, (LPARAM)hIcon, NULL, x, y, 0, 0, DST_ICON|DSS_DISABLED );
	}
	else
	{
		crOldColor	= ::SetTextColor( hDC, RGB(0,0,0));
		::DrawIconEx( hDC, x, y, hIcon, 0, 0, 0, 0, DI_NORMAL );
	}

	// draw focus
	if (lpDrawItemStruct->itemState & ODS_FOCUS)
	{
		CRect focusRect(r);
		focusRect.InflateRect(-4, -4);
		DrawFocusRect( hDC, focusRect );
	}		

	// draw text
	if( *lpText != 0 )
	{
		r.top = r.bottom - menuH;
		::SetBkMode( hDC, TRANSPARENT );
		::DrawText( hDC, lpText, strlen(lpText), &r, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
		::SetTextColor( hDC, crOldColor);
	}
}

LRESULT sIconButton::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_LBUTTONUP:
		{
			CPoint	pt(GetMessagePos());
			CRect	r;

			// was mouse unpressed under button ?
			GetWindowRect( &r );
			if( r.PtInRect( pt ) )
			{
				if( bCheckBtn )
					bIsCheck = !bIsCheck;
			}
			break ;
		}

		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS ;

		case WM_KEYDOWN:
		case WM_KEYUP:
			GetParent()->PostMessage( message, wParam, lParam ) ;
			return 0 ;
	}
	return CButton::WindowProc( message, wParam, lParam ) ;
}


/********************************************************************************/
/*                                 sDrawControl                                 */
/********************************************************************************/


IMPLEMENT_DYNAMIC( sDrawControl, CWnd );
BEGIN_MESSAGE_MAP( sDrawControl, CWnd)
	ON_WM_ERASEBKGND()
    ON_WM_PAINT()
END_MESSAGE_MAP()

/*
#LCreate draw control
#BReturn value:
  #IFunction return TRUE on success

#BParameters:
  #Iw   -#Npointer to the parent window
  #Ir   -#Nsize and position of the control
  #If   -#Npointer to the drawing fuction.
            typedef void (CWnd::*sDrawControlFun)( CDC *, int, int ) ; 
  #Iobj -#Npointer to the object which contain draw function (if NULL -> use w ) 
  #Istl -#Nwindow styles which characterize control
  #Iid  -#Nsize of the control will read from control width id.

#BRemarks:
  #NThis function create control to which is possible draw. 


*/
BOOL sDrawControl::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, int nID)
{
	return CWnd::Create( NULL, NULL, dwStyle|WS_CHILD, rect, pParentWnd, nID);
}

BOOL sDrawControl::AttachPane(int id, CWnd* w, sDrawControlFun f, CWnd *obj )
{
    CWnd   *ctrl = w->GetDlgItem( id );
    CRect   r   ;
    int     ret, ctrlStyle, offset=0 ;

    ASSERT( ctrl );
    ctrlStyle = ctrl->GetStyle();
    ctrl->GetWindowRect( &r );
    w->ScreenToClient(   &r );
    if( ctrlStyle&SS_SUNKEN )
        offset += 1;
    if( ctrlStyle&WS_BORDER )
        offset += 1;

    r.left  += offset;
    r.top   += offset;
    r.right -= offset;
    r.bottom-= offset;
    SetDrawFnc( f, obj );
    if( (ret = Create( ctrlStyle|WS_CHILD|WS_VISIBLE, r, w, id )) )
    {
        SetDlgCtrlID( id );
        ctrl->SetDlgCtrlID( -1 );
        ctrl->DestroyWindow();
    }
    return ret;
}

void sDrawControl::DDX_DrawControl( CDataExchange* pDX, UINT id, sDrawControlFun f )
{
    if( !pDX->m_bSaveAndValidate && !m_hWnd )
    {
        backgroundColor( ::GetSysColor(COLOR_3DFACE) );
        AttachPane( id, pDX->m_pDlgWnd, f );
    }
};

BOOL sDrawControl::OnEraseBkgnd(CDC* pDC)
{
	ASSERT_VALID(pDC);
    CRect   rect;
    CBrush  brs( m_color );
	CBrush *pOldBrush = pDC->SelectObject( &brs );
	
    pDC->GetClipBox(&rect);     // Erase the area needed
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
	pDC->SelectObject(pOldBrush);
	return TRUE;
}

void sDrawControl::OnDraw( CDC *pDC )
{
    CRect   rect;

    GetClientRect( &rect );

    if(!m_drawObj )
        m_drawObj = GetParent();
    if( m_drawObj )
        (m_drawObj->*m_drawFnc)( pDC, rect.Width(), rect.Height());
};

void sDrawControl::SetDrawFnc( sDrawControlFun f, CWnd *obj )
{
    m_drawFnc = f;
    m_drawObj = obj;
};

/*******************************************************************************************************/
/*                                          EXTERN FUNCTIONS                                           */
/*******************************************************************************************************/
/********************************************************************************/

void sWriteEditedFloat( CWnd *edt, float num, const char *fmt )
{
	if( fmt == NULL )
		fmt = "%.2f" ;
    CString str;
    str.Format( fmt, num );
    edt->SetWindowText( str );
};

void sWriteEditedInt( CWnd *edt, int num, const char *fmt  )
{
	if( fmt == NULL )
		fmt = "%d" ;
    CString str;
    str.Format( fmt, num );
    edt->SetWindowText( str );
};

/*******************************************************************************************************/
/*                                         DialogConfigFile                                            */
/*******************************************************************************************************/

#define INVALID_CFG		((ConfigClass*)1)

 ConfigClass *sDialogConfigFile :: config()
 {
	 if( cfg == NULL )
	 {
		char *appCfgFile = "DlgSett.cfg";

		cfg = new ConfigClass( appCfgFile );
		if( cfg->open( getGlbLogin()->usrName, FALSE, TRUE, TRUE) )
		{
			delete cfg;
			cfg = INVALID_CFG ;
		}
	 }
	 return cfg == INVALID_CFG ? NULL : cfg ;
 }


void sDialogConfigFile :: closeConfig( void )
{
    if( cfg != NULL  &&  cfg != INVALID_CFG )
    {
        if( cfg->isChanged() )
            cfg->save( FALSE );
        cfg->close();
        delete cfg;
        cfg = NULL;
    }
};

void sDialogConfigFile::SaveWndState( CWnd *w, const char *sect )
{
    WINDOWPLACEMENT	p;
	ConfigClass *c = config() ;
    if( !c )
		return;

	w->GetWindowPlacement( &p );
    c->printf( sect, "State", "%d,%d,%d,[%d,%d],[%d,%d],[%d,%d,%d,%d]",
		p.length, p.showCmd, p.flags, p.ptMinPosition.x, p.ptMinPosition.y,
		p.ptMaxPosition.x, p.ptMaxPosition.y, p.rcNormalPosition.left, 
		p.rcNormalPosition.top, p.rcNormalPosition.right, p.rcNormalPosition.bottom );
};

BOOL sDialogConfigFile::LoadWndState( CWnd *w, const char *sect, WINDOWPLACEMENT *p )
{
    WINDOWPLACEMENT	place;
	ConfigClass *c = config() ;
    if( !c )
		return FALSE ;
	char *ptr = c->get( sect, "State" ) ;
	if( !ptr )
		return FALSE ;

    if( !p )
         p = &place;
	if( sscanf( ptr, "%d,%d,%d,[%d,%d],[%d,%d],[%d,%d,%d,%d]", 
		&p->length, &p->showCmd, &p->flags, &p->ptMinPosition.x, &p->ptMinPosition.y,
		&p->ptMaxPosition.x, &p->ptMaxPosition.y, &p->rcNormalPosition.left, 
		&p->rcNormalPosition.top, &p->rcNormalPosition.right, &p->rcNormalPosition.bottom ) != 11 )
		return FALSE;

	// validity check
	if( p->length != sizeof(WINDOWPLACEMENT) )
		return FALSE ;
	switch( p->showCmd )
	{
		case SW_HIDE		:
		case SW_MINIMIZE	:
		case SW_SHOWMINIMIZED:
		case SW_SHOWMINNOACTIVE:
			p->showCmd = SW_SHOW ;
			break ;
		case SW_SHOWNOACTIVATE:
		case SW_SHOWNA		:		// show no active
		case SW_RESTORE		:
		case SW_SHOW		:
		case SW_SHOWMAXIMIZED:
		case SW_SHOWNORMAL	:
			break ;
		default:
			return FALSE ;
	}
	int x1 = p->rcNormalPosition.left ;
	int y1 = p->rcNormalPosition.top ;
	int x2 = p->rcNormalPosition.right ;
	int y2 = p->rcNormalPosition.bottom ;
	if( x1 > x2-10  ||  y1 > y2-10 )
		return  FALSE ;

    if( p == &place )
    {
        if( w->SetWindowPlacement( p) )
	        w->UpdateWindow();
    }
	return TRUE;
}

/********************************************************************************/
/*                                 sFontSizeInfo                                */
/********************************************************************************/
void sFontSizeInfo :: refreshFontSizes( CDC *dp)
{
	int           i;
	unsigned char c;

    for( i = 0; i < 256; i++ )
		charWidths[c] = (dp->GetTextExtent( (char*)&(c=i), 1)).cx ;
   _charHeight = (dp->GetTextExtent( "a", 1)).cy ;
}

int sFontSizeInfo::length(char *s, int c)
{
	int len = 0;
	while (c--) 
		len += charWidths[*s++];
	return len;
}

/********************************************************************************/
/*					                sHistoryBox      							*/
/********************************************************************************/
//#*
BOOL sHistoryBox::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message != WM_KEYDOWN) || (pMsg->wParam != VK_RETURN))
		return CComboBox::PreTranslateMessage(pMsg);
	if ((pMsg->lParam & 0x40000000) == 0)
	{
		CString strText = GetFindString();
        FindString( 0 );
        //if( !_parId )
        //     _parId = GetDlgCtrlID();
        //if( _parId && GetParent() )
	    //    GetParent()->SendMessage( WM_COMMAND, LOWORD(_parId), long(0) );
    }
	return TRUE;
}

int sHistoryBox :: AddString( LPCTSTR lpszString )
{
    int     ret=0;
    if( lpszString && *lpszString )
    {
        if( (ret=FindStringExact( 0, lpszString )) >= 0 )
            return ret;
	    ret = InsertString(0, lpszString);
	    //SetCurSel(0);
	    while( GetCount() > int(m_StringCount) )
		    DeleteString(GetCount()-1);
    }
    return ret;
};

BOOL sHistoryBox::FindString( long lValue )
{
    CString str = GetFindString();
    if( !str.IsEmpty() )
	{
		AddString( str );
		return (_proc && _obj) ? (_obj->*_proc)( str, lValue ) : FALSE;        
	}
	return FALSE;
};
 
CString sHistoryBox::GetFindString()
{
	CString strText;
	GetWindowText(strText);
    return strText;     
};

static const char *cfgSection  = "FindDialog";
static const char *cfgCount    = "Count";
static const char *cfgText     = "Text%d";
void sHistoryBox::Save( ConfigClass *cfg )
{
    char        name[15];
    
    if( !cfg )  return ;
    cfg->printf( cfgSection, cfgCount, "%d", GetCount() ); 
    for( int i=0; i<GetCount(); i++)
    {
        CString str;
        GetLBText( i, str );
        sprintf  ( name, cfgText, i ); 
        cfg->set( cfgSection, name, str ); 
    }
}

void sHistoryBox::Load( ConfigClass *cfg )
{
    char       *str, name[15];
    int         count;
    
    if( cfg && cfg->getInt( cfgSection, cfgCount, &count ) )
    {
        if( count > m_StringCount )
            count = m_StringCount; 
        for( int i=0; i<count; i++)
        {
            sprintf( name, cfgText, i ); 
            if( (str = cfg->get( cfgSection, name )) )
                AddString( str );            
        }
    }
}

/*************************************************************************************************/
/*                                           sListCtrl                                           */
/*************************************************************************************************/


IMPLEMENT_DYNCREATE( sListCtrl, CListCtrl )

BEGIN_MESSAGE_MAP( sListCtrl, CListCtrl )
    ON_NOTIFY_REFLECT( LVN_ITEMCHANGED,  OnItemChanged )
    ON_NOTIFY_REFLECT( LVN_COLUMNCLICK, OnSort         )
END_MESSAGE_MAP()

void sListCtrl :: OnSort( LPNMHDR pnmhdr, LRESULT *pResult)
{
    int          code = pnmhdr->code;
    NM_LISTVIEW *pnmtv= (NM_LISTVIEW FAR *)pnmhdr; 
    
    if( code == LVN_COLUMNCLICK && GetSortFun( pnmtv->iSubItem ))
        SortItems( GetSortFun(pnmtv->iSubItem), long(pnmtv->iSubItem));
   *pResult = 0;
};

void sListCtrl::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW *sel = (NM_LISTVIEW *)pNMHDR;

    if( _selRow != sel->iItem && sel->iItem != -1 )
    {
       _selRow = sel->iItem;
        GetParent()->SendMessage( WM_COMMAND, LOWORD(GetDlgCtrlID()), long( m_hWnd ) );
    }
    RepaintSelectedItems();
   *pResult = 0;
}

LV_ITEM *sListCtrl :: GetItem( int index, int subItem )
{
    static LV_ITEM item;
    static char    buf[100];
    
    item.pszText  = buf;
    item.cchTextMax=99;
    item.iItem    = index;
    item.iSubItem = subItem;
    item.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
	return CListCtrl::GetItem( &item ) ? &item : NULL;
};

void sListCtrl :: SetItemStruct( LV_ITEM &lvitem, int col, int actItem )
{
    lvitem.iSubItem = col ;
    lvitem.lParam   = (long)GetLValue( _row			);
	lvitem.iItem    = (!col ? _row : actItem		);
    lvitem.pszText  = GetText( col, lvitem.lParam	);
	lvitem.iImage	= GetBitmap( lvitem.lParam		);
    lvitem.mask     = LVIF_TEXT | (!col ? LVIF_PARAM : 0) | (lvitem.iImage ? LVIF_IMAGE : 0);
};		

BOOL sListCtrl :: AddRow( int row )
{
    LV_ITEM		lvitem;
	int         item=0, numCol = GetNCol();

   _row = row;
    for( int col = 0; col < numCol; col++)
	{
        SetItemStruct( lvitem, col, item );
        if( !col )      item = InsertItem(&lvitem);
		else            SetItem(&lvitem);
	}
    return TRUE;
};


BOOL sListCtrl :: UpdateItem( int row )
{
    LV_ITEM		lvitem;
	int         col, numCol=GetNCol();

	ASSERT( GetItemCount() > row );
   _row = row;
    for( col = 0; col < numCol; col++)
	{
	    lvitem.iSubItem = col ;
		lvitem.lParam   = (long)GetItemData( row );
		lvitem.iItem    = row;
	    lvitem.pszText  = GetText( col, lvitem.lParam	);
		lvitem.iImage	= GetBitmap( lvitem.lParam		);
	    lvitem.mask     = LVIF_TEXT | (!col ? LVIF_PARAM : 0) | (lvitem.iImage ? LVIF_IMAGE : 0);
        SetItem(&lvitem);
    }
    return TRUE;
}


BOOL sListCtrl :: UpdateItems()
{
    int act      = 0;
    int rowCount = GetNRow();

    if( GetItemCount() )
	{
        act = GetCurSel();
	    DeleteAllItems( );
	}
    for( _row = 0; _row < rowCount; _row++ )
	    AddRow( _row );

	if( rowCount )
		SetCurSel( act > 0 ? act : 0 );
    return TRUE;
};

BOOL sListCtrl :: InitControl()
{
	int	        colCount = GetNCol(), col;
	LV_COLUMN	lvcolumn;

   _selRow = 0;
    ModifyStyle( 0L, LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS);
    if( GetItemCount() )
	    DeleteAllItems( );
    while( DeleteColumn( 0 ) );
    for( col = 0; col < colCount; col++)
	{
		lvcolumn.fmt        = LVCFMT_LEFT;
        lvcolumn.pszText    = GetText( col, NULL );
		lvcolumn.cx         = GetColWidth( col );
        lvcolumn.iSubItem   = col;
		lvcolumn.mask       = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
        InsertColumn( col, &lvcolumn);
	}
    UpdateItems();
    return TRUE;
};

int sListCtrl :: GetCurSel()
{
    int count = GetSelectedCount(), i=-1;
    if( count )
        while( (i = GetNextItem( i, LVNI_SELECTED )) != -1 )
            return i;
    return -1; 
}

void sListCtrl :: SetCurSel( int i )
{
    if( i >= 0 && i < GetItemCount() )
        SetItemState( i, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED | LVIS_CUT );
}

long sListCtrl::GetLValue( CPoint &p, UINT supportedFlags )
{
    UINT    nFlags;
    int		pos = HitTest( p, &nFlags ); 

    if( pos != -1 && nFlags&supportedFlags )
		return GetItemData( pos );
    return NULL;
}

int	sListCtrl :: FindByParam( long lParam )
{
	LV_FINDINFO	item;
	
	item.flags = LVFI_PARAM	;
	item.lParam= lParam		;
	return FindItem( &item, -1 );
};

BOOL sListCtrl :: DelRow( long lValue )
{
    return TRUE;
};

void sListCtrl :: DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC        *pDC = CDC::FromHandle( lpDrawItemStruct->hDC );
	CRect       rcItem( lpDrawItemStruct->rcItem );
	UINT        uiFlags = ILD_TRANSPARENT;
	CImageList *pImageList;
	int         nItem = lpDrawItemStruct->itemID;
	BOOL        bFocus= (GetFocus()==this);
	COLORREF    clrTextSave, clrBkSave;
	COLORREF    clrImage=::GetSysColor(COLOR_WINDOW);
	static     _TCHAR szBuff[MAX_PATH];
	CString     pszText;

// get item data
	LV_ITEM lvi;
	lvi.mask        = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
	lvi.iItem       = nItem;
	lvi.iSubItem    = 0;
	lvi.pszText     = szBuff;
	lvi.cchTextMax  = sizeof(szBuff);
	lvi.stateMask   = 0xFFFF;       //get all state flags
    CListCtrl::GetItem( &lvi );

	BOOL bSelected  = (bFocus   || (GetStyle() & LVS_SHOWSELALWAYS)) && lvi.state & LVIS_SELECTED;
	bSelected       = bSelected || (lvi.state  & LVIS_DROPHILITED);

// set colors if item is selected
	CRect rcAllLabels, rcLabel, r;
    GetItemRect( nItem, rcAllLabels, LVIR_BOUNDS);
    GetItemRect( nItem, rcLabel,     LVIR_LABEL );
    GetClientRect( &r );
    rcAllLabels.left  = rcLabel.left;
//	if( rcAllLabels.right < r.Width())
//		rcAllLabels.right = r.Width();

	if( bSelected )
	{
		clrTextSave= pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		clrBkSave  = pDC->SetBkColor(  ::GetSysColor(COLOR_HIGHLIGHT    ));
		pDC->FillRect(rcAllLabels,&CBrush(::GetSysColor(COLOR_HIGHLIGHT)));
	}
	else
		pDC->FillRect(rcAllLabels,&CBrush(::GetSysColor(COLOR_WINDOW   )));

// set color and mask for the icon
	if( lvi.state & LVIS_CUT )
		uiFlags |= ILD_BLEND50;
	else if(bSelected)
	{
		clrImage=::GetSysColor(COLOR_HIGHLIGHT);
		uiFlags|=ILD_BLEND50;
	}

// draw state icon
	UINT nStateImageMask = lvi.state & LVIS_STATEIMAGEMASK;
	if(nStateImageMask)
	{
		int nImage = (nStateImageMask>>12)-1;
		pImageList = GetImageList(LVSIL_STATE);
		if(pImageList)
			pImageList->Draw( pDC,nImage, CPoint(rcItem.left,rcItem.top), ILD_TRANSPARENT );
	}

// draw normal and overlay icon
	CRect rcIcon;
	GetItemRect(nItem,rcIcon,LVIR_ICON  );
	pImageList =GetImageList(LVSIL_SMALL);
	if(pImageList)
	{
		UINT nOvlImageMask=lvi.state & LVIS_OVERLAYMASK;
		if( rcItem.left < rcItem.right-1 )
			ImageList_DrawEx(pImageList->m_hImageList,lvi.iImage,pDC->m_hDC,rcIcon.left,rcIcon.top,16,16, ::GetSysColor(COLOR_WINDOW),clrImage,uiFlags | nOvlImageMask);
	}

// draw item label
	GetItemRect( nItem, rcItem, LVIR_LABEL );
    DrawCell( pDC, rcItem,         0, NULL );
    DrawText( pDC, rcItem, szBuff, 0, NULL );

// draw labels for extra columns
	LV_COLUMN lvc;
	lvc.mask = LVCF_WIDTH;
	for(int nColumn=1; GetColumn(nColumn,&lvc); nColumn++)
	{
		rcItem.left  = rcItem.right;
		rcItem.right+= lvc.cx;
        DrawCell( pDC, rcItem, nColumn, GetItemData( nItem ) );
		int nRetLen  = GetItemText( nItem, nColumn, szBuff, sizeof(szBuff));
		if(nRetLen==0) continue;
        DrawText( pDC, rcItem, szBuff, nColumn, GetItemData( nItem ) );
	}

// draw focus rectangle if item has focus
	if(lvi.state & LVIS_FOCUSED && bFocus)
		pDC->DrawFocusRect( rcAllLabels );

// set original colors if item was selected
	if(bSelected)
	{
        pDC->SetTextColor(clrTextSave);
		pDC->SetBkColor(  clrBkSave  );
	}
}

LPCTSTR sListCtrl :: MakeShortString( CDC* pDC,  CRect r, LPCTSTR lpszLong )
{
	static const _TCHAR szThreeDots[]=_T("...");

	int           len = lstrlen(lpszLong);

	if( !len || pDC->GetTextExtent(lpszLong, len).cx <= r.Width())
		return(lpszLong);

	static _TCHAR szShort[MAX_PATH];

	lstrcpy(szShort,lpszLong);
	int nAddLen=pDC->GetTextExtent(szThreeDots,sizeof(szThreeDots)).cx;

	for(int i=len-1; i>0; i--)
	{
		szShort[i]=0;
		if(pDC->GetTextExtent(szShort,i).cx + nAddLen <= r.Width() )
			break;
	}

	lstrcat( szShort, szThreeDots);
	return(szShort);
}

#define OFFSET_FIRST	2
#define OFFSET_OTHER	6
void sListCtrl :: DrawText( CDC *dc, CRect r, CString str, int col, long item )
{
	LV_COLUMN   lvc;
    LPCTSTR     retStr;
    UINT        nJustify= DT_LEFT,
                off     = col ? OFFSET_OTHER : OFFSET_FIRST;
	int			len     = str.GetLength();

	lvc.mask = LVCF_FMT;
	GetColumn( col, &lvc);
    if( len == str.GetLength() )
    {
	    if( (lvc.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT )
            nJustify=DT_RIGHT;
        else
        if( (lvc.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_CENTER)
		    nJustify=DT_CENTER;
    }

	r.left   += off;
	r.right  -= off;
    retStr    = MakeShortString( dc, r, (LPCTSTR)str );
	dc->DrawText( retStr, r, nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER );
};

void sListCtrl::RepaintSelectedItems()
{
    CRect   r;
	int     nItem = GetNextItem(-1,LVNI_FOCUSED);

    GetClientRect( &r );
	if(nItem!=-1)
        RepaintItems( r, nItem );

	if(!(GetStyle() & LVS_SHOWSELALWAYS))
		for(nItem=GetNextItem(-1,LVNI_SELECTED); nItem!=-1; nItem=GetNextItem(nItem,LVNI_SELECTED))
            RepaintItems( r, nItem );
	UpdateWindow();
}

void sListCtrl::RepaintItems( CRect &r, int nItem )
{
	CRect   rcItem, rcLabel;

    GetItemRect( nItem,rcItem, LVIR_BOUNDS );
	GetItemRect( nItem,rcLabel,LVIR_LABEL  );
	rcItem.left = rcLabel.left;
    if( rcItem.right < r.Width())
		rcItem.right = r.Width();
    InvalidateRect(rcItem,FALSE);
};

