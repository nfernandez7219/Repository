// AddPrgWizard.cpp : implementation file
//

#include "stdafx.h"
#include "AddPrgWizard.h"
#include "Descriptors.h"
#include "TableHolders.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable:4355)

static void DDX_Hex( CDataExchange* pDX, UINT id, UINT &var, UINT min, UINT max, const char *msg )
{
	CString st ;
	const char *pSt;
	char *end ;
	
	if (!pDX->m_bSaveAndValidate)
		st.Format("0x%x", var) ;

	DDX_Text(pDX, id, st);

	if (pDX->m_bSaveAndValidate)
	{
		pSt = LPCTSTR(st) ;
		var = strtol(pSt, &end, 16) ;
		if (*end!='\x0' || var < min || var > max)
		{
			AfxGetMainWnd()->MessageBox(msg, "Error", MB_OK|MB_ICONERROR) ;
			pDX->Fail() ;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// AddPrgWizardPage1 dialog


AddPrgWizardPage1::AddPrgWizardPage1( AddProgramWizardDlg* pParent )
	: CDialog(AddPrgWizardPage1::IDD, pParent)
{
	//{{AFX_DATA_INIT(AddPrgWizardPage1)
	m_PmtPid = 0 ;
	m_PrgNum = 0 ;
	//}}AFX_DATA_INIT
	
	_pParent = pParent ;
}

BOOL AddPrgWizardPage1::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_lstPrgType.InsertColumn(0, "Program Type", LVCFMT_LEFT, 500) ;
	m_lstPrgType.InsertItem(0,"TV channel") ;
	m_lstPrgType.InsertItem(1,"Radio channel") ;
	m_lstPrgType.InsertItem(2,"Data channel") ;
	m_lstPrgType.InsertItem(3,"Blank program") ;
	
	m_lstPrgType.SetItemState(0, LVNI_SELECTED, LVNI_SELECTED) ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int	AddPrgWizardPage1::GetPrgType( )
{
	return m_lstPrgType.GetNextItem(-1, LVNI_SELECTED);
}

void AddPrgWizardPage1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(AddPrgWizardPage1)
	DDX_Control(pDX, IDC_PRGWIZ_PrgType, m_lstPrgType);
	//}}AFX_DATA_MAP

	char buf[256] ;

	DDX_Hex(pDX, IDC_PRGWIZ_PmtPid, m_PmtPid, 0x20, 0x1FFF, "You must enter hexadecimal integer between 0x20 and 0x1FFF.");
	if (pDX->m_bSaveAndValidate && TableHolder::CheckPidUse(m_PmtPid, buf) )
	{
		char txt[256] ;
		sprintf(txt, 
			"PID 0x%x is already used in :\n\n%s\n\n"
			"Choose another value.",
			m_PmtPid, buf
		) ;
		MessageBox( txt, "Error", MB_OK | MB_ICONERROR ) ;
		pDX->Fail() ;
	}

	DDX_Text(pDX, IDC_PRGWIZ_PrgNum, m_PrgNum);
	DDV_MinMaxUInt(pDX, m_PrgNum, 0, 65535);

	if ( pDX->m_bSaveAndValidate && _pParent->programExists(m_PrgNum) )
	{
		char txt[256] ;
		sprintf(txt, 
			"Program number must be unique.\n"
			"Program with number %d already exists.",
			m_PrgNum
		) ;
		MessageBox( txt, "Error", MB_OK | MB_ICONERROR ) ;
		pDX->Fail() ;
	}
}


BEGIN_MESSAGE_MAP(AddPrgWizardPage1, CDialog)
	//{{AFX_MSG_MAP(AddPrgWizardPage1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AddPrgWizardPage1 message handlers
/////////////////////////////////////////////////////////////////////////////
// AddPrgWizardPage2 dialog


AddPrgWizardPage2::AddPrgWizardPage2(CWnd* pParent /*=NULL*/)
	: CDialog(AddPrgWizardPage2::IDD, pParent)
{
	//{{AFX_DATA_INIT(AddPrgWizardPage2)
	m_ProvName = _T("");
	m_ServName = _T("");
	m_ServType = 0;
	m_ServId = 0;
	//}}AFX_DATA_INIT
}

BOOL AddPrgWizardPage2::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CComboBox *combo = (CComboBox*)GetDlgItem(IDC_PRGWIZ_ServType) ;
	for (int i = 0; i <= 12; i++)
	{
		char buf[128] ;
		ServiceDescriptor::serviceTypeAsText(i,buf) ;
		combo->AddString(buf) ;
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void AddPrgWizardPage2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AddPrgWizardPage2)
	DDX_Text(pDX, IDC_PRGWIZ_ProvName, m_ProvName);
	DDX_Text(pDX, IDC_PRGWIZ_ServName, m_ServName);
	DDX_CBIndex(pDX, IDC_PRGWIZ_ServType, m_ServType);
	DDX_Text(pDX, IDC_PRGWIZ_ServId, m_ServId);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AddPrgWizardPage2, CDialog)
	//{{AFX_MSG_MAP(AddPrgWizardPage2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AddPrgWizardPage2 message handlers
/////////////////////////////////////////////////////////////////////////////
// AddPrgWizardPage3 dialog


AddPrgWizardPage3::AddPrgWizardPage3(CWnd* pParent /*=NULL*/)
	: CDialog(AddPrgWizardPage3::IDD, pParent)
{
	//{{AFX_DATA_INIT(AddPrgWizardPage3)
	m_PcrEnabled = FALSE;
	m_DataBroadId = 0;
	m_StreamType = 0;
	m_ElemPid = 0;
	m_PcrPid = 0;
	//}}AFX_DATA_INIT

	m_Streams = new PmtStreamHolderArray ;
	m_SelectedStreamIndex = -1 ;
}

AddPrgWizardPage3::~AddPrgWizardPage3( )
{
	delete m_Streams ;
}

void AddPrgWizardPage3::updatePcrPidCtrl( )
{
	GetDlgItem(IDC_ADDPRGWIZ_PcrPid)->EnableWindow(m_PcrEnabled) ;
}

void AddPrgWizardPage3::enablePcrPidCtrl( BOOL enable )
{
	((CButton*)GetDlgItem(IDC_ADDPRGWIZ_PcrPidEnabled))->SetCheck(enable!=0) ;
	m_PcrEnabled = enable ;
	updatePcrPidCtrl() ;		
}

BOOL AddPrgWizardPage3::OnInitDialog() 
{
	CDialog::OnInitDialog();
	updatePcrPidCtrl( ) ;
	m_StreamList.InsertColumn(0, "Item", LVCFMT_LEFT, 500) ;

	return TRUE;
}

void AddPrgWizardPage3::updateStreamList( )
{
	int nStreams = m_Streams->count() ;

	for ( int i = 0; i<nStreams; i++ )
	{
		PmtStreamHolder *stream = m_Streams->item(i) ;
		char buf[256] ;
		PmtStream::streamTypeName( stream->getStreamType(), buf ) ;
		int index = m_StreamList.InsertItem(i, buf) ;
		m_StreamList.SetItemData(index, (DWORD)stream) ;
	}

	if (nStreams)
	{
		m_StreamList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED) ;
		selectStream(0) ;
	}
}

