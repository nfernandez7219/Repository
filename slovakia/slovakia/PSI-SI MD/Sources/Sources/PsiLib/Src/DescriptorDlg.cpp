// DescriptorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tables.h"
#include "DescriptorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DescriptorDlg dialog


DescriptorDlg::DescriptorDlg( int numDesc, uchar *descIds, CWnd* pParent /*=NULL*/)
	: CDialog(DescriptorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(DescriptorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	_numDesc = numDesc;
	_descIds = descIds ;
	_descDlg = NULL ;
}


void DescriptorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DescriptorDlg)
	DDX_Control(pDX, IDC_LST_DescrType, _descList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DescriptorDlg, CDialog)
	//{{AFX_MSG_MAP(DescriptorDlg)
	ON_LBN_SELCHANGE(IDC_LST_DescrType, OnDescrTypeChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DescriptorDlg message handlers

BOOL DescriptorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	for ( int i = 0; i < _numDesc; i++ )
	{
		uchar descType = _descIds[i] ;
		const char* descName = descriptorName(descType) ;
		int index = _descList.AddString( descName ) ;
		_descList.SetItemData(index, descType) ;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DescriptorDlg::OnDescrTypeChanged() 
{
	if (_descDlg)
	{
		_descDlg->DestroyWindow() ;
		//delete _descDlg ;
		_descDlg = NULL ;
	}

	int index = _descList.GetCurSel() ;
	if ( index == LB_ERR )
		return ;

	UINT selDescType = _descList.GetItemData(index) ;
	UINT descDlgId = GetDescDlgID((uchar)selDescType) ;
	
	if (descDlgId<=0)
		descDlgId = IDD_Desc_unsupported ;

	_descDlg = new CDialog() ;
	if ( _descDlg->Create( descDlgId ,this ) )
	{
		CRect r ;
		_descList.GetClientRect(&r) ;
		_descDlg->SetWindowPos(&_descList, r.right+20, r.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW ) ;
	}
	else
	{
		delete _descDlg ;
		_descDlg = NULL ;
	}
}

#define CASE_STATEMENT(desc)	case TAG_##desc: return IDD_Desc_##desc ;

UINT DescriptorDlg::GetDescDlgID( uchar descTag )
{
	switch (descTag)
	{
/*		CASE_STATEMENT(network_name_descriptor				)
		CASE_STATEMENT(service_list_descriptor 				)
		CASE_STATEMENT(stuffing_descriptor 					)
		CASE_STATEMENT(satellite_delivery_system_descriptor	)
		CASE_STATEMENT(cable_delivery_system_descriptor 	)
		CASE_STATEMENT(bouquet_name_descriptor				)	
		CASE_STATEMENT(service_descriptor 					)	
		CASE_STATEMENT(country_availability_descriptor 		)	
		CASE_STATEMENT(linkage_descriptor 					)	
		CASE_STATEMENT(NVOD_reference_descriptor 			)	
		CASE_STATEMENT(time_shifted_service_descriptor 		)	
		CASE_STATEMENT(short_event_descriptor 				)	
		CASE_STATEMENT(extended_event_descriptor 			)	
		CASE_STATEMENT(time_shifted_event_descriptor 		)	
		CASE_STATEMENT(component_descriptor 				)
		CASE_STATEMENT(mosaic_descriptor					)
		CASE_STATEMENT(stream_identifier_descriptor 		)
		CASE_STATEMENT(CA_identifier_descriptor 			)
		CASE_STATEMENT(content_descriptor 					)	
		CASE_STATEMENT(parental_rating_descriptor 			)	
		CASE_STATEMENT(teletext_descriptor 					)	
		CASE_STATEMENT(telephone_descriptor 				)
		CASE_STATEMENT(local_time_offset_descriptor 		)
		CASE_STATEMENT(subtitling_descriptor 				)	
		CASE_STATEMENT(terrestrial_delivery_system_descriptor)	
		CASE_STATEMENT(multilingual_network_name_descriptor )
		CASE_STATEMENT(multilingual_bouquet_name_descriptor )
		CASE_STATEMENT(multilingual_service_name_descriptor )
		CASE_STATEMENT(multilingual_component_descriptor	)
		CASE_STATEMENT(private_data_specifier_descriptor 	)	
		CASE_STATEMENT(service_move_descriptor 				)	
		CASE_STATEMENT(short_smoothing_buffer_descriptor	)
		CASE_STATEMENT(frequency_list_descriptor 			)	
		CASE_STATEMENT(partial_transport_stream_descriptor	)	
		CASE_STATEMENT(data_broadcast_descriptor 			)	
		CASE_STATEMENT(CA_system_descriptor		 			)	
*/		CASE_STATEMENT(data_broadcast_id_descriptor			)	
	}

	return 0 ;
}
