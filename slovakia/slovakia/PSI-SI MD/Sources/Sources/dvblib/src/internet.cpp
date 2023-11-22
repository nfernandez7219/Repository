/*
 *	Filename:		internet.cpp
 *
 *	Version:		1.00
 *
 *	Description: InternetChannel + InternetReceiver
 *				 (wrapper classes for access to HNet)
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"
#include  <math.h>

#include "FileIo.hpp"
#include "clientCfg.hpp"
#include "servCfg.hpp"
#include "internet.hpp"
#include "BaseRegistry.hpp"


#define MAXDATA	10000					/*size of the buffer for data exchange with HNet*/

//#define _EMULATE_INTERNET

// Internet delay is used for simulation of real time process when packets are being
// delayed on the server.
// It does not work with _EMULATE_INTERNET.
//#define USE_INTERNET_DELAY

// To measure the delay in internet channel sendData() activate MEASURE_SENDDATA_DELAY and
// specify IMPORTANT_SENDDATA_DELAY. Each delay above this limit will be announced via TRACE.
// This delay normally takes place when the Internet channel has insufficient transport capacity
// or the traffic is very irregular and the remaining channels use up capacity originally
// assigned to Internet channel.
#define MEASURE_SENDDATA_DELAY
#define IMPORTANT_SENDDATA_DELAY		50		/*msec*/


//-----------------------------------------------------------------------------
//	IOHnet
//-----------------------------------------------------------------------------


#include "..\..\IODrivers\IOHNet\IOHNetInterface.h"


IOHNet::IOHNet()
{
	_dll=0 ;
	_ioCtl=0 ;
	_isOpened= FALSE ;
	_status  = 0 ;

	const char *dllName = "IOHNet.dll" ;

	char path[1024] ;
	GetModuleFileName( NULL, path, sizeof(path) ) ;
	char drv[20], dir[1024] ;
	_splitpath( path, drv, dir, NULL, NULL ) ;
	_makepath( path, drv, dir, dllName, NULL ) ;

	_dll = LoadLibrary( path ) ;

	if ( _dll == NULL )
	{
		char msgBuf[512] ;
		throw Msg( -1, "%s - %s", path, "Load of DLL failed." ) ;
	}

	// load driver function
	_ioCtl = (HNETDRIVER_IOCTL)GetProcAddress( _dll, "HNETDRIVER_IOCTL" ) ;
	if ( _ioCtl == NULL )
	{
		throw Msg( -1, "%s - %s", path, "Dll does not support HNETDRIVER_IOCTL interface." ) ;
	}
}

IOHNet::~IOHNet()
{
	close() ;
	char expl[1024] ;
	if( _dll )
	{
		callDriverIoctl( expl, HNETDRV_CloseIO ) ;
		callDriverIoctl( expl, HNETDRV_Detach ) ;
		FreeLibrary( _dll ) ;
	}
}


short IOHNet::callDriverIoctl( char *expl, int cmd, long par1, long par2, long par3 )
{
	#define IOCTL	((HNETDRIVER_IOCTL)_ioCtl)
	if( expl != NULL )
		*expl = 0 ;
	if( _ioCtl == NULL )
	{
		if( expl != NULL )
			strcpy( expl, "Driver not initialized" ) ;
		return -1 ;
	}

	short ret = -1 ;
	HNETIOCTL_RETURN_STATUS status = (*IOCTL)( (HNETDRV_COMMANDS)cmd, par1, par2, par3 ) ;
	switch( status )
	{
		case HNETIOCTL_OK :
			return 0 ;
		case HNETIOCTL_WARNING :
			ret = 0 ;
			break ;
		case HNETIOCTL_UNSUPPORTED :
			if( expl != NULL )
				strcpy( expl, "Unsupported driver command." ) ;
			return -1 ;
		//default: error
	}

	// IOCTLRET_ERROR / IOCTLRET_WARNING
	if( expl != NULL )
	{
		if( (*IOCTL)( HNETDRV_GetLastError, (long)&ret, 0, 0) == 0 )
			if( (*IOCTL)( HNETDRV_EventAsText, (long)ret, (long)expl, 256) == 0 )
				return ret ;
		strcpy( expl, "Unknown error" ) ;
	}
	return -1 ;
}

