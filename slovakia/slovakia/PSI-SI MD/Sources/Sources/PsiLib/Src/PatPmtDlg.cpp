// PatPmtDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tables.h"
#include "PatPmtDlg.h"
#include "DescriptorDlg.h"
#include "ProgramPidDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PatPmtDlg dialog


PatPmtDlg::PatPmtDlg(CWnd* pParent /*=NULL*/)
	: CDialog(PatPmtDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(PatPmtDlg)
	m_ProgramMapping = FALSE;
	//}}AFX_DATA_INIT
}


void PatPmtDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PatPmtDlg)
	DDX_Control(pDX, IDC_LST_Streams, m_StreamList);
	DDX_Control(pDX, IDC_LST_Programs, m_ProgramList);
	DDX_Check(pDX, IDC_CHK_ProgramMapping, m_ProgramMapping);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PatPmtDlg, CDialog)
	//{{AFX_MSG_MAP(PatPmtDlg)
	ON_BN_CLICKED(IDC_BTN_AddDesc, OnAddNetworkDesc)
	ON_BN_CLICKED(IDC_CHK_ProgramMapping, OnProgramMapping)
	ON_BN_CLICKED(IDC_BTN_AddStreamDesc, OnAddStreamDesc)
	ON_BN_CLICKED(IDC_BTN_AddProgram, OnAddProgram)
	ON_CBN_SELCHANGE(IDC_CB_StreamType, OnSelchangeCBStreamType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PatPmtDlg message handlers


BOOL PatPmtDlg::OnInitDialog() 
{
	m_ProgramMapping = TRUE ;

	CDialog::OnInitDialog();

	m_ProgramList.InsertColumn(0, "Prog. Number", LVCFMT_LEFT, 75);
	m_ProgramList.InsertColumn(1, " PID", LVCFMT_LEFT, 45);
	
	m_StreamList.InsertColumn(0, "PID", LVCFMT_LEFT, 50);
	m_StreamList.InsertColumn(1, "Type", LVCFMT_LEFT, 100);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void PatPmtDlg::OnProgramMapping() 
{
	UpdateData(TRUE) ;

	GetDlgItem(IDC_LST_Descriptors)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_EDT_PcrPid)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_BTN_AddDesc)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_BTN_DelDesc)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_LST_Streams)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_LST_StreamDescr)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_EDT_StreamPid)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_CB_StreamType)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_BTN_AddStream)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_BTN_DelStream)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_BTN_AddStreamDesc)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_BTN_DelStreamDesc)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_RDB_PcrPidHex)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_RDB_PcrPidDec)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_RDB_ElemPidHex)->EnableWindow(m_ProgramMapping) ;
	GetDlgItem(IDC_RDB_ElemPidDec)->EnableWindow(m_ProgramMapping) ;

}

void PatPmtDlg::OnAddNetworkDesc() 
{
	DECL_DESC_ARR() ;

	ADD_DESC_TO_ARR(TAG_mosaic_descriptor) ;
	ADD_DESC_TO_ARR(TAG_stream_identifier_descriptor) ;
	ADD_DESC_TO_ARR(TAG_teletext_descriptor) ;
	ADD_DESC_TO_ARR(TAG_subtitling_descriptor) ;
	ADD_DESC_TO_ARR(TAG_private_data_specifier_descriptor) ;
	ADD_DESC_TO_ARR(TAG_service_move_descriptor) ;
	ADD_DESC_TO_ARR(TAG_CA_descriptor) ;
	ADD_DESC_TO_ARR(TAG_data_broadcast_id_descriptor) ;

	DescriptorDlg dlg(DESC_ARR_LENGTH(), DESC_ARRAY(), this) ;
	dlg.DoModal() ;
}

void PatPmtDlg::OnAddStreamDesc() 
{
	DECL_DESC_ARR() ;

	ADD_DESC_TO_ARR(TAG_mosaic_descriptor) ;
	ADD_DESC_TO_ARR(TAG_service_move_descriptor) ;
	ADD_DESC_TO_ARR(TAG_stream_identifier_descriptor) ;

	ADD_DESC_TO_ARR(TAG_teletext_descriptor) ;
	ADD_DESC_TO_ARR(TAG_subtitling_descriptor) ;
	ADD_DESC_TO_ARR(TAG_private_data_specifier_descriptor) ;
	ADD_DESC_TO_ARR(TAG_CA_descriptor) ;
	ADD_DESC_TO_ARR(TAG_data_broadcast_id_descriptor) ;

	DescriptorDlg dlg(DESC_ARR_LENGTH(), DESC_ARRAY(), this) ;
	dlg.DoModal() ;
}

void PatPmtDlg::OnAddProgram() 
{
	ProgramPidDlg dlg(this) ;
	dlg.DoModal() ;
}

void PatPmtDlg::OnSelchangeCBStreamType() 
{
	// TODO: Add your control notification handler code here
	
}
