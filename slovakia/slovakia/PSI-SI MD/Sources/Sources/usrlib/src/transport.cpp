#include "tools2.hpp"
#include "except.hpp"
#include "transport.hpp"
#include <winsock2.h>
#include "MessageQueue.hpp"
#include "transportPrivate.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning( disable: 4243 )

DWORD WINAPI rcvThreadFnc( LPVOID param ) ;

// WinSocket version required
#define MAJORSOCKVER	1
#define MINORSOCKVER	1

// 4 MB (limit imposed by MSMQ)
#define MAXDATALEN		0x3FFFFF

// -------------- Timeouts --------------

#define DEFAULT_MSMQ_RCV_TIMEOUT	4000

#define DEFAULT_TCP_SEND_TIMEOUT	4000	// must be >= 500
#define DEFAULT_TCP_RCV_TIMEOUT		4000 	// must be >= 500
#define TCP_CONNECT_TIMEOUT			100		// during connection if old socket was not closed yet

// Within this time thread should quit, else it is killed.
// Should be bigger than
//		max(sndTimeout,rcvTimeout)
// Rcv timeout should be bigger than processing in the callback function can take.
#define KILL_RCV_THREAD_TIMEOUT		2000


// ********************************************************************************************
//
//							TransportMediumMsg
//
// ********************************************************************************************


void TransportMediumMsgHdr::setCrc()
{
	// For backward compatibility we clear _dataCrc and reset it back.
	ULONG old_crc = _dataCrc ;
	old_crc = _dataCrc ;
	_flags |= CrcComputed ;
	_crc = crc32( (const uchar*)this, sizeof(TransportMediumMsgHdr)-sizeof(_crc) ) ;
	_dataCrc = old_crc ;
}

BOOL TransportMediumMsgHdr::isHdrValid()
{
	if( _flags & CrcComputed )
		return _crc == crc32( (const uchar*)this, sizeof(TransportMediumMsgHdr)-sizeof(_crc) ) ;

	return  _reserved == 0  &&  _dataLength < MAXDATALEN ;
}

TransportMediumMsg::TransportMediumMsg( DWORD cmd, ULONG dataLen, char *data, DWORD packet_id )
{
	clear() ;
	_cmd			= cmd ;
	_dataLength		= dataLen ;
	_data			= data ;
	setPacketId( packet_id ) ;
}

void TransportMediumMsg::setDataCrc()
{
	if( _dataLength <= 0 )
	{
		if( _flags & DataCrcComputed )
			_flags &= ~DataCrcComputed ;
		else
			return ;
	}
	else
	{
		_dataCrc = crc32( (const uchar*)_data, _dataLength ) ;
		_flags |= DataCrcComputed ;
	}

	if( _flags & CrcComputed )
		setCrc() ;
}

BOOL TransportMediumMsg::isDataValid()
{
	if( _flags & DataCrcComputed )
		return _dataCrc == crc32( (const uchar*)_data, _dataLength ) ;

	return  TRUE ;
}

void TransportMediumMsg::setPacketId( DWORD packet_id )
{
	static DWORD indexCounter = 0 ;	// used to make differences between each sent message

	if( packet_id != -1 )
		_packetId = packet_id ;
	else
	{
		if( ++indexCounter == -1 )
			++indexCounter ;
		_packetId = indexCounter ;
	}
	setCrc() ;
}

TransportMediumMsg::~TransportMediumMsg( )
{
	if( _allocated > 0 )
		free( _data ) ;
}

TransportMediumMsg &TransportMediumMsg::operator= ( const TransportMediumMsg &src )
{
	*(TransportMediumMsgHdr*)this = (TransportMediumMsgHdr&)src ;
	setData( src._cmd, src._data, src._dataLength, src._packetId ) ;
	return *this ;
}

char *TransportMediumMsg::allocData( int length )
{
	if( length == 0 )
	{
		if( _allocated )
		{
			free( _data ) ;
			_allocated = 0 ;
		}
		_data = NULL ;
		return _data;
	}

	if( _allocated >= length )
		return _data ;
	if( _allocated == 0 )
		_data = NULL ;
	_allocated = length ;
	_data = (char*)realloc( _data, _allocated ) ;
	_dataLength = length ;
	setCrc() ;
	return _data ;
}

void TransportMediumMsg::addData( const char *data, int length )
{
	if( !_allocated )
		setData( _cmd, _data, _dataLength ) ;

	int newLen = _dataLength + length ;
	if( _allocated < newLen )
		allocData( newLen ) ;
	else
	{
		_dataLength = newLen ;
		setCrc() ;
	}

	memcpy( _data+_dataLength, data, length ) ;
}

char *TransportMediumMsg::setData( DWORD cmd, const char *data, int length, DWORD packetId )
{
	_cmd = cmd ;
	if( packetId != -1 )
		_packetId = packetId ;
	allocData( length ) ;
	if( _data != NULL  &&  data != NULL )
		memcpy( _data, data, length ) ;
	return _data ;
}

void TransportMediumMsg::setLabel( const char *lbl )
{
	strncpy(_label,lbl,sizeof(_label)) ;
	setCrc() ;
}


// ********************************************************************************************
//
//							TransportMedium
//
// ********************************************************************************************


TransportMedium::TransportMedium( )
{
	_connectString	 = NULL ;
	_isOpen			 = FALSE ;
	_isServer		 = FALSE ;
	_callbackFnc	 = NULL ;
	_callbackFncParam= NULL ;
	_rcvThread		 = NULL ;
	_rcvThreadFnc	 = rcvThreadFnc ;
	_wantDisconnect	 = FALSE ;
	_terminated		 = FALSE ;
	_handshakeFnc	 = NULL ;
	_handshakeParam  = NULL ;

	clearLastError() ;
}

