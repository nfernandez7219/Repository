#include "tools2.hpp"
#include "dlg_tool.hpp" 

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=======================================================================
//							sSheetInfo
//=======================================================================
void sSheetInfo::clear()
{
	aChildPages.clearList();
	cTitle.Empty();
	pParentSheet= NULL;
	pDialog		= NULL;
	hTreeItem	= NULL;
	uImageID	= 0;
	lValue		= 0;
}

sSheetInfo::sSheetInfo( sDllDialog *dlg, LPCTSTR titl, UINT imgID )
{
	if ( titl == NULL )
		titl = dlg->getDlgTitle() ;

	clear();
	pDialog	= dlg;
	cTitle	= titl ? titl : "";
	uImageID= imgID;
}

//=======================================================================
//						   sSheetDialog
//=======================================================================
IMPLEMENT_DYNAMIC( sSheetDialog, sDllDialog)
BEGIN_MESSAGE_MAP( sSheetDialog, sDllDialog)
	//{{AFX_MSG_MAP(CFireDlg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

sSheetDialog::sSheetDialog( ResDllDesc *mod, PageWndType type, UINT dlgID,
							UINT tabCtrlID, UINT areaID, CWnd* pParent )
		   : sDllDialog( mod, dlgID, pParent )
{
	initialize( type, tabCtrlID, areaID );
}

sSheetDialog::sSheetDialog( ResDllDesc *mod, PageWndType type, UINT  dlgID, 	
							UINT  areaID, int prevBtnId, int nextBtnId, CWnd* pParent )
		   : sDllDialog( mod, dlgID, pParent )
{
	ASSERT( type == sSheetDialog::WizardDlg) ;
	initialize( WizardDlg, 0, areaID );
	uPrevButtonId = prevBtnId ;
	uNextButtonId = nextBtnId ;
}

static void delSheetDlgs( sSheetInfo **d )
{
	delete (*d)->getDialog();
	delete	*d;
};

void sSheetDialog :: initialize( PageWndType type, UINT ctrlID, UINT areaID )
{
	iActivePage 	= -1	;
	m_uPageAreaCtrlID=areaID;
	m_uMngControlID	= ctrlID;
	m_eDialogType	= type	;
	m_pMngControl	= NULL	;
	bInInitMode		= TRUE ;	 
	bInProtectMode	= FALSE;
	uPrevButtonId	= 0 ;
	uNextButtonId	= 0 ;
	aPageList.setDelFunc( delSheetDlgs );
}

void sSheetDialog::OnDestroy()
{
	m_ImagesList.DeleteImageList();
	sDllDialog::OnDestroy();
}

void sSheetDialog :: createTreeItem( sSheetInfo *pPageInfo )
{
	TV_INSERTSTRUCT tvstruct;
	LPCTSTR 		txt = (LPCTSTR)pPageInfo->cTitle;		 
	sSheetInfo	   *par = pPageInfo->getParent();
	CTreeCtrl	   *pTreeCtrl= (CTreeCtrl*)m_pMngControl;
	int 			childNum = pPageInfo->aChildPages.count();

	if( !txt || !*txt )
		return;

	// fill tree ctrl structure
	tvstruct.item.iImage	= findImageByID( pPageInfo->uImageID );	
	tvstruct.item.iSelectedImage = tvstruct.item.iImage;
	tvstruct.hInsertAfter	= TVI_LAST ;
	tvstruct.hParent		= HTREEITEM( par ? par->hTreeItem : NULL );
	tvstruct.item.pszText	= LPSTR( txt );
	tvstruct.item.cChildren = childNum ? TRUE : FALSE;
	tvstruct.item.mask		= TVIF_PARAM | TVIF_TEXT | TVIF_CHILDREN | ( tvstruct.item.iImage==-1 ? 0 : TVIF_IMAGE|TVIF_SELECTEDIMAGE );
	tvstruct.item.lParam	= long( pPageInfo );
	pPageInfo->hTreeItem	= pTreeCtrl->InsertItem( &tvstruct );

	// create all subpages
	for( int i=0; i<childNum; i++ )
		createTreeItem( pPageInfo->aChildPages[i] );
}

