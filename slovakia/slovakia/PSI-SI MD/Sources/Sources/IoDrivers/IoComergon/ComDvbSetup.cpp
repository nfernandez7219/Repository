
#include "tools2.hpp"

#include "resource.h"
#include "ComDvb.hpp"
#include "BaseRegistry.hpp"

#define MAX_TEXT	1024

#define ALLOWED_IRQ			{0x05,0x09,0x10,0x11,0x12}		// allowed IRQ's
#define	MIN_BASEADR			0x200	// range for Card base address
#define	MAX_BASEADR			0x3F8


void showHelp()
{
	char buf[512], path[512], drv[10], dir[512] ;
	GetModuleFileName( theApp.m_hInstance, path, sizeof(path) ) ;
	_splitpath( path, drv, dir, NULL, NULL ) ;
	_makepath( buf, drv, dir, "Doc", NULL ) ;
	_makepath( path, NULL, buf,
		runningAsServer ? "ReadmeComergonSrv" : "ReadmeComergonRcv", "txt" ) ;

	sprintf( buf, "notepad.exe %s", path );
	WinExec( buf, SW_SHOW);		// call notepad to show the help file
}


//-----------------------------------------------------------------------------
//	general utilities
//-----------------------------------------------------------------------------


static void	ErrorBox(HWND hWndParent,char *text)
{
	MessageBox (hWndParent,text,"Error",MB_OK | MB_ICONSTOP);
}

static void getSystemMessage( int err, char *buf )
{
	char sysMsg[1024] ;
	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, sysMsg, 256, NULL );
	sprintf( buf, "Operating system announced following error:\n%s", sysMsg ) ;
}

/*
static void CenterDialog( HWND hWndDlg,HWND hWndParent)
{
	RECT	Crect,Drect;
	int	CX,CY,DX,DY;
	
	if ( hWndParent )
		GetWindowRect (hWndParent,&Crect);
	else
		GetClientRect (GetDesktopWindow(),&Crect);
	GetWindowRect (hWndDlg,&Drect);
	CX = Crect.right - Crect.left; CY = Crect.bottom - Crect.top;
	DX = Drect.right - Drect.left; DY = Drect.bottom - Drect.top;
	SetWindowPos (hWndDlg,0,Crect.left+CX/2-DX/2,Crect.top+CY/2-DY/2,0,0,SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}
*/


//-----------------------------------------------------------------------------
//	testDVB()
//-----------------------------------------------------------------------------


static BOOL isDVBused=FALSE ;

static BOOL testDVB( DWORD base, DWORD irq, char *reason )
{
	int err = 0 ;

	DVBDRIVERINFO oldI, newI ;
	if( RegistryRetrieveDVBDriverEntry(&oldI) != 0 )
	{
		sprintf( reason, "Can't read DVB Driver setup; driver probably not installed." ) ;
		return FALSE ;
	}
	newI      = oldI ;
	newI.Base = base ;
	newI.Irq  = irq ;
	if( oldI.Base != newI.Base  ||  oldI.Irq != newI.Irq )
	{
		if( RegistryModifyDVBDriverEntry(&newI) != 0 )
		{
			sprintf( reason, "Write setup into Registry failed; can't test." ) ;
			return FALSE ;
		}
	}

	// test usage of the driver
	if( isDVBused )
	{
		strcpy( reason, "DVB driver (md_DvbDriver.sys) is in use!\nStop applications using this driver and try again." ) ;
		return FALSE;
	}

	// restart the driver with new values
	if( oldI.Base != newI.Base  ||  oldI.Irq != newI.Irq )
	{
		if( !restartDvbDrv( reason) )
			return FALSE ;
	}

	BOOL ret=FALSE ;

	DvbDrvInfo infoDrv;
	if ( err == 0 )
	{
		DvbDrvMode modeDrv;
		modeDrv.drvMemory	     = 100;
		modeDrv.connectToHNet    = 0;
		modeDrv.flags            = 0;
		err = dvbDrvOpen ( &modeDrv, &infoDrv );
	}

	if ( err != 0 )
	{
		char buf[1024] ;
		ComergonErrorAsText( err, buf ) ;
		sprintf( reason, "Failed to open DVB Driver:\n%s", buf ) ;
	}
	else
	if( stricmp( (const char *)infoDrv.name, DVB_DRVPASSWORD) )
		sprintf( reason, "DVB Driver running, but the handshake failed." ) ;
	else
	if( !infoDrv.cardDetected )
		sprintf( reason, "DVB Driver running, but could not detect DVB Card." ) ;
	else
		ret = TRUE ;

	Sleep (200);

	dvbDrvClose();

	if( oldI.Base != newI.Base  ||  oldI.Irq != newI.Irq )
		RegistryModifyDVBDriverEntry(&oldI) ;

	return ret ;
}