TransportMedium::~TransportMedium()
{
	close() ;
	// don't free _connectString in close() so that reopen is possible
	free( _connectString ) ;
}

BOOL TransportMedium::__open( const char *openString )
{
	_terminated	= FALSE ;

	// Attention: it might be reopen with the same string!
	if( _connectString == NULL )
		_connectString = strdup( openString ) ;
	else
	if( _connectString != openString  &&  stricmp(_connectString,openString) != 0 )
	{
		free( _connectString ) ;
		_connectString = strdup( openString ) ;
	}
	_isOpen = _open( openString) ;
	if( !_isOpen )
		return FALSE ;

	if( _isServer  ||  _handshakeFnc==NULL  ||  _handshakeSendMsg==NULL  ||  !connectionOriented() )
		return TRUE ;

	if( sendData( _handshakeSendMsg) )
	{
		TransportMediumMsg reply ;
		if( receiveData( &reply) )
		{
			if( (*_handshakeFnc)( &reply, _handshakeParam) )
				return TRUE ;
		}
	}
	close() ;
	setLastError( ErrHandshakeFailed ) ;
	return FALSE ;
}

BOOL TransportMedium::openAsClient( const char *openString )
{
	if ( _isOpen )
		return TRUE ;
	_isServer = FALSE ;

	return __open( openString ) ;
}

BOOL TransportMedium::_startRcvThread()
{
	DWORD dw ;
	_rcvThread = CreateThread( NULL, 0, _rcvThreadFnc, this, 0, &dw ) ;
	if( _rcvThread != NULL )
		return TRUE ;

	// if thread not created successfully, close the medium and set the error
	close() ;
	setLastError( ErrUnknownError ) ;
	return FALSE ;
}

BOOL TransportMedium::openAsServer( const char *openString, TransportMediumRcvFun f, LPARAM param )
{
	ASSERT( f != NULL ) ;
	if ( _isOpen )
		return TRUE ;
	_isServer			= TRUE ;
	_callbackFnc		= f ;
	_callbackFncParam	= param ;

	if( __open( openString) )
		_isOpen = _startRcvThread() ;

	return _isOpen ;
}

BOOL TransportMedium::close()
{
	if ( !_isOpen )
		return TRUE ;

	if ( _isServer && _rcvThread )
	{
		// If opened as server and the server thread was created, end it.
		// Send first the flag tested by the server and then wait until it completes.
		_wantDisconnect = TRUE ;

		if ( WaitForSingleObject( _rcvThread, KILL_RCV_THREAD_TIMEOUT/2 ) == WAIT_TIMEOUT )
		{
			prepareKillRcvThread() ;
			if ( WaitForSingleObject( _rcvThread, KILL_RCV_THREAD_TIMEOUT/2 ) == WAIT_TIMEOUT )
				TerminateThread( _rcvThread, 0 ) ;
		}

		CloseHandle( _rcvThread ) ;
		_rcvThread		= NULL ;
		_wantDisconnect	= FALSE ;
	}

	_close() ;
	_isOpen = FALSE ;
	return TRUE ;
}

void TransportMedium::setHandshake( TransportMediumHandshakeFun f,
				LPARAM param, TransportMediumMsg *handshakeSendPacket )
{
	_handshakeFnc		= f ;
	_handshakeParam		= param ;
	_handshakeSendMsg	= handshakeSendPacket ;
}

BOOL TransportMedium::testConnection( const char *openString, char *reason )
{
	if( __open( openString) )
	{
		close() ;
		return TRUE ;
	}
	getErrorString( reason ) ;
	return FALSE ;
}

const char*	TransportMedium::getErrorString( char *buffer, int maxLen ) const
{
	const char *errStr  = NULL ;
	BOOL mediumSpecific = FALSE ;

	switch( _error )
	{
		case ErrNoError:			errStr = "No error" ;	break ;

		case ErrMediumOpenFailed:	errStr = "Open failed" ; mediumSpecific = TRUE ; break ;
		case ErrMediumCloseFailed:	errStr = "Close failed" ; mediumSpecific = TRUE ; break ;
		case ErrMediumSndFailed:	errStr = "Send failed" ; mediumSpecific = TRUE ; break ;
		case ErrMediumRcvFailed:	errStr = "Receive failed" ; mediumSpecific = TRUE ; break ;

		case ErrHandshakeFailed:	errStr = "Handshake failed" ; break ;
		case ErrTimeout:			errStr = "The operation timed out." ; break ;

		case ErrInvalidData:		errStr = "Invalid data." ; break ;
		case ErrInvalidParameter:	errStr = "Invalid parameter." ; break ;
		case ErrInvalidAddress:		errStr = "Bad address" ; break ;
		case ErrUnknownAddress:		errStr = "Invalid address" ; break ;

		case ErrNoMemory:			errStr = "Memory error. Possible not enough memory." ; break ;

		case ErrUnknownError:
		default:					errStr = "Unknown error" ; break ;
	}

	strcpy( buffer, errStr ) ;

	if( mediumSpecific )
	{
		const char *mediumStr = mediumError2String() ;
		int len = strlen( buffer ) ;
		maxLen -= len ;
		if( maxLen > (int)strlen(mediumStr) + 2 )
		{
			buffer[len++] = '\n' ;
			strcpy( buffer+len, mediumStr ) ;
		}
	}

	return buffer ;
}

BOOL TransportMedium::refresh( )
{
	if( !_terminated )
		return TRUE ;
	_terminated = FALSE ;
	if( _rcvThread )
	{
		CloseHandle( _rcvThread ) ;
		_rcvThread		= NULL ;
		_wantDisconnect	= FALSE ;
		_close() ;
		_isOpen = FALSE ;
	}
	return FALSE ;
}


