/*
 *	Filename:		card.cpp
 *
 *	Version:		1.00
 *
 *	Description:
 *
 *	History:
*/

#include "tools2.hpp"
#include <time.h>
#include "Card.h"
#include "BaseRegistry.hpp"
#include "ComDvb.hpp"

extern BOOL		runningAsServer ;
static HANDLE	hVXD = 0;
static BOOL		fOpened = FALSE;

const char *getLastSystemError( char *buf ) ;		// screen.lib
static BOOL stopDvbDrv ( char *expl ) ;
static BOOL startDvbDrv( char *expl ) ;

#define INC_DrvStateDialog


///////////////////////////////////////////////////////////////////////////////
///		errors
///////////////////////////////////////////////////////////////////////////////


// uncomment following line to send packets to nowhere
//#define DONT_SEND_PACKETS_TO_DRIVER

#define NonWin32Event		0x20000000
// 
// The functions may return either local error codes or WIN32 error codes.
// Local error codes must be constructed as
//		(err | NonWin32Event)
// where err = 1..255
//


static const char *getLastSystemError( char *buf )
{
	DWORD err = GetLastError ();
	DWORD cnt = FormatMessage
	( 
		FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		buf,
		511,
		NULL
	);
	if( cnt <= 0 )
		sprintf( buf, "Unknown Win32 error 0x%X", err ) ;
	else
	{
		// remove trailing EOL's (CF_TEXT format ending with "\r\n\0")
		while( --cnt > 0  &&  (buf[cnt] == '\n'  ||  buf[cnt] == '\r') )
			buf[cnt] = 0 ;
	}
	return buf ;
}


///////////////////////////////////////////////////////////////////////////////
///		versions
///////////////////////////////////////////////////////////////////////////////


/*
#define	CARDPRGNAME			"Cardprg.bin"

USHORT dvbQueryCardDriverVersion()
{
	return dvbQueryFileVersion( 1, DVBDRVNAME ) ;
}

USHORT dvbQueryCardPrgVersion   ()
{
	return dvbQueryFileVersion( 0, CARDPRGNAME ) ;
}
*/

///////////////////////////////////////////////////////////////////////////////
///		lock/unlockDVB()
///////////////////////////////////////////////////////////////////////////////


// This mutex guarranties exclusive ownership of the DVB driver.
static HANDLE setupExlusiveObject = NULL;

BOOL lockDVB( char *reason )
{
	// test usage of the driver
	if( setupExlusiveObject == NULL )
	{
		HANDLE h = CreateMutex( NULL, TRUE, "DvbDriverInUse" );
		if( GetLastError() != ERROR_ALREADY_EXISTS )
		{
			setupExlusiveObject = h ;
			return TRUE ;
		}
		if( h != 0 )
			CloseHandle( h );
	}

	if( reason != 0 )
		strcpy( reason, "DVB driver (md_DvbDriver.sys) is in use!\nStop applications using this driver and try again." );
	return FALSE ;
}

void unlockDVB( )
{
	if( setupExlusiveObject )
	{
		CloseHandle( setupExlusiveObject );
		setupExlusiveObject = 0 ;
	}
}



///////////////////////////////////////////////////////////////////////////////
///		dvbDrvControl
///////////////////////////////////////////////////////////////////////////////


inline int dvbDrvControl( DWORD ctrlCode, void *inBuff, DWORD inSize, void *outBuff,DWORD outSize, LPDWORD bytesReturned )
{
	if( DeviceIoControl (hVXD,ctrlCode,inBuff,inSize,outBuff,outSize,bytesReturned,NULL) )
		return 0 ;
	return GetLastError() ;
}


///////////////////////////////////////////////////////////////////////////////
///		testDrvRunning
///////////////////////////////////////////////////////////////////////////////