void sSheetDialog::createMngItem( sSheetInfo *pPageInfo )
{
	if( m_pMngControl==NULL )
		return;	

	bInProtectMode = TRUE;
	if( m_eDialogType == PropertyDlg )
		createTreeItem( pPageInfo );
	else
	if( m_eDialogType == PagesDlg )
	{
		CTabCtrl *pTabCtrl	= (CTabCtrl*)m_pMngControl;
		char	 *txt		= (char*)LPCTSTR(pPageInfo->getTitle());
		int		  index		= pTabCtrl->GetItemCount();
		int 	  image		= findImageByID( pPageInfo->uImageID );
		TC_ITEM   tcItem	= { TCIF_PARAM, 0, 0, txt, 0, image, long(pPageInfo) };

		// add pPageInfo to tab control
		if( txt )
			tcItem.mask |= TCIF_TEXT;
		if( image!=-1)
			tcItem.mask |= TCIF_IMAGE; 
		pPageInfo->hTreeItem = HTREEITEM( index );
		pTabCtrl->InsertItem( index, &tcItem );
	}
	bInProtectMode = FALSE;
}

BOOL sSheetDialog::OnInitDialog()
{
	CWnd *frame;
 
	sDllDialog::OnInitDialog();

	// Init control Wnd
	if( m_uMngControlID )
		m_pMngControl = GetDlgItem( m_uMngControlID );

	// Get page area
	if( m_uPageAreaCtrlID && (frame=GetDlgItem(m_uPageAreaCtrlID)) )
	{
		frame->GetWindowRect( &m_rPagesAreaRect );
		ScreenToClient( &m_rPagesAreaRect );
		frame->DestroyWindow();
	}
	else
	if( m_eDialogType==PagesDlg && m_pMngControl )
	{
		m_pMngControl->GetWindowRect( &m_rPagesAreaRect );
		ScreenToClient( &m_rPagesAreaRect );
		m_rPagesAreaRect.DeflateRect( 5, 30, 5, 5 );
	}
	else
	{
		CRect parentRect;
		GetWindowRect( &parentRect );
		m_rPagesAreaRect = CRect( 5, 5, parentRect.Width()-10, parentRect.Height()-10);
	}

	// Init page structure
	int nCount = aPageList.count();
	
	if( nCount )
	{
		for( int i=0; i < nCount; i++ )
			if(  aPageList[i]->getParent() == NULL )
				createMngItem( aPageList[i] );
		expandAll();
		showPage( aPageList[0], TRUE, TRUE ); 
	}

	if( m_eDialogType == WizardDlg )
		enableNextPrev();
	bInInitMode=FALSE;
	return TRUE;  
}

BOOL sSheetDialog :: OnCommand( WPARAM wParam, LPARAM lParam )
{
	UINT id	= LOWORD(wParam);
	switch( id )
	{
		case IDOK:
		case IDCANCEL:
			EndDialog( id );
			return 1;
		case 0 :
			break ;
		default:
			if( id == uPrevButtonId )
				showPrevPage();
			else
			if( id == uNextButtonId )
				showNextPage();
	}
	return sDllDialog::OnCommand( wParam, lParam ) ;
};

BOOL sSheetDialog::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
	UINT ctrlID = wParam;		 
	int  page	= iActivePage;

	if( bInInitMode )
		return sDllDialog::OnNotify( wParam, lParam, pResult );
	if( ctrlID == m_uMngControlID )
	{
		if( m_eDialogType == PagesDlg )
		{
			LPNMHDR pnmh = (LPNMHDR)lParam;
			CTabCtrl *tab= (CTabCtrl*)m_pMngControl;

			if( pnmh->code == TCN_SELCHANGING )
			{
				*pResult = !isPageChangingAllowed(-1);
				return *pResult;
			}
			else
			if( pnmh->code == TCN_SELCHANGE )
			{
				int newSheet = tab->GetCurSel();
				onPageChanged( page, newSheet );
			}
		}
		else
		if( m_eDialogType == PropertyDlg )
		{
			CTreeCtrl	   *pTreeCtrl  = (CTreeCtrl*)m_pMngControl;
			NM_TREEVIEW    *pnmtv = (NM_TREEVIEW FAR *) lParam;
			TV_ITEM 	   *itemNew=&pnmtv->itemNew;

			if( pnmtv->hdr.code == TVN_SELCHANGING )
			{
				*pResult = !isPageChangingAllowed( -1 );
				return *pResult;
			}
			else
			if( pnmtv->hdr.code == TVN_SELCHANGED )
			{
				LPNMTREEVIEW pnmtv  = (LPNMTREEVIEW)lParam; 
				sSheetInfo  *newPage= (sSheetInfo*)(pnmtv->itemNew.lParam);
				sSheetInfo  *oldPage= (sSheetInfo*)(pnmtv->itemOld.lParam);

				if(   newPage &&
					!(pnmtv->itemOld.state&TVIS_SELECTED) &&
					 (pnmtv->itemNew.state&TVIS_SELECTED) )
				{
					onPageChanged( findPageInfo( oldPage ), findPageInfo( newPage ) );
				}
			}
		}
	}
	return sDllDialog::OnNotify( wParam, lParam, pResult );
};

