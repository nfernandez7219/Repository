//
// This file implements ioctl function for tcp driver.
//

#include "tools2.hpp"
#include "tcp.hpp"
#include "DrvInterface.hpp"


CWinApp theApp ;
BOOL runningAsServer=FALSE ;
//BOOL USE_PESHEADER = 0;
BOOL bOutputOpened = FALSE ;

#define CARDNAME	"Simulation via Tcp"

MessageHookType		g_MessageHook = NULL ;

BOOL MfxMessageHook( UINT msg, long wParam, long lParam )
{
	if (g_MessageHook==NULL)
		return FALSE ;

	return g_MessageHook( msg, wParam, lParam ) ;
}


//------------------------------------------------------------------------------
//	setup
//------------------------------------------------------------------------------


ComTCPSetup _setup ;

static BOOL getDiskId( GlobalUserID *glb_id )
{
	char	path[1024], drive[20];
	ULONG	id;

	memset( drive, 0, 20 );
	GetModuleFileName( NULL, path, 1024 );
	_splitpath( path, drive, NULL, NULL, NULL );
	drive[strlen( drive )] = '\\';
	GetVolumeInformation( drive, NULL, 0, (LPDWORD)&id, NULL, NULL, NULL, 0 );
	glb_id->set( id );
	return TRUE ;
}

void ComTCPSetup::save( BaseConfigClass *cfg )
{
	cfg->setFloat( CFGSECT, "Speed", _speed ) ;
	cfg->set(CFGSECT, "Host", _host ) ;
	cfg->set(CFGSECT, "Protocol", _protocol ) ;
	cfg->set(CFGSECT, "Port", _port ) ;
}

void ComTCPSetup::load( BaseConfigClass *cfg )
{
	getDiskId( &_userId ) ;
	cfg->getFloat( CFGSECT, "Speed", &_speed ) ;
	_speed = __max( 0.01f , _speed ) ;
	_speed = __min( 1000.f, _speed ) ;

	char *host = cfg->get(CFGSECT, "Host" ) ;
	if (host)
		strncpy(_host, host, 63) ;
	else
		strcpy(_host, "127.0.0.1") ;

	char *protocol = cfg->get(CFGSECT, "Protocol" ) ;
	if (protocol)
		strncpy(_protocol, protocol, 5) ;
	else
		strcpy(_protocol,"udp") ;
	
	char *port = cfg->get(CFGSECT, "Port" ) ;
	if (port)
		strncpy(_port, port, 7) ;
	else
		strcpy(_port,"4321") ;
}


//------------------------------------------------------------------------------
//	ioctl function
//------------------------------------------------------------------------------


static IOCTL_RETURN_STATUS _lastReturn = IOCTLRET_OK ;
static char	_lastError[1024] = "" ;
inline void setLastError( const char *msg )
{
	_lastReturn = IOCTLRET_ERROR ;
	strncpy( _lastError, msg, sizeof(_lastError)-1 ) ;
}


// The basic function dispatching each command call.
IOCTL_RETURN_STATUS tcp_driver_ioctl( DVBDRV_COMMANDS cmd, long par1, long par2, long par3 )
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
			// Initialization
			case DVBDRV_GetDrvVersion:
				*(long*)par1 = IOTCP_VERSION / 100 ;
				*(long*)par2 = IOTCP_VERSION % 100 ;
				break ;
			case DVBDRV_DrvName:
				strncpy( (LPSTR)par1, CARDNAME, par2 );
				break;

			case DVBDRV_Init :
				g_MessageHook = (MessageHookType)par3 ;
				runningAsServer = par1 ;
				//USE_PESHEADER = par2 ;
				break ;

			// ComIO objects
			case DVBDRV_NewComInp:
				try
				{
					BaseConfigClass *cfg = (BaseConfigClass*)par3 ;
					_setup.load( cfg ) ;
					*(TcpInpDriver**)par1 = new TcpInpDriver( (BaseComInp*)par2 ) ;
				}
				catch( ... )
				{
					setLastError( "WinSocket initialization error" ) ;
				}
				break ;
			case DVBDRV_DelComInp:
				delete (TcpInpDriver*)par1 ;
				break ;
			case DVBDRV_NewComOut:
				try
				{
					BaseConfigClass *cfg = (BaseConfigClass*)par2 ;
					_setup.load( cfg ) ;
					*(ComOutTcp**)par1 = new ComOutTcp() ;
					bOutputOpened = TRUE ;
				}
				catch( ... )
				{
					setLastError( "WinSocket initialization error" ) ;
				}
				break ;
			case DVBDRV_DelComOut:
				delete (ComOutTcp*)par1 ;
				bOutputOpened = FALSE ;
				break ;

			// Interactive procedures
			case DVBDRV_RunSetupDialog:
			{
				ConfigClass	*cfg = (ConfigClass*)par2 ;
				CWnd* wnd = (CWnd*)par1 ;
				TcpSetupDialog( (CWnd*)par1, cfg, runningAsServer) ;
				break ;
			}

			case DVBDRV_EventAsText:
				tcpEventAsText( par1, (char *)par2 ) ;
				break ;

			case DVBDRV_DriverProps :
				if( (BOOL)par2 )
				{
					// Server
					DvbDriverSrvProps *props = (DvbDriverSrvProps *)par1 ;
					memset( props, 0, sizeof(DvbDriverSrvProps) ) ;
					strcpy( props->commInterface, CARDNAME ) ;
					props->outputRate = _setup._speed ;
					props->flags = DvbDriverSrvProps::GeneratesClock ;
				}
				else
				{
					// Receiver
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
	//	setLastError( msg.shortString ) ;
	//}
	catch( ... )
	{
		setLastError( "Unknown error" ) ;
	}

	return _lastReturn ;
}