//---------------------------------------------------------------


void TransportMedium::onThreadTerminated()
{
	_terminated = TRUE ;
}

// Server thread always prepared to receive the data
static DWORD WINAPI rcvThreadFnc( LPVOID param )
{
	TransportMedium    *srv     = (TransportMedium*)param ;
	TransportMediumMsg &rcvData = srv->_rcvData ;

	while( !srv->closeRequested() )
	{
		if( !srv->receiveData( &rcvData ) )
		{
			if( srv->error() != ErrTimeout )	// fatal error, exit
			{
				srv->onThreadTerminated() ;
				return -1 ;
			}
		}
		else
		// data received successfully, send to process by the application
		{
			try
			{
				if( srv->closeRequested() )
					break ;

				if( srv->connectionOriented() )
				{
					TransportMediumMsg replyData ;
					if( srv->callback( &rcvData, &replyData) )
					{
						if( srv->closeRequested() )
							break ;
						replyData.setPacketId( rcvData.packetId() ) ;
						srv->sendData( &replyData ) ;
					}
				}
				else
					srv->callback( &rcvData, NULL ) ;
			}
			catch( ... ) {}
		}
	}
	return 0 ;
}


// ********************************************************************************************
//
//							Socket Functions
//
// ********************************************************************************************


//
// Socket functions get SOCKET as input/output parameter and return error code. (0 on success)
//

static DWORD socketOpen( SOCKET &sock, hostent *host, u_short port, BOOL isServer )
{
	// default timeouts = 0 = infinite
	static int sendTimeout    = DEFAULT_TCP_SEND_TIMEOUT ;		// [msec]
	static int receiveTimeout = DEFAULT_TCP_RCV_TIMEOUT ;

	// create socket
	sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
	if ( sock == INVALID_SOCKET )
		return WSAGetLastError() ;

	DWORD err ;
	if ( setsockopt( sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&sendTimeout, sizeof(int) ) != 0 ||
		 setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&receiveTimeout, sizeof(int) ) != 0 )
	{
		err = WSAGetLastError() ;
		goto labelError ;
	}

	struct sockaddr_in sin ;
	memset( &sin, 0, sizeof(sin) ) ;

	// set socket family
	sin.sin_family = AF_INET ;

	// set socket port
	sin.sin_port = htons( port ) ;

	if ( isServer )
	{
		for( int j=0 ; j < 3 ; ++j )
		{
			if( bind( sock, (struct sockaddr*)&sin, sizeof(sin) ) == 0 )
			{
				err = 0 ;
				break ;
			}
			err = WSAGetLastError() ;
			if( err == WSAEADDRINUSE )
				// !!!!!!!!!!
				// Could be the socket is in process of closing; let's give him a chance.
				// This really happens when the socket is closed and immediatelly re-created.
				// Can lead to instable system.
				delay( TCP_CONNECT_TIMEOUT ) ;
			else
				goto labelError ;
		}
		if( err != 0 )
			goto labelError ;
		if( listen( sock, SOMAXCONN ) != 0 )
		{
			err = WSAGetLastError() ;
			goto labelError ;
		}
	}
	else
	{
		ASSERT( host != NULL ) ;
		memcpy( (char*)&sin.sin_addr, host->h_addr, host->h_length ) ;	// set socket address
		if ( ::connect( sock, (struct sockaddr*)&sin, sizeof(sin) ) != 0 )
		{
			err = WSAGetLastError() ;
			goto labelError ;
		}
	}
	return 0 ;

  labelError:
	closesocket( sock ) ;
	sock = INVALID_SOCKET ;
	return err ;
}

static DWORD socketAcceptConnection( SOCKET sock, SOCKET &newSocket )
{
	newSocket = accept( sock, NULL, NULL ) ;
	if ( newSocket == INVALID_SOCKET )
		return WSAGetLastError() ;
	return 0 ;
}

static DWORD socketClose( SOCKET &sock )
{
	if ( sock != INVALID_SOCKET )
	{
		if ( closesocket( sock ) != 0 )
			return WSAGetLastError() ;

		sock = INVALID_SOCKET ;
	}
	return 0 ;
}

static DWORD socketSendData( SOCKET sock, const char* dataPtr, ULONG dataLength )
{
	while ( dataLength > 0 )
	{
		int cnt = send( sock, dataPtr, dataLength, 0 ) ;
		if ( cnt > 0 )
		{
			dataLength -= cnt ;
			dataPtr += cnt ;
		}
		else
		if ( cnt == 0 )
			return WSAECONNRESET ;
		else
			return WSAGetLastError() ;
	}

	return 0 ;
}

static DWORD socketReceiveData( SOCKET sock, char* dataPtr, ULONG dataLength )
{
	while ( dataLength > 0 )
	{
		int cnt = recv( sock, dataPtr, dataLength, 0 ) ;
		if ( cnt > 0 )
		{
			dataLength -= cnt ;
			dataPtr += cnt ;
		}
		else
		if ( cnt == 0 )
			return WSAECONNRESET ;
		else
			return WSAGetLastError() ;
	}

	return 0 ;
}