sSheetInfo *sSheetDialog :: getShownPageInfo( )
{
	if( !aPageList.count() || iActivePage < 0 || iActivePage >= aPageList.count() )
	{
		iActivePage = -1;
		return NULL;
	}
	return aPageList[iActivePage];
}; 

BOOL sSheetDialog :: createPage( sSheetInfo *pPageInfo )
{
	sDllDialog *pDlg= pPageInfo->getDialog(); 
	UINT		uID = pPageInfo->getDialogID();
 
	// Return if dialog not defined or is created already.
	if( pPageInfo->isPageEmpty() || pDlg->m_hWnd )
		return TRUE;

	// create dialog
	if( !pDlg->Create( pDlg->getResMod(), uID, this) )
		return FALSE ;

	// Hide unneeded dialog elements (border, caption...)
	DWORD delStyle = WS_CAPTION | WS_POPUPWINDOW | WS_DLGFRAME | WS_THICKFRAME |
					 DS_SYSMODAL | DS_MODALFRAME | DS_3DLOOK;
	pDlg->ModifyStyle( delStyle, DS_CONTROL | WS_CHILD );

	pDlg->ModifyStyleEx( WS_EX_APPWINDOW | WS_EX_CONTEXTHELP | WS_EX_OVERLAPPEDWINDOW |
						 WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME, 0 );

	pDlg->SetParent( this );
	pDlg->SetDlgCtrlID( uID );

	//
	// Adjust page rectangle:
	//	- cut to max. available size if too big
	//	- center if too small
	//

	int   y1 = m_rPagesAreaRect.top ;
	int   x1 = m_rPagesAreaRect.left ;

	CRect rect;
	pDlg->GetWindowRect(rect );

	int max_wid = m_rPagesAreaRect.Width() ;
	if( rect.Width() > max_wid )
		rect.right = rect.left + max_wid ;
	else
	if( m_eDialogType!=WizardDlg )
		x1 += (max_wid - rect.Width())/2 ;

	int max_hgh = m_rPagesAreaRect.Height() ;
	if( rect.Height() > max_hgh )
		rect.bottom = rect.top + max_hgh ;
	else
	if( m_eDialogType!=WizardDlg )
		y1 += (max_hgh - rect.Height())/2 ;

	// SWP_DRAWFRAME is needed to hide eventual caption.
	pDlg->SetWindowPos( this, x1, y1, rect.Width(), rect.Height(),
		SWP_NOZORDER | SWP_HIDEWINDOW | SWP_DRAWFRAME);

	// Hide page window
	pDlg->ShowWindow( SW_HIDE);
	return	TRUE ;
}

void sSheetDialog :: createAllPages()
{
	for( int i=0; i<aPageList.count(); i++ )
		createPage( aPageList[i] );
}

BOOL sSheetDialog :: updateData( BOOL saveAndValidate, BOOL onlyShown )
{
	if( onlyShown )
	{
		sSheetInfo *pg = getShownPageInfo();
		if( pg != NULL )
		{
			CDialog *dlg = pg->getDialog();

			if( dlg && dlg->m_hWnd && dlg->IsWindowVisible() )
			{
				if( dlg->IsKindOf( RUNTIME_CLASS( sSheetDialog) ) )
				{
					if( !((sSheetDialog*)dlg)->updateData( saveAndValidate, onlyShown ) )
						return FALSE;
				}
				else
				{
					if( !dlg->UpdateData( saveAndValidate ) )
						return FALSE;
				}
			}
		}
		if( IsWindowVisible() && !UpdateData( saveAndValidate ) )
			return FALSE;
	}
	else
	{
		int count = aPageList.count();
		for( int i=0; i < count; i++ )
		{
			CDialog *dlg = aPageList[i]->getDialog(); 
			if( dlg && dlg->m_hWnd )
			{
				if( dlg->IsKindOf( RUNTIME_CLASS(sSheetDialog) ) )
				{
					if(!((sSheetDialog*)dlg)->updateData( saveAndValidate, onlyShown ) )
						return FALSE;
				}
				else
				{
					if( !dlg->UpdateData( saveAndValidate ) )
						return FALSE;
				}
			}
		}
		if( !UpdateData( saveAndValidate ) )
			return FALSE;
	}
	return TRUE;
};

