/*
 *	Filename:		services.cpp
 *
 *	Version:		1.00
 *
 *	Description:
 *  implementation of various classes and routines connected to the transfer of services:
 *		DvbServerSetup::openServiceChannel()
 *		DvbServerSetup::closeServiceChannel()
 *		sendFileToServiceChannel()
 *		sendUpdateToServiceChannel()
 *		sendMsgToServiceChannel()
 *		sendUserTableToServiceChannel( SerialNumber userID, BOOL forUser )
 *		ServiceManager class
 *		ServiceChannel class
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"

#include "ServCfg.hpp"
#include "Service.hpp"
#include "DvbUser.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// This structure is a building block of the event queue.
// All service request are first queued and then taken 1 by 1 for processing.
struct ServiceEvent
{
	enum BroadcastFlags {
		Unicast=0, Multicast=1, Broadcast=2
	} ;
	enum Event{ Unknown=0,
				UnicastMessage=4 |0,	MulticastMessage=4 |1,	BroadcastMessage=4 |2,
				UnicastFile	  =8 |0,	MulticastFile	=8 |1,	BroadcastFile	=8 |2,
				UnicastCATable=12|0,	MulticastCATable=12|1,	BroadcastCATable=12|2,
				UnicastUpgrade=16|0,	MulticastUpgrade=16|1,	BroadcastUpgrade=16|2
	} ;
	Event	_event;				// event code	
	char   *_data ;				// event data
	ulong	_dataSize;			// data size
	union
	{
		ulong			_channelID	;
		UnicastUserID	_userId ;
	} ;
	BOOL	_forUser ;

	ServiceEvent()
	{
		_event			= Unknown;
		_channelID		= 0;
		_data			= NULL;
		_forUser		= FALSE ;
	}
	ServiceEvent( ServiceEvent &ev )
	{
		_event			= ev._event;
		_forUser		= ev._forUser ;
		if ( _forUser )
			_userId		= ev._userId ;
		else
			_channelID	= ev._channelID;
		_dataSize		= ev._dataSize;
		_data			= (char *)MALLOC( _dataSize );
		memcpy( _data, ev._data, _dataSize );
	}
   ~ServiceEvent()
	{
		FREE( _data );
	}

	// ServiceEvent( {Uni/Broad/Multi}castMessage, id , "text", strlen(txt )+1 )
	// ServiceEvent( {Uni/Broad/Multi}castFile   , id , "file", strlen(file)+1 )
	// ServiceEvent( {Uni/Broad/Multi}castUpgrade , id , "file", strlen(file)+1 )
	// ServiceEvent( {Uni/Broad/Multi}castCATable, id ,  data , dataBytes      )
	// id = Unicast   ... usr_id
	//		Multicast ... channel
	//		Broadcast ... 0
	ServiceEvent( Event evn, ulong id, const char *data, ulong size )
	{
		_event			= evn;
		_channelID		= id;
		_forUser		= FALSE ;
		_dataSize		= size;
		_data			= (char *)MALLOC( _dataSize+1 );
		memcpy( _data, data, _dataSize );
	}
	ServiceEvent( Event evn, const GlobalUserID& id, const char *data, ulong size )
	{
		_event			= evn;
		_userId			= id;
		_forUser		= TRUE ;
		_dataSize		= size;
		_data			= (char *)MALLOC( _dataSize+1 );
		memcpy( _data, data, _dataSize );
	}
};


// ServiceManager controls queue processing.
typedef sTemplateArray<ServiceEvent*> ServiceEventPtrArray;
class ServiceManager: public CWinThread
{
	enum {					// indexes to _threadEvent[]
		KillEvent,			// kill the manager
		NewEventInStack,	// set by addEvent()
		StartTransfer,		// (re)starts sending from the sending thread
		StopTransfer,		// stops sending from the sending thread
		SendingThread		// sending thread id
	};

	ServiceEventPtrArray _events		;
	HANDLE				 _threadEvent[5];
	HANDLE				 _sendThread	;
	BOOL				 _serviceStarted;
	int					 _status ;
	ServiceChannel		*_serviceChannel ;

	BOOL			isSending	   ()			{ return _sendThread != INVALID_HANDLE_VALUE ; }
	void			sendLastEvent  ();
	void			deleteLastEvent();
	DECLARE_DYNAMIC( ServiceManager )

  public:
	enum Status {
		Started			= 0x01,
		PendingRequests	= 0x02,
		Sending			= 0x04,
		StopDueToError	= 0x08
	} ;
	virtual BOOL InitInstance();

			void stop();
			void start();
			void addEvent( ServiceEvent &event );
			void kill();

	ServiceManager( ServiceChannel *ch );
   ~ServiceManager();
};


//----------------------------------------------------------------------------------
//	global service utilities
//----------------------------------------------------------------------------------


// These 2 functions are here to hide ServiceManager implementation from the rest
// of the program.
BOOL DvbServerSetup::openServiceChannel( ServiceChannel *ch )
{
	_serviceChannel = ch ;
	
	if ( dvbSetup()->useSinglePID() )
		_serviceChannelPid = pidStreamManager()->getPidStreamAttrib(dvbSetup()->getSinglePID()) ;
	else
		_serviceChannelPid = ch->channelPID() ;

	_serviceManager	= new ServiceManager( ch );
	_serviceManager->CreateThread() ;
	return TRUE;
}

BOOL DvbServerSetup::closeServiceChannel()
{
	pidStreamManager()->releasePidStreamAttrib(_serviceChannelPid) ;
	_serviceChannelPid = NULL ;

	_serviceChannel = NULL ;
	_serviceManager->kill();
	delete _serviceManager ;
	_serviceManager = NULL ;
	return TRUE;
}


static BOOL confirmIsOutputOpened()
{
	if( ((BigComOut*)bigComIO)->isOpened() )
		return TRUE ;
	int ret = AfxMessageBox(
		"Output channel is closed and no data can be broadcast.\n"
		"Your request can only be queued until the output is operational again.\n"
		"\n"
		"Do you want to carry on the request anyway?",
		MB_YESNO|MB_ICONQUESTION) ;
	return ret == IDYES ;
}

static BOOL sendToServiceChannel
( int unicastEvent, const char *data, ulong dataSize, const GlobalUserID& userID )
{
	if( !confirmIsOutputOpened() )
		return FALSE ;

	ServiceManager *man = serverSetup->serviceManager() ;
	if( man == NULL )
	{
		MessageBox( NULL,
			"Define Service Channel (channel with id=0)\nand restart the application.",
			"Service Channel undefined", MB_OK|MB_ICONSTOP ) ;
		return FALSE ;
	}

	try
	{
		ServiceEvent event( (ServiceEvent::Event)(ServiceEvent::Unicast|unicastEvent), userID, data, dataSize );
		man->addEvent( event );
		return TRUE;
	}
	catch( ... )
	{
		return FALSE ;
	}
}


static BOOL sendToServiceChannel
( int unicastEvent, const char *data, ulong dataSize, ulong channel )
{
	if( !confirmIsOutputOpened() )
		return FALSE ;

	ServiceManager *man = serverSetup->serviceManager() ;
	if( man == NULL )
	{
		MessageBox( NULL,
			"Define Service Channel (channel with id=0)\nand restart the application.",
			"Service Channel undefined", MB_OK|MB_ICONSTOP ) ;
		return FALSE ;
	}

	try
	{
		int eventFlag = channel == 0 ? ServiceEvent::Broadcast : ServiceEvent::Multicast ;
		ServiceEvent event( (ServiceEvent::Event)(eventFlag|unicastEvent), channel, data, dataSize );
		man->addEvent( event );
		return TRUE;
	}
	catch( ... )
	{
		return FALSE ;
	}
}


//   unicast: (data,  userID, TRUE)
// multicast: (data, channel, 0)
// broadcast: (data, 0, 0)
BOOL sendFileToServiceChannel( const char *file, ulong channel )
{
	return sendToServiceChannel( ServiceEvent::UnicastFile, file, strlen(file)+1, channel ) ;
}

BOOL sendFileToServiceChannel( const char *file, const GlobalUserID& userID )
{
	return sendToServiceChannel( ServiceEvent::UnicastFile, file, strlen(file)+1, userID ) ;
}

BOOL sendUpdateToServiceChannel( const char *file, ulong channel )
{
	return sendToServiceChannel( ServiceEvent::UnicastUpgrade, file, strlen(file)+1, channel ) ;
}

BOOL sendUpdateToServiceChannel( const char *file, const GlobalUserID& userID )
{
	return sendToServiceChannel( ServiceEvent::UnicastUpgrade, file, strlen(file)+1, userID ) ;
}

BOOL sendMsgToServiceChannel( const char *msg, ulong channel, ulong dataLength )
{
	return sendToServiceChannel( ServiceEvent::UnicastMessage, msg, dataLength, channel ) ;
}

BOOL sendMsgToServiceChannel( const char *msg, const GlobalUserID& userID, ulong dataLength )
{
	return sendToServiceChannel( ServiceEvent::UnicastMessage, msg, dataLength, userID ) ;
}

//----------------------------------------------------------------------------------
//	user CA table
//----------------------------------------------------------------------------------


BOOL sendUserTableToServiceChannel()
{
	try
	{
		BOOL ret;

		ret = MfxUsersSetup()->sendUserTableForAllUser();

		ulong	tabSize;
		char	*tab;
		tabSize	= MfxChannelsSetup()->createChannelNamesTable( &tab );
		if( tab != NULL )
		{
			ret = ret && sendToServiceChannel( ServiceEvent::UnicastCATable, tab, tabSize, 0 );
			FREE( tab );
		}
		else
			ret = FALSE;

		return ret;
	}
	catch( ... )
	{
		return FALSE ;
	}
}

BOOL sendUserTableToServiceChannel( const GlobalUserID& userID )
{
	try
	{
		return MfxUsersSetup()->sendUserTable( userID );
	}
	catch( ... )
	{
		return FALSE ;
	}
}

//----------------------------------------------------------------------------------
//	ServiceManager
//	Provides support for storing send requests in a queue and for sending these
//	requests one by one in a background thread.
//----------------------------------------------------------------------------------


static CRITICAL_SECTION sendServiceLock;

IMPLEMENT_DYNAMIC(ServiceManager, CWinThread)

static void deleteEvents( ServiceEvent** ev )	{ delete *ev; };
ServiceManager::ServiceManager( ServiceChannel *ch )
{
	InitializeCriticalSection( &sendServiceLock );
	_events.setDelFunc( &deleteEvents );

	// synchronisation events (see ServiceManager calss decleration)
	_threadEvent[ KillEvent		 ] = ::CreateEvent( NULL, FALSE, FALSE, NULL );
	_threadEvent[ NewEventInStack] = ::CreateEvent( NULL, FALSE, FALSE, NULL );
	_threadEvent[ StartTransfer  ] = ::CreateEvent( NULL, FALSE, FALSE, NULL );
	_threadEvent[ StopTransfer	 ] = ::CreateEvent( NULL, FALSE, FALSE, NULL );
	_threadEvent[ SendingThread	 ] = INVALID_HANDLE_VALUE ;

	_sendThread					   = INVALID_HANDLE_VALUE ;
	_serviceStarted				   = TRUE ;
	m_bAutoDelete				   = FALSE;
	_serviceChannel				   = ch ;
}

ServiceManager::~ServiceManager()
{
	::CloseHandle( _threadEvent[ KillEvent		]);
	::CloseHandle( _threadEvent[ NewEventInStack]);
	::CloseHandle( _threadEvent[ StartTransfer	]);
	::CloseHandle( _threadEvent[ StopTransfer	]);
	DeleteCriticalSection( &sendServiceLock );
}

void ServiceManager::kill()
{
	_serviceChannel->initiateStop() ;
	VERIFY( SetEvent( _threadEvent[ KillEvent		] ) );
	SetThreadPriority	( THREAD_PRIORITY_ABOVE_NORMAL );
	WaitForSingleObject	( m_hThread, 500);
}


// Append new event to the queue.
// If more events are wating, check for possible reasons
void ServiceManager::addEvent( ServiceEvent &event )
{
	EnterCriticalSection( &sendServiceLock );
	int cnt = _events.count() ;

	ServiceChannel *sc = MfxServiceChannel() ;
	BOOL            minRateOk = (sc->minRate() > 0  ||  sc->relPriority() > 0) ;

	_events.add( new ServiceEvent( event ) );
	_serviceChannel->setStatusBit( MuxChannel::PendingRequests );
	::SetEvent( _threadEvent[NewEventInStack] );
	LeaveCriticalSection( &sendServiceLock );
	if( !minRateOk  &&  sc->absPriority() <= 0 )
		AfxMessageBox( 
			"Request for service was queued but cannot be carried out because\n"
			"Service Channel has assigned 0 output capacity.\n"
			"\n"
			"You specified 0 min. rate and did not assign any abs. priority\n"
			"to the Service Channel.\n"
			"Correct in the channel setup, please.", MB_ICONERROR ) ;
	else
	if( cnt > 0 )
	{
		char buf[1024] ;
		sprintf( buf, "Request for service was queued and will be carried out\n"
			"after previously sent requests (%d) will be completed.%s", cnt,
			minRateOk ? "" : "\n\nWarning:\nSetup for the Service Channel is suspiceous and\nmay lead to 0 output capacity.") ;
		AfxMessageBox( buf, MB_ICONINFORMATION ) ;
	}
}

void ServiceManager::stop()
{
	::SetEvent( _threadEvent[StopTransfer] );
}

void ServiceManager::start()
{
	::SetEvent( _threadEvent[StartTransfer] );
}

// Send oldest event in the queue.
void ServiceManager::sendLastEvent()
{
	if( !_serviceStarted || isSending() )
		return;

	ServiceEvent *event = _events.count() ? _events[0] : NULL ;

	if( event != NULL )
		_sendThread = _serviceChannel->sendData( event ) ;
	if ( !_events.count() )
		_serviceChannel->delStatusBit( MuxChannel::PendingRequests );
}

// Delete oldest event in the queue.
// Used after event send was completed.
void ServiceManager::deleteLastEvent()
{
	EnterCriticalSection( &sendServiceLock );
	if( _events.count() )
		_events.del( 0 ); 
	LeaveCriticalSection( &sendServiceLock );
}

// ServiceManager runs an infinite loop (in a thread) which serves both for
// accepting new events and sending already queued events.
BOOL ServiceManager::InitInstance( )
{
	while(1)
	{
		_threadEvent[4] = _sendThread ;
		switch( WaitForMultipleObjects( isSending()?5:4, _threadEvent, FALSE, INFINITE) )
		{
			case WAIT_OBJECT_0:		// KillEvent
				return FALSE;
			
			case WAIT_OBJECT_0+1:	// NewEvent
				sendLastEvent();
				break;

			case WAIT_OBJECT_0+2:	// StartTransfer
				if( !_serviceStarted )
				{
					_serviceStarted = TRUE;
					//_serviceChannel->create() ;
					_serviceChannel->start() ;
					sendLastEvent();
				}
				break;

			case WAIT_OBJECT_0+3:	// StopTransfer
				if( _serviceStarted )
				{
					_serviceStarted = FALSE;
					_serviceChannel->stop() ;
				}
				break;

			case WAIT_OBJECT_0+4:	// SendingThread
				// DWORD  errorCode ;
				//::GetExitCodeThread( _sendThread, &errorCode) ;
				CloseHandle( _sendThread );
				_sendThread	= INVALID_HANDLE_VALUE ;
				deleteLastEvent();
				sendLastEvent();
				break ;
			case WAIT_FAILED:
				break ;
		}
	}

	return FALSE;
}


//----------------------------------------------------------------------------------
//	ServiceChannel
//	This is normal channel with additional support for sending data in a thread.
//	ServiceManager uses this support for sending service data.
//----------------------------------------------------------------------------------


ServiceChannel::ServiceChannel( Mux *mux, const MuxChannelSetup *s, BOOL online ) :
MuxChannel( MuxChannel::FileType | MuxChannel::ServiceType, mux, s, online )
{
//	_dataSender->start() ;
	_sendingThread = NULL ;
}

BOOL ServiceChannel::start()
{
	if( isStarted() )
		return TRUE ;
	if( !isOnline() )
		return FALSE ;
	//_isStarted = TRUE ;
	MuxChannel::start() ;
	setStatusBit( Started ) ;
	return TRUE ;
}

BOOL ServiceChannel::initiateStop()
{
	if( !isStarted() )
		return TRUE ;
	if( isSending() )
	{
		SetEvent( _hKillEvent );
	}
	return TRUE ;
}

BOOL ServiceChannel::stop()
{
	if( !isStarted() )
		return TRUE ;
	BOOL ret = TRUE ;
	//if( _dataSender->isSending() )
	if( isSending() )
	{
		SetEvent( _hKillEvent );
		if( WaitForSingleObject( _sendingThread, 500) == WAIT_FAILED )
			ret = FALSE ;
		ResetEvent( _hKillEvent );
	}
	//_isStarted = FALSE ;
	delStatusBit( Started | PendingRequests | StopDueToError ) ;
	return ret ;
}


// Thread sending function responsible for sending data for 1 event.
// It only calls different sending functions of the ServiceChannel.
static DWORD WINAPI sendThreadFunc( LPVOID arg )
{
	ServiceChannel *ch = (ServiceChannel *)arg ;
	ServiceEvent   *ev = ch->event() ;
	ch->setSending( ) ;

	try
	{
		switch( ev->_event )
		{
			case ServiceEvent::  UnicastMessage	  :
			case ServiceEvent::MulticastMessage :
			case ServiceEvent::BroadcastMessage :
				ch->sendData( ev, MuxPacket::Message ) ;
				break ;

			case ServiceEvent::UnicastFile	  :
				ch->unicastFile  ( ev->_data, ev->_userId) ;
				break ;
			case ServiceEvent::MulticastFile	  :
				ch->multicastFile( ev->_data, (ushort)ev->_channelID) ;
				break ;
			case ServiceEvent::BroadcastFile  :
				ch->broadcastFile( ev->_data) ;
				break ;

			case ServiceEvent::UnicastUpgrade	  :
				ch->unicastFile  ( ev->_data, ev->_userId, MuxPacket::Upgrade) ;
				break ;
			case ServiceEvent::MulticastUpgrade	  :
				ch->multicastFile( ev->_data, (ushort)ev->_channelID, MuxPacket::Upgrade) ;
				break ;
			case ServiceEvent::BroadcastUpgrade  :
				ch->broadcastFile( ev->_data, MuxPacket::Upgrade) ;
				break ;

			case ServiceEvent::  UnicastCATable:
				ch->sendData( ev, MuxPacket::UserLog ) ;
				break;
			case ServiceEvent::MulticastCATable:
				break;
			case ServiceEvent::BroadcastCATable:
				ch->sendData( ev, MuxPacket::UserTable ) ;
				break ;
		}
	}
	catch(...) {}
	ch->clearSending( ) ;
	return 0 ;
}


// Takes one event (from the ServiceManager event queue) and creates a thread
// which in turn sends the event data.
HANDLE ServiceChannel::sendData( ServiceEvent *ev )
{
	ASSERT( !_dataSender->isSending() ) ;
	_ev = ev ;
	DWORD  id ;
	_sendingThread = ::CreateThread( NULL, NULL, sendThreadFunc, this, NULL, &id ) ;
	return _sendingThread ;
}


// This is function which is called from sending thread.
void ServiceChannel::sendData( ServiceEvent *ev, ushort flag )
{
	if( ev->_event & ServiceEvent::Broadcast )
	{
		_dataSender->sendData( flag, ev->_data,
			ev->_dataSize, 0, _setup.streamFormat, _setup.numRebroadcasts, channelPID() ) ;
	}
	else
	if( ev->_event & ServiceEvent::Multicast )
	{
		_dataSender->sendData( flag|MuxPacket::Multicast, ev->_data,
			ev->_dataSize, (ushort)ev->_channelID, _setup.streamFormat, _setup.numRebroadcasts,
			channelPID() ) ;
	}
	else
	{
		_dataSender->sendData( flag|MuxPacket::Unicast, ev->_data,
			ev->_dataSize, 0, _setup.streamFormat, _setup.numRebroadcasts,
			channelPID(), ev->_userId ) ;
	}
}
