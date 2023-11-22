// NitSdtDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tables.h"
#include "NitSdtDlg.h"
#include "ServiceDlg.h"
#include "DescriptorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// NitSdtDlg dialog


NitSdtDlg::NitSdtDlg(CWnd* pParent /*=NULL*/)
	: CDialog(NitSdtDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(NitSdtDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void NitSdtDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NitSdtDlg)
	DDX_Control(pDX, IDC_LST_TransportStreams, m_StreamList);
	DDX_Control(pDX, IDC_LST_Services, m_ServiceList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NitSdtDlg, CDialog)
	//{{AFX_MSG_MAP(NitSdtDlg)
	ON_BN_CLICKED(IDC_BTN_AddService, OnAddService)
	ON_BN_CLICKED(IDC_BTN_AddNtwDesc, OnAddNtwDesc)
	ON_BN_CLICKED(IDC_BTN_AddTSDesc, OnAddTSDesc)
	ON_BN_CLICKED(IDC_BTN_AddServDesc, OnAddServDesc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NitSdtDlg message handlers

BOOL NitSdtDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_StreamList.InsertColumn(0, "TS ID", LVCFMT_LEFT,60);
	m_StreamList.InsertColumn(1, "Orig. network ID", LVCFMT_LEFT,80);
	
	m_ServiceList.InsertColumn(0, "Serv. ID", LVCFMT_LEFT, 52);
	m_ServiceList.InsertColumn(1, "Status", LVCFMT_LEFT, 50);
	m_ServiceList.InsertColumn(2, "CA mode", LVCFMT_LEFT, 50);
	m_ServiceList.InsertColumn(3, "Event Info", LVCFMT_LEFT, 50);
	m_ServiceList.InsertColumn(4, "Event schedule", LVCFMT_LEFT, 50);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void NitSdtDlg::OnAddService() 
{
	ServiceDlg dlg(this) ;
	dlg.DoModal() ;
}

void NitSdtDlg::OnAddNtwDesc() 
{
	DECL_DESC_ARR() ;
	ADD_DESC_TO_ARR(TAG_network_name_descriptor) ;
	ADD_DESC_TO_ARR(TAG_multilingual_network_name_descriptor) ;
	ADD_DESC_TO_ARR(TAG_linkage_descriptor) ;

	ADD_DESC_TO_ARR(TAG_private_data_specifier_descriptor) ;

	DescriptorDlg dlg(DESC_ARR_LENGTH(), DESC_ARRAY(), this) ;
	dlg.DoModal() ;
}

void NitSdtDlg::OnAddTSDesc() 
{
	DECL_DESC_ARR() ;
	ADD_DESC_TO_ARR(TAG_satellite_delivery_system_descriptor) ;
	ADD_DESC_TO_ARR(TAG_cable_delivery_system_descriptor) ;
	ADD_DESC_TO_ARR(TAG_terrestrial_delivery_system_descriptor) ;
	ADD_DESC_TO_ARR(TAG_service_list_descriptor) ;
	ADD_DESC_TO_ARR(TAG_frequency_list_descriptor) ;

	ADD_DESC_TO_ARR(TAG_private_data_specifier_descriptor) ;

	DescriptorDlg dlg(DESC_ARR_LENGTH(), DESC_ARRAY(), this) ;
	dlg.DoModal() ;
}

void NitSdtDlg::OnAddServDesc() 
{
	DECL_DESC_ARR() ;

	ADD_DESC_TO_ARR(TAG_bouquet_name_descriptor				)
	ADD_DESC_TO_ARR(TAG_CA_identifier_descriptor) ;
	ADD_DESC_TO_ARR(TAG_country_availability_descriptor 	)
	ADD_DESC_TO_ARR(TAG_data_broadcast_descriptor) ;
	ADD_DESC_TO_ARR(TAG_linkage_descriptor 					)
	ADD_DESC_TO_ARR(TAG_mosaic_descriptor) ;
	ADD_DESC_TO_ARR(TAG_multilingual_service_name_descriptor) ;
	ADD_DESC_TO_ARR(TAG_NVOD_reference_descriptor 			)
	ADD_DESC_TO_ARR(TAG_service_descriptor 					)
	ADD_DESC_TO_ARR(TAG_telephone_descriptor) ;
	ADD_DESC_TO_ARR(TAG_time_shifted_service_descriptor 	)

	ADD_DESC_TO_ARR(TAG_private_data_specifier_descriptor) ;

	DescriptorDlg dlg(DESC_ARR_LENGTH(), DESC_ARRAY(), this) ;
	dlg.DoModal() ;
}