static BOOL findDialogViaName( void *cTitle, sSheetInfo **lst )   { return !strcmp( (*lst)->getTitle(), (LPCTSTR )cTitle); } 
sSheetInfo *sSheetDialog::getPageInfo( LPCTSTR cTitle)
{
	int ind = aPageList.find( (void*)cTitle, findDialogViaName );
	return ind==-1 ? NULL : aPageList[ind];		
};

static BOOL findDialogViaID( void *id, sSheetInfo **lst )		  { return (*lst)->getDialogID() == (UINT)id; } 
sSheetInfo *sSheetDialog::getPageInfo( UINT dlgId )
{
	int ind = aPageList.find( (void*)dlgId, findDialogViaID );
	return ind==-1 ? NULL : aPageList[ind];
}

void sSheetDialog :: sendMsgToAllPages( UINT message, WPARAM wParam, LPARAM lParam )
{
	bInProtectMode = TRUE;
	for( int i=0; i<aPageList.count(); i++ )
	{
		sSheetInfo *page = aPageList[i];
		CDialog    *dlg  = page->getDialog() ;
		if( dlg  &&  dlg->m_hWnd )
			dlg->SendMessage( message, wParam, lParam );
	}
	bInProtectMode = FALSE;   
};

BOOL sSheetDialog :: sendOkToAllPages( )
{
	CWnd *idOk = GetDlgItem( IDOK );
	LPARAM lparam = LPARAM(idOk ? idOk->m_hWnd : NULL) ;

	BOOL ret = TRUE ;
	bInProtectMode = TRUE;
	for( int i=0; i<aPageList.count(); i++ )
	{
		sSheetInfo *page = aPageList[i];
		sDllDialog *dlg  = page->getDialog() ;
		if( dlg  &&  dlg->m_hWnd )
			if( !dlg->SendMessage( WM_COMMAND, WPARAM(IDOK), lparam) )
			{
				ret = FALSE ;
				break ;
			}
	}
	bInProtectMode = FALSE;   
	return ret ;
};

void sSheetDialog :: onPageShowed( sSheetInfo *cur, BOOL show )
{
	if( !show || !cur)
		return;

	if( m_eDialogType == PagesDlg )
	{
		CTabCtrl *tab = (CTabCtrl*)m_pMngControl;
		int 	  ind = int(cur->hTreeItem);
		
		if( tab->GetCurSel() != ind )
			tab->SetCurSel( ind );
	}
	else
	if( m_eDialogType == PropertyDlg )
	{
		CTreeCtrl *pTreeCtrl= (CTreeCtrl*)m_pMngControl;
		HTREEITEM  ind = pTreeCtrl->GetSelectedItem( );
		
		if( cur->hTreeItem != ind )
			pTreeCtrl->SelectItem( cur->hTreeItem );
	}
};

BOOL sSheetDialog :: showPage( sSheetInfo *page, BOOL swShow, BOOL callOnPageShowed )
{
	if( bInProtectMode )
		return TRUE;

	CDialog	*pPageDlg	= page->getDialog();
	int		 iPageIndex = findPageInfo( page );

	if( swShow )
	{
		sSheetInfo *curPage	= getShownPageInfo();
		
		// Hide last active page
		if( curPage )
			showPage( curPage, FALSE, callOnPageShowed );

		// Show new page page
		if( pPageDlg )
		{
			if(!pPageDlg->m_hWnd && !createPage(page) )
				return FALSE;
			pPageDlg->ShowWindow( SW_SHOW );
		}

		if( callOnPageShowed )
			onPageShowed( page, TRUE );
		iActivePage = iPageIndex;
	}
	else
	{
		// Hide page
		if( pPageDlg && pPageDlg->m_hWnd )
			pPageDlg->ShowWindow( SW_HIDE );
		iActivePage = -1;
	}
	return TRUE;
};

