// TableTree.cpp : implementation file
//

#include "stdafx.h"
#include "TableHolders.h"
#include "TableTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TableTree

IMPLEMENT_DYNCREATE(TableTree, CTreeView)

TableTree::TableTree()
{
	_TableHolder = NULL ; 
}

TableTree::~TableTree()
{
}

void TableTree::CreateTreeItems( TableHolder *tableHolder, BOOL bViewMode )
{
	_bViewMode   = bViewMode ;
	_TableHolder = tableHolder ;

	GetTreeCtrl().DeleteAllItems( ) ;

	CreateSubtreeNIT(&tableHolder->_nitHolder) ;
	CreateSubtreePAT(&tableHolder->_patHolder, &tableHolder->_pmtHolders) ;
	CreateSubtreeSDT(&tableHolder->_sdtHolder) ;
	CreateSubtreeCAT(&tableHolder->_catHolder ) ;

}

void TableTree::RefreshItem( )
{
	HTREEITEM hItem = GetTreeCtrl().GetSelectedItem() ;
	BaseHolder *holder = (BaseHolder*)GetTreeCtrl().GetItemData(hItem) ;
	if (holder==NULL)
	{
		hItem = GetTreeCtrl().GetParentItem(hItem) ;
		holder = (BaseHolder*)GetTreeCtrl().GetItemData(hItem) ;
	}
	if (holder==NULL)
		return ;

	switch (holder->getHolderType())
	{
		case BaseHolder::PMT_STREAM_HOLDER:
		{
			PmtStreamHolder *streamHolder = (PmtStreamHolder*)holder ;
			char streamTitle[50] ;
			sprintf(streamTitle, "PID 0x%x", streamHolder->_ElementaryPID) ;
			strupr(streamTitle+6);
			GetTreeCtrl().SetItemText(hItem, streamTitle) ;
		}
		case TAG_CA_descriptor:
		{
			CASystemDescHolder* desc = (CASystemDescHolder*)holder ;
			
			HTREEITEM parentItem = GetTreeCtrl().GetParentItem(hItem) ;
			BaseHolder *parentHolder = (BaseHolder*)GetTreeCtrl().GetItemData(parentItem) ;
			if (parentHolder && parentHolder->getHolderType()==BaseHolder::CAT_HOLDER)
			{
				char streamTitle[500] ;
				desc->getTextDescription(streamTitle) ;
				GetTreeCtrl().SetItemText(hItem, streamTitle) ;
			}
		}
	} ;
}

HTREEITEM TableTree::InsertItem( const char *name, BaseHolder *itemHolder, HTREEITEM parent, BOOL bold )
{
	HTREEITEM treeItem = GetTreeCtrl().InsertItem(name, parent) ;
	GetTreeCtrl().SetItemData(treeItem, (DWORD)itemHolder) ;
	if (itemHolder)
		itemHolder->_treeItem = treeItem ;

	if (bold)
		GetTreeCtrl().SetItemState(treeItem,TVIS_BOLD,TVIS_BOLD);

	return treeItem ;
}

