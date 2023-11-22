/*
 *	Filename:		comio.cpp
 *
 *	Version:		1.00
 *
 *	Description: implements classes
 *		ComOut - generic i/o
 *		BigComOut - wrapper classes for specialized i/o classes
 *
 *	History:
*/

#include "stdafx.h"
#include "ComOut.h"
#include "DrvInterface.hpp"

#define DvbErr_LoadDriverDll	-1
#define	USE_PESHEADER	FALSE

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
//	generalized Com classes
//  Wrapper classes for different Com classes to unify the treatment.
//-----------------------------------------------------------------------------


BOOL BigComOut::hasCapability( ComIOCapability cap )
{
	if( _com == NULL )
		return FALSE ;
	return _com->hasCapability( cap ) ;
}

// Depending on the analysis of the connectString specialozed ComOut is created.
int BigComOut::open( BaseConfigClass *cfg )
{
	int comErr = 0 ;
	_com = NULL ;

	if( _connectString == NULL  ||  _connectString[0]=='\x0' )
		_com = new ComOut() ;
	else
	{
		try
		{
			char expl[1024] ;

			if( !callDriverIoctl( expl, DVBDRV_NewComOut, (long)&_com, long(cfg)) )
				_com = NULL ;

			if( _com == NULL  &&  comErr != 0 )
			{
				comErr = DvbErr_LoadDriverDll ;
				AfxMessageBox( expl ) ;
			}
		}
		catch( int err )
		{
			comErr = err ;
		}
	}

	if( _com != NULL )
		comErr = _com->open( _connectString ) ;

	_openFailed = comErr ;
	return comErr ;
}

void BigComOut::close()
{
	TRACE( "\nEnter BigComOut::close()" ) ;
	if( _com != NULL )
	{
		char expl[1024] ;
		TRACE( "\ncallDriverIoctl( DVBDRV_DelComOut)" ) ;
		callDriverIoctl( expl, DVBDRV_DelComOut, (long)_com ) ;
		_com = NULL ;
	}
	TRACE( "\nExit BigComOut::close()" ) ;
}

BOOL BigComOut::getDrvProperties( DvbDriverSrvProps *props )
{
	char expl[1024] ;
	return callDriverIoctl( expl, DVBDRV_DriverProps, (long)props, TRUE ) ;
}


//----------------------------------------------------------------------
//	BigComIO
//----------------------------------------------------------------------


BigComIO::BigComIO()
{
	_dll=0 ;
	_ioCtl=0 ;
	_connectString=0;
}

BigComIO::~BigComIO()
{
	char expl[1024] ;
	if( _dll )
	{
		callDriverIoctl( expl, DVBDRV_Close ) ;
		FreeLibrary( _dll ) ;
	}
	free( _connectString ) ;
}

BOOL BigComIO::callDriverIoctl( char *expl, int cmd, long par1, long par2, long par3 )
{
	#define IOCTL	((DVBDRIVER_IOCTL)_ioCtl)
	if( expl != NULL )
		*expl = 0 ;
	if( _ioCtl == NULL )
	{
		if( expl != NULL )
			strcpy( expl, "Driver not initialized" ) ;
		return FALSE ;
	}

	BOOL ret = FALSE ;
	IOCTL_RETURN_STATUS status = (*IOCTL)( (DVBDRV_COMMANDS)cmd, par1, par2, par3 ) ;
	switch( status )
	{
		case IOCTLRET_OK :
			return TRUE ;
		case IOCTLRET_WARNING :
			ret = TRUE ;
			break ;
		case IOCTLRET_UNSUPPORTED :
			if( expl != NULL )
				strcpy( expl, "Unsupported driver command." ) ;
			return FALSE ;
	}

	// IOCTLRET_ERROR / IOCTLRET_WARNING
	if( expl != NULL )
		(*IOCTL)( DVBDRV_GetLastError, (long)expl, 256, 0 ) ;
	return ret ;
}

const char *BigComIO::getDllName( const char *connectStr, BOOL *isTestConnection )
{
	BOOL test=FALSE ;

	char buf[80] ;
	strcpy( buf, connectStr ) ;
	strlwr( buf ) ;

	const char *dllName = NULL ;

	if( strstr(buf,"tcp") != NULL  ||  strstr(buf,"udp") != NULL ||  strstr(buf,"ethernet") != NULL )
	{
		test = TRUE ;
		dllName = "IoTcp.dll" ;
	}
	else
	if( strstr(buf,"lpt") != NULL  ||  strstr(buf,"com") != NULL )
	{
		test = TRUE ;
		dllName = "IoLpt.dll" ;
	}
	else
	if( strstr(buf,"dvbasi") != NULL )
	{
		dllName = "IoDvbAsi.dll" ;
	}
	else
	if( strstr(buf,"dvb") != NULL )
	{
		dllName = "IoComergon.dll" ;
	}
	else
		return NULL ;
	
	if( isTestConnection != NULL )
		*isTestConnection = test ;
	return dllName ;
}