void sSheetDialog :: destroyPageDlg( sSheetInfo *pInfo )
{
	CWnd		*pCtrl = m_pMngControl;
	sDllDialog	*pDlg  = pInfo->getDialog();

	// delete items from controls
	if( pCtrl && pCtrl->m_hWnd )
	{
		if( m_eDialogType == PropertyDlg )
			((CTreeCtrl*)pCtrl)->DeleteItem( pInfo->hTreeItem );
		else
		if( m_eDialogType == PagesDlg )
		{
			int	iPos= int(pInfo->hTreeItem);

			((CTabCtrl*)pCtrl)->DeleteItem( iPos );
		}
	}

	// delete page if was created
	if( pDlg && pDlg->m_hWnd )
		pDlg->DestroyWindow();
}

BOOL sSheetDialog :: delAllSubpages( sSheetInfo *pInfo )
{
	sSheetInfoPtrArray &aChildPages = pInfo->aChildPages;

	for( int i=aChildPages.count()-1; i>=0; i-- )
	{
		sSheetInfo *pInfo = aChildPages[i];
		int			pPos  = findPageInfo( pInfo );

		// recursivelly delete pages
		delAllSubpages( pInfo );
		destroyPageDlg( pInfo );
		aPageList.del( pPos );
		if( pPos == iActivePage )
			iActivePage = -1;
	}
	aChildPages.clearList();
	return TRUE;
};

BOOL sSheetDialog :: delPage( sSheetInfo *pInfo )
{
	int	pPos, iOldActivePage = iActivePage;

	// delete all subpages
	if( pInfo==NULL || (pPos = findPageInfo( pInfo )) < 0 || !delAllSubpages( pInfo ) )
		return FALSE;

	sSheetInfo *pPar = pInfo->getParent();

	// destroy control
	destroyPageDlg( pInfo );

	// delete from parents list
	if( pPar )
	{
		for( int i=pPar->aChildPages.count()-1; i>=0; i-- )
			if( pPar->aChildPages[i] == pInfo )
			{
				pPar->aChildPages.del( i );
				break;
			}
	}
		
	aPageList.del( pPos );
	if( pPos == iActivePage )
		iActivePage = -1;


	// make new selection if selected page was deleted
	if( iActivePage==-1 )
	{
		if(!pPar )
		{
			if( iOldActivePage >= aPageList.count() )
				iOldActivePage  = aPageList.count();
			if( iOldActivePage >= 0 )
				pPar = aPageList[iOldActivePage];
		}
		if( pPar )
			showPage( pPar );
	}
	return TRUE;
};
		
sSheetInfo &sSheetDialog :: addSubpage( sSheetInfo *pParent, sDllDialog *page, LPCTSTR cTitle, long lValue, UINT imageId )
{
	sSheetInfo	*pPageInfo = new sSheetInfo( page, cTitle, imageId );
	
	// add sheet info to array(s)
	pPageInfo->lValue	= lValue;
	aPageList.add( pPageInfo ); 
	if( pParent!=NULL )
		pParent->addNewSheet( *pPageInfo );

	// create pPageInfo if dialog is created
	createMngItem( pPageInfo );
	return *pPageInfo;
};		  

BOOL findDialogID( void *uID, sSheetInfo **lst )	 { return (*lst)->getDialogID() == (UINT)uID ? TRUE : FALSE; } 
BOOL sSheetDialog :: showPage( UINT id )
{
	int iIndex = aPageList.find( (void*)id, findDialogID );
	return iIndex>=0 ? showPage( aPageList[iIndex], TRUE, TRUE ) : FALSE;
}

void sSheetDialog::onPageChanged( int oldPage, int newPage )
{
	sSheetInfo	*oldDlg = oldPage>=0 && oldPage<aPageList.count() ? aPageList[oldPage] : NULL,
				*newDlg = newPage>=0 && newPage<aPageList.count() ? aPageList[newPage] : NULL;
	if( oldDlg || newDlg )
		onPageChanged( oldDlg, newDlg );
}

void sSheetDialog::onPageChanged( sSheetInfo *oldP, sSheetInfo *newP )
{
	if( newP )
		showPage( newP, TRUE, FALSE );
	else
	if( oldP )
		showPage( oldP, FALSE,FALSE );
}