void TableTree::CreateSubtreeNIT( NIT_Holder *nitHolder )
{
	if (_bViewMode && _TableHolder->_nitFreq==0)
		return ;

	HTREEITEM netInfo = InsertItem("NIT (Network Information Table)", nitHolder) ;

	HTREEITEM netID	  = InsertItem("Network ID", NULL, netInfo) ;

	DECL_DESC_ARR() ;
	ADD_DESC_TO_ARR(TAG_network_name_descriptor) ;
	ADD_DESC_TO_ARR(TAG_multilingual_network_name_descriptor) ;
	ADD_DESC_TO_ARR(TAG_linkage_descriptor 					);
	ADD_DESC_TO_ARR(TAG_private_data_specifier_descriptor) ;

	CreateDescriptorSubtree(&nitHolder->_descriptors, DESC_ARR_LENGTH(), DESC_ARRAY(), netInfo) ;

	HTREEITEM nitTs   = InsertItem("Transport Streams", NULL, netInfo) ;

	NitTSHolderArray *tsArr = &nitHolder->_transpStreams ;
	for ( int i = 0; i < tsArr->count(); i++ )
	{
		NitTransportStreamHolder *tsHolder = tsArr->item(i) ;
		char tsTitle[50] ;
		sprintf(tsTitle, "TS %d", tsHolder->_transportStreamId ) ;
		
		HTREEITEM tsItem = InsertItem(tsTitle, tsHolder, nitTs) ;
		
		InsertItem("Original network ID", NULL, tsItem) ;

		DESC_ARR_LENGTH() = 0 ;
		ADD_DESC_TO_ARR(TAG_satellite_delivery_system_descriptor) ;
//		ADD_DESC_TO_ARR(TAG_cable_delivery_system_descriptor) ;
//		ADD_DESC_TO_ARR(TAG_terrestrial_delivery_system_descriptor) ;
//		ADD_DESC_TO_ARR(TAG_service_list_descriptor) ;
//		ADD_DESC_TO_ARR(TAG_frequency_list_descriptor) ;
		ADD_DESC_TO_ARR(TAG_private_data_specifier_descriptor) ;

		CreateDescriptorSubtree(&tsHolder->_descriptors, DESC_ARR_LENGTH(), DESC_ARRAY(), tsItem) ;
	}
}

void TableTree::CreateSubtreePAT( PAT_Holder *patHolder, PMT_HolderArray *pmtHolders )
{
	HTREEITEM patInfo = TVI_ROOT ;

	if (!_bViewMode || _TableHolder->_patFreq!=0)
	{
		patInfo = InsertItem("PAT (Program Association Table)", patHolder) ;
	
		InsertItem("Network Information PID", NULL, patInfo) ;
	}

	if (!_bViewMode || _TableHolder->_pmtFreq!=0)
	{
		for ( int i = 0; i < pmtHolders->count(); i++ )
		{
			PMT_Holder *pmtHolder = pmtHolders->item(i) ;
			CreateSubtreePMT( pmtHolder, patInfo ) ;
		}
	}
}

void TableTree::CreateSubtreePMT( PMT_Holder *pmtHolder, HTREEITEM parent )
{
	char prgTitle[50] ;
	sprintf(prgTitle, "Program %d PMT (Program Map Table)", pmtHolder->_ProgramNumber) ;

	HTREEITEM pmtItem = InsertItem(prgTitle, pmtHolder, parent) ;

	if (!_bViewMode || pmtHolder->getPcrPid()!=0x1fff)
		InsertItem("Program Clock Reference PID", NULL, pmtItem) ;
	
	DECL_DESC_ARR() ;
	ADD_DESC_TO_ARR(TAG_CA_descriptor) ;
	ADD_DESC_TO_ARR(TAG_private_data_specifier_descriptor) ;

	CreateDescriptorSubtree(&pmtHolder->_descriptors, DESC_ARR_LENGTH(), DESC_ARRAY(), pmtItem) ;

	BaseHolder *strHolder = _TableHolder->CreateSubstHolder(BaseHolder::PMT_STREAMS_ROOT) ;
	HTREEITEM streams = InsertItem("Streams", strHolder, pmtItem) ;

	PmtStreamHolderArray *streamArr = &pmtHolder->_streams ;
	for ( int i = 0; i < streamArr->count(); i++ )
	{
		PmtStreamHolder *stream = streamArr->item(i) ;
		CreateProgramStreamSubtree(stream, streams) ;
	}
}

