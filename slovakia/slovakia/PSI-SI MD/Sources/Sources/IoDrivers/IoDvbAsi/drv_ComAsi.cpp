//
// This file implements ioctl function for DVB Asi driver.
//

#include "tools2.hpp"
#include "DrvInterface.hpp"
#include "ComDvbAsi.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWinApp theApp ;

BOOL runningAsServer=FALSE ;
DvbAsiConfig _setup ;
DvbAsiStats  stats  ;

ComInpDvbAsi *comInp = NULL ;
ComOutDvbAsi *comOut = NULL ;

BOOL USE_PESHEADER = 0;

#define CARDNAME	"DVB ASI"

MessageHookType		g_MessageHook = NULL ;

BOOL MfxMessageHook( UINT msg, long wParam, long lParam )
{
	if (g_MessageHook==NULL)
		return FALSE ;

	return g_MessageHook( msg, wParam, lParam ) ;
}



//------------------------------------------------------------------------------
//	ioctl function
//------------------------------------------------------------------------------


static IOCTL_RETURN_STATUS _lastReturn = IOCTLRET_OK ;
static char	_lastError[1024] = "" ;
inline void setLastError( IOCTL_RETURN_STATUS ret, const char *msg )
{
	_lastReturn = IOCTLRET_ERROR ;
	strncpy( _lastError, msg, sizeof(_lastError)-1 ) ;
}


// The basic function dispatching each command call.
IOCTL_RETURN_STATUS DvbAsi_driver_ioctl( DVBDRV_COMMANDS cmd, long par1, long par2, long par3 )
{
	try
	{
		if( cmd == DVBDRV_GetLastError )
		{
			switch( _lastReturn )
			{
				case IOCTLRET_OK:
					strcpy( (char*)par1, "Success" ) ;
					break ;
				case IOCTLRET_ERROR:
					strcpy( (char*)par1, _lastError ) ;
					break ;
				case IOCTLRET_UNSUPPORTED:
					strcpy( (char*)par1, "Unsupported" ) ;
					break ;
			}
			return IOCTLRET_OK ;
		}

		_lastReturn = IOCTLRET_OK ;

		switch ( cmd )
		{
			case DVBDRV_Install :
				if ( par2 & eDVBDRV_Uninstall )
					uninstallDialog( (HWND)par1,(ConfigClass*)par3, par2 & eDVBDRV_InstallServer ) ;
				else
					installDialog( (HWND)par1, (ConfigClass*)par3, par2 & eDVBDRV_InstallServer ) ;
				break ;
			// Initialization
			case DVBDRV_GetDrvVersion:
			{
				char ver[32] ;
				char path[128] ;
				GetModuleFileName( theApp.m_hInstance, path, sizeof(path) ) ;
				getFileVersion( path, ver, UPPER_VERSION ) ;
				double version = atof( ver ) ;
				long hiVer = long(version) ;
				long loVer = long((version-hiVer)*100) ;
				*(long*)par1 = hiVer ;
				*(long*)par2 = loVer ;
				break ;
			}
			case DVBDRV_DrvName:
				strncpy( (LPSTR)par1, CARDNAME, par2 );
				break;

			case DVBDRV_Init :
				runningAsServer = par1 ;
				USE_PESHEADER = par2 ;
				g_MessageHook = (MessageHookType)par3 ;
				break ;

			// ComIO objects
			case DVBDRV_NewComInp:
			{
				BaseConfigClass *cfg = (BaseConfigClass*)par3 ;
				_setup.load( cfg ) ;
				*(ComInpDvbAsi**)par1 = comInp = new ComInpDvbAsi( (BaseComInp*)par2, cfg ) ;
				break ;
			}
			case DVBDRV_DelComInp:
				delete (ComInpDvbAsi*)par1 ;
				comInp = NULL ;
				break ;
			case DVBDRV_NewComOut:
			{
				BaseConfigClass *cfg = (BaseConfigClass*)par2 ;
				_setup.load( cfg ) ;
				*(ComOutDvbAsi**)par1 = comOut = new ComOutDvbAsi( cfg ) ;
				break ;
			}
			case DVBDRV_DelComOut:
				delete (ComOutDvbAsi*)par1 ;
				comOut = NULL ;
				break ;

			// Interactive procedures
			case DVBDRV_RunSetupDialog:
			{
				ConfigClass	*cfg = (ConfigClass*)par2 ;
				CWnd* wnd = (CWnd*)par1 ;
				DvbAsiSetupDialog( (CWnd*)par1, cfg, runningAsServer) ;
				break ;
			}
			case DVBDRV_EventAsText:
				DvbAsiErrorAsText( par1, (char *)par2 ) ;
				break ;

			case DVBDRV_DriverProps :
				if( (BOOL)par2 )
				{
					DvbDriverSrvProps *props = (DvbDriverSrvProps *)par1 ;
					memset( props, 0, sizeof(DvbDriverSrvProps) ) ;
					strcpy( props->commInterface, "DVB ASI" ) ;
					props->outputRate = _setup._speed ;
					if( _setup._204Bytes )
						props->flags |= DvbDriverSrvProps::UsesReedSolomon ;
				}
				else
				{
					DvbDriverRcvProps *props = (DvbDriverRcvProps *)par1 ;
					memset( props, 0, sizeof(DvbDriverRcvProps) ) ;
					strcpy( props->cardName, CARDNAME ) ;
				}
				break ;

			// Unsupported
			default:
				_lastReturn = IOCTLRET_UNSUPPORTED ;
				break ;
		}
	}

	catch( ... )
	{
		setLastError( IOCTLRET_ERROR, "Unknown error" ) ;
	}

	return _lastReturn ;
}