BOOL IOHNet::create( char *expl, BOOL runningAsServer )
{
	// This function must be called only once (because detach is called only once - in destructor)
	// Later calls simply return old result.
	if( _status != 0 )
	{
		if( expl != NULL )
			*expl = 0 ;
		return _status == 1 ? TRUE : FALSE ;
	}

	//
	// Attach and test version
	//

	long major, minor ;
	BOOL isServer ;
	expl[0] = 0 ;
	if( callDriverIoctl( expl, HNETDRV_Attach, (long)g_fnHNetCallback, runningAsServer) == 0 )
	if( callDriverIoctl( expl, HNETDRV_GetDrvVersion, (long)&isServer, (long)&major, (long)&minor) == 0 )
	{
		if( isServer && !runningAsServer )
		{
			strcpy( expl, "HNet Server driver installed - should be Receiver." ) ;
		}
		else
		if( !isServer && runningAsServer )
		{
			strcpy( expl, "HNet Receiver driver installed - should be Server." ) ;
		}
		else
		{
			_status = 1 ;
			return TRUE ;
		}
	}

	strcat( expl, " - Hybrid Net not available" ) ;
	_status = -1 ;
	return FALSE ;
}

BOOL IOHNet::open( char *expl, CardSerialNumber *adr )
{
	if( _isOpened )
		return TRUE ;
	_isOpened = TRUE ;
	#ifndef _EMULATE_INTERNET
		ASSERT( _ioCtl ) ;
		_isOpened =  (callDriverIoctl( expl, HNETDRV_InitIO, (long)adr, MAXDATA) == 0) ;
	#endif
	if( !_isOpened )
		strcat( expl, " - Hybrid Net not active" ) ;
	return _isOpened ;
}

void IOHNet::close()
{
	if( !_isOpened )
		return ;
	char expl[1024] ;
	callDriverIoctl( expl, HNETDRV_CloseIO ) ;
	_isOpened = FALSE ;
}

const char *IOHNet::errorCodeAsText( int code, char *buf )
{
	char expl[1024] ;
	if( callDriverIoctl( expl, HNETDRV_EventAsText, code, (long)buf) != 0 )
		sprintf( buf, "HNet driver error %d", code & 0xFFFF ) ;
	return buf ;
}

BOOL IOHNet::isOnline( BOOL *yes, char *expl )
{
	short err = callDriverIoctl( expl, HNETDRV_IsHNetOnline, (long)yes) ;
	return err == 0 ;
}

BOOL IOHNet::setOnline( BOOL yes, char *expl )
{
	short err = callDriverIoctl( expl, HNETDRV_SetHNetOnline, yes) ;
	if( err == 0 )
		MfxPostMessage( EMsg_HNetSetOnOff, 0xFFFF, yes ? "HNet set online" : "HNet set offline" ) ;
	else
	{
		char buf[1024] ;
		sprintf( buf, "Detect IsHNetOnline failed: %s", expl ) ;
		MfxPostMessage( EMsg_HNetError, 0xFFFF, buf );
	}
	return err==0 ;
}

void IOHNet::runSetupDialog( HWND parent, BOOL asServer, UCHAR *cfgData, BOOL bModal )
{
	char expl[1024] ;
	UINT uFlag = asServer ? SHNET_SERVER : 0;

	if( bModal )
		uFlag |= SHNET_MODALDIALOG;
	if( callDriverIoctl( expl, HNETDRV_RunSetupDialog, (long)parent, (long)cfgData, uFlag ) == 0 )
		return ;
	::MessageBox( parent, expl, "HNet error", MB_OK | MB_ICONSTOP | MB_TOPMOST ) ;
}

BOOL IOHNet::getSetupData( BOOL asServer, UCHAR **cfgData, int *cfgDataLen, char *expl )
{
	short err = callDriverIoctl( expl, HNETDRV_CreateCfgData, (long)cfgData, (long)cfgDataLen ) ;
	if( err == 0 )
	{
		err = callDriverIoctl( expl, HNETDRV_LoadCfgData, (long)cfgData, asServer ) ;
		if( err != 0 )
			callDriverIoctl( NULL, HNETDRV_DestroyCfgData, (long)*cfgData ) ;
	}
	return err == 0 ;
}

void IOHNet::destroySetupData( UCHAR *cfgData )
{
	callDriverIoctl( NULL, HNETDRV_DestroyCfgData, (long)cfgData ) ;
}


//-----------------------------------------------------------------------------
//	InternetChannel
//-----------------------------------------------------------------------------