const char *socketErrorMsg( DWORD err )
{
	#define XXX(code,txt)	case code : msg=txt ; break ;
	const char *msg=NULL ;
	switch( err )
	{
		XXX( WSAEACCES				,"Permission denied." )
		XXX( WSAEADDRINUSE			,"Socket is in use." )
		XXX( WSAEADDRNOTAVAIL		,"Cannot assign requested address." )
		XXX( WSAEALREADY			,"Operation already in progress." )
		XXX( WSAECONNABORTED		,"Connection was aborted." )
		XXX( WSAECONNREFUSED		,"Connection refused by the target machine." )
		XXX( WSAECONNRESET			,"Connection closed by the remote host." )
		XXX( WSAEFAULT				,"Bad data pointer." )
		XXX( WSAEHOSTDOWN			,"Destination host is down." )
		XXX( WSAEHOSTUNREACH		,"Host unreachable." )
		XXX( WSAEINPROGRESS			,"A blocking operation is currently executing." )
		XXX( WSAEINTR				,"A blocking operation was interrupted." )
		XXX( WSAEINVAL				,"Invalid argument." )
		XXX( WSAEISCONN				,"Socket is already connected." )
		XXX( WSAEMFILE				,"Too many open sockets." )
		XXX( WSAEMSGSIZE			,"Message too long." )
		XXX( WSAENETDOWN			,"Network is down." )
		XXX( WSAENETRESET			,"Network dropped connection on reset." )
		XXX( WSAENETUNREACH			,"Network is unreachable." )
		XXX( WSAENOBUFS				,"No buffer space available." )
		XXX( WSAENOTCONN			,"Socket is not connected." )
		XXX( WSAEOPNOTSUPP			,"Operation not supported." )
		XXX( WSAEPROCLIM			,"Too many processes." )
		XXX( WSAETIMEDOUT			,"Connection timed out." )
		XXX( WSAHOST_NOT_FOUND		,"Host not found." )
		XXX( WSA_NOT_ENOUGH_MEMORY	,"Insufficient memory." )
		XXX( WSANOTINITIALISED		,"Sockets not initialized." )
		XXX( WSANO_DATA				,"Valid name but no additional data. (Usu. if requested host unreachable.)" )
		XXX( WSANO_RECOVERY			,"Non-recoverable database error." )
		XXX( WSASYSNOTREADY			,"Network subsystem is unavailable. (Reinstall)" )
		XXX( WSATRY_AGAIN			,"Host not found; try again." )
		XXX( WSAVERNOTSUPPORTED		,"Requested WinSock version not supported." )
		XXX( WSAEDISCON				,"Remote party initiated connection shutdown." )
	}
	return msg ;
}

/*
// Remaining socket errors which seem to be not so important:
WSAPROVIDERFAILEDINIT
WSAEDESTADDRREQ
WSAEAFNOSUPPORT
WSAENOPROTOOPT 
WSASYSCALLFAILURE 
WSAENOTSOCK
WSAEPFNOSUPPORT 
WSAEPROTONOSUPPORT 
WSAEPROTOTYPE 
WSAESHUTDOWN 
WSATYPE_NOT_FOUND 
WSAEWOULDBLOCK
WSA_INVALID_HANDLE 
WSA_INVALID_PARAMETER 
WSAINVALIDPROCTABLE 
WSAINVALIDPROVIDER 
WSA_IO_INCOMPLETE 
WSA_IO_PENDING 
WSA_OPERATION_ABORTED	
WSAESOCKTNOSUPPORT
*/


static const char*	socketMediumError2String( DWORD err )
{
	static char buf[256] ;
	int len = sprintf( buf, "WinSock error %d", err ) ;

	const char *msg = socketErrorMsg( err ) ;
	if( msg != NULL )
		sprintf( buf+len, " (%s)", msg ) ;

	return buf ;
}


// ********************************************************************************************
//
//							SocketTransport
//
// ********************************************************************************************


static void SocketTransportDelFunction( SocketTransport **channel )
{
	delete *channel ;
}

SocketTransport::SocketTransport( const SocketTransport &src ) :
		TransportMedium(), _allSockets(SocketTransportDelFunction)
{
	// used to create socket transport medium server for new connection
	_sock				= INVALID_SOCKET ;
	_callbackFnc		= src._callbackFnc ;
	_callbackFncParam	= src._callbackFncParam ;
	_handshakeFnc		= NULL ;
	_handshakeParam		= NULL ;

	// simulate the successfull open
	_isOpen		= TRUE ;
	_isServer	= TRUE ;
}

SocketTransport::SocketTransport() :
		TransportMedium(), _allSockets(SocketTransportDelFunction)
{
	_sock		= INVALID_SOCKET ;

	// On the server side:
	//   First socket must be listening socket - this is done in newSocketConnectionAcceptThread().
	//   Once the connection request appears, new socket for that request is created
	//	 using copy constructor. Here already standard TransportMedium receiving
	//   procedure will be applied.
	// On the client side no receiving thread is created (i.e. thread function is not used).
	_setRcvThreadFunction( newSocketConnectionAcceptThread ) ;
}

SocketTransport::~SocketTransport()
{
	close() ;
}

BOOL SocketTransport::isConnected( )
{
	if( _sock == INVALID_SOCKET )
		return FALSE ;
	return TRUE ;
}

BOOL SocketTransport::refresh( )
{
	int cnt = _allSockets.count() ;
	for( int j=cnt-1 ; j >= 0 ; j-- )
	{
		SocketTransport *channel = _allSockets[j] ;
		if( !channel->refresh() )
			_allSockets.del( j ) ;
	}
	return TransportMedium::refresh() ;
}

BOOL SocketTransport::decodeConnectString( const char *openString, char *computer, int *port )
{
	*port = 0 ;
	char *portNumber = strchr( openString, ':' ) ;
	if( portNumber != NULL )
		*port = atoi(portNumber+1) ;

	if( *port == 0 )
		return FALSE ;

	int len = portNumber - openString ;
	memcpy( computer, openString, len ) ;
	computer[len] = 0 ;
	return TRUE ;
}