void sSheetDialog :: enableNextPrev()
{
	if( uNextButtonId == 0	||	uPrevButtonId == 0 )
		return ;
	CWnd	 *next = GetDlgItem( uNextButtonId );
	CWnd	 *prev = GetDlgItem( uPrevButtonId );
	int nextEnable = iActivePage < aPageList.count()-1	? TRUE : FALSE;
	int prevEnable = iActivePage > 0				? TRUE : FALSE;

	if( next )
		next->EnableWindow( nextEnable );
	if( prev )
		prev->EnableWindow( prevEnable );
}

BOOL sSheetDialog :: showNextPage()
{
	if( iActivePage >= aPageList.count()-1 )
		return FALSE;
 
	if( isPageChangingAllowed( TRUE ) )
	{
		onPageChanged( iActivePage, iActivePage+1 );
		enableNextPrev();
	}
	else
		return FALSE; 
	return TRUE;
}

BOOL sSheetDialog :: showPrevPage()
{
	if( iActivePage <= 0 )
		return FALSE;
	
	if( isPageChangingAllowed( FALSE ) )
	{
		onPageChanged( iActivePage, iActivePage-1 );
		enableNextPrev();
	}
	else
		return FALSE; 
	return TRUE;
}

BOOL sSheetDialog :: setPageLabel( sSheetInfo *pInfo, LPCTSTR newLabel )
{
	if( pInfo == NULL )
		return FALSE ;

	pInfo->cTitle = newLabel;
	if( m_eDialogType == PropertyDlg )
	{
		CTreeCtrl *ctrl = (CTreeCtrl *)m_pMngControl ;
		if( ctrl && ctrl->m_hWnd )
			return ctrl->SetItem( pInfo->hTreeItem, TVIF_TEXT , newLabel, 0,0,0,0,0 );
	}
	else
	if( m_eDialogType == PagesDlg )
	{
		CTabCtrl *ctrl = (CTabCtrl *)m_pMngControl ;

		if( ctrl && ctrl->m_hWnd )
		{
			TCITEM	info;
			int		iPos = int(pInfo->hTreeItem);

			info.mask	 = TCIF_TEXT;
			info.pszText =(LPTSTR)newLabel ;
			return ctrl->SetItem( iPos, &info );
		}
	}
	else
		return TRUE;
	return FALSE ;
}

void sSheetDialog :: expandAll( BOOL expand )
{
	if( m_eDialogType != PropertyDlg || !m_pMngControl || !m_pMngControl->m_hWnd )
		return;

	CTreeCtrl  *pTreeCtrl= (CTreeCtrl*)m_pMngControl;
	UINT		uExpand  = expand ? TVE_EXPAND : TVE_COLLAPSE;	

	for( int i=0; i<aPageList.count(); i++ )
	{
		HTREEITEM hItem = aPageList[i]->hTreeItem;

		if( hItem && !(pTreeCtrl->GetItemState( hItem, uExpand)&uExpand) )
			pTreeCtrl->Expand( hItem, uExpand );
	}
};

/********************** sSheetDialog::save ****************************/
static BOOL findImageID( void *ID, UINT *id )	 { return (( *((UINT*)ID) == *id )	? 1 : 0); }; 
int sSheetDialog::findImageByID( UINT uImageID )
{
	int index = aImageIDList.find( (void*)&uImageID, findImageID );
	  
	if( uImageID && index == -1 )
	{
		CBitmap bitmap	;
		
		if( bitmap.LoadBitmap( uImageID ) )
		{
			BITMAP	bmp;

			bitmap.GetBitmap( &bmp );
			if( m_ImagesList.m_hImageList == NULL )
			{
				// create image list
				m_ImagesList.Create( bmp.bmWidth, bmp.bmHeight, ILC_MASK, 2, 5 );

				// set image list to Tab or Tree Ctrl
				if( m_eDialogType==PagesDlg )
				{
					m_pMngControl->ModifyStyle( 0, TCS_FORCEICONLEFT );
					((CTabCtrl*)m_pMngControl)->SetImageList( &m_ImagesList );
				}
				else
				if( m_eDialogType==PropertyDlg )
				{
					CTreeCtrl *pTreeCtrl= (CTreeCtrl*)m_pMngControl;

					pTreeCtrl->SetImageList( &m_ImagesList, TVSIL_NORMAL );		
					pTreeCtrl->SetImageList( &m_ImagesList, TVSIL_STATE  );	
				}
			}

			index		= m_ImagesList.Add( &bitmap, (COLORREF)0xFFFFFF );
			aImageIDList.add( uImageID );
			bitmap.DeleteObject ();
		}
	}
	return index;
};	   