inline void InternetChannel::setFatalError( DWORD err )
{
	MuxChannel::setFatalError( "HNetError", err ) ;
}

inline BOOL InternetChannel::isKilled()
{
	return WaitForSingleObject(_hKillEvent,0) == WAIT_OBJECT_0 ;
}

// Internet data are sent through ServiceSender object (used also by service channel)
inline int InternetChannel::sendData( const char *buf, int cnt, const CardSerialNumber &usrId )
{
	//usrId = 0xc46deeb9 ; //0x80941dd8 ;
	//#pragma message("InternetChannel::sendData(): USER_ID set to c46deeb9")
	//ASSERT( usrId.isValid() ) ;
	
	if (_setup.streamFormat==CfgChannel::MPE_HNET_PROTOCOL)
		return _dataSender->sendHNetDataAsMPE( buf, cnt, _setup.numRebroadcasts, channelPID() ) ;
	else
		return _dataSender->sendData( MuxPacket::Internet | MuxPacket::Unicast, buf, cnt,
			_setup.channel, _setup.streamFormat, _setup.numRebroadcasts, channelPID(),
			*(GlobalUserID*)&usrId ) ;
}

//static GlobalUserID staticUsrId = 0 ;

InternetChannel::InternetChannel( Mux *mux, const MuxChannelSetup *s, BOOL online )
: MuxChannel( MuxChannel::ServiceType, mux, s, online )
{
	if( hnetConnection == NULL )
		//throw 1 ;
		return ;

	_sendingThread = NULL ;

	#ifndef _EMULATE_INTERNET

	char expl[1024] ;
	if( !hnetConnection->create( expl, TRUE) )
	{
		if( expl[0] != 0 )
		{
			MfxPostMessage( EMsg_HNetError, 0xFFFF, expl ) ;
			BEEP_WARNING ;
		}
		return ;
	}

	if( !hnetConnection->open( expl, NULL) )
	{
		MfxPostMessage( EMsg_HNetError, 0xFFFF, expl ) ;
		setFatalError( -1 ) ;
		BEEP_WARNING ;
	}
	#endif
}


InternetChannel::~InternetChannel( )
{
	stop() ;
}


// Start sending the data
BOOL InternetChannel::start()
{
	if( isStarted() )
		return TRUE ;
	if( !isOnline() )
		return FALSE ;
	if( hnetConnection == NULL  )
		return FALSE ;
	#ifndef _EMULATE_INTERNET
		if( !hnetConnection->isOpened() )
			return FALSE ;
	#endif
	//if( !MfxDvbSetup()->connectToHNetDrv() )
	//{
	//	HNSrv_opened = FALSE ;
	//	return FALSE ;
	//}

	MuxChannel::start() ;
	MfxPostMessage( EMsg_InboxStart, _setup.channel );

	// create sending thread
	DWORD threadId ;
	_sendingThread = ::CreateThread( NULL, NULL, internetSendThreadFunc, this, NULL, &threadId ) ;

	setStatusBit( Started ) ;
	return TRUE ;
}

// issue signal to stop the channel
BOOL InternetChannel::initiateStop()
{
	if( !isStarted() )
		return TRUE ;
	if( isSending() )
	{
		SetEvent( _hKillEvent );
	}
	return TRUE ;
}

// stop sending data
BOOL InternetChannel::stop()
{
	if( !isStarted() )
		return TRUE ;
	BOOL ret = TRUE ;
	if( isSending() )
	{
		SetEvent( _hKillEvent );
		if( _sendingThread != NULL  &&  WaitForSingleObject( _sendingThread, 200) == WAIT_FAILED )
			ret = FALSE ;
	}
	ResetEvent( _hKillEvent );
	if( _sendingThread != NULL )
	{
		CloseHandle( _sendingThread ) ;
		_sendingThread = NULL ;
	}
	//_isStarted = FALSE ;
	delStatusBit( Started | PendingRequests | StopDueToError ) ;
	MfxPostMessage( EMsg_InboxEnd, _setup.channel );
	return ret ;
}



//---------------------------------------------------------------------------------
//	Packet Queue
//  This is just a profiling tool to measure influence of the delays in sending packets.
//---------------------------------------------------------------------------------


