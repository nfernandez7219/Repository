// PsiSiGen.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "..\resource.h"
#include "PsiSiGen.h"
#include "MainFrm.h"
#include "..\..\server\DvbSentinel\H\DVBSentinel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum eProgramLevel
{
	ProgramLevel_Demo		 = 0,
	ProgramLevel_Basic		 = 1,		
	ProgramLevel_Advanced	 = 2
};

int GetProgramLevel( ) ;

// uncomment to disable HW key (Sentinel) checking
//#define NO_HW_KEY_CHECK

#ifdef NO_HW_KEY_CHECK
#pragma message("Warning: HW Key checking disabled !!!")
#endif

/////////////////////////////////////////////////////////////////////////////
// CPsiSiGenApp

BEGIN_MESSAGE_MAP(CPsiSiGenApp, CWinApp)
	//{{AFX_MSG_MAP(CPsiSiGenApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPsiSiGenApp construction

CPsiSiGenApp::CPsiSiGenApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPsiSiGenApp object

CPsiSiGenApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPsiSiGenApp initialization

BOOL CPsiSiGenApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("MainData"));
	 CString dataFileName = GetProfileString ("Params", "FileName", "default.psi") ;
	 strcpy(_DataFileName, LPCTSTR(dataFileName));

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create and load the frame with its resources

	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	pFrame->UpdateWindow();

	// The one and only window has been initialized, so show and update it.
	SetDataFileName( dataFileName ) ;

	pFrame->ShowWindow(SW_SHOW);

	return TRUE;
}

int CPsiSiGenApp::ExitInstance() 
{
	WriteProfileString ("Params", "FileName", GetDataFileName()) ;
	return CWinApp::ExitInstance();
}

void CPsiSiGenApp::SetDataFileName( const char *fileName )
{
	strcpy(_DataFileName, fileName) ;
	((CMainFrame*)AfxGetMainWnd())->UpdateTitle() ;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnChangePrgLevel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_ChangePrgLevel, OnChangePrgLevel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CPsiSiGenApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CAboutDlg::OnChangePrgLevel() 
{
	PsiSentinel senti;
	senti.changeProgramLevel( this );
	GetProgramLevel( );
}

int GetProgramLevel( char *versionName, float *fMaxSpeed, BOOL bReadKey )
{
	#define			OneDay	86400
	static time_t	lastCheck	= time(NULL);
	static BOOL		bFirstTime	= TRUE;
	static int		eLevel		= -2;
	static float	fMaxSpeedVal= 0.f;
	static char		chName[64];

	if( bReadKey || eLevel==-2 || lastCheck < time(NULL)-OneDay )
	{
		PsiSentinel		key;
		PrgLevelStruc	level;

		if( !key.isInstalled() )
		{
			if( bFirstTime )
				AfxMessageBox(	"Can't find HW key. "
								"Check if Sentinel Driver is correctly connected and installed.\n\n"
								"MainData PSI/SI Generator will be running in DEMO mode.", MB_ICONSTOP);
			eLevel		= ProgramLevel_Demo;
			fMaxSpeedVal= 1000.f;
			key.getNameOfLevel(eLevel, chName ) ;
		}
		else
		{
			if( bFirstTime )
				key.decrementNumberOfRuns();
			eLevel		= key.getActivePrgLevel( &level );
			fMaxSpeedVal= eLevel == 0 ? 1000.f : (float)level.dwMaxSpeed/1024.f;

			key.getNameOfLevel(eLevel, chName);

			// level 3=Basic Trial version, level4
			if( eLevel==1 || eLevel==3 )
				eLevel= ProgramLevel_Basic;
			else
			if( eLevel==2 || eLevel==4 )
				eLevel= ProgramLevel_Advanced;
			else
			{
				eLevel= ProgramLevel_Demo;
				key.getNameOfLevel(eLevel, chName);
			}
		}
		bFirstTime= FALSE;
		lastCheck = time(NULL);
	}

	if( fMaxSpeed )
		*fMaxSpeed = fMaxSpeedVal;
	if( versionName )
		strcpy( versionName, chName ) ;
	return eLevel;
}

int GetProgramLevel( )
{
	#ifdef NO_HW_KEY_CHECK
		return ProgramLevel_Advanced ;
	#else
		return GetProgramLevel(NULL,NULL,TRUE) ;
	#endif
}
