// ValueList.cpp : implementation file
//

#include "stdafx.h"
#include "TableHolders.h"
#include "ValueList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ValueList

IMPLEMENT_DYNCREATE(ValueList, CListView)

ValueList::ValueList()
{
	m_pHolder  = NULL ;
	_bViewMode = FALSE ;
}

ValueList::~ValueList()
{
}

void ValueList::Refresh(BaseHolder *holder)
{
	m_pHolder = holder ;

	int nItems = holder?holder->getItemNumber():0 ;

	GetListCtrl().DeleteAllItems() ;

	for( int i = 0; i < nItems; i++ )
	{
		LVITEM item ;
	    item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = 0 ;
		item.state = 0 ;
		item.stateMask = 0;
		item.pszText = LPSTR_TEXTCALLBACK ;
		item.cchTextMax = 255 ;
		item.iImage = -1 ;
		item.lParam = 0 ;
		GetListCtrl().InsertItem(&item) ;
	}
}

BEGIN_MESSAGE_MAP(ValueList, CListView)
	//{{AFX_MSG_MAP(ValueList)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ValueList message handlers

BOOL ValueList::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style  &= ~LVS_TYPEMASK ;
    cs.style |= LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL ;
	cs.dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_TRACKSELECT ;

	return CListView::PreCreateWindow(cs);
}

int ValueList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	GetListCtrl().InsertColumn(0, "Item", LVCFMT_LEFT, 200) ;
	GetListCtrl().InsertColumn(1, "Value", LVCFMT_LEFT, 300) ;
	
	return 0;
}

void ValueList::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LVITEM* item = &pDispInfo->item ;

    if (item->mask & LVIF_TEXT)
	{
		int itemIndex = item->iItem ;
		char txt[512] ;
		txt[0] = '\x0' ;

		if (m_pHolder)
		{
			if (item->iSubItem==1)
    			m_pHolder->getValueText(itemIndex, txt) ;
			else
    			m_pHolder->getItemName(itemIndex, txt) ;
		}

        ::lstrcpy( item->pszText, (LPCTSTR)txt );
	}
	
	*pResult = 0;
}

void ValueList::OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLISTVIEW *pNMLV = (NMLISTVIEW*)pNMHDR ;
	int selItem = pNMLV->iItem ;

	if (!_bViewMode && m_pHolder && m_pHolder->editItem(selItem) )
		Invalidate() ;

	*pResult = 0;
}