//-----------------------------------------------------------------------------
//	Setup dialog utilities
//-----------------------------------------------------------------------------


static BOOL	TestBase (DWORD Base,HWND hWndDlg)
{
	char	tmp[MAX_TEXT];

	if( Base < MIN_BASEADR  ||  Base > MAX_BASEADR )
	{
		sprintf (tmp,"Base address value is wrong\nAllowed range (hexadecimally): %x - %x , step 8",
			MIN_BASEADR, MAX_BASEADR);
		ErrorBox (hWndDlg,tmp);
		SetFocus (GetDlgItem(hWndDlg,IDC_BASE));
		return FALSE;
	}

	DWORD MyBase = Base/8*8 ;
	if( MyBase != Base )
	{
		sprintf (tmp,"%X",MyBase);
		SetDlgItemText (hWndDlg,IDC_BASE,tmp);
	}
	return TRUE;
}

static BOOL	TestIrq (DWORD Irq,HWND hWndDlg)
{
	static const DWORD IRQ[] = ALLOWED_IRQ;			// allowed IRQ's
	int MAX_IRQ	= (sizeof(IRQ) / sizeof(DWORD)) ;

	WORD	i;
	char	tmp[MAX_TEXT];
	DWORD	MyIrq = IRQ[0];

	for (i=0;i<MAX_IRQ;i++)
		if (Irq == IRQ[i])
			return TRUE;

	sprintf (tmp,"Irq value is wrong\nAllowed values (hexadecimally): 5, 9, 10, 11, 12");
	ErrorBox (hWndDlg,tmp);
	SetFocus (GetDlgItem(hWndDlg,IDC_IRQ));
	return FALSE;
}

static void TestHardware (HWND hWndDlg,DWORD Base,DWORD Irq)
{
	SetDlgItemText (hWndDlg,IDC_MSG,"Attempting to detect ....\n");
	UpdateWindow (hWndDlg);

	char errReason[1024];
	if( isDVBused )
	{
		if( testDVB( Base, Irq, errReason) )
			SetDlgItemText (hWndDlg,IDC_MSG,"DVB Driver contacted.\nI/O card is in use and cannot be tested.");
		else
		{
			MessageBox( hWndDlg, errReason, "No contact", MB_ICONINFORMATION | MB_OK ) ;
			SetDlgItemText (hWndDlg,IDC_MSG,errReason);
		}
	}
	else
	{
		int err = openDVB( errReason) ;
		closeDVB( ) ;
		if( err == 0 )
			SetDlgItemText (hWndDlg,IDC_MSG,"DVB Card contacted");
		else
		{
			MessageBox( hWndDlg, errReason, "No contact", MB_ICONINFORMATION | MB_OK ) ;
			SetDlgItemText (hWndDlg,IDC_MSG,errReason);
		}
	}
}	


//-----------------------------------------------------------------------------
//	DvbSetupDialog()
//-----------------------------------------------------------------------------


static DVBDRIVERINFO oldInfo;
static HWND hOpenedWnd=0 ;

static void checkResults( HWND hWndParent, DVBDRIVERINFO &HWInfo )
{
	char buf[1024] ;
	if( oldInfo.Base == HWInfo.Base  &&  oldInfo.Irq == HWInfo.Irq )
		return ;

	int err = RegistryModifyDVBDriverEntry (&HWInfo);
	if( err != 0 )
	{
		sprintf( buf, "Failed to store modified settings:\n%s", RegistryErrorMessage(err) ) ;
		MessageBox( hWndParent, buf, "ISA Card", MB_OK | MB_ICONSTOP);
		return ;
	}

	// restart the driver with new settings
	char errmsg[256] ;
	if( !restartDvbDrv( errmsg) )
	{
		sprintf( buf, "Card settings were modified, but driver restart failed:\n%s\n\n"
			"Close the application and restart the driver manually:\n"
			"\t- Either from the Control Panel/Devices/%s, or\n"
			"\t- reboot", errmsg, DVBDRVNAME ) ;
		MessageBox( hWndParent, buf, "ISA Card", MB_OK | MB_ICONSTOP);
	}
	else
		MessageBox( hWndParent, "Card settings were successfully modified.", "ISA Card", MB_OK | MB_ICONINFORMATION);
}