HTREEITEM TableTree::CreateProgramStreamSubtree( PmtStreamHolder *streamHolder, HTREEITEM parent )
{
	char streamTitle[50] ;
	sprintf(streamTitle, "PID 0x%x", streamHolder->_ElementaryPID) ;
	strupr(streamTitle+6);

	HTREEITEM streamItem = InsertItem(streamTitle, streamHolder, parent) ;

	InsertItem("Stream Type", NULL, streamItem) ;
	InsertItem("Elementary PID", NULL, streamItem) ;

	DECL_DESC_ARR() ;

	ADD_DESC_TO_ARR(TAG_video_stream_descriptor) ;
	ADD_DESC_TO_ARR(TAG_audio_stream_descriptor) ;
//	ADD_DESC_TO_ARR(TAG_mosaic_descriptor) ;
//	ADD_DESC_TO_ARR(TAG_service_move_descriptor) ;
	ADD_DESC_TO_ARR(TAG_CA_descriptor) ;
	ADD_DESC_TO_ARR(TAG_data_broadcast_id_descriptor) ;
	ADD_DESC_TO_ARR(TAG_ISO_639_language_descriptor) ;
	ADD_DESC_TO_ARR(TAG_stream_identifier_descriptor) ;
	ADD_DESC_TO_ARR(TAG_subtitling_descriptor) ;
	ADD_DESC_TO_ARR(TAG_teletext_descriptor) ;
	ADD_DESC_TO_ARR(TAG_private_data_specifier_descriptor) ;

	CreateDescriptorSubtree(&streamHolder->_descriptors, DESC_ARR_LENGTH(), DESC_ARRAY(), streamItem) ;
	return streamItem ;
}

void TableTree::CreateSubtreeSDT( SDT_Holder *sdtHolder)
{
	if (_bViewMode && _TableHolder->_sdtFreq==0)
		return ;

	HTREEITEM servDesc = InsertItem("SDT (Service Description Table)", sdtHolder) ;

	InsertItem("Transport Stream ID", NULL, servDesc) ;
	InsertItem("Original Network ID", NULL, servDesc) ;
	HTREEITEM servList = InsertItem("Services", NULL, servDesc) ;

	SdtServiceHolderArray *servArr = &sdtHolder->_services ;
	for ( int i = 0; i < servArr->count(); i++ )
	{
		SdtServiceHolder *servHolder = servArr->item(i) ;

		char servTitle[50] ;
		sprintf(servTitle, "Service %d", servHolder->_serviceId ) ;

		HTREEITEM servItem = InsertItem(servTitle, servHolder, servList) ;

		InsertItem("Event Information present", NULL, servItem) ;
		InsertItem("Event Information schedule present", NULL, servItem) ;
		InsertItem("Running Status", NULL, servItem) ;
		InsertItem("CA Mode", NULL, servItem) ;

		DECL_DESC_ARR() ;
//		ADD_DESC_TO_ARR(TAG_bouquet_name_descriptor				);
		ADD_DESC_TO_ARR(TAG_CA_identifier_descriptor			);
//		ADD_DESC_TO_ARR(TAG_copyright_descriptor				);
		ADD_DESC_TO_ARR(TAG_country_availability_descriptor 	);
		ADD_DESC_TO_ARR(TAG_data_broadcast_descriptor			);
		ADD_DESC_TO_ARR(TAG_ISO_639_language_descriptor			);
		ADD_DESC_TO_ARR(TAG_linkage_descriptor 					);
//		ADD_DESC_TO_ARR(TAG_mosaic_descriptor					);
		ADD_DESC_TO_ARR(TAG_multilingual_service_name_descriptor);
//		ADD_DESC_TO_ARR(TAG_NVOD_reference_descriptor 			);
//		ADD_DESC_TO_ARR(TAG_registration_descriptor				);
		ADD_DESC_TO_ARR(TAG_service_descriptor 					);
//		ADD_DESC_TO_ARR(TAG_telephone_descriptor				);
//		ADD_DESC_TO_ARR(TAG_time_shifted_service_descriptor 	);
		ADD_DESC_TO_ARR(TAG_private_data_specifier_descriptor	);

		CreateDescriptorSubtree(&servHolder->_descriptors, DESC_ARR_LENGTH(), DESC_ARRAY(), servItem) ;
	}
}