BOOL BigComIO::isConnectStringOk( const char *connectStr, char *expl )
{
	BOOL isTestConnection ;
	if( connectStr==NULL  ||  getDllName(connectStr, &isTestConnection)==NULL )
	{
		sprintf( expl, "The connect string (%s) is invalid or not specified.\n(No connection will be attempted.)",
			connectStr );
		return FALSE ;
	}

/*	if( isTestConnection )
	{
		if( GetProgramLevel() == 0 )	// OK in demo mode
			expl[0] = 0 ;
		else
			sprintf( expl, "Test connect string is specified (%s).\n(Simulated connection will be attempted.)",
				connectStr );
		return TRUE ;
	}
	if( GetProgramLevel() == 0 )
	{
		// Demo level
		if( !isTestConnection )
		{
			sprintf( expl,
				"Illegal connection mode:\n"
				"\tconnectString = %s\n"
				"(Only Tcp connection is allowed in the Demo mode.)",
				connectStr ) ;
			return FALSE ;
		}
	}
*/
	expl[0] = 0 ;
	return TRUE ;
}


static BOOL MfxMessageHook( UINT msg, long wParam, long lParam )
{
	return ::AfxGetMainWnd()->PostMessage( msg, wParam, lParam ) ;
}

BOOL BigComIO::create( const char *connectStr, char *expl, BOOL runningAsServer )
{
	if ( _dll || _ioCtl )
		return TRUE ;

	char buf[80] ;
	strcpy( buf, connectStr ) ;
	strlwr( buf ) ;
	_connectString = strdup( connectStr ) ;

	BOOL isTestConnection ;
	const char *dllName = getDllName( connectStr, &isTestConnection ) ;
/*	if( GetProgramLevel() == 0 )
	{
		if( dllName != NULL  &&  !isTestConnection )
		{
			strcpy( expl, "Illegal connection in the Demo mode" ) ;
			return FALSE ;
		}
	}
*/	if( dllName == NULL )
	{
		strcpy( expl, "Failed to load driver Dll." ) ;
		return FALSE ;
	}

	char path[1024] ;
	GetModuleFileName( NULL, path, sizeof(path) ) ;
	char drv[20], dir[1024] ;
	_splitpath( path, drv, dir, NULL, NULL ) ;
	_makepath( path, drv, dir, dllName, NULL ) ;

	_dll = LoadLibrary( path ) ;

	if ( _dll == NULL )
	{
		sprintf( expl, "%s - %s", path, "Load of DLL failed." ) ;
		return FALSE ;
	}

	// load driver function
	_ioCtl = (DVBDRIVER_IOCTL)GetProcAddress( _dll, "DVBDRIVER_IOCTL" ) ;
	if ( _ioCtl == NULL )
	{
		sprintf( expl, "%s - %s", path, "IO driver does not support DVBDRIVER_IOCTL interface." ) ;
		goto labelError ;
	}

	// test version
	long major, minor ;
	if( callDriverIoctl( expl, DVBDRV_GetDrvVersion, (long)&major, (long)&minor) )
	{
		//if( major != (int)PLT_LIB_VERSION )
		//{
		//	strcpy( expl, "Unsupported driver dll version." ) ;
		//	goto labelError ;
		//}
		if( callDriverIoctl( expl, DVBDRV_Init, runningAsServer, USE_PESHEADER, (long)MfxMessageHook ) )
			return TRUE ;
	}

  labelError:
	FreeLibrary( _dll ) ;
	_dll = NULL ;
	_ioCtl = NULL ;
	return FALSE ;
}

const char *BigComIO::errorCodeAsText( int code, char *buf )
{
	char expl[1024] ;
	if( !callDriverIoctl( expl, DVBDRV_EventAsText, code, (long)buf) )
		sprintf( buf, "I/O driver error %d", code & 0xFFFF ) ;
	return buf ;
}

BOOL BigComIO::showDriverStatusDialog( CWnd *w )
{
	if( !hasCapability(ComIO_DriverStatusDlg) )
		return FALSE ;
	char expl[1024] ;
	callDriverIoctl( expl, DVBDRV_DriverStateDialog ) ;
	return TRUE ;
}

void BigComIO::driverDump( )
{
	if( !hasCapability(ComIO_DriverDump) )
		return ;
	char expl[1024] ;
	callDriverIoctl( expl, DVBDRV_DriverDump ) ;
}

BOOL BigComIO::runSetupDialog( CWnd *parent, ConfigClass *cfg )
{
	char expl[1024] ;
	BOOL ret ;
	if( callDriverIoctl( expl, DVBDRV_RunSetupDialog, (long)parent, (long)cfg, (long)&ret) )
		return ret ;
	AfxMessageBox( expl ) ;
	return FALSE ;
}