// uModeID = 0 - use old mode
// uModeID > 1 - ID of the decimal check box
static BOOL bIsDecimalMode = FALSE;
BOOL DDX_HexaInt( HWND hDlg, UINT uID, unsigned short &iVal, BOOL bDecMode, BOOL bSave )
{
	char buff[32];

	if( bSave )
	{
		int val=0;

		iVal = val;
		if( GetDlgItemText( hDlg, uID, buff, 32 ) == 0 )
			strcpy( buff, "0" );
		buff[31] = 0;
		if( bDecMode )
		{
			if( !isInt( buff, &val ) )
			{
				::MessageBox( hDlg, "Insert valid decimal value, please", "Error", MB_OK );
				SetFocus( GetDlgItem( hDlg, uID ) );
				return FALSE;
			}
		}
		else
		{
			BOOL bErr = FALSE;

			for( char *p=buff+strlen(buff)-1; p>=buff; p-- )
				if( !isxdigit(*p) )
				{
					bErr=TRUE;
					break;
				}
			if( bErr || sscanf( buff, "%X", &val ) != 1 )
			{
				::MessageBox( hDlg, "Insert valid hexadecimal value, please", "Error", MB_OK );
				SetFocus( GetDlgItem( hDlg, uID ) );
				return FALSE;
			}
		}
		iVal = (unsigned short)val;
	}
	else
	{
		if( bDecMode )
			SetDlgItemInt( hDlg, uID, iVal, FALSE );
		else
		{
			sprintf( buff, "%X", iVal );
			SetDlgItemText( hDlg, uID, buff );
		}
	}
	return TRUE;
}

static void getDllVersion( char *ver )
{
	char exeFile[512], dllFile[512], drv[10], dir[512] ;

	GetModuleFileName( NULL, exeFile, sizeof(exeFile) ) ;

	_splitpath( exeFile, drv, dir, NULL, NULL ) ;
	_makepath( dllFile, drv, dir, "IoComergon.dll", NULL ) ; 

	getFileVersion( dllFile, ver, UPPER_VERSION ) ;
}