void AddPrgWizardPage3::saveStream( ) 
{
	if (m_SelectedStreamIndex!=-1)
	{
		PmtStreamHolder *stream = m_Streams->item(m_SelectedStreamIndex) ;
		stream->setStreamType(m_StreamType) ;
		stream->setElemPid(m_ElemPid) ;
		DataBroadcastIdDescHolder *descHolder = (DataBroadcastIdDescHolder*)stream->getDescHolder(0) ;
		if (descHolder)
			descHolder->getDesc()->create(m_DataBroadId) ;
	}
}

void AddPrgWizardPage3::selectStream(int index)
{
	if (m_SelectedStreamIndex!=-1 && !UpdateData(TRUE))
	{
		m_StreamList.SetItemState(m_SelectedStreamIndex, LVIS_SELECTED, LVIS_SELECTED) ;
		return ;
	}

	PmtStreamHolder *stream = m_Streams->item(index) ;

	m_StreamType = stream->getStreamType() ;
	m_ElemPid = stream->getElemPid() ;
	DataBroadcastIdDescHolder *descHolder = (DataBroadcastIdDescHolder*)stream->getDescHolder(0) ;
	if (descHolder)
		m_DataBroadId = descHolder->getDesc()->getBroadcastId() ;

	m_SelectedStreamIndex = index ;
	UpdateData(FALSE) ;
}