void TableTree::CreateSubtreeCAT( CAT_Holder *catHolder)
{
	if (_bViewMode && _TableHolder->_catFreq==0)
		return ;

	HTREEITEM catItem = InsertItem("CAT (Conditional Access Table)", catHolder) ;

	for ( int i = 0; i < catHolder->getItemNumber(); i++ )
	{
		CASystemDescHolder* caDescHolder = catHolder->getItem(i) ;
		char title[500] ;
		caDescHolder->getTextDescription(title) ;
		HTREEITEM servItem = InsertItem(title, caDescHolder, catItem) ;
	}
}

void TableTree::CreateDescriptorSubtree( DescriptorArray *descArr, int nDesc, uchar *descIds, HTREEITEM parent )
{
	BOOL bHasDescr = (descArr->count()>0) ;
	if (_bViewMode && !bHasDescr )
		return ;

	HTREEITEM descRoot = InsertItem("Descriptors", NULL, parent, bHasDescr ) ;

	for ( int i = 0; i < nDesc; i++ )
	{
		uchar descTag = descIds[i] ;
		BaseHolder *descHolder = descArr->findHolder(descTag);
		if (descHolder==NULL)
		{
			if (_bViewMode)
				continue ;
			descHolder = _TableHolder->CreateSubstHolder(descTag) ;
		}

		HTREEITEM descItem = InsertItem(descriptorName(descTag), descHolder, descRoot, !(descHolder->isSubstHolder()) ) ;
	}
}

BEGIN_MESSAGE_MAP(TableTree, CTreeView)
	//{{AFX_MSG_MAP(TableTree)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelectionChanged)
	ON_WM_LBUTTONDBLCLK()
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeyDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TableTree message handlers

BOOL TableTree::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	dwStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_DISABLEDRAGDROP | TVS_INFOTIP | TVS_LINESATROOT | TVS_SHOWSELALWAYS ;
	
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

#include "resource.h"

void TableTree::OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	NMTREEVIEW* pNMTreeView = (NMTREEVIEW*)pNMHDR;

	TVITEM selItem = pNMTreeView->itemNew ;
	if ( (selItem.state&TVIS_SELECTED)==0 )
		return ;

	LPARAM lParam = selItem.lParam ;
	BaseHolder *holder = (BaseHolder*)lParam ;

	if (lParam==NULL)
	{
		HTREEITEM parent = GetTreeCtrl().GetParentItem(selItem.hItem) ;
		if (parent!=TVI_ROOT)
			lParam = GetTreeCtrl().GetItemData(parent) ;
	}

	AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_RefreshValueList, lParam ) ;
}