static LONGLONG getSystemTime_msec()
{
	FILETIME		ft;
	SYSTEMTIME		st;
	LARGE_INTEGER	t;

	GetLocalTime( &st );
	SystemTimeToFileTime( &st, &ft );
	t.LowPart	= ft.dwLowDateTime;
	t.HighPart	= ft.dwHighDateTime;
	return t.QuadPart/10000;
}

//////////////////////////////////////////////////////////
//
#ifdef USE_INTERNET_DELAY

#define PACKET_DELAY	500		/* [msec]; each packet will be delayed by this amount of time */

struct RECORD {
	void     *data ;
	int       n_bytes ;
	int		  n_allocBytes ;
	LONGLONG  tim ;
	CardSerialNumber	usrId ;
	inline void set( const void *dat, int numBytes, const CardSerialNumber& usr_id )
	{
		if( numBytes > n_allocBytes )
		{
			data         = realloc( data, numBytes ) ;
			n_allocBytes = numBytes ;
		}
		memcpy( data, dat, numBytes ) ;
		n_bytes = numBytes ;
		tim     = getSystemTime_msec() ;
		usrId   = usr_id ;
	}
} ;

#define MAXRECS	1000
static RECORD recs[MAXRECS] ;
static int jrec=0 ;
static int n_recs=0 ;

inline BOOL canPushRec()
{
	return n_recs < MAXRECS ;
}

static void pushRec( const void *dat, int n_bytes, const CardSerialNumber& usrId )
{
	recs[jrec].set( dat, n_bytes, usrId ) ;
	n_recs++ ;
	jrec = (jrec+1) % MAXRECS ;
}

static BOOL popRec( void **dat, int *n_bytes, CardSerialNumber *usrId )
{
	if( n_recs <= 0 )
		return FALSE ;
	int ind = jrec - n_recs ;
	if( ind < 0 )
		ind += MAXRECS ;
	RECORD   *rec   = recs+ind ;
	LONGLONG timNow = getSystemTime_msec() ;
	LONGLONG dif    = timNow - rec->tim ;
	if( dif < PACKET_DELAY )
		return FALSE ;
	*dat     = rec->data ;
	*n_bytes = rec->n_bytes ;
	*usrId   = rec->usrId ;
	n_recs-- ;
	return TRUE ;
}

#endif
//
//////////////////////////////////////////////////////////


//---------------------------------------------------------------------------------
//	thread sending IP packets
//  Algorithm:
//		while( !killed )
//			read data
//			send data
//
//	(All remaining code serves for debugging and error tretment.)
//---------------------------------------------------------------------------------


#ifdef MEASURE_SENDDATA_DELAY
	static void sendAndMeasureDelay( InternetChannel *ch, void *dataBuf, int cnt, CardSerialNumber &usrId )
	{
		static int bytesCnt=0 ;
		static LONGLONG t0 ;
		static BOOL firstTime=TRUE ;

		if( firstTime )
		{
			firstTime = FALSE ;
			t0 = getSystemTime_msec() ;
		}

		bytesCnt += cnt ;
		LONGLONG t1 = getSystemTime_msec() ;
		ch->sendData( (const char*)dataBuf, cnt, usrId ) ;
		LONGLONG t2 = getSystemTime_msec() ;
		if( (t2-t1) > IMPORTANT_SENDDATA_DELAY )
		{
			TRACE( "\nIP packet delayed in sendData() by %ld msec (sent %ld By in %ld msecs)",
				(long)(t2-t1), bytesCnt, (long)(t1-t0) ) ;
			t0 = t2 ;
			bytesCnt = 0 ;
		}
	}
#endif

BOOL isInternetWorking=FALSE ;

static void terminateInternetChannel( InternetChannel *ch, int err )
{
	err = HNetDrvError2DVBError(err) ;
	char buf[1024] ;
	DvbEventText( err, buf ) ;
	ch->setFatalError( err ) ;
	stdErrorDialog( "Error in the Internet Channel:\n%s\n\nThe channel will be stopped", buf ) ;
	isInternetWorking = FALSE ;
	ch->initiateStop() ;
}