BOOL SocketTransport::_open( const char *open_string )
{
	char computer[256] ;

	// decoded open_string
	ULONG	ipAddress ;
	char	hostName[_MAX_PATH] ;
	BOOL	byIPAddress ;			// TRUE if ipAddress is valid
	int		port ;

	if( !decodeConnectString( open_string, computer, &port) )
	{
		setLastError( ErrInvalidAddress ) ;
		return FALSE ;
	}

	// retrieve the host
	struct hostent *host = NULL ;
	if ( !isServer() )
	{
		// retrieve the host name or IP address
		ipAddress = inet_addr( computer ) ;
		if ( ipAddress == INADDR_NONE )
		{
			byIPAddress = FALSE ;
			strcpy( hostName, computer ) ;
		}
		else
			byIPAddress = TRUE ;

		// retrieve the host info
		if ( byIPAddress )
			host = gethostbyaddr( (char*)&ipAddress, sizeof(ipAddress), AF_INET ) ;
		else
			host = gethostbyname( hostName ) ;

		// test the host
		if ( host == NULL )
		{
			setLastError( ErrUnknownAddress ) ;
			return FALSE ;
		}
	}

	// open socket
	DWORD err = socketOpen( _sock, host, port, isServer() ) ;
	if ( err != 0 )
		setLastError( ErrMediumOpenFailed, err ) ;
	else
		clearLastError() ;

	return err == 0 ;
}

BOOL SocketTransport::_close()
{
	DWORD err = socketClose( _sock ) ;
	if ( err != 0 )
		setLastError( ErrMediumCloseFailed, err ) ;
	else
		clearLastError() ;

	return err == 0 ;
}

void SocketTransport::prepareKillRcvThread()
{
	_close() ;
}


BOOL SocketTransport::isLocalConnection( const char *connectStr )
{
	char computer[256] ;
	int	 port ;
	if( !decodeConnectString( connectStr, computer, &port) )
		return FALSE ;
	if( strcmp(computer,"0.0.0.0") == 0 )
		return TRUE ;

	char  myComputer[1024] ;
	DWORD siz = sizeof(myComputer) ;
	if( !GetComputerName( myComputer, &siz) )
		return FALSE ;
	return stricmp( computer, myComputer) == 0 ;
}

BOOL SocketTransport::isLocalConnection( )
{
	return isLocalConnection( _connectString ) ;
}

void SocketTransport::getPeerLocation( char *locationBuffer ) const
{
	locationBuffer[0] = '\x0';

	char computer[256] ;
	int	 port ;
	if( !decodeConnectString( _connectString, computer, &port) )
		return ;

	strcpy(locationBuffer, computer) ;
}

BOOL SocketTransport::connectStringEqual( const char *newConnectString ) const
{
	char newComputer[256] ;
	int	 newPort ;
	if( !decodeConnectString( newConnectString, newComputer, &newPort) )
		return FALSE ;

	char oldComputer[256] ;
	int	 oldPort ;
	if( !decodeConnectString( _connectString, oldComputer, &oldPort) )
		return FALSE ;

	if( oldPort != newPort )
		return FALSE ;

	if( !isServer() )
	{
		if( stricmp( oldComputer, newComputer) != 0 )
			return FALSE ;
	}
	return TRUE ;
}

BOOL SocketTransport::_sendData( const char* dataPtr, ULONG dataLength )
{
	DWORD err = socketSendData( _sock, dataPtr, dataLength ) ;
	if( err == 0 )
		return TRUE ;

	if( !_isServer )
	if( err == WSAECONNRESET  ||  err == WSAECONNABORTED )
	{
		// Client connection was terminated; try reset
		close() ;
		if( openAsClient( _connectString) )
			err = socketSendData( _sock, dataPtr, dataLength ) ;
	}

	if ( err == WSAETIMEDOUT )
		setLastError( ErrTimeout ) ;					// timeout
	else
	if ( err != 0 )
		setLastError( ErrMediumSndFailed, err ) ;		// error sending
	else
		clearLastError() ;

	return err == 0 ;
}

BOOL SocketTransport::_receiveData( char* dataPtr, ULONG dataLength )
{
	DWORD err = socketReceiveData( _sock, dataPtr, dataLength ) ;
	if ( err == WSAETIMEDOUT )
		setLastError( ErrTimeout ) ;				// timeout
	else
	if ( err != 0 )
		setLastError( ErrMediumRcvFailed, err ) ;	// error receiving
	else
		clearLastError() ;

	return err == 0 ;
}

const char* SocketTransport::mediumError2String() const
{
	return socketMediumError2String( _mediumError ) ;
}

//---------------------------------------------------------------


BOOL SocketTransport::sendData( const TransportMediumMsg *userData )
{
	ASSERT( userData != NULL ) ;

	try
	{
		TransportMediumMsgHdr *hdr = (TransportMediumMsgHdr*)userData ;
		if( !_sendData( (const char*)hdr, sizeof(TransportMediumMsgHdr)) )
			return FALSE ;
		const char *data = (const char *)userData->data() ;
		ULONG       len  = userData->dataLength() ;
		return _sendData( data, len ) ;
	}
	catch( ... )
	{
		setLastError( ErrInvalidParameter ) ;
		return FALSE ;
	}
}

BOOL SocketTransport::receiveData( TransportMediumMsg *userData )
{
	ASSERT( userData != NULL ) ;

	try
	{
		// Receive header
		TransportMediumMsgHdr *hdr = (TransportMediumMsgHdr*)userData ;
		if( !_receiveData( (char*)hdr, sizeof(TransportMediumMsgHdr)) )
			return FALSE ;

		// Check header
		if( !hdr->isHdrValid() )
		{
			userData->setData( 0 ) ;	// clear data
			setLastError( ErrInvalidData ) ;
			return FALSE ;
		}

		// receive data
		int len = userData->dataLength() ;
		userData->allocData( len ) ;
		if(	!_receiveData( (char*)userData->data(), len) )		// receive data
			return FALSE ;

		// Check data
		if( !userData->isDataValid() )
		{
			userData->setData( 0 ) ;	// clear data
			setLastError( ErrInvalidData ) ;
			return FALSE ;
		}

		clearLastError() ;
		return TRUE ;
	}
	catch( ... )
	{
		setLastError( ErrInvalidParameter ) ;
		return FALSE ;
	}
}