BOOL testDrvRunning()
{
	if (fOpened)
		return TRUE;

	SC_HANDLE srvControlMngr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if ( srvControlMngr )
	{
		SC_HANDLE dvbDriverHandle = OpenService( srvControlMngr, "md_DVBDriver", SERVICE_QUERY_STATUS );


		if ( dvbDriverHandle )
		{
			SERVICE_STATUS deviceStatus;

			QueryServiceStatus( dvbDriverHandle, &deviceStatus );

			CloseServiceHandle( dvbDriverHandle );
			CloseServiceHandle( srvControlMngr );

			if ( deviceStatus.dwCurrentState == SERVICE_RUNNING ||
				 deviceStatus.dwCurrentState == SERVICE_START_PENDING ||
				 deviceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING )
				return TRUE;
		}
		else
		{
			CloseServiceHandle( srvControlMngr );
			return FALSE;
		}
	}

	return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
///		install utilities
///////////////////////////////////////////////////////////////////////////////


// Create the device driver and start it
BOOL installDvbDrv( char *expl )
{
	if( !stopDvbDrv( expl) )
		return FALSE ;

	SC_HANDLE srvControlMngr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	SC_HANDLE dvbDriverHandle= NULL ;

	if ( srvControlMngr )
	{
		static char deviceName[] = "%SystemRoot%\\system32\\drivers\\md_DvbDriver.sys";
		dvbDriverHandle = CreateService( srvControlMngr, "md_DVBDriver",
				"md_DVBDriver", GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
				SERVICE_KERNEL_DRIVER, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
				deviceName, NULL, NULL, NULL, NULL, NULL );

		if( dvbDriverHandle == NULL  &&  GetLastError() == ERROR_SERVICE_EXISTS )
			dvbDriverHandle = OpenService( srvControlMngr, "md_DVBDriver", SERVICE_START );

		if( dvbDriverHandle != NULL )
		{
			if( StartService( dvbDriverHandle, 0, NULL ) )
			{
				CloseServiceHandle( dvbDriverHandle );
				CloseServiceHandle( srvControlMngr );
				return TRUE ;
			}
		}
	}

	char buf[256] ;
	getLastSystemError( buf ) ;
	sprintf( expl, "Installation failed due to system error:\n%s", buf ) ;

	if( dvbDriverHandle )
		CloseServiceHandle( dvbDriverHandle );
	return FALSE ;
}

// Stop the device driver and remove it
BOOL uninstallDvbDrv( char *expl )
{
	if (fOpened)
	{
		strcpy( expl, "Driver in use" ) ;
		return FALSE ;
	}

	BOOL ret = FALSE ;

	SC_HANDLE srvControlMngr = 0 ;
	SC_HANDLE dvbDriverHandle= 0 ;

	srvControlMngr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( srvControlMngr )
	{
		dvbDriverHandle = OpenService( srvControlMngr, "md_DVBDriver", DELETE | SERVICE_STOP );
		if ( dvbDriverHandle )
		{
			SERVICE_STATUS drvStatus;

			if( !testDrvRunning()  ||
				ControlService( dvbDriverHandle, SERVICE_CONTROL_STOP, &drvStatus ) )
			{
				if( DeleteService( dvbDriverHandle) )
					ret = TRUE ;
			}
		}
	}

	if( !ret )
		getLastSystemError( expl ) ;

	if( dvbDriverHandle )
		CloseServiceHandle( dvbDriverHandle );
	if( srvControlMngr )
		CloseServiceHandle( srvControlMngr );

	return ret;
}


///////////////////////////////////////////////////////////////////////////////
///		start/stop
///////////////////////////////////////////////////////////////////////////////


// start the driver
static BOOL startDvbDrv( char *expl )
{
	if( testDrvRunning() )
		return TRUE;

	SC_HANDLE srvControlMngr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	SC_HANDLE dvbDriverHandle= NULL ;

	BOOL ret = FALSE ;

	if ( srvControlMngr )
	{
		dvbDriverHandle = OpenService( srvControlMngr, "md_DVBDriver", SERVICE_START );
		if( dvbDriverHandle )
			if( StartService( dvbDriverHandle, 0, NULL) )
				ret = TRUE ;
	}

	if( !ret )
		getLastSystemError( expl ) ;

	if( dvbDriverHandle )
		CloseServiceHandle( dvbDriverHandle );
	if( srvControlMngr )
		CloseServiceHandle( srvControlMngr );
	return ret;
}

// stop the driver
static BOOL stopDvbDrv( char *expl )
{
	if( fOpened )
	{
		strcpy( expl, "The driver is in use." ) ;
		return FALSE;
	}

	if ( !testDrvRunning() )
		return TRUE;

	BOOL ret = FALSE ;

	SC_HANDLE srvControlMngr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	SC_HANDLE dvbDriverHandle= NULL ;
	SERVICE_STATUS drvStatus;

	if( srvControlMngr )
	{
		dvbDriverHandle = OpenService( srvControlMngr, "md_DVBDriver", SERVICE_STOP );
		if ( dvbDriverHandle )
			if ( ControlService( dvbDriverHandle, SERVICE_CONTROL_STOP, &drvStatus ) )
				ret = TRUE ;
	}

	if( !ret )
		getLastSystemError( expl ) ;

	if( dvbDriverHandle )
		CloseServiceHandle( dvbDriverHandle );
	if( srvControlMngr )
		CloseServiceHandle( srvControlMngr );
	return ret;
}

BOOL restartDvbDrv( char *reason )
{
	char expl[512] ;
	if( !stopDvbDrv( expl) )
	{
		sprintf( reason, "Can't stop DVB driver:\n%s", expl ) ;
		return FALSE ;
	}
	if( !startDvbDrv( expl) )
	{
		sprintf( reason, "Can't start DVB driver:\n%s", expl ) ;
		return FALSE ;
	}
	return TRUE ;
}


///////////////////////////////////////////////////////////////////////////////
///		dvbDrvOpen
///////////////////////////////////////////////////////////////////////////////


int dvbDrvOpen( DvbDrvMode *m, DvbDrvInfo *inf )
{
	int		result ;
	DWORD	r;

	//inf->drvVersion = 100;
	//strcpy ((char *)(inf->name),"Main Data DvbDriver");
	hVXD = CreateFile( DVB_DRVNTSysName, GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, 0, NULL );
	
	if (hVXD == INVALID_HANDLE_VALUE)
	{
		fOpened = FALSE;
		return GetLastError() ;
	}
	fOpened = TRUE;

	result = dvbDrvControl ( (DWORD)IOCTL_INIT, (void *) m, sizeof (DvbDrvMode), OUT (void *) inf, sizeof (DvbDrvInfo), &r);

	if( result != 0 )
	{
		fOpened = FALSE;
		CloseHandle( hVXD ) ;
		hVXD = INVALID_HANDLE_VALUE ;
	}
	return result;
}

BOOL dvbDrvOpened()
{
	return fOpened ;
}

int dvbDrvClose()
{
	if (!fOpened)
		return 0;

	fOpened = FALSE;
	if ( CloseHandle (hVXD) )
		return 0;
	else
		return GetLastError() ;
}

///////////////////////////////////////////////////////////////////////////////
///		dvbDrvRead
///////////////////////////////////////////////////////////////////////////////

#define PACKETSIZE	188
int dvbDrvRead( void *buff, int maxPackets2read, int *numPacketsRead )
{
	BOOL	result;
	DWORD	r;

	if ( !fOpened )
		return DVBERR_NOT_OPENED ;

	result = ReadFile (hVXD,buff,maxPackets2read * PACKETSIZE ,&r,NULL);
	if (r >= 900)
		{
		r = r+1;
		}
	*numPacketsRead = (int) r / PACKETSIZE;

	if (result)
		return 0;
	else
		return GetLastError() ;
}

///////////////////////////////////////////////////////////////////////////////
///		dvbDrvWrite
///////////////////////////////////////////////////////////////////////////////

int dvbDrvWrite( void *buff, int maxPackets2write, int *numPacketsWritten )
{
	BOOL	result;
	DWORD	r;

	if ( !fOpened ) 
		return DVBERR_NOT_OPENED ;

#ifdef DONT_SEND_PACKETS_TO_DRIVER
	static sentCnt = 0;

	// to be sure DrvOpen and CardOpen is sent
	if ( sentCnt == 2 )
	{
		*numPacketsWritten = maxPackets2write;
		return 0;
	}
#endif

	result = WriteFile (hVXD,buff,maxPackets2write * PACKETSIZE,&r,NULL);
	if (r >= 900)
		{
		r = r+1;
		}
	*numPacketsWritten = r / PACKETSIZE;

#ifdef DONT_SEND_PACKETS_TO_DRIVER
	sentCnt += *numPacketsWritten;
#endif
	
	if (result)
		return 0;
	else
		return GetLastError() ;
}


///////////////////////////////////////////////////////////////////////////////
///		dvbDrvStatistics
///////////////////////////////////////////////////////////////////////////////

int dvbDrvStatistics( DvbDrvStatistics *stat )
{
	DWORD	r;
	if ( !fOpened ) 
	{
		stat->numPacketsArrived	= 0;
		stat->numPacketsHNet	= 0;
		stat->numPacketsSent	= 0;
		stat->isValid			= 0;
		return 0;
	}

	int result = dvbDrvControl ( (DWORD) IOCTL_STAT, NULL, 0, OUT (void *) stat, sizeof (DvbDrvStatistics), &r);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
///		dvbDrvDbgInfo
///////////////////////////////////////////////////////////////////////////////

USHORT	dvbDrvDbgInfo ( OUT void *b)
{
	USHORT	result;
	DWORD	r;
	result = dvbDrvControl ( (DWORD) IOCTL_DBG, NULL, 0, OUT (void *) b, DVB_DRVDumpBufSize*sizeof(MY_ENTRY_INFO), &r);
	return 0;
}






//==============================================================================================
//==============================================================================================
//	Debug dialog
//==============================================================================================
//==============================================================================================


#ifdef INC_DrvStateDialog

#include	"..\\..\\client\\winnt\\rc\\ClientRC.h"


static	MY_DRV_STATE	DrvState;


//#ifdef _DEBUG
#define AUTOMATIC_DUMP_FILE_NAME	"automatic.dump"
#define MANUAL_DUMP_FILE_NAME		"manual.dump"


static char automatic_dump_file_path[1024];
static char manual_dump_file_path[1024];
static char dvbDrvDumpFirstCall = 1;



USHORT	dvbDrvDump ( char manually )
{
	FILE			*ofile;
	int				i,n;
	DWORD			r;
	USHORT			result;
	char			datebuff[128], timebuff[128];

	static CCriticalSection _dumpCriticalSection;

	_dumpCriticalSection.Lock();
	if( dvbDrvDumpFirstCall == 1 )
	{
		char path[1024], drive[20], dir[1024];

		GetModuleFileName( NULL, path, 1024 );
		_splitpath( path, drive, dir, NULL, NULL );
		_makepath( automatic_dump_file_path, drive, dir, AUTOMATIC_DUMP_FILE_NAME, NULL );
		_makepath( manual_dump_file_path, drive, dir, MANUAL_DUMP_FILE_NAME, NULL );
		dvbDrvDumpFirstCall = 0;

		if( fileExist( automatic_dump_file_path ) )
		{
			_makepath( path, drive, dir, AUTOMATIC_DUMP_FILE_NAME, ".old" );
			if( fileExist( path ) )
				remove( path );
			rename( automatic_dump_file_path, path );
		}
		if( fileExist( manual_dump_file_path ) )
		{
			_makepath( path, drive, dir, MANUAL_DUMP_FILE_NAME, ".old" );
			if( fileExist( path ) )
				remove( path );
			rename( manual_dump_file_path, path );
		}
	}
	_dumpCriticalSection.Unlock();


	int     GlobalBuffSize = max(max(sizeof(MY_ENTRY_INFO),sizeof(MY_PACKET1_INFO)),sizeof(MY_PACKET2_INFO)) * DVB_DRVDumpBufSize + 10 ;
	HGLOBAL mem  = GlobalAlloc( GHND, GlobalBuffSize );
	if (!mem)
		return 100; // Command failed
	BYTE *GlobalBuff = (BYTE *) GlobalLock (mem);

	if( manually == 1 )
		ofile = fopen ( manual_dump_file_path, "a+t" );
	else
		ofile = fopen ( automatic_dump_file_path, "a+t" );
	if( ofile == NULL )
	{
		GlobalFree( mem );
		return 101;
	}

	result = dvbDrvControl ( (DWORD) IOCTL_DBG, NULL, 0, OUT (void *) GlobalBuff, GlobalBuffSize, &r);
	if ( !result )
	{
		PMY_ENTRY_INFO	s = (PMY_ENTRY_INFO) GlobalBuff;

		fprintf( ofile, "\n============================================================================================================================\n");
		_strdate( datebuff );
		_strtime( timebuff );
		fprintf( ofile, "\n %s %s\n\n", datebuff, timebuff );
		fprintf( ofile, " N \t T \t SB \t SA \tInB\tInA\t OB\t OA\tSdB\tSdA\tRdB\tRdA\tInAppB\tInAppA\tOutAppB\tOutAppA\tTimB\tTimA\n" );

		for (i=0;i<DVB_DRVDumpBufSize;i++)
		{
			char	tmp[30];
			switch (s[i].TypeBefore) {
				case 1:		strcpy (tmp,"DWr");		break;
				case 0:		strcpy (tmp,"DRe");		break;
				case 2:		strcpy (tmp,"IRe");		break;
				case 3:		strcpy (tmp,"IWr");		break;
				case 4:		strcpy (tmp,"IUn");		break;
				default:	strcpy (tmp,"---");		break;
			}

			FILETIME ftB ;
			ftB.dwHighDateTime = s[i].TimeBefore.HighPart ;
			ftB.dwLowDateTime  = s[i].TimeBefore.LowPart ;

			SYSTEMTIME stB ;
			FileTimeToSystemTime( &ftB, &stB ) ;

			FILETIME ftA ;
			ftA.dwHighDateTime = s[i].TimeBefore.HighPart ;
			ftA.dwLowDateTime  = s[i].TimeBefore.LowPart ;

			SYSTEMTIME stA ;
			FileTimeToSystemTime( &ftA, &stA ) ;

						//N   Typ   Sb    Sa   InB  InA  OuB  OuA  SfB  SfA  RfB  RfA  Cnt  InAppB  InAppA  OutAppB  OutAppA  TimB  TimA
			//fprintf (ofile,"%3d\t%s\t%04X\t%04X\t%3d\t%3d\t%3d\t%3d\t%3d\t%3d\t%3d\t%3d\t%3d\t%I64d\t%I64d\n",
			fprintf (ofile,
				"%3d\t%s"
				"\t%04X\t%04X"
				"\t%3d\t%3d"
				"\t%3d\t%3d"
				"\t%3d\t%3d"
				"\t%3d\t%3d"
				"\t%3d\t%3d"
				"\t%3d\t%3d"
				"\t%d:%d:%d:%d"
				"\t%d:%d:%d:%d\n",
				s[i].Number,tmp,
				s[i].StatusBefore,s[i].StatusAfter,
				s[i].InPacketsBefore,s[i].InPacketsAfter,
				s[i].OutPacketsBefore,s[i].OutPacketsAfter,
				s[i].SendDataBefore,s[i].SendDataAfter,
				s[i].ReadDataBefore,s[i].ReadDataAfter,		
				s[i].InAppPointerBefore,s[i].InAppPointerAfter,
				s[i].OutAppPointerBefore,s[i].OutAppPointerAfter,
				stB.wHour,stB.wMinute,stB.wSecond,stB.wMilliseconds,
				stA.wHour,stA.wMinute,stA.wSecond,stA.wMilliseconds );
				//s[i].TimeBefore.QuadPart,s[i].TimeAfter.QuadPart );
			}
	}
	else
		fprintf( ofile, "!!! DeviceIoControl error: %i", result );


	fprintf( ofile, "\n----------------------------------------------------------------------------------------------------------------------------\n\n");
	
	result = dvbDrvControl ( (DWORD) IOCTL_DMP, NULL, 0, OUT (void *) GlobalBuff, GlobalBuffSize, &r);
	if ( !result )
	{
		USHORT typ = *(USHORT*)GlobalBuff ;
		if( typ == 1 )
		{
			PMY_PACKET1_INFO p = (PMY_PACKET1_INFO) (GlobalBuff+sizeof(USHORT));
			n = (r-sizeof(USHORT)) / sizeof(MY_PACKET1_INFO);
			fprintf ( ofile, "No.  \t SyB \t Cha \t Fl \t Pind \t Crc \t Dir \n" );
			for (i=0;i<n;i++)
			{
				if (p[i].Dir == 0) 
					p[i].Dir = '-';
				fprintf( ofile, "%3d  \t%4x \t%4x \t%4x \t%08x\t\t%4x\t%c  \n",
					i+1, p[i].SyncByte, p[i].Channel, p[i].Flags, p[i].PacketIndex,
					p[i].Crc16, p[i].Dir );
			}
		}
		else
		{
			PMY_PACKET2_INFO p = (PMY_PACKET2_INFO) (GlobalBuff+sizeof(USHORT));
			n = (r-sizeof(USHORT)) / sizeof(MY_PACKET2_INFO);
			fprintf ( ofile, "No. \tJob \tPind1 \tPind2 \tFlags \tDir\n" );
			for( i=0 ; i<n ; i++ )
			{
				if (p[i].Dir == 0) 
					p[i].Dir = '-';
				fprintf( ofile, "%3d \t%4x \t%4x \t%4x \t%x \t%c\n",
					i+1, p[i].JobId, p[i].PacketInd1, p[i].PacketInd2, p[i].Flags,p[i].Dir );
			}
		}
	}
	else
		fprintf( ofile, "!!! DeviceIoControl error: %i", result );

	fclose( ofile );
	if (mem)
		GlobalFree (mem);
	return 0;
}
//#endif


///////////////////////////////////////////////////////////////////////////////
///		DrvStateDialog
///////////////////////////////////////////////////////////////////////////////

static	HWND hWndStateDlg = NULL;
static  BOOL dvbDrvDumpAllowed = FALSE ;
static  BOOL showDrvStatusDialog = FALSE;
#define DRVSTATUSDLG_TIMER		8

static void UpdateStateDialog (HWND hWndDlg)
{
	char	tmp[100];

	sprintf (tmp,"%X",DrvState.StatusCount);
	SetDlgItemText (hWndDlg,IDC_STATUS,tmp);
	
	SetDlgItemInt (hWndDlg,IDC_PACKETSSENT	,DrvState.NumPacketsTransferred,FALSE);
	SetDlgItemInt (hWndDlg,IDC_INPACKETS	,DrvState.InPackets		,FALSE);
	SetDlgItemInt (hWndDlg,IDC_OUTPACKETS	,DrvState.OutPackets	,FALSE);
	SetDlgItemInt (hWndDlg,IDC_BUFFERSIZE	,DrvState.BufferSize	,FALSE);
	SetDlgItemInt (hWndDlg,IDC_INAPP		,DrvState.InAppPointer	,FALSE);	
	SetDlgItemInt (hWndDlg,IDC_OUTAPP		,DrvState.OutAppPointer	,FALSE);	
	SetDlgItemInt (hWndDlg,IDC_INCARD		,DrvState.InCardPointer	,FALSE);	
	SetDlgItemInt (hWndDlg,IDC_OUTCARD		,DrvState.OutCardPointer,FALSE);	
	SetDlgItemInt (hWndDlg,IDC_WRITE		,DrvState.NeedWriteInterrupt,FALSE);
	SetDlgItemInt (hWndDlg,IDC_READ			,DrvState.NeedReadInterrupt ,FALSE);

	if( !dvbDrvDumpAllowed )
		EnableWindow  (GetDlgItem(hWndDlg, IDC_DUMP), FALSE);
}

static BOOL CALLBACK DrvStateDlgProc (HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {

		case WM_INITDIALOG:
			hWndStateDlg = hWndDlg;
			UpdateStateDialog (hWndDlg);
			ShowWindow (hWndDlg,SW_SHOW);
			SetTimer( hWndDlg, DRVSTATUSDLG_TIMER, 1000, NULL );
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_DUMP:
					dvbDrvDump ( 1 );
					return TRUE;

				case IDOK:
					showDrvStatusDialog = FALSE;
					//::SendMessage( MfxGetMainWindowHandle(), WM_COMMAND, sSrvM_DrvStatus, 0 );
					hWndStateDlg = NULL;
					DestroyWindow (hWndDlg);
					return TRUE;
			}
			return FALSE;

		case WM_TIMER:
			dvbDrvDbgStateDlg( 0, dvbDrvDumpAllowed );
			return 0 ;
		}

	return FALSE;
}


extern "C"
{


static void DoDrvStateDialog( HINSTANCE hRes, MY_DRV_STATE *DS)
{
	memcpy( &DrvState, DS, sizeof(MY_DRV_STATE) );
	if (!hWndStateDlg)
		CreateDialog( hRes,"DRVSTATEDLG",NULL,(DLGPROC)DrvStateDlgProc);
	else
		UpdateStateDialog (hWndStateDlg);
	showDrvStatusDialog = TRUE;
}

void StopDrvStateDialog()
{
	if (hWndStateDlg)
	{
		PostMessage(hWndStateDlg,WM_COMMAND,IDOK,NULL);
	}
	showDrvStatusDialog = FALSE;
}

}

USHORT	dvbDrvDbgStateDlg( HINSTANCE hRes, BOOL dumpAllowed )
{
	USHORT	result;
	DWORD	r;
	dvbDrvDumpAllowed = dumpAllowed ;

	result = dvbDrvControl ( (DWORD) IOCTL_DRVSTATE, NULL, 0, (void *) &DrvState, sizeof(MY_DRV_STATE), &r);

	if (!result)
		DoDrvStateDialog( hRes, &DrvState);
	else
	{
		char buf[1024] ;
		strcpy( buf, "DVB Driver - communication error\n\nLast system error:\n" ) ;
		getLastSystemError( buf+strlen(buf) ) ;
		AfxMessageBox( buf ) ;
	}

	return	result;
}

#endif