static DWORD WINAPI internetSendThreadFunc( LPVOID arg )
{
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
	InternetChannel *ch = (InternetChannel *)arg ;

	ch->setSending( ) ;

	#ifdef _EMULATE_INTERNET
		srand( (unsigned)time(NULL) );
	#endif

	static UCHAR dataBuf[MAXDATA] ;
	static int inactiveCounter=0 ;

	char expl[1024] ;
	#define HNSrv_readPackets( err, dataBuf, cnt, usrId ) {	\
		cnt = sizeof(dataBuf) ;								\
		err = hnetConnection->callDriverIoctl( expl, HNETDRV_ReadData, (long)dataBuf, (long)&cnt, (long)&usrId ) ;	\
	}

	HANDLE sleepH = CreateEvent( NULL, FALSE, FALSE, NULL ) ;

	int err=0 ;
	try
	{
		while( 1 )
		{
			if( ch->isKilled() )
				break ;
			int  cnt ;
			CardSerialNumber usrId ;

		#ifdef USE_INTERNET_DELAY		//------------------------------------------
			if( canPushRec() )
			{
				HNSrv_readPackets( err, dataBuf, cnt, usrId ) ;
			}
			else
				err = cnt = 0 ;

			if( err != 0 )
			{
				terminateInternetChannel( ch, err ) ;
				inactiveCounter   = 0 ;
			}
			else
			if( cnt <= 0 )
			{
				if( ++inactiveCounter > 10 )
					isInternetWorking = FALSE ;
				WaitForSingleObject( sleepH, DELAY_WAITING_FOR_HNET_DATA) ;
				//Sleep(DELAY_WAITING_FOR_HNET_DATA) ;
			}
			else
			{
				inactiveCounter   = 0 ;
				isInternetWorking = TRUE ;
				pushRec( dataBuf, cnt, usrId ) ;
			}

			void *data ;
			while( popRec(&data,&cnt,&usrId) )
			{
				#ifdef MEASURE_SENDDATA_DELAY
					sendAndMeasureDelay( ch, data, cnt, usrId ) ;
				#else
					ch->sendData( (const char*)data, cnt, usrId ) ;
				#endif
			}
		#else	//------------------------------------------------------------------
			#ifdef _EMULATE_INTERNET
				static int ip_counter=0 ;
				ip_counter = (ip_counter+1) % 1800 ;
				if( ip_counter % 2 )
					cnt = 0 ;
				else
				if( ip_counter <= 600 )
					cnt = rand() % 1000;
				else
				if( ip_counter <= 1200 )
					cnt = (ip_counter % 5 == 0) ? (rand() % 200) : 0 ;
				else
					cnt = 0 ;
				memset( &usrId, 0, sizeof(usrId) ) ;
				usrId.bytes[0] = 1 ;
				err   = 0 ;
			#else
				HNSrv_readPackets( err, dataBuf, cnt, usrId ) ;
				//usrId = staticUsrId ;

				if( err != 0 )
				{
					terminateInternetChannel( ch, err ) ;
					inactiveCounter   = 0 ;
				}
			#endif
			if( cnt <= 0 )
			{
				if( ++inactiveCounter > 10 )
					isInternetWorking = FALSE ;
				WaitForSingleObject( sleepH, DELAY_WAITING_FOR_HNET_DATA) ;
				//Sleep(DELAY_WAITING_FOR_HNET_DATA) ;
			}
			else
			{
				inactiveCounter   = 0 ;
				isInternetWorking = TRUE ;

				#ifdef MEASURE_SENDDATA_DELAY
					sendAndMeasureDelay( ch, dataBuf, cnt, usrId ) ;
				#else
					ch->sendData( (const char*)dataBuf, cnt, usrId ) ;
				#endif
			}
		#endif	//------------------------------------------------------------------
		}
	}
	catch(...)
	{
		err = HNETERR_UnknownErr ;
		ch->setFatalError( err ) ;
		stdErrorDialog( "Unexpected error in the Internet Channel;\nthe channel will be stopped" ) ;
		ch->initiateStop() ;
	}
	ch->clearSending( ) ;
	CloseHandle( sleepH ) ;
	return err ;
}



//-----------------------------------------------------------------------------
//	InternetReceiver
//-----------------------------------------------------------------------------


static InternetReceiver *internetReceiver = NULL;