//---------------------------------------------------------------

// listening socket thread
// Once a connection request arrives, socket is duplicated and new data thread is created.
static DWORD WINAPI newSocketConnectionAcceptThread( LPVOID param )
{
	SocketTransport *srv = (SocketTransport*)param ;

	while ( !srv->closeRequested() )
	{
		SOCKET newSocket ;
		DWORD err = socketAcceptConnection( srv->_sock, newSocket ) ;
		if ( newSocket == INVALID_SOCKET )
		{
			if ( err == WSAEWOULDBLOCK )
			{
				delay( TCP_CONNECT_TIMEOUT ) ;	// currently no meaning as the socket is blocking
				continue ;
			}
			char buf[1024] ;
			srv->getErrorString( buf ) ;
			TRACE( "\nSocket accept thread unexpectidly terminated:\n%s\n", buf ) ;
			srv->onThreadTerminated() ;
			return -1 ;							// fatal error
		}
		else
		{
			// new server to receive client packets
			SocketTransport *newSrv = new SocketTransport( *srv ) ;

			if ( !newSrv->startReceiveUserPackets( newSocket ) )
				delete newSrv ;
			else
				srv->_allSockets.add( newSrv ) ;
		}
	}

	return 0 ;
}



// ********************************************************************************************
//
//								MSMQTransport
//
// ********************************************************************************************


MSMQTransport::MSMQTransport( ) : TransportMedium()
{
	_sendQueue		= NULL;
	_receiveQueue	= NULL;
	_sendQueue		= new MessageQueue();
	_receiveQueue	= new MessageQueue();

	_timeout		= DEFAULT_MSMQ_RCV_TIMEOUT;
}

MSMQTransport::~MSMQTransport( )
{
	if (_sendQueue)
		delete _sendQueue;
	if (_receiveQueue)
		delete _receiveQueue;
}

BOOL MSMQTransport::connectStringEqual( const char *newConnectString ) const
{
	char fName[MAX_Q_FORMATNAME_LEN];
	MessageQueue::decomposeConnectString( fName, NULL, newConnectString );
	return _stricmp(fName, _queueFormatName) == 0;
}

void MSMQTransport::getPeerLocation( char *locationBuffer ) const
{
	locationBuffer[0] = '\x0';

	// get MSMQ path name
	char path[MAX_Q_PATHNAME_LEN];
	MessageQueue::decomposeConnectString( NULL, path, _connectString) ;

	// extract computer name from path name
	// find '\' character which is end of computer name
	char *lom = strchr(path,'\\') ;
	if (lom==NULL)
		return ;

	*lom = '\x0';

	strcpy(locationBuffer, path) ;
}

BOOL MSMQTransport::_open( const char *openString )
{
	char path[MAX_Q_PATHNAME_LEN];
	MessageQueue::decomposeConnectString(_queueFormatName, path, openString);
	try
	{ 
		if ( _queueFormatName[0] == 0 )
			if ( ! MessageQueue::findFormatName(_queueFormatName, MAX_Q_FORMATNAME_LEN, path) )
			{
				setLastError(ErrMediumOpenFailed, erInvalidPath);
				return FALSE;
			}

		// open sending queue for client and both queues for server
		if ( isServer() )
		{
			if ( ! _receiveQueue->_open(_queueFormatName, MessageQueue::ReceiveAccess) )
			{
				HRESULT medErr = _receiveQueue->getLastResult();
				setLastError(ErrMediumOpenFailed, medErr);
				return FALSE;
			}
		}

		if ( ! _sendQueue->_open(_queueFormatName, MessageQueue::SendAccess) )
		{
			HRESULT medErr = _sendQueue->getLastResult();
			setLastError(ErrMediumOpenFailed, medErr);
			return FALSE;
		}

		return TRUE ;
	}
	catch(...)
	{
		setLastError(ErrMediumOpenFailed, erFailedToLoadLib);
		return FALSE;
	}
}

BOOL MSMQTransport::_close( )
{
	BOOL bSucceed;
	try
	{
		bSucceed = _sendQueue->isOpened() && _sendQueue->close();
		if ( ! bSucceed )
		{
			HRESULT medErr = _sendQueue->getLastResult();
			setLastError(ErrMediumCloseFailed, medErr);
		}

		if ( _receiveQueue->isOpened() && ! _receiveQueue->close() )
		{
			bSucceed = FALSE;
			HRESULT medErr = _receiveQueue->getLastResult();
			setLastError(ErrMediumCloseFailed, medErr);
		}
	}
	catch(...)
	{
		setLastError(ErrMediumCloseFailed, erFailedToLoadLib);
		bSucceed = FALSE;
	}
	return bSucceed;
}

BOOL MSMQTransport::_sendData( const char *dataPtr, ULONG dataLength, const char *label )
{
	try 
	{
		if ( ! _sendQueue->isOpened() )
			if ( ! _sendQueue->_open(_queueFormatName, MessageQueue::SendAccess) )
				return FALSE;

		if ( ! _sendQueue->send((UCHAR*)dataPtr, dataLength, label) )
		{
			HRESULT medErr = _sendQueue->getLastResult();
			setLastError( ErrMediumSndFailed, medErr );
			return FALSE;
		}

		return TRUE;
	} catch(...) 
	{
		setLastError( ErrMediumSndFailed, erFailedToLoadLib );
		return FALSE;
	}
}