void AddPrgWizardPage3::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AddPrgWizardPage3)
	DDX_Control(pDX, IDC_ADDPRGWIZ_StreamList, m_StreamList);
	DDX_CBIndex(pDX, IDC_ADDPRGWIZ_DataBroadId, m_DataBroadId);
	DDX_CBIndex(pDX, IDC_ADDPRGWIZ_StreamType, m_StreamType);
	//}}AFX_DATA_MAP

	char buf[256] ;

	DDX_Hex(pDX, IDC_ADDPRGWIZ_ElemPid, m_ElemPid, 0x20, 0x1FFF, "You must enter hexadecimal integer between 0x20 and 0x1FFF.");
	if (pDX->m_bSaveAndValidate && TableHolder::CheckPidUse(m_ElemPid, buf) )
	{
		char txt[256] ;
		sprintf(txt, 
			"PID 0x%x is already used in :\n\n%s\n\n"
			"Choose another value.",
			m_ElemPid, buf
		) ;
		MessageBox( txt, "Error", MB_OK | MB_ICONERROR ) ;
		pDX->Fail() ;
	}
	DDX_Hex(pDX, IDC_ADDPRGWIZ_PcrPid, m_PcrPid, 0x20, 0x1FFF, "You must enter hexadecimal integer between 0x20 and 0x1FFF.");
	if (pDX->m_bSaveAndValidate && TableHolder::CheckPidUse(m_PcrPid, buf) )
	{
		char txt[256] ;
		sprintf(txt, 
			"PID 0x%x is already used in :\n\n%s\n\n"
			"Choose another value.",
			m_PcrPid, buf
		) ;
		MessageBox( txt, "Error", MB_OK | MB_ICONERROR ) ;
		pDX->Fail() ;
	}

	if (pDX->m_bSaveAndValidate)
		saveStream() ;
}