// This function is called once per sec by the receiver
// to display info about transfer characteristics.
void CALLBACK  internetTimerFunct( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime )
{
	static ulong	lastNumOfInternetPackets = 0;
	static ulong	lastLostPackets = 0;
	static double	lastSpeed = 0;

	if( internetReceiver == NULL )
		return;

	ulong numInternetPackets = MfxClientSetup()->getNumInternetPackets();
	double speed = 0.0f;
	char buf[256];
	if( numInternetPackets > lastNumOfInternetPackets )
	{
		#ifdef REPORT_KB_SPEED
			speed = ( (float)( numInternetPackets - lastNumOfInternetPackets ) * (float)MUXPACKETSIZE ) / 1024.0f;
		#else
			speed = ( (float)( numInternetPackets - lastNumOfInternetPackets ) * (float)TSPACKET_SIZE ) * 8.f / 1000000.f ;
		#endif
		lastNumOfInternetPackets = numInternetPackets;
	}
	if( fabs( speed - lastSpeed ) >= 0.001 || ( speed == 0.0f && lastSpeed != 0.0f ) )
	{
		sprintf( buf, "%.3f", speed );
		MfxPostMessage( EMsg_FRcvrSpeed, 0xffff, buf );
		lastSpeed = speed;
	}

	if( lastLostPackets < internetReceiver->lostPackets )
	{
		sprintf( buf, "Number of lost internet packets is %lu", internetReceiver->lostPackets );
		MfxPostMessage( EMsg_ChannelNamesChanged, 0xffff, buf );
		lastLostPackets = internetReceiver->lostPackets;
	}
}


// constructor - only handshake with HNet
InternetReceiver::InternetReceiver( )
{
	/*
	if( !MfxClientSetup()->connectToHNetDrv() )
	{
		const char *txt = "Internet not connected because of Config setting." ;
		MfxPostMessage( EMsg_HNetError, 0xFFFF, txt ) ;
		//AfxMessageBox( txt, MB_OK | MB_ICONINFORMATION ) ;
		::MessageBox( NULL, txt, NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST ) ;
		return ;
	}
	*/
	if( hnetConnection == NULL )
		throw 1 ;

	_started= FALSE ;
	timerID = 0 ;

	char expl[1024] ;
	if( !hnetConnection->create( expl, FALSE) )
	{
		if( expl[0] != 0 )
		{
			MfxPostMessage( EMsg_HNetError, 0xFFFF, expl ) ;
			BEEP_WARNING ;
		}
		throw 1 ;
	}

	GlobalUserID HNETusrId = MfxClientSetup()->userID() ;
	if( !hnetConnection->open( expl, (CardSerialNumber*)&HNETusrId) )
	{
		MfxPostMessage( EMsg_HNetError, 0xFFFF, expl ) ;
		BEEP_WARNING ;
	}
	else
	{
		internetReceiver = this;
		MfxPostMessage( EMsg_HNetStart, 0xFFFF, "Internet channel" ) ;
		start();
	}

	//char buf1[1024] ;
	//sprintf( buf1, "Internet channel could not be started:\n%s", buf ) ;
	//::MessageBox( NULL, buf1, NULL, MB_OK | MB_ICONERROR | MB_TOPMOST );
}

InternetReceiver::~InternetReceiver( )
{
	stop() ;
}

BOOL InternetReceiver::start()
{
	if( _started )
		return TRUE ;
	if( !hnetConnection->isOpened() )
		return FALSE ;

	//UINT SetTimer( HWND hWnd, UINT nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc ); 
	timerID = ::SetTimer( NULL, 0, 1000, internetTimerFunct );
	DataReceiver::start();
	MfxPostMessage( EMsg_FRcvrStartProcess, 0xffff, (long)0 );
	_started = TRUE ;
	return TRUE ;
}

void InternetReceiver::stop()
{
	if( !_started )
		return ;
	::KillTimer( NULL, timerID );
	DataReceiver::stop();
	MfxPostMessage( EMsg_FRcvrEndProcess, 0xffff, (long)0 );
	_started = FALSE ;
}


// Data processing does nothing more but to pass incoming data to HNet.
int InternetReceiver::processData( ushort flags, uchar *data, int n_bytes, ushort channel )
{
	static int lastError = 0 ;
	if( !_started )
		return 0 ;

	char  expl[1024] ;
	short err = hnetConnection->callDriverIoctl( expl, HNETDRV_WriteData, (long)data, (long)n_bytes) ;
	if( err == 0 )
		return 0 ;

	//err = HNetDrvError2DVBError( err ) ;
	if( lastError != err )
	{
		MfxPostMessage( EMsg_HNetError, 0xFFFF, expl ) ;
	}
	lastError = err ;
	return err ;
}