BOOL MSMQTransport::sendData( const TransportMediumMsg *x )
{
	TransportMediumMsgHdr *hdr = (TransportMediumMsgHdr*)x ;
	
	const char *label = x->label();
	if ( label == NULL )
		label = "";

	DWORD hdrSize = sizeof(TransportMediumMsgHdr);
	DWORD dataSize = x->dataLength();
	DWORD packetSize =  hdrSize + dataSize ;
	char *srBuffer = new char[packetSize];

	// prepare message body
	memcpy(srBuffer, hdr, hdrSize);
	memcpy(srBuffer + hdrSize, x->data(), dataSize);

	BOOL retVal = _sendData( srBuffer, packetSize, x->label() ) ;
	delete srBuffer;

	return retVal;
}

BOOL MSMQTransport::_receiveData( char* dataPtr, ULONG& dataLength, char *label, BOOL peek )
{
	BOOL bSucceed;
	try 
	{
		if ( ! _receiveQueue->isOpened() )
			if ( ! _receiveQueue->_open(_queueFormatName, MessageQueue::ReceiveAccess) )
				return FALSE;

		if ( peek )
			bSucceed = _receiveQueue->peek((UCHAR*)dataPtr, dataLength, label, _timeout );
		else
			bSucceed = _receiveQueue->receive((UCHAR*)dataPtr, dataLength, label, TRUE, _timeout );

		if ( ! bSucceed )
			if ( _receiveQueue->isLastErrTimeout( ) )
				setLastError( ErrTimeout, 0 );
			else
			{
				HRESULT medErr = _receiveQueue->getLastResult();
				setLastError( ErrMediumRcvFailed, medErr );
			}

		return bSucceed;
	} catch(...) 
	{ 
		setLastError( ErrMediumRcvFailed, erFailedToLoadLib );
		return FALSE; 
	}
}

BOOL MSMQTransport::receiveData	( TransportMediumMsg *x )
{
	TransportMediumMsgHdr *hdr = (TransportMediumMsgHdr*)x ;
	DWORD size;
	char label[MAX_Q_PATHNAME_LEN];
	
	int headerSize = sizeof(TransportMediumMsgHdr);
	size = headerSize;
	do
	{
		// peek message to get header
		_receiveData((char*)hdr, size, label, TRUE) ;
		if ( _receiveQueue->getLastResult() &&			// any error ...
			!_receiveQueue->isLastErrBufferSmall() )	// ... except "buffer too small"
			return FALSE;					//  (We supplied small buffer intentionally.)

		if ( hdr->isHdrValid() )
			break;

		// Remove all invalid messages from the queue
		setLastError( ErrInvalidData ) ;
		x->allocData( size ) ;
		_receiveData( (char*)x->data(), size );			// this way the message gets removed
	} while ( 1 );

	int len = x->dataLength() ;

	size = headerSize + len;
	x->allocData( size ) ;
	char *buffer = (char*)x->data();
	// receive data
	BOOL bReceived = _receiveData( buffer, size, label ) ;

	if (bReceived)
	{
		// fill TransportMediumMsg structure ( header is already filled by peeking message )
		memmove( buffer, buffer + headerSize, len );
		x->allocData( len ) ;
		clearLastError() ;

		// Check data
		if( !x->isDataValid() )
		{
			x->setData( 0 ) ;	// clear data
			setLastError( ErrInvalidData ) ;
			return FALSE ;
		}
	}

	return bReceived ;
}

const char*	 MSMQTransport::mediumError2String( ) const
{
	static char	errBuf[256];

	switch (_mediumError)
	{
		case erFailedToLoadLib:
			strcpy( errBuf, "Failed to load MSMQ library.\nMSMQ probably not installed.");
			break;
		case erInvalidPath:
			strcpy( errBuf, "Invalid queue specification.");
			break;
		default:
			HRESULTasText( _mediumError, errBuf ) ;
	}
	return errBuf;
}


// ********************************************************************************************
//
//							UniversalTransportMedium
//
// ********************************************************************************************


UniversalTransportMedium::UniversalTransportMedium()
{
	_medium		= NULL ;
	_mediumType	= UnknownMedium ;
}

UniversalTransportMedium::~UniversalTransportMedium()
{
	delete _medium ;
}

BOOL UniversalTransportMedium::isLocalConnection( )
{
	return _medium ? _medium->isLocalConnection() : FALSE ;
}

BOOL UniversalTransportMedium::isConnected()
{
	return _medium ? _medium->isConnected() : FALSE ;
}

BOOL UniversalTransportMedium::isOpened() const
{
	return _medium ? _medium->isOpened() : FALSE ;
}

BOOL UniversalTransportMedium::isServer() const
{
	return _medium ? _medium->isServer() : FALSE ;
}

BOOL UniversalTransportMedium::isConnectionOriented( ) const
{
	return _medium ? _medium->connectionOriented() : FALSE ;
}

void UniversalTransportMedium::getPeerLocation( char *locationBuffer ) const
{
	if (_medium)
		_medium->getPeerLocation(locationBuffer) ; 
	else
		locationBuffer[0]='\x0';
}

BOOL UniversalTransportMedium::_openObject( TransportMediumType type )
{
	if ( _medium != NULL )
	{
		if( !_medium->isOpened()  &&  type == _mediumType )
			return TRUE ;
		close() ;
	}

	_mediumType = type ;

	switch ( _mediumType )
	{
		case SocketMedium:
			_medium = new SocketTransport() ;
			break ;
		case MSMQMedium:
			_medium = new MSMQTransport() ;
			break ;
		default:
			ASSERT( 0 ) ;
			return FALSE ;	// the object will remain invalid
	}

	return TRUE ;
}