BEGIN_MESSAGE_MAP(AddPrgWizardPage3, CDialog)
	//{{AFX_MSG_MAP(AddPrgWizardPage3)
	ON_BN_CLICKED(IDC_ADDPRGWIZ_PcrPidEnabled, OnPcrPidEnable)
	ON_NOTIFY(NM_CLICK, IDC_ADDPRGWIZ_StreamList, OnStreamListClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void AddPrgWizardPage3::OnPcrPidEnable() 
{
	m_PcrEnabled = ((CButton*)GetDlgItem(IDC_ADDPRGWIZ_PcrPidEnabled))->GetCheck() ;
	updatePcrPidCtrl() ;		
}

void AddPrgWizardPage3::OnStreamListClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMITEMACTIVATE *nmItemActivate = (NMITEMACTIVATE*)pNMHDR ;	
	int item = nmItemActivate->iItem ;
	selectStream(item) ;

	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
// AddPrgWizardPage3 message handlers

AddProgramWizardDlg::AddProgramWizardDlg( TableHolder *tableHolder, CWnd *parent )
 :
	_page1(this),
	_page2(this),
	_page3(this),
	WizardDlg(parent, &_page1, &_page2, &_page3) 
{
	_tableHolder = tableHolder ;

	_bWasOnPage2 = _bWasOnPage3 = FALSE ;
}

BOOL AddProgramWizardDlg::OnInitDialog() 
{
	WizardDlg::OnInitDialog();

	SetWindowText("New Program Wizard") ;

	return TRUE ;
}
void AddProgramWizardDlg::OnPageChange( int newPageIndex )
{
	if (newPageIndex==1)
	{
		_page2.UpdateData(TRUE) ;
		_page2.m_ServId = _page1.m_PrgNum ;

		if (!_bWasOnPage2 )
		{
			_bWasOnPage2 = TRUE ;

			switch (_page1.GetPrgType())
			{
				case 0: _page2.m_ServType = ServiceDescriptor::Digital_television_service; break ;
				case 1: _page2.m_ServType = ServiceDescriptor::Digital_radio_sound_service; break ;
				case 2: _page2.m_ServType = ServiceDescriptor::Data_broadcast_service; break ;
				case 3: _page2.m_ServType = 0 ; break ;
			};
		}
		_page2.UpdateData(FALSE) ;
	}
	else if (newPageIndex==2 && !_bWasOnPage3)
	{
		_bWasOnPage3 = TRUE ;
		
		switch (_page1.GetPrgType())
		{
			case 0: 
			{
				int pid = (_page1.m_PmtPid+1)&0x1FFF ;

				_page3.enablePcrPidCtrl(TRUE) ; 
				_page3.m_PcrPid = pid ; 

				PmtStreamHolder *stream1 = new PmtStreamHolder(PmtStream::video_MPEG2, pid) ;
				DataBroadcastIdDescHolder *descHolder1 = new DataBroadcastIdDescHolder ;
				DataBroadcastIdDescriptor *desc1 = descHolder1->getDesc() ;
				desc1->create(DataBroadcastDescriptor::Synchronous_data_stream) ;
				stream1->addDescriptor(descHolder1) ;
				_page3.m_Streams->add(stream1) ;

				pid = (pid+1) & 0x1FFF ;
				PmtStreamHolder *stream2 = new PmtStreamHolder(PmtStream::audio_MPEG2, pid) ;
				DataBroadcastIdDescHolder *descHolder2 = new DataBroadcastIdDescHolder ;
				DataBroadcastIdDescriptor *desc2 = descHolder2->getDesc() ;
				desc2->create(DataBroadcastDescriptor::Synchronous_data_stream) ;
				stream2->addDescriptor(descHolder2) ;
				_page3.m_Streams->add(stream2) ;

				_page3.updateStreamList() ;
				break ;
			}
			case 1: 
			{
				int pid = (_page1.m_PmtPid+1)&0x1FFF ;

				PmtStreamHolder *stream = new PmtStreamHolder(PmtStream::audio_MPEG2, pid) ;
				DataBroadcastIdDescHolder *descHolder = new DataBroadcastIdDescHolder ;
				DataBroadcastIdDescriptor *desc = descHolder->getDesc() ;
				desc->create(DataBroadcastDescriptor::Synchronous_data_stream) ;
				stream->addDescriptor(descHolder) ;
				_page3.m_Streams->add(stream) ;

				_page3.updateStreamList() ;
				break ;
			}
			case 2: 
			{
				int pid = (_page1.m_PmtPid+1)&0x1FFF ;

				PmtStreamHolder *stream = new PmtStreamHolder(PmtStream::DSMCC, pid) ;
				DataBroadcastIdDescHolder *descHolder = new DataBroadcastIdDescHolder ;
				DataBroadcastIdDescriptor *desc = descHolder->getDesc() ;
				desc->create(DataBroadcastDescriptor::Multi_protocol_encapsulation) ;
				stream->addDescriptor(descHolder) ;
				_page3.m_Streams->add(stream) ;

				_page3.updateStreamList() ;
				break ;
			}
			case 3: break ;
		};

		_page3.UpdateData(FALSE) ;
	}
}

void AddProgramWizardDlg::OnOK() 
{
	if (!_page1.UpdateData(TRUE) )
	{
		SetDlgActive(0) ;
		return ;
	}
	if (!_page2.UpdateData(TRUE) )
	{
		SetDlgActive(1) ;
		return ;
	}
	if (!_page3.UpdateData(TRUE) )
	{
		SetDlgActive(2) ;
		return ;
	}

	// Now the values are valid
	// Update tables

	ushort prgNum = _page1.m_PrgNum ;
	ushort pmtPid = _page1.m_PmtPid ;

	PMT_Holder *pmtHolder = new PMT_Holder(prgNum) ;
	pmtHolder->setPcrPid(_page3.m_PcrPid) ;
	
	PmtStreamHolderArray *arr = _page3.m_Streams ;
	for ( int i = 0; i < arr->count(); i++ )
		pmtHolder->addStream(arr->item(i)) ;
	arr->setDelFunc(NULL) ;

	SdtServiceHolder *service = new SdtServiceHolder(prgNum) ;

	ServiceDescHolder *servDescHolder = new ServiceDescHolder ;
	ServiceDescriptor *servDesc = servDescHolder->getDesc() ;

	servDesc->create( (uchar)_page2.m_ServType, _page2.m_ProvName, _page2.m_ServName ) ;

	service->addDescriptor(servDescHolder) ;

	_tableHolder->AddProgram( prgNum, pmtPid, pmtHolder, service ) ;

	CDialog::OnOK() ;
}

BOOL AddProgramWizardDlg::programExists( int prgNum )
{ 
	return _tableHolder->programExists(prgNum) ; 
}