void TableTree::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	UINT flags;
	
	HTREEITEM hItem = GetTreeCtrl().HitTest( point, &flags ) ;	
	BaseHolder *holder = (BaseHolder*)GetTreeCtrl().GetItemData(hItem) ;

	if (!_bViewMode && (hItem != NULL) && (TVHT_ONITEM & flags) && (holder!=NULL))
	{
		if (holder->getHolderType()==BaseHolder::PAT_HOLDER)
		{
			AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_AddProgram, 0 ) ;
			return ;
		}
		if (holder->getHolderType()==BaseHolder::NIT_HOLDER)
		{
			AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_NewNetwork, 0 ) ;
			return ;
		}
		if (holder->getHolderType()==BaseHolder::CAT_HOLDER)
		{
			CAT_Holder *catHolder = (CAT_Holder*)holder ;
			DescriptorHolder *desc = catHolder->addItem() ;
			if (desc!=NULL)
			{
				HTREEITEM newItem = InsertItem("", desc, hItem) ;
				GetTreeCtrl().SelectItem(newItem) ;
				RefreshItem() ;
			}
			return ;
		}
		else if (holder->isStreamRootHolder())
		{
//			AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_AddProgramStream, 0 ) ;
			HTREEITEM parentItem = GetTreeCtrl().GetParentItem(hItem) ;	
			BaseHolder *parentHolder = (BaseHolder*)GetTreeCtrl().GetItemData(parentItem) ;
			if (parentHolder->getHolderType()!=BaseHolder::PMT_HOLDER)
				return ;

			PMT_Holder *pmtHolder = (PMT_Holder*)parentHolder ;
			PmtStreamHolder *newStream = pmtHolder->addStream() ;
			
			if ( newStream )
			{
				HTREEITEM newItem = CreateProgramStreamSubtree(newStream, hItem) ;
				GetTreeCtrl().SelectItem(newItem) ;
			}

			return ;
		}		
		else if ( holder->isSubstHolder() && holder->isDescHolder() )
		{
			HTREEITEM parentItem = GetTreeCtrl().GetParentItem(hItem) ;	
			BaseHolder *parentHolder = (BaseHolder*)GetTreeCtrl().GetItemData(parentItem) ;
			if (!(parentHolder && parentHolder->isTableHolder()) )
			{
				parentItem = GetTreeCtrl().GetParentItem(parentItem) ;	
				parentHolder = (BaseHolder*)GetTreeCtrl().GetItemData(parentItem) ;

				if (!(parentHolder && parentHolder->isTableHolder()) )
				{
					CTreeView::OnLButtonDblClk(nFlags, point);
					return ;
				}
			}

			BaseHolder *newHolder = _TableHolder->ReplaceSubstHolder(holder, parentHolder) ;
			if (newHolder!=holder)
			{
				GetTreeCtrl().SetItemData(hItem, (DWORD)newHolder) ;
				GetTreeCtrl().SetItemState(hItem, TVIS_BOLD, TVIS_BOLD) ;
				AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_RefreshValueList, (DWORD)newHolder ) ;
			}
		}
	}

	CTreeView::OnLButtonDblClk(nFlags, point);
}

void TableTree::OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;

	if (_bViewMode || pTVKeyDown->wVKey != 46)
		return ;
	
	HTREEITEM hItem = GetTreeCtrl().GetSelectedItem() ;
	BaseHolder *holder = (BaseHolder*)GetTreeCtrl().GetItemData(hItem) ;

	if (holder && !holder->isSubstHolder())
	{
		HTREEITEM parentItem = GetTreeCtrl().GetParentItem(hItem) ;
		if (parentItem==0)
			return ;
		BaseHolder *parentHolder = (BaseHolder*)GetTreeCtrl().GetItemData(parentItem) ;
		if (!(parentHolder && parentHolder->isTableHolder()) )
		{
			parentItem = GetTreeCtrl().GetParentItem(parentItem) ;	
			parentHolder = (BaseHolder*)GetTreeCtrl().GetItemData(parentItem) ;
		}
		if (!(parentHolder && parentHolder->isTableHolder()) )
			return ;

		BOOL bDeletable = !holder->isDescHolder() ;
		int type = holder->getHolderType() ;
		
		if (type==BaseHolder::PMT_HOLDER)
		{
			int prgNum = ((PMT_Holder*)holder)->getProgramNum() ;
			if (_TableHolder->DeleteProgram(prgNum) )
				CreateTreeItems(_TableHolder);
		}
		else if (type==BaseHolder::SDT_SERVICE_HOLDER)
		{
			int prgNum = ((SdtServiceHolder*)holder)->getServiceId() ;
			if (_TableHolder->DeleteProgram(prgNum) )
				CreateTreeItems(_TableHolder);
		}
		else if ( parentHolder->delChild(holder) )
		{
			if (bDeletable)
			{
				AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_RefreshValueList, 0 ) ;
				GetTreeCtrl().DeleteItem(hItem) ;
			}
			else
			{
				DWORD substHolder = (DWORD)_TableHolder->CreateSubstHolder(type) ;
				GetTreeCtrl().SetItemState(hItem, 0, TVIS_BOLD) ;
				GetTreeCtrl().SetItemData(hItem, substHolder);
				AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_RefreshValueList, substHolder ) ;
			}
		}
	}
}