BOOL UniversalTransportMedium::openAsClient( TransportMediumType type, const char *openString, char *errReason,
				  TransportMediumHandshakeFun f, LPARAM param, TransportMediumMsg *handshakeSendPacket )
{
	if( !_openObject( type) )
		return FALSE ;
	_medium->setHandshake( f, param, handshakeSendPacket ) ;
	if( _medium->openAsClient( openString) )
		return TRUE ;
	if( errReason )
		_medium->getErrorString( errReason ) ;
	close() ;
	return FALSE ;
}

BOOL UniversalTransportMedium::openAsServer( TransportMediumType type, const char *openString, TransportMediumRcvFun f, LPARAM param, char *errReason, BOOL *isMediumModuleInstalled )
{
	if( !_openObject( type) )
		return FALSE ;
	if( _medium->openAsServer( openString, f, param) )
		return TRUE ;
	if( errReason )
	{
		_medium->getErrorString( errReason ) ;
		*isMediumModuleInstalled = _medium->isInstalled();
	}
	close() ;
	return FALSE ;
}

void UniversalTransportMedium::close()
{
	if( _medium != NULL )
	{
		delete _medium ;
		_medium		= NULL ;
		_mediumType	= UnknownMedium ;
	}
}

BOOL UniversalTransportMedium::sendData( const TransportMediumMsg *userData )
{
	if ( _medium == NULL )
	{
		ASSERT(0) ;
		return FALSE ;
	}

	(const_cast<TransportMediumMsg*>(userData))->setDataCrc( ) ;
	return _medium->sendData( userData ) ;
}

BOOL UniversalTransportMedium::receiveData( TransportMediumMsg *userData )
{
	if ( _medium == NULL )
	{
		ASSERT(0) ;
		return FALSE ;
	}

	return _medium->receiveData( userData ) ;
}

BOOL UniversalTransportMedium::hasError() const
{
	return _medium ? _medium->hasError() : FALSE ;
}

BOOL UniversalTransportMedium::hasTimeoutError( ) const
{
	if( !_medium )
		return FALSE ;
	return _medium->error() == ErrTimeout ;
}

const char*	UniversalTransportMedium::errorString( char *buffer, int maxLen ) const
{
	if( _medium == NULL )
		strcpy( buffer, "Transport medium closed" ) ;
	else
		_medium->getErrorString( buffer, maxLen ) ;
	return buffer ;
}

UTM_TYPE UniversalTransportMedium::mediumType( ) const
{
	return _mediumType ;
}

const char* UniversalTransportMedium::mediumType2String( TransportMediumType type )
{
	switch ( type )
	{
		case SocketMedium:	return "TCPIP" ;
		case MSMQMedium:	return "MSMQ" ;
		default:			return NULL ;
	}
}

UTM_TYPE UniversalTransportMedium::mediumName2Type( const char* name )
{
	if( name != NULL )
	{
		if( stricmp( name, "TCPIP" ) == 0 )
			return SocketMedium ;
		else
		if( stricmp( name, "MSMQ" ) == 0 )
			return MSMQMedium ;
	}
	return UnknownMedium ;
}

const char *UniversalTransportMedium::connectString( ) const
{
	return _medium ? _medium->connectString() : NULL ;
}

BOOL UniversalTransportMedium::connectStringEqual( const char *newConnectString ) const
{
	return _medium ? _medium->connectStringEqual( newConnectString) : TRUE ;
}

BOOL UniversalTransportMedium::refresh()
{
	if( _medium == NULL )
		return FALSE ;
	return _medium->refresh() ;
}


//--------------------------------------------------------


// the count of socket usage, how many times you want to initialize sockets
static ULONG socketUsageCount = 0 ;

BOOL UniversalTransportMedium::socketInitialize( char *buf )
{
	if( socketUsageCount == 0 )
	{
		WORD    wVersionRequested = MAKEWORD( MAJORSOCKVER, MINORSOCKVER ) ;
		WSADATA wsaData ;

		int err = WSAStartup( wVersionRequested, &wsaData ) ;

		if( err == 0 )
		{
			if( LOBYTE(wsaData.wVersion) <  MAJORSOCKVER  ||
			  ((LOBYTE(wsaData.wVersion) == MAJORSOCKVER) && HIBYTE(wsaData.wVersion) < MINORSOCKVER) )
			{
				WSACleanup() ;
				err = WSAVERNOTSUPPORTED ;
			}
		}

		if( err != 0 )
		{
			// If WSAStartup() fails, we can't call WSAGetLastError().
			// Instead, error calls must be handled directly.
			switch( err )
			{
				case WSASYSNOTREADY :
					strcpy( buf, "(WSASYSNOTREADY) Network subsystem is not ready" ) ;
					break ;
				case WSAVERNOTSUPPORTED :
					sprintf( buf, "(WSAVERNOTSUPPORTED) WinSocket version %d.%d not supported.", MAJORSOCKVER, MINORSOCKVER ) ;
					break ;
				case WSAEINPROGRESS :
					strcpy( buf, "(WSAEINPROGRESS) A blocking WinSocket 1.1 operation is in progress." ) ;
					break ;
				case WSAEPROCLIM :
					strcpy( buf, "(WSAEPROCLIM) Too many tasks use Windows Sockets." ) ;
					break ;
				default :
					strcpy( buf, "Unknown WinSocket initialization error" ) ;
					break ;
			}
			return FALSE ;
		}
	}

	socketUsageCount++ ;
	return TRUE ;
}

void UniversalTransportMedium::socketDeinitialize()
{
	//ASSERT( socketUsageCount > 0 ) ;
	if ( socketUsageCount == 1 )
		WSACleanup() ;

	socketUsageCount-- ;
}
