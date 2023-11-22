/*
 *	Filename:		comio.cpp
 *
 *	Version:		1.00
 *
 *	Description: implements classes
 *		ComOut,ComInp		- generic i/o
 *		BigComOut,BigComInp - wrapper classes for specialized i/o classes
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"

#include "DvbUser.hpp"
#include "FileIO.hpp"
#include "ClientCfg.hpp"
#include "DrvInterface.hpp"
#include "MfxGlobals.hpp"

//#define SCRTRACE
#include "ScrTrace.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// a = no aliasing
// g = global
// t = fast machine code
// y = no stack frame
//#define OPTOPTIONS	"agty"
//#define OPTOPTIONS	"agt"



BOOL BaseComInp::getUserId( GlobalUserID *glb_id )
{
	if( _inpDriver == NULL )
		return FALSE ;
	return _inpDriver->getUserId( glb_id) ;
}


//-----------------------------------------------------------------------------
//	ServiceReceiver
//	used solelly by ComInp to process Message and UserTable data
//-----------------------------------------------------------------------------


class ServiceReceiver : public DataReceiver
{
  public:
	virtual int processData( ushort flags, uchar *data, int n_bytes, ushort channel ) ;
} ;

int ServiceReceiver::processData( ushort flags, uchar *data, int n_bytes, ushort channel )
{
	DvbClientSetup *cSetup	= MfxClientSetup();
	if( !_isPaused && ( flags & MuxPacket::Message ) )
	{
		char *message = (char *)MALLOC( n_bytes );
		memcpy( message, data, n_bytes );
		MfxPostMessage( EMsg_SRcvrCreateDlg, (long)channel, (long)message );
		cSetup->incNumMessagesAcceptedSuccessfully();
	}
	else
	if( flags & MuxPacket::UserTable )
	{
		BOOL			changed = FALSE;
		sChannelsTable *ct = (sChannelsTable *)data;

		for( int i = 0; i < ct->numChannels; i++ )
		{
			sUserChannel *ch	  = &ct->channels[i] ;
			const char   *oldName = cSetup->getChannelNameByID( ch->channelID );

			if( ( oldName == NULL ) || ( strcmp( oldName, ch->channelName ) != 0 ) )
			{
				// MfxPostMessage( MSG_ChannelNameChanged, , );
				cSetup->setChannelNameByID( ch->channelID, ch->channelName );
				changed = TRUE;
			}
		}
		if( changed )
		{
			CATCH_AND_DISPLAY_EXCEPTION( cSetup->userLog()->save() );
			MfxPostMessage( EMsg_ChannelNamesChanged );
		}
	}
	return 0 ;
}


//-----------------------------------------------------------------------------
//	ComInp
//	Base class for accepting incoming data.
//	Specialized classes are derived from this class which implement data receiving
//  for special case like tcp, DVB...
//-----------------------------------------------------------------------------


// This thread runs infinite loop accepting incoming data.
// The actual implementation is done in workKernel()
static DWORD WINAPI inputThreadFnc( void *param )
{
	ComInp *com = (ComInp *)param ;
	ASSERT( com->isOpened() ) ;
	com->_status = ComInp::Started ;

	int ret ;
	__try
	{
		ret = com->_inpDriver->workKernel() ;
	}
	CATCH_EXCEPTION_CODE
	{
		ret = EXCEPTION_CODE ;
	}

	com->_status = ComInp::Stoped ;
	if( ret )
	{
		//com->close() ;
		char buf[1024];
		const char *msg = DvbEventText( ret, buf );
		MfxPostMessage( EMsg_CommunicationError, 0, msg );
		MessageBox( NULL, msg, "Fatal error", MB_OK | MB_ICONERROR | MB_TOPMOST );
		//MfxAbortApplication();
	}
	return ret ;
}

// Constructor - only simplest data initialization.
ComInp::ComInp()
{
	_hInputThread				= NULL;
	_hUpdateFileRecieversThread	= NULL;
	_updChannels				= NULL;
	_updChannelsIndex			= 0;
	_newUserCA					= FALSE;
	_isStarted					= FALSE;
	_status						= Closed;
	_doFiltering				= TRUE;
	_openHook					= NULL;

	_commandReceiver			= NULL;
	_internetReceiver			= NULL;
	_serviceReceiver			= NULL;
	_serviceChannelFileReceiver	= NULL;
	_upgradeReceiver			= NULL;

	memset( _fileReceivers, 0, USHRT_MAX * sizeof( FileReceiver * ) );

	InitializeCriticalSection( &_acceptDataLock );
	InitializeCriticalSection( &_freceiverLock );
	InitializeCriticalSection( &_userCALock );
}

// open() takes all preparations for receiving data.
// To start the receiving start() must be called in addition.
int ComInp::open( const char *setupStr )
{
	ASSERT( !isOpened() ) ;

	int err = _inpDriver->open( setupStr ) ;
	if( err != 0 )
		return err ;

	GlobalUserID	userId ;
	DvbClientSetup*	cSetup	= MfxClientSetup();
	DvbUserLog*		_userLog= cSetup->userLog();
	char			path[1024], *drive, *dir;

	getUserId( &userId ) ;
	CATCH_AND_DISPLAY_EXCEPTION( _userLog->load( userId ) );
	drive	= cSetup->drive();
	dir		= cSetup->dir();

	setFiltering( !_userLog->isHWFilteringAllowed() );

	// processor for incoming commands
	if( _commandReceiver == NULL )
		_commandReceiver = new CommandReceiver;

	// processor for internet packets
	try
	{
		_internetReceiver = new InternetReceiver;
	}
	catch(...)
	{
		// Exception means there is no possibility to communicate with HNet
		_internetReceiver = NULL ;
	}

	// processor for packets composing program upgrade
	_makepath( path, drive, dir, "Installs", NULL );
	_upgradeReceiver = new InstallsReceiver( path );

	// processor for files transferred via service channel
	_makepath( path, drive, dir, "Received", NULL );
	_serviceChannelFileReceiver = new FileReceiver( 0, path );

	// processor for services (eg. messages, CA-table...)
	// started immediatelly (before start())
	_serviceReceiver = new ServiceReceiver;
	_serviceReceiver->start();
	_serviceReceiver->pause();

	// 1 file receiver per existing channel
	DvbUserLog *userLog = cSetup->userLog();
	for( ushort i = 0; i < userLog->nChannels(); i++ )
	{
		USHORT channel = userLog->channels( i );
		ASSERT( channel != 0x0000 && channel != 0xffff );
		_fileReceivers[channel] = new FileReceiver( channel );
		if ( i != 0 && i % 10 == 0 )
			if( _openHook != NULL )
				(*_openHook)() ;
	}

	_status = Opened;

	// create thread accepting incoming data
	DWORD threadId;
	_hInputThread  = CreateThread( NULL, 0, inputThreadFnc, this, 0, &threadId );
	if( _hInputThread == NULL )
		return FALSE ;
	if( _status != Started )
		Sleep( 100 ) ;

	return 0;
}


// stops accepting incoming data; reverse actions as for open()
int	ComInp::close()
{
	trace( "\nenter ComInp::close()" ) ;
	if( _status == Closed )
	{
		trace( " ... not opened" ) ;
		return 0;
	}

	if( _inpDriver != NULL )
	{
		trace( "\nCall _inpDriver->close( )" ) ;
		_inpDriver->close( ) ;
		trace( "\nReturned _inpDriver->close( )" ) ;
	}

	if( _hInputThread != NULL )
	{
		if( _status == Started )
		{
			_status = AskStop;

			if( WaitForSingleObject( _hInputThread, 500 ) == WAIT_TIMEOUT )
			{
				// do not process any data during shutdown
				BOOL entered=FALSE ;
				for( int j=0 ; j < 100 ; ++j )
				{
					if( TryEnterCriticalSection( &_acceptDataLock) )
					{
						entered = TRUE ;
						break ;
					}
					Sleep( 1 ) ;
				}
				trace( "\nCall TerminateThread( _hInputThread)" ) ;
				TerminateThread( _hInputThread , 0);
				trace( " ... OK" ) ;
				if( entered )
					LeaveCriticalSection( &_acceptDataLock );
				else
				{
					trace( "\nComInp::close() Input thread terminated by force" ) ;
					TRACE( "\nComInp::close() Input thread terminated by force" ) ;
				}
			}
		}
		CloseHandle( _hInputThread );
		_hInputThread = NULL ;
		_status = Stoped;
	}
	if( _hUpdateFileRecieversThread != NULL )
	{
		trace( "\nTerminating _hUpdateFileRecieversThread" ) ;
		_status = AskStop;

		if( WaitForSingleObject( _hUpdateFileRecieversThread, 100 ) == WAIT_TIMEOUT )
		{
			EnterCriticalSection( &_freceiverLock );
			TerminateThread( _hUpdateFileRecieversThread , 0);
			LeaveCriticalSection( &_freceiverLock );
		}
		CloseHandle( _hUpdateFileRecieversThread );
		_hUpdateFileRecieversThread = NULL ;
		trace( "\n_hUpdateFileRecieversThread terminated" ) ;
	}


	trace( "\nDeleting channels" ) ;
	DvbUserLog *userLog = MfxClientSetup()->userLog();
	for( ushort i = 0; i < userLog->nChannels(); i++ )
	{
		ushort channel = userLog->channels( i );
		if( _fileReceivers[channel] != NULL )
		{
			delete _fileReceivers[channel];
			_fileReceivers[channel] = NULL;
		}
	}
	trace( "\nChannels deleted" ) ;

	if( _serviceChannelFileReceiver )
	{
		trace( "\nDeleting _serviceChannelFileReceiver" ) ;
		delete _serviceChannelFileReceiver;
		_serviceChannelFileReceiver = NULL;
		trace( "\n_serviceChannelFileReceiver deleted" ) ;
	}

	if( _upgradeReceiver )
	{
		trace( "\nDeleting _upgradeReceiver" ) ;
		delete _upgradeReceiver;
		_upgradeReceiver = NULL;
		trace( "\nDeleted _upgradeReceiver" ) ;
	}

	if( _serviceReceiver )
	{
		trace( "\nDeleting _serviceReceiver" ) ;
		delete _serviceReceiver;
		_serviceReceiver = NULL;
		trace( "\nDeleted _serviceReceiver" ) ;
	}

	if( _internetReceiver )
	{
		trace( "\nDeleting _internetReceiver" ) ;
		delete _internetReceiver;
		_internetReceiver = NULL;
		trace( "\nDeleted _internetReceiver" ) ;
	}

	if( _commandReceiver )
	{
		trace( "\nDeleting _commandReceiver" ) ;
		delete _commandReceiver;
		_commandReceiver = NULL;
		trace( "\nDeleted _commandReceiver" ) ;
	}

	_status = Closed;
	trace( "\nExit ComInp::close()" ) ;
	return 0;
}


// start accepting data; open() must be called before
BOOL ComInp::start()
{
	if( !isOpened() )
		return FALSE ;
	EnterCriticalSection( &_freceiverLock );

	if( _serviceChannelFileReceiver )
		_serviceChannelFileReceiver->start();

	if( _upgradeReceiver )
		_upgradeReceiver->start();

	if( _serviceReceiver )
		_serviceReceiver->start();

	DvbUserLog *userLog = MfxClientSetup()->userLog();
	for( ushort i = 0; i < userLog->nChannels(); i++ )
	{
		ushort channel = userLog->channels( i );
		if( _fileReceivers[channel] != NULL )
			_fileReceivers[channel]->start();
	}

	_isStarted = TRUE;

	LeaveCriticalSection( &_freceiverLock );
	return TRUE ;
}


// Is some of the data processors receiving data in this moment?
// Simply loop on receivers and ask every one the same question.
BOOL ComInp::isReceiving()
{
	EnterCriticalSection( &_freceiverLock );

	BOOL retval = FALSE;

	DvbUserLog *userLog = MfxClientSetup()->userLog();
	for( ushort i = 0; i < userLog->nChannels(); i++ )
	{
		ushort channel = userLog->channels( i );
		if( _fileReceivers[channel] != NULL  && _fileReceivers[channel]->isReceiving() )
		{
			retval = TRUE;
			break;
		}
	}

	if( retval )
		;
	else
	if( _serviceChannelFileReceiver && _serviceChannelFileReceiver->isReceiving() )
		retval = TRUE;
	else
	if( _upgradeReceiver && _upgradeReceiver->isReceiving() )
		retval = TRUE;
	else
	if( _serviceReceiver && _serviceReceiver->isReceiving() )
		retval = TRUE;

	LeaveCriticalSection( &_freceiverLock );
	return retval;
}


// reverse action to start()
// Data receiving is stopped but the receivers are not destroyed.
void ComInp::stop()
{
	EnterCriticalSection( &_freceiverLock );

	_isStarted = FALSE;
	DvbUserLog *userLog = MfxClientSetup()->userLog();
	for( ushort i = 0; i < userLog->nChannels(); i++ )
	{
		ushort channel = userLog->channels( i );
		if( _fileReceivers[channel] != NULL )
			_fileReceivers[channel]->stop();
	}

	if( _serviceChannelFileReceiver )
		_serviceChannelFileReceiver->stop();

	if( _upgradeReceiver )
		_upgradeReceiver->stop();

	if( _serviceReceiver )
		_serviceReceiver->pause();

	LeaveCriticalSection( &_freceiverLock );
}


//-----------------------------------------------------------------------------
//	ComInp - acceptData & processPacket()
//-----------------------------------------------------------------------------


// actions for driver, must be greater than 0 and less than 184(MUXPACKETSIZE)

static int curruptedPacketCounter = 0;
static int outputDeviceSpecialActionNumber = 0;


#pragma optimize( OPTOPTIONS, on )

// If too many consecutive bad packets, generate clear buffer command.
// If does not help, send reset request.
inline void ComInp::_processCorruptedPacket( )
{
	curruptedPacketCounter++;
	if ( curruptedPacketCounter > 15 )
	{
		outputDeviceSpecialActionNumber++;
		if ( outputDeviceSpecialActionNumber > 3 )
		{
			TRACE( "\nHARD CARD RESET" );
			resynchronize( TRUE );
			outputDeviceSpecialActionNumber = 0;
		}
		else
		{
			TRACE( "\nCARD RESET" );
			resynchronize( FALSE );
		}
		curruptedPacketCounter = 0;
	}
}

BOOL ComInp::_acceptMuxPacket( MuxPacket *mp )
{
	DvbClientSetup *cSetup	= MfxClientSetup();

	cSetup->incPackets();							// increment packet counter

	if( mp->syncByte() == MuxPacketSyncByte )		// Test if packet valid (test SYNC byte)
	{
		if( mp->isFillPacket() )
		{
			if( !mp->isFillPacketOk() )
				return FALSE ;
			cSetup->decPackets();
			return TRUE ;
		}
		else
		if( mp->isCrcAndCheckSumOk() )
		{
			curruptedPacketCounter = 0;
			outputDeviceSpecialActionNumber = 0;

			// update  packet counters
			BOOL is_cmd = mp->isCommandPacket() ;
			if( is_cmd )
				cSetup->decPackets();
			else
				cSetup->incSuccPackets();

			// Packet filtering (if required).
			if( !_doFiltering  ||  filterPacket(mp) )
			{
				if( !is_cmd )
					cSetup->incProcessedPackets();

				// Packet is OK - send for further processing.
				processPacket( mp );
			}
			return TRUE ;
		}
	}
	_processCorruptedPacket() ;
	return FALSE ;
}

void ComInp::_acceptTsData( char *buf, int n_bytes )
{
	static char remainedData[2*TSPACKET_SIZE] ;
	static int  remDataSize = 0 ;

	#define IS_TSPACKET(x)	( ((TsPacket*)(x))->sync==TsPacketSyncByte  &&		\
							  ((TsPacket*)(x))->dataLength() == MUXPACKETSIZE )
	
	// to prevent terminatig thread while data is processed
	EnterCriticalSection( &_acceptDataLock );

	if( remDataSize )
	{
		// Extend remainedData from newly supplied data so that all packets
		// originating from remainedData are covered (if possible).
		int cnt = __min( TSPACKET_SIZE-1, n_bytes ) ;
		memcpy( remainedData+remDataSize, buf, cnt ) ;
		cnt += remDataSize ;

		char *ptr = remainedData ;
		while( 1 )
		{
			if( cnt < TSPACKET_SIZE )
			{
				// New data had less than 
				if( remainedData != ptr )
					memmove( remainedData, ptr, cnt ) ;
				remDataSize = cnt ;
				LeaveCriticalSection( &_acceptDataLock );
				return ;
			}

			TsPacket *tp = (TsPacket*)ptr ;
			if( IS_TSPACKET( tp)  &&  _acceptMuxPacket( &tp->data) )
			{
				int diff = (TSPACKET_SIZE - remDataSize) ;
				n_bytes -= diff ;
				buf     += diff ;
				remDataSize = 0 ;
				break ;
			}

			if( --remDataSize == 0 )
				break ;
			ptr++ ;
			cnt-- ;
		}
	}

	do
	{
		if( n_bytes < TSPACKET_SIZE )
		{
			// incomplete TS packet - save remainder
			remDataSize = n_bytes ;
			memcpy(remainedData, buf, remDataSize ) ;
			break ;
		}

		TsPacket *tp = (TsPacket*)buf ;
		if( IS_TSPACKET( tp)  &&  _acceptMuxPacket( &tp->data) )
		{
			buf += TSPACKET_SIZE ;
			n_bytes -= TSPACKET_SIZE ;
		}
		else				// invalid packet
		{
			buf++ ;
			n_bytes-- ;
			char *ptr = (char *)memchr( buf, TsPacketSyncByte, n_bytes ) ;
			if( ptr == NULL )
				break ;
			int diff = ptr - buf ;
			buf += diff ;
			n_bytes -= diff ;
		}
	}
	while( n_bytes > 0 ) ;

	LeaveCriticalSection( &_acceptDataLock );
}


// Processing of raw data delivered from specialized data receivers.
// Merge new data with the buffer and look for Transport Packets.
// Send packets found for further processing.
void ComInp::_acceptMuxData( char *buf, int n_allBytes )
{
	// to prevent terminatig thread while data is processed
	EnterCriticalSection( &_acceptDataLock );

	MuxPacket *mp;
	int cnt=0;

	while( cnt < n_allBytes )
	{
		mp = (MuxPacket *)( buf + cnt );

		_acceptMuxPacket(mp) ;

		cnt += MUXPACKETSIZE;
	}

	LeaveCriticalSection( &_acceptDataLock );
}


// Check if the packet should be accepted by this user.
BOOL ComInp::filterPacket( MuxPacket *mp )
{
	DvbClientSetup *cSetup	= MfxClientSetup();
	ushort f = mp->flags();
	if( f & (MuxPacket::Unicast | MuxPacket::Internet)  )
	{
		const GlobalUserID &usr_id = cSetup->userID() ;
		if( usr_id != mp->userId()  )
			return FALSE ;				// this unicasted packet is not for this user
		if( f & MuxPacket::Internet )
		{
			if( !cSetup->isHNetAllowed() )
				return FALSE ;			// internet packets is not allowed
		}
	}
	else
	if( mp->channel() != 0 )
	{
		if( !cSetup->hasChannel( mp->channel()) )
			return FALSE ;				// this channel is not allowed
	}
	return TRUE ;
}


// Process (already validated) packet.
void ComInp::processPacket( MuxPacket *p )
{
	ASSERT( _commandReceiver != NULL ) ;

	DvbClientSetup *cSetup	= MfxClientSetup();

	// Internet  packets
	if( p->isInternetPacket() )
	{
		if( _internetReceiver )
		{
			cSetup->incInternetPackets();
			_internetReceiver->push( p );
		}
	}
	else
	// Packets sent via service channel.
	// (For Multicast channel serves only to check the permission; otherwise the channel is ignored)
	// In most cases the packets are sent to specialized packet receivers for further processing.
	if( (p->channel() == 0)  ||  (p->flags() & MuxPacket::Multicast) )
	{
		// 1. alive packet
		if( p->isAliveSignalPacket() )
		{
			// Process data sent in alive packet body:
			// Check whether current user is legal; if not delete all channels.

			UserIDs			*userIDs	= (UserIDs *)p->data();
			GlobalUserID	usrID		= cSetup->userID();
			if( userIDs->fromUser <= usrID  &&  usrID <= userIDs->toUser )
			{
				// see CfgUsersSetup::sendAliveSignal()
				// 4 lines below are hard-copied from that file - should be improved
				static uchar synchronisationBytes[] = { 0xAC, 0xEC, 0x38, 0xF0, 0xF0,
														0xAC, 0xEC, 0x38, 0xF0, 0xF0 };
				const int	userIdsHdrSize		= sizeof(ushort) + 2*sizeof(GlobalUserID) ;
				const int   max_users_in_packet = (MUXDATASIZE - userIdsHdrSize - sizeof(synchronisationBytes)) / sizeof(GlobalUserID) ;
				int  r = userIDs->numOfID;
				if( r > max_users_in_packet )	// else: wrong packet
				{
					BOOL found = FALSE ;
					int  l = 0;
					while( l <= r )
					{
						int m = ( l + r ) / 2;
						if( userIDs->ids[m] == usrID )
						{
							found = TRUE ;
							break ;
						}
						if( userIDs->ids[m] < usrID )
							l = m + 1;
						else
							r = m - 1;
					}
					if( !found )
					{
						// Current user not listed - all channels will be deleted
						EnterCriticalSection( &_freceiverLock );
						DvbUserLog *userLog = cSetup->userLog();
						for( ushort i = 0; i < userLog->nChannels(); i++ )
						{
							ushort channel = userLog->channels( i );
							if( _fileReceivers[channel] != NULL )
							{
								//if( _fileReceivers[channel]->isStarted() )
								//	_fileReceivers[channel]->stop();
								delete _fileReceivers[channel];
								_fileReceivers[channel] = NULL;
							}
						}
						LeaveCriticalSection( &_freceiverLock );

						if( userLog->reset() )
							CATCH_AND_DISPLAY_EXCEPTION( userLog->save() );
					}
				}
			}
			MfxPostMessage( EMsg_AliveSignal );
		}
		else
		// 2. User log packet
		if( p->isUserLogPacket() )
		{
			static char *userCASig = USERCASIGNATURE;

			int sigLength = strlen( userCASig );
			char *data = (char *)p->data() + MUXDATASIZE - sigLength - sizeof( UnicastUserID );
			if( memcmp( data, userCASig, sigLength ) == 0 )
				updateUserCA( (sUserCA *)p->data() );
		}
		else
		// 3. upgrade packet
		if( p->isUpgradePacket() )
		{
			_upgradeReceiver->push( p );
		}
		else
		// 4. file packet
		if( p->isFilePacket() )
		{
			_serviceChannelFileReceiver->push( p );
		}
		else
		// 5. command packet
		if ( p->isCommandPacket() )
		{	// only statistics packets can be received as command packets
			if( !_inpDriver->processCommandPacket( p) )
				_commandReceiver->push( p );
		}
		else
		// 6. remaining service packets
		if( p->isMessagePacket() || p->isUserTablePacket() )
		{
			_serviceReceiver->push( p );		// message or permission table
		}
	}
	else
	// Remaining packets must be file packets sent via other channels.
	// They are sent to particular file receiver.
	{
		// just for surity
		if( p->isFilePacket() )
		{
			ushort channel = p->channel();

			EnterCriticalSection( &_freceiverLock );
			if( _fileReceivers[channel] != NULL )
				_fileReceivers[channel]->push( p );
			LeaveCriticalSection( &_freceiverLock );
		}
	}
}

#pragma optimize( "", on )			// restore original optimization options


//-----------------------------------------------------------------------------
//	ComInp::updateUserCA()
//	called from ComInp::processPacket() on receiving user log packet:
//	If some change in user permissions is detected, then
//	updateFileRecievers() is started in separate thread (not to block next packets).
//-----------------------------------------------------------------------------


static int cmpChannels( const void *e1, const void *e2 )
{
	ushort elem1 = *((ushort *)e1);
	ushort elem2 = *((ushort *)e2);

	if( elem1 < elem2 )
		return -1;
	else if( elem1 > elem2 )
		return 1;
	return 0;
}

	
//  Processing:
//  - delete channels which are not allowed in new permissions
//  - create channels which are newly allowed in new permissions
int ComInp::updateFileRecievers()
{
	FileReceiver	*fileReceiver;
	ushort			channel;

	// first eliminate operation with no effect, like add and delete on the same channel
	// or delete then add the same channel
	_updChannels->sort( cmpChannels );
	ushort i = 0;
	ushort j = 1;
	while( j < _updChannels->count() )
	{
		if( _updChannels->item( i ) == _updChannels->item( j ) )
		{
			_updChannels->del( i );
			_updChannels->del( i );
		}
		else
		{
			i++;
			j++;
		}
	}

	DvbUserLog *userLog = MfxClientSetup()->userLog();
	for( i = 0; i < _updChannels->count(); i++ )
	{
		channel = _updChannels->item( i );
		if( userLog->hasChannel( channel ) )
		{
			if( _fileReceivers[channel] == NULL )
			{
				fileReceiver = new FileReceiver( channel );
				if( _isStarted )
					fileReceiver->start();
				EnterCriticalSection( &_freceiverLock );
				_fileReceivers[channel] = fileReceiver;
				LeaveCriticalSection( &_freceiverLock );
			}
		}
		else
		{
			if( _fileReceivers[channel] != NULL )
			{
				fileReceiver = _fileReceivers[channel];

				EnterCriticalSection( &_freceiverLock );
				_fileReceivers[channel] = NULL;
				LeaveCriticalSection( &_freceiverLock );

				if( fileReceiver->isStarted() )
					fileReceiver->stop();
				delete fileReceiver;
			}
		}

		if( _status == AskStop )
		{
			_updChannels->clearList();
			return 1;
		}
	}
	_updChannels->clearList();

	return 0;
}

	
static DWORD WINAPI updateFileRecieversThreadFnc( void *param )
{
	ComInp	*com = (ComInp *)param;
	BOOL	quit = FALSE;
	int		ret;

	while( !quit )
	{
		// Processing of data transferred in the alive packet body.
		ret = com->updateFileRecievers();

		if( ret == 0)
		{
			EnterCriticalSection( &com->_userCALock );

			// Additional processing of new CA-table.
			if( com->_newUserCA )
			{
				com->_updChannelsIndex = 1 - com->_updChannelsIndex;
				com->_updChannels = &com->_updatedChannels[com->_updChannelsIndex];
				com->_newUserCA			= FALSE;
			}
			else
				quit = TRUE;

			LeaveCriticalSection( &com->_userCALock );

		}
		else
			quit = TRUE;
	}
	return 0;
}

// Based on the results of updateCA() file receivers are updated (if something changed).
// Processes 1 packet at a time (which can be incomplete information).
void ComInp::updateUserCA( sUserCA *userCA )
{
	EnterCriticalSection( &_userCALock );

	// Basic check. A failure would be a fatal error.
	DvbClientSetup *cSetup	= MfxClientSetup();
	if( !cSetup->userLog()->isUserCAOk( userCA ) )
		return;

	// If the processing thread is not running, start it.
	// Because the thread is blocked by the same critical section, it is efectivelly
	// waiting until next call to updateCA() (bellow) completes.
	// I.e. the thread processes the results of updateCA().
	if( _hUpdateFileRecieversThread == NULL || WaitForSingleObject( _hUpdateFileRecieversThread, 100 ) == WAIT_OBJECT_0 )
	{
		if( _hUpdateFileRecieversThread )
			::CloseHandle( _hUpdateFileRecieversThread );

		if( cSetup->userLog()->updateCA( userCA, &_updatedChannels[_updChannelsIndex] ) )
		{
			_updChannels = &_updatedChannels[_updChannelsIndex];

			DWORD dw;
			_hUpdateFileRecieversThread  = ::CreateThread( NULL, 0, updateFileRecieversThreadFnc, this, 0, &dw );
			if( _hUpdateFileRecieversThread == NULL )
				MfxPostMessage( EMsg_ChannelsUpdateFailed );
		}
		else
			_hUpdateFileRecieversThread = NULL;
	}
	// (Next packet) If the thread is stil running then updateCA() is called immediatelly
	// to pass the results to the running thread.
	else
	{
		int i = 1 - _updChannelsIndex;
		if( cSetup->userLog()->updateCA( userCA, &_updatedChannels[i] ) )
			_newUserCA = TRUE;
	}
	if( hasCapability(ComIO_CanDoHWFiltering) )
		_doFiltering = !cSetup->isHWFilteringAllowed();

	LeaveCriticalSection( &_userCALock );
	CATCH_AND_DISPLAY_EXCEPTION( cSetup->userLog()->save() );
}


//-----------------------------------------------------------------------------
//	ComInp::synchUserCAWithCard()
//	used by CommandReceiver on getting DVB_SETCAUSER command
//-----------------------------------------------------------------------------


// Processing identical to updateUserCA except synchChannelsWithCard() is called instead of
// updateCA().
void ComInp::synchUserCAWithCard( void *bitmap )
{
	EnterCriticalSection( &_userCALock );

	DvbClientSetup *cSetup	= MfxClientSetup();
	if( _hUpdateFileRecieversThread == NULL || WaitForSingleObject( _hUpdateFileRecieversThread, 100 ) == WAIT_OBJECT_0 )
	{
		if( _hUpdateFileRecieversThread )
			::CloseHandle( _hUpdateFileRecieversThread );

		if( cSetup->userLog()->synchChannelsWithCard( bitmap, &_updatedChannels[_updChannelsIndex] ) )
		{
			_updChannels = &_updatedChannels[_updChannelsIndex];
			_hUpdateFileRecieversThread  = ::CreateThread( NULL, 0, updateFileRecieversThreadFnc, this, 0, NULL );
			if( _hUpdateFileRecieversThread == NULL )
				MfxPostMessage( EMsg_ChannelsUpdateFailed );
		}
		else
			_hUpdateFileRecieversThread = NULL;
	}
	else
	{
		int i = 1 - _updChannelsIndex;
		if( cSetup->userLog()->synchChannelsWithCard( bitmap, &_updatedChannels[i] ) )
			_newUserCA = TRUE;
	}
	if( hasCapability(ComIO_CanDoHWFiltering) )
		_doFiltering = !cSetup->isHWFilteringAllowed();

	LeaveCriticalSection( &_userCALock );
	CATCH_AND_DISPLAY_EXCEPTION( cSetup->userLog()->save() );
}


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

	if( _connectString == NULL  ||  isStrEmpty(_connectString) )
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
	trace( "\nEnter BigComOut::close()" ) ;
	if( _com != NULL )
	{
		char expl[1024] ;
		trace( "\ncallDriverIoctl( DVBDRV_DelComOut)" ) ;
		callDriverIoctl( expl, DVBDRV_DelComOut, (long)_com ) ;
		_com = NULL ;
	}
	trace( "\nExit BigComOut::close()" ) ;
}

BOOL BigComOut::getDrvProperties( DvbDriverSrvProps *props )
{
	char expl[1024] ;
	return callDriverIoctl( expl, DVBDRV_DriverProps, (long)props, TRUE ) ;
}


//----------------------------------------------------------------------


// Depending on the analysis of the connectString specialozed ComInp is created.
int BigComInp::open( BaseConfigClass *cfg, void (*hook)() )
{
	int comErr = 0 ;

	_com = NULL ;
	_com = new ComInp() ;
	BaseInpDriver *drv=NULL ;
	try
	{
		char expl[1024] ;
		callDriverIoctl( expl, DVBDRV_NewComInp, (long)&drv, (long)_com, (long)cfg ) ;

		if( drv != NULL )
			_com->_inpDriver = drv ;
		else
		{
			comErr = DvbErr_LoadDriverDll ;
			AfxMessageBox( expl ) ;
		}
	}
	catch( int err )
	{
		comErr = err ;
	}

	if( drv != NULL )
	{
		_com->setOpenHook( hook ) ;
		comErr = _com->open( _connectString ) ;
	}

	_openFailed = comErr ;
	return comErr ;
}

void BigComInp::close()
{
	if( _com == NULL )
		return ;

	BaseInpDriver *drv = _com->_inpDriver ;
	trace( "\nEnter BigComInp::close()" ) ;
	delete _com ;
	trace( "\nBigComInp::close() delete _com OK " ) ;
	callDriverIoctl( NULL, DVBDRV_DelComInp, (long)drv) ;
	trace( "\nExit BigComInp::close()" ) ;
	_com = NULL ;
}

BOOL BigComInp::hasCapability( ComIOCapability cap )
{
	if( _com == NULL )
		return FALSE ;
	return _com->hasCapability( cap ) ;
}

BOOL BigComInp::getDrvProperties( DvbDriverRcvProps *props )
{
	char expl[1024] ;
	return callDriverIoctl( expl, DVBDRV_DriverProps, (long)props, FALSE ) ;
}

BOOL BigComInp::getStatisticsLog( char *txt )
{
	if( _com == NULL )
		return FALSE ;
	return _com->getStatisticsLog( txt ) ;
}

sDllDialog *BigComInp::createStatisticsPage()
{
	if( _com == NULL )
		return NULL ;
	return _com->createStatisticsPage() ;
}

void BigComInp::clearStatistics( )
{
	if( _com != NULL )
		_com->clearStatistics() ;
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

	if( strstr(buf,"tcp") != NULL  ||  strstr(buf,"udp") != NULL )
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
	if( strstr(buf,"adp") != NULL )
	{
		dllName = "IoAdp.dll" ;
	}
	else
	if( strstr(buf,"skymedia200") != NULL )
	{
		dllName = "IoSkyMedia200.dll" ;
	}
	else
	if( strstr(buf,"skymedia") != NULL )
	{
		dllName = "IoSkyMedia.dll" ;
	}
	else
	if( strstr(buf,"skystar")!=NULL || strstr(buf,"ttdvb")!=NULL )
	{
		dllName = "IoSkyStar.dll" ;
	}
	else
	if( strstr(buf,"mpe") != NULL )
	{
		dllName = "IoMPE.dll" ;
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

	if( isTestConnection )
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

	expl[0] = 0 ;
	return TRUE ;
}


static BOOL MfxMessageHook( UINT msg, long wParam, long lParam )
{
	return MfxPostMessage( msg, wParam, lParam ) ;
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
	if( GetProgramLevel() == 0 )
	{
		if( dllName != NULL  &&  !isTestConnection )
		{
			strcpy( expl, "Illegal connection in the Demo mode" ) ;
			return FALSE ;
		}
	}
	if( dllName == NULL )
	{
		DvbEventText( DvbErr_UnknownComDevice, expl ) ;
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
		char msgBuf[512] ;
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
