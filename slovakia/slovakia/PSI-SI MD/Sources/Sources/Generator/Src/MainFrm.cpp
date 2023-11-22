// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "PsiSiGen.h"

#include "MainFrm.h"
#include "TableHolders.h"
#include "BroadcastMan.h"

#include "PatPmtDlg.h"
#include "NitSdtDlg.h"
#include "TableTree.h"
#include "ValueList.h"
#include "TablesFreqDlg.h"
#include "PhysInterfaceDlg.h"
#include "AddPrgWizard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(IDC_Start, OnStart)
	ON_COMMAND(IDC_Stop, OnStop)
	ON_UPDATE_COMMAND_UI(IDC_Start, OnUpdateUI)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN_DontApply, OnOpenDontApply)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveDontApply)
	ON_WM_DESTROY()
	ON_COMMAND_RANGE(ID_VIEWEDIT_VIEWRUNNINGPSITABLES, ID_VIEWEDIT_EDITPSITABLES, OnViewModeChanged)
	ON_COMMAND(ID_OUTPUT_TABLESFREQUENCIES, OnTablesFreq)
	ON_COMMAND(ID_OUTPUT_PHYSICALINTERFACE, OnPhysInterfaceSetup)
	ON_COMMAND(ID_AddProgram, OnProgramWizard)
	ON_COMMAND(ID_AddProgramWizard, OnProgramWizard)
	ON_COMMAND(ID_NewNetworkWizard, OnNewNetworkWizard)
	ON_COMMAND(ID_NewNetwork, OnNewNetworkWizard)
	ON_UPDATE_COMMAND_UI(IDC_Stop,	OnUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_VIEWEDIT_EDITPSITABLES, OnUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_VIEWEDIT_VIEWEDITEDPSITABLES, OnUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_VIEWEDIT_VIEWMODIFICATIONS, OnUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_VIEWEDIT_VIEWRUNNINGPSITABLES, OnUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_Device_DvbAsi, OnUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_Device_Dvb, OnUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_Device_Ethernet, OnUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_AddProgramWizard, OnUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_ApplyChanges, OnUpdateUI)
	ON_UPDATE_COMMAND_UI(ID_NewNetworkWizard, OnUpdateUI)
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_PsiDataArray	= NULL ;
	m_TableHolder	= NULL ;
	m_RunningHolder = NULL ;
	m_ViewMode	= Edit ;
	m_DevMode	= BroadcastManager::NoDevice ;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_PsiDataArray	= new PsiDataArray ;
	BOOL bFileRead = m_PsiDataArray->loadFromFile(theApp.GetDataFileName()) ;

	m_TableHolder	= new TableHolder(m_PsiDataArray) ;
	m_RunningHolder = new TableHolder(m_PsiDataArray) ;

	if ( !bFileRead )
	{
		if ( RunNetworkWizard(m_TableHolder, this) != IDOK )
			MessageBox(
				"Skipping \"New Network Wizard\" your NIT table will be invalid.\n\n"
				"Run \"New Network Wizard\" from the application menu\n"
				"or disable NIT transmition in \"Table Frequencies\" option",
				"Warning",
				MB_OK|MB_ICONEXCLAMATION
			);
	}

	// Main creation
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	char fileName[512], drive[5], dir[256] ;
	GetModuleFileName( theApp.m_hInstance, fileName, 511) ;
	_splitpath( fileName, drive, dir, NULL, NULL ) ;
	_makepath( fileName, drive,dir, "PSI_Output.cfg",NULL) ;

	m_BroadcastManager = new BroadcastManager(this, fileName, m_PsiDataArray) ;
	m_DevMode = m_BroadcastManager->GetDevMode() ;

	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar) )
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	CRect r ;
	GetClientRect(&r) ;
	int xSize = r.Width() / 2 ;
	int ySize = r.Height() ;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(TableTree), CSize(xSize, ySize), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(ValueList), CSize(xSize, ySize), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}

	m_TableTree = (TableTree*)m_wndSplitter.GetPane(0,0) ;
	m_valueListView = (ValueList*)m_wndSplitter.GetPane(0,1) ;

	m_TableTree->CreateTreeItems(m_TableHolder) ;

	return TRUE;
}

