//
// This file implements ioctl function for Comergon ISA driver.
//

#include "tools2.hpp"
#include "DrvInterface.hpp"
#include "ComDvb.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


MyCWinApp theApp ;

BOOL runningAsServer=FALSE ;
ComDVBSetup _setup ;

ComInpDVB *comInp = NULL ;
ComOutDVB *comOut = NULL ;

BOOL USE_PESHEADER = 0;

#define CARDNAME	"Main Data, Rs-422, ISA card"

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
IOCTL_RETURN_STATUS Comergon_driver_ioctl( DVBDRV_COMMANDS cmd, long par1, long par2, long par3 )
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
				runningAsServer = (par2 & eDVBDRV_InstallServer) ? TRUE : FALSE ;
				if( par2 & eDVBDRV_Uninstall )
					uninstallDialog( (HWND)par1 ) ;
				else
					installDialog( (HWND)par1 ) ;
				break ;

			// Initialization
			case DVBDRV_GetDrvVersion:
				*(long*)par1 = IOComergon_VERSION / 100 ;
				*(long*)par2 = IOComergon_VERSION % 100 ;
				break ;
			case DVBDRV_DrvName:
				strncpy( (LPSTR)par1, CARDNAME, par2 );
				break;

			case DVBDRV_Init :
				runningAsServer = par1 ;
				USE_PESHEADER = par2 ;
				g_MessageHook = (MessageHookType)par3 ;
				break ;
			case DVBDRV_Close :
				DvbDestroySetupDialog() ;
				break ;

			// ComIO objects
			case DVBDRV_NewComInp:
			{
				BaseConfigClass *cfg = (BaseConfigClass*)par3 ;
				_setup.load( cfg ) ;
				*(ComInpDVB**)par1 = comInp = new ComInpDVB( (BaseComInp*)par2 ) ;
				break ;
			}
			case DVBDRV_DelComInp:
				StopDrvStateDialog();
				delete (ComInpDVB*)par1 ;
				comInp = NULL ;
				break ;
			case DVBDRV_NewComOut:
			{
				BaseConfigClass *cfg = (BaseConfigClass*)par2 ;
				_setup.load( cfg ) ;
				*(ComOutDVB**)par1 = comOut = new ComOutDVB() ;
				break ;
			}
			case DVBDRV_DelComOut:
				delete (ComOutDVB*)par1 ;
				comOut = NULL ;
				break ;

			// Interactive procedures
			case DVBDRV_RunSetupDialog:
				DvbSetupDialog( ((CWnd*)par1)->m_hWnd, (ConfigClass *)par2 ) ;
				break ;

			case DVBDRV_EventAsText:
				ComergonErrorAsText( par1, (char *)par2 ) ;
				break ;

			case DVBDRV_DriverStateDialog :
				if( !dvbDrvOpened() )
					AfxMessageBox( "ISA-422 driver not opened" ) ;
				else
					dvbDrvDbgStateDlg( theApp.m_hInstance, _setup.driverDumpOn() ) ;
				break ;

			case DVBDRV_DriverDump :
				dvbDrvDump( 0 ) ;
				break ;

			case DVBDRV_DriverProps :
				if( (BOOL)par2 )
				{
					DvbDriverSrvProps *props = (DvbDriverSrvProps *)par1 ;
					memset( props, 0, sizeof(DvbDriverSrvProps) ) ;
					strcpy( props->commInterface, "RS-422 synch." ) ;
					props->outputRate = _setup._cardSpeed / 1000.F ;
					props->flags = (_setup._flagsCard & DVBFLAG_ClockSource) ? 
						0 : DvbDriverSrvProps::GeneratesClock ;
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
	//catch( Msg &msg )
	//{
	//	setLastError( IOCTLRET_ERROR, msg.shortString ) ;
	//}
	catch( ... )
	{
		setLastError( IOCTLRET_ERROR, "Unknown error" ) ;
	}

	return _lastReturn ;
}