static BOOL CALLBACK HWDlgProc (HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	char	tmp[MAX_TEXT], dllVersion[80] ;
	static	PDVBDRIVERINFO pInfo;
	DWORD	base,irq;

	switch( msg )
	{
		case WM_INITDIALOG:
			{
				hOpenedWnd = hWndDlg ;
				pInfo = (PDVBDRIVERINFO) lParam;
				sprintf (tmp,"%X",pInfo->Irq);
				SetDlgItemText (hWndDlg,IDC_IRQ,tmp);
				sprintf (tmp,"%X",pInfo->Base);
				SetDlgItemText (hWndDlg,IDC_BASE,tmp);

				getDllVersion( dllVersion ) ;
				sprintf( tmp, "(Dll version V%s, Driver version V%s)", dllVersion, _setup._drvVersion ) ;
				SetDlgItemText (hWndDlg, IDC_Version, tmp);

				SetWindowPos( hOpenedWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE ) ;
				if( isDVBused )
				{
					HWND h = ::GetDlgItem( hWndDlg, IDC_BASE ) ;
					if( h != NULL )
						EnableWindow( h, FALSE ) ;
					h = ::GetDlgItem( hWndDlg, IDC_IRQ ) ;
					if( h != NULL )
						EnableWindow( h, FALSE ) ;
					h = ::GetDlgItem( hWndDlg, IDC_PID ) ;
					if( h != NULL )
						EnableWindow( h, FALSE ) ;
					h = ::GetDlgItem( hWndDlg, IDC_DataRate ) ;
					if( h != NULL )
						EnableWindow( h, FALSE ) ;
				}
				
				if( !runningAsServer )
				{
					CheckDlgButton( hWndDlg, IDC_RADIO_DEC, bIsDecimalMode );
					CheckDlgButton( hWndDlg, IDC_RADIO_HEX,!bIsDecimalMode );
					DDX_HexaInt(hWndDlg, IDC_PID, _setup._PID, bIsDecimalMode, FALSE );

					HWND h = ::GetDlgItem( hWndDlg, IDC_DataRate ) ;
					if( h != NULL )
						EnableWindow( h, FALSE ) ;
				}
				else
				{
					char	chBuff[32];
					float	Hz	= _setup._cardSpeed*1000.f;
					float	Mbs = (float)(Hz/1000000.f);
					sprintf( chBuff, "%.3f", Mbs );
					SetDlgItemText( hWndDlg, IDC_DataRate, chBuff );

					HWND h = ::GetDlgItem( hWndDlg, IDC_PID ) ;
					if( h != NULL )
						EnableWindow( h, FALSE ) ;
					h = ::GetDlgItem( hWndDlg, IDC_RADIO_HEX ) ;
					if( h != NULL )
						EnableWindow( h, FALSE ) ;
					h = ::GetDlgItem( hWndDlg, IDC_RADIO_DEC ) ;
					if( h != NULL )
						EnableWindow( h, FALSE ) ;
				}
				//CenterDialog (hWndDlg,GetParent(hWndDlg));			
				return TRUE;
			}
			
		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_ViewHelp:
					showHelp() ;
					return TRUE;

				case IDC_RADIO_HEX :
				case IDC_RADIO_DEC :
					{
						unsigned short iVal;
						bIsDecimalMode = IsDlgButtonChecked( hWndDlg, IDC_RADIO_DEC );
						if( DDX_HexaInt( hWndDlg, IDC_PID, iVal, !bIsDecimalMode, TRUE) )
							DDX_HexaInt( hWndDlg, IDC_PID, iVal,  bIsDecimalMode, FALSE );
						else
							CheckRadioButton( hWndDlg, IDC_RADIO_HEX, IDC_RADIO_DEC, bIsDecimalMode ? IDC_RADIO_HEX : IDC_RADIO_DEC );
					}
					return TRUE;

				case IDC_TESTHW:
				case IDOK:
					{
						GetDlgItemText (hWndDlg,IDC_BASE,tmp,MAX_TEXT-1);
						sscanf (tmp,"%X",&base);
						BOOL valuesOk = TestBase (base,hWndDlg);
						GetDlgItemText (hWndDlg,IDC_IRQ,tmp,MAX_TEXT-1);
						sscanf (tmp,"%X",&irq);
						if( valuesOk )
							valuesOk = TestIrq (irq,hWndDlg);
						if( LOWORD(wParam) == IDC_TESTHW )
						{
							if( !valuesOk )
								return FALSE ;
							TestHardware (hWndDlg,base,irq);
							return TRUE ;
						}

						// get data rate & PID

						if( runningAsServer )
						{
							char	chBuff[32];
							GetDlgItemText( hWndDlg, IDC_DataRate, chBuff, 31 );
							chBuff[31]	= 0;
							float fMBs	= (float)(atof( chBuff )/8.f);
							int   kBit	= int(fMBs * 1000.f * 8.f);
							if( kBit<MIN_CARD_SPEED || kBit>MAX_CARD_SPEED )
							{
								char buf[1024] ;
								sprintf( buf, "Out of range.\nOutput rate has to be from %.3f Mb/s to %.3f Mb/s.",
									MIN_CARD_SPEED/1000.f, MAX_CARD_SPEED/1000.f );
								MessageBox( hWndDlg, buf, "", MB_OK );
								return FALSE;
							}
							_setup._cardSpeed = kBit;
						}
						else
						{
							bIsDecimalMode = IsDlgButtonChecked( hWndDlg, IDC_RADIO_DEC );
							if( !DDX_HexaInt( hWndDlg, IDC_PID, _setup._PID, bIsDecimalMode, TRUE ) )
								return FALSE;
							if( _setup._PID>8191 )
							{
								MessageBox( hWndDlg, "Illegal PID value.\nAllowed interval is 0..8191", NULL, MB_OK );
								return FALSE;
							}
						}

						if( valuesOk )
						{
							pInfo->Base = base;
							pInfo->Irq = irq;
							checkResults( hWndDlg, *pInfo ) ;
							EndDialog (hWndDlg,TRUE);
							hOpenedWnd = 0 ;
						}
					}
					return TRUE;

				case IDCANCEL:
					//SetDlgItemText (hWndDlg,IDC_IRQ,"aaaa");
					EndDialog (hWndDlg,FALSE);
					hOpenedWnd = 0 ;
					return TRUE;
			}
			return FALSE;
	}
	return FALSE;
}

void DvbSetupDialog( HWND hWndParent, ConfigClass *cfg )
{
	if( hOpenedWnd != 0 )
		return ;				// already running

	isDVBused = !lockDVB(NULL) ;

	DVBDRIVERINFO	HWInfo;
	if ( RegistryRetrieveDVBDriverEntry (&HWInfo) )
	{
		HWInfo.Base		= 0x330 ;
		HWInfo.Irq		= 10 ;
		HWInfo.Sender	= runningAsServer ;
		RegistryCreateDVBDriverEntry (&HWInfo);
	}

	oldInfo=HWInfo;

	try
	{
		DialogBoxParam( theApp.m_hInstance,
			runningAsServer ? "HWDLG_SRV" : "HWDLG_RCV",
			hWndParent,(DLGPROC) HWDlgProc,(LPARAM)&HWInfo) ;
		if( !isDVBused )
			unlockDVB() ;
		isDVBused = FALSE ;
		if( cfg != NULL )
			_setup.save( cfg ) ;
	}
	catch( ... )
	{
	}
}

void DvbDestroySetupDialog( )
{
	if( hOpenedWnd != 0 )
	{
		EndDialog( hOpenedWnd, FALSE ) ;
		hOpenedWnd = 0 ;
	}
}