void CMainFrame::OnDestroy() 
{
	int res = ::MessageBox( NULL,
		"Would you like to save the changes into a new profile ?",
		"Question",
		MB_YESNO | MB_ICONQUESTION ) ;

	if ( res==IDYES )
		OnFileSaveDontApply() ;

	CFrameWnd::OnDestroy();

	delete m_BroadcastManager ;
	delete m_PsiDataArray ;
	delete m_TableHolder ;
	if (m_RunningHolder)
		delete m_RunningHolder ;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::UpdateTitle( )
{
	char title[512], fName[256], tables[128], *status ;

	m_TableHolder->GetRunningTablesNames(tables) ;

	switch (m_ViewMode)
	{
	case Edit:
		_splitpath( theApp.GetDataFileName(), NULL, NULL, fName, NULL) ;
		break ;
	case ViewEdited:
		strcpy(fName, "VIEW EDITED" ) ; break ;
	case ViewRunning:
		strcpy(fName, "VIEW RUNNING" ) ; break ;
	case ViewModifications:
		strcpy(fName, "VIEW MODIFICATIONS" ) ; break ;
	}

	if (m_BroadcastManager->IsRunning())
		status = "RUNNING" ;
	else
		status = "STOPPED" ;

	sprintf(title, 
		"Main Data PSI/SI Generator - %s (%d kbps - %s) %s", 
		fName, 
		m_PsiDataArray->getTotalSpeed(),
		status,
		tables
	) ;
	SetWindowText(title) ;
}

void CMainFrame::OnStart() 
{
	m_BroadcastManager->Start() ;
	UpdateTitle() ;
}

void CMainFrame::OnStop() 
{
	m_BroadcastManager->Stop() ;
	UpdateTitle() ;
}

void CMainFrame::OnUpdateUI(CCmdUI* pCmdUI) 
{
	switch (pCmdUI->m_nID)
	{
	case IDC_Start:
		pCmdUI->Enable(	!m_BroadcastManager->IsRunning() ) ;
		break ;
	case IDC_Stop:
		pCmdUI->Enable(	m_BroadcastManager->IsRunning() ) ;
		break ;
	case ID_VIEWEDIT_EDITPSITABLES:
		pCmdUI->SetCheck(m_ViewMode==Edit) ;
		break ;
	case ID_VIEWEDIT_VIEWEDITEDPSITABLES:
		pCmdUI->SetCheck(m_ViewMode==ViewEdited) ;
		break ;
	case ID_VIEWEDIT_VIEWMODIFICATIONS:
		pCmdUI->Enable(	FALSE ) ;
		//pCmdUI->SetCheck(m_ViewMode==ViewModifications) ;
		break ;
	case ID_VIEWEDIT_VIEWRUNNINGPSITABLES:
		pCmdUI->SetCheck(m_ViewMode==ViewRunning) ;
		break ;
	case ID_Device_DvbAsi:
		pCmdUI->Enable() ;
		pCmdUI->SetCheck(m_DevMode==BroadcastManager::DvbAsi) ;
		break ;
	case ID_Device_Dvb:
		pCmdUI->Enable() ;
		pCmdUI->SetCheck(m_DevMode==BroadcastManager::Dvb) ;
		break ;
	case ID_Device_Ethernet:
		pCmdUI->Enable() ;
		pCmdUI->SetCheck(m_DevMode==BroadcastManager::Ethernet) ;
		break ;
	case ID_AddProgramWizard:
	case ID_ApplyChanges:
	case ID_NewNetworkWizard:
		pCmdUI->Enable(m_ViewMode==Edit) ;
		break ;
	}
}

//----------------------------------------------//
//		File management
//----------------------------------------------//


BOOL CMainFrame::RunOpenSaveAsDlg( BOOL bSaveDlg, CString &path )
{
	UINT attr = 
		bSaveDlg ? 
		(OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR) : 
		(OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR) ;

	CFileDialog dlg( 
		!bSaveDlg, 
		"*.psi",
		NULL, 
		attr,
		"PSI Generator profiles (*.psi)|*.psi|All Files (*.*)|*.*||",
		this );

	if (dlg.DoModal() != IDOK)
		return FALSE ;

	path = dlg.GetPathName() ;
	return TRUE ;
}

void CMainFrame::OnFileOpen() 
{
	CString path ;
	if ( !RunOpenSaveAsDlg(FALSE, path) )
		return ;

	PsiDataArray *arr = new PsiDataArray ;
	
	if ( !arr->loadFromFile(path) )
	{
		MessageBox(
			"Error loading the profile.\n"
			"Not a valid PSI/SI Generator profile", 
			"Error", 
			MB_OK|MB_ICONERROR
		) ;
		delete arr ;
		return ;
	}

	m_ViewMode	= Edit ;

	theApp.SetDataFileName(LPCTSTR(path)) ;	

	m_BroadcastManager->SetData(arr) ;

	delete m_PsiDataArray ;
	m_PsiDataArray	= arr ;

	m_TableTree->DeleteAllItems() ;
	m_valueListView->Refresh(NULL) ;

	delete m_TableHolder ;
	m_TableHolder	= new TableHolder(m_PsiDataArray) ;

	m_TableTree->CreateTreeItems(m_TableHolder) ;
	
	UpdateTitle( ) ;
}

void CMainFrame::OnOpenDontApply() 
{
	CString path ;
	if ( !RunOpenSaveAsDlg(FALSE, path) )
		return ;

	PsiDataArray *arr = new PsiDataArray ;
	
	if ( !arr->loadFromFile(path) )
	{
		MessageBox(
			"Error loading the profile.\n"
			"Not a valid PSI/SI Generator profile", 
			"Error", 
			MB_OK|MB_ICONERROR
		) ;
		delete arr ;
		return ;
	}

	m_ViewMode	= Edit ;

	m_TableTree->DeleteAllItems() ;
	m_valueListView->Refresh(NULL) ;

	delete m_TableHolder ;
	m_TableHolder	= new TableHolder(arr) ;

	m_TableTree->CreateTreeItems(m_TableHolder) ;
	
	UpdateTitle( ) ;
}

void CMainFrame::OnFileSave() 
{
	CString path ;
	if ( !RunOpenSaveAsDlg(TRUE, path) )
		return ;

	PsiDataArray *arr = m_TableHolder->GeneratePsiDataArray() ;
	m_BroadcastManager->SetData(arr) ;

	delete m_PsiDataArray ;
	m_PsiDataArray = arr ;

	if (m_RunningHolder)
		delete m_RunningHolder ;
	m_RunningHolder = new TableHolder(m_PsiDataArray) ;

	theApp.SetDataFileName( path ) ;	

	UpdateTitle( ) ;

	if ( !m_PsiDataArray->saveToFile( path ) )
		MessageBox(
			"Error saving your profile.", 
			"Error",
			MB_YESNO | MB_ICONERROR ) ;
}

void CMainFrame::OnFileSaveDontApply() 
{
	CString path ;
	if ( !RunOpenSaveAsDlg(TRUE, path) )
		return ;

	PsiDataArray *arr = m_TableHolder->GeneratePsiDataArray() ;

	if ( !arr->saveToFile( path ) )
		MessageBox(
			"Error saving your profile.", 
			"Error",
			MB_YESNO | MB_ICONERROR ) ;

	delete arr ;
}

//----------------------------------------------//
//		Modes
//----------------------------------------------//

void CMainFrame::OnViewModeChanged(UINT nID)
{
	switch (nID)
	{
		case ID_VIEWEDIT_EDITPSITABLES:
			if (m_ViewMode==Edit)
				return ;

			m_TableTree->CreateTreeItems(m_TableHolder) ;
			m_ViewMode=Edit ;
			break ;
		case ID_VIEWEDIT_VIEWEDITEDPSITABLES:
			if(m_ViewMode==ViewEdited)
				return;
			m_ViewMode=ViewEdited ;
			m_TableTree->CreateTreeItems(m_TableHolder, TRUE) ;
			break ;
		case ID_VIEWEDIT_VIEWMODIFICATIONS:
			m_ViewMode=ViewModifications ;
			break ;
		case ID_VIEWEDIT_VIEWRUNNINGPSITABLES:
			if (m_ViewMode==ViewRunning)
				return ;
			m_ViewMode=ViewRunning ;

			m_TableTree->CreateTreeItems(m_RunningHolder, TRUE) ;
			break ;
	}
	m_valueListView->SetViewMode(m_ViewMode!=Edit) ;
	UpdateTitle( ) ;
}

void CMainFrame::SetDevMode( int mode )
{
	if (m_BroadcastManager->GetDevMode()==mode)
		return  ;

	m_DevMode = m_BroadcastManager->SetDevMode(mode) ;
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(LOWORD(wParam))
	{
		case ID_RefreshValueList:
		{
			BaseHolder *holder = (BaseHolder*)lParam ;
			m_valueListView->Refresh(holder) ;

			return TRUE ;
		}
		case ID_RefreshTreeItem:
			m_TableTree->RefreshItem() ;
			return TRUE ;

		case ID_CheckPidUnique:
			m_TableHolder->FindPidUse(HIWORD(wParam), (char*)lParam) ;
			return TRUE ;
		
		case ID_Device_DvbAsi:
			SetDevMode(BroadcastManager::DvbAsi) ;
			return TRUE ;
		case ID_Device_Dvb:
			SetDevMode(BroadcastManager::Dvb) ;
			return TRUE ;
		case ID_Device_Ethernet:
			SetDevMode(BroadcastManager::Ethernet) ;
			return TRUE ;
	}
	return CFrameWnd::OnCommand(wParam, lParam);
}

//----------------------------------------------//
//		Setups and wizards
//----------------------------------------------//

void CMainFrame::OnTablesFreq() 
{
	UINT patSize, pmtSize, catSize, sdtSize, nitSize ;

	m_PsiDataArray->getTableSizes( &patSize, &pmtSize, &catSize, &sdtSize, &nitSize ) ;

	CTablesFreqDlg dlg(m_TableHolder, patSize, pmtSize, catSize, sdtSize, nitSize, this) ;
	
	int res = dlg.DoModal() ;
	if (res==IDOK)
	{
		m_TableHolder->UpdateSpeed(m_PsiDataArray) ;
		if (m_RunningHolder)
			m_RunningHolder->UpdateSpeed(m_TableHolder) ;

		if(m_ViewMode==ViewEdited)
			m_TableTree->CreateTreeItems(m_TableHolder, TRUE) ;
		if(m_ViewMode==ViewRunning)
			m_TableTree->CreateTreeItems(m_RunningHolder, TRUE) ;

		UpdateTitle( ) ;
	}
}

void CMainFrame::OnPhysInterfaceSetup() 
{
	m_BroadcastManager->RunDeviceSetup() ;
}

void CMainFrame::OnProgramWizard() 
{
	AddProgramWizardDlg wizard(m_TableHolder, this) ;
	if ( wizard.DoModal() == IDOK )
		m_TableTree->CreateTreeItems(m_TableHolder) ;
}

void CMainFrame::OnNewNetworkWizard() 
{
	if ( RunNetworkWizard(m_TableHolder, this) == IDOK )
		m_TableTree->CreateTreeItems(m_TableHolder) ;
}
