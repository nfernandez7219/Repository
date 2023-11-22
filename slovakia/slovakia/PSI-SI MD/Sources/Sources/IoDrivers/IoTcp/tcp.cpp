//
// This file implements following classes:
//		TcpInpDriver, ComOutTcp
//

#include "tools2.hpp"
#include "tcp.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// WinSocket version required
#define MAJORSOCKVER	1
#define MINORSOCKVER	1


const char *tcpEventAsText( int code, char *buf )		// max 256 chars
{
	if( code == 0 )
	{
		strcpy( buf, "success" ) ;
		return buf ;
	}

	const char *msg=NULL ;
	switch( code )
	{
		case TcpErr_Open:				msg = "Socket initialization failed." ; break ;
		case TcpErr_UnknownHost:		msg = "Unknown host." ; break ;
		case TcpErr_UnknownService:		msg = "Unknown service." ; break ;
		case TcpErr_UnknownProtocol:	msg = "Unknown protocol. (Must be 'tcp' or 'udp'.)" ; break ;
		case TcpErr_CreateSocketFailed:	msg = "Create socket failed." ; break ;
		case TcpErr_ConnectSocketFailed:msg = "Connect socket failed." ; break ;
		case TcpErr_BindSocketFailed:	msg = "Bind socket failed." ; break ;
		case TcpErr_SocketError :		msg = "Socket error." ; break ;
		default :						msg = "Unknown tcp error" ; break ;
	}
	strcpy( buf, msg ) ;
	return buf ;
}


//-----------------------------------------------------------------------------
//	Socket utilities
//-----------------------------------------------------------------------------


static BOOL socketsOpened=FALSE ;

static BOOL sockConnectionStartup()
{
	if( socketsOpened == FALSE )
	{
		WORD wVersionRequested = MAKEWORD( MAJORSOCKVER, MINORSOCKVER ) ;
		WSADATA wsaData;
 
		if ( WSAStartup(wVersionRequested,&wsaData) )
			return FALSE;
		
		// Confirm that the WinSock DLL supports 2.0.
		if ( LOBYTE(wsaData.wVersion) != MAJORSOCKVER ||
				HIBYTE(wsaData.wVersion) != MINORSOCKVER )
		{
			WSACleanup( );
			return FALSE; 
		}
		socketsOpened = TRUE ;
	}
	return TRUE;
}


static void sockConnectionCleanup()
{
	if( socketsOpened == TRUE )
	{
		socketsOpened = FALSE ;
		WSACleanup ();
	}
}

static const char *reportWinSockError()
{
	#define XXX(code,txt)	case code : msg=txt ; break ;

	const char *msg=NULL ;
	static char buf0[40] ;

	int err = WSAGetLastError() ;
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
		default:
			msg = buf0 ;
			sprintf( buf0, "Error %d", err ) ;
	}
	return msg ;
}


//-----------------------------------------------------------------------------
//	UdpInpQueue
//	Queue used on the input side for udp connection.
//	Needed because udp buffer (if any?) is too small to compensate for the
//	fluctuations.
//	Currently fixed 1MB buffer is used, but the tests show that under increased
//	processor load overflows happen.
//	Introducing of the buffered input increased udp reliability from some 50%
//	to approx. 99.95%. Errors which remain seem to originate outside of Server/
//	Receiver programs.
//-----------------------------------------------------------------------------


class UdpInpQueue : public CWinThread
{
	char	   *_buffer ;		// buffer for packets
	int			_maxBytes ;		// buffer size in Bytes
	int			_inPtr ;		// index of the next packet to be put
	int			_outPtr ;		// index of the next packet to be written
	BaseComInp *_baseComInp ;
	HANDLE		_eventKill;		// SetEvent() to kill this process

  public:
	inline int nData()		{ return (_inPtr+_maxBytes-_outPtr) % _maxBytes ; }
	BOOL put( const char *buf, int n_bytes ) ;

	inline void shutdown( )
	{
		VERIFY( SetEvent(_eventKill) );
		SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
		if( WaitForSingleObject( m_hThread, 500 ) == WAIT_TIMEOUT )
			TerminateThread( m_hThread, 0 );
	}
	UdpInpQueue( BaseComInp *out ) ;
   ~UdpInpQueue() ;

  protected:
	virtual BOOL InitInstance();
};

UdpInpQueue::UdpInpQueue( BaseComInp *out )
{
	_inPtr		 = 0 ;
	_outPtr		 = 0 ;
	_maxBytes	 = 5000*188 ;	// 5000 TS packets
	_buffer		 = (char*)malloc( _maxBytes ) ;
	m_bAutoDelete= FALSE;
	_baseComInp  = out ;
	_eventKill	 = CreateEvent(NULL, TRUE, FALSE, NULL);
}

UdpInpQueue::~UdpInpQueue()
{
	free( _buffer ) ;
	CloseHandle( _eventKill);
}

BOOL UdpInpQueue::put( const char *buf, int n_bytes )
{
	BOOL ret ;
	int nFree = _maxBytes - nData() - 1 ;
	if( nFree < n_bytes )
	{
		ret = FALSE ;
		TRACE( "\nUdp OVF" ) ;
	}
	else
	{
		int end = _inPtr+n_bytes ;
		if( end < _maxBytes )
		{
			memcpy( _buffer+_inPtr, buf, n_bytes ) ;
		}
		else
		{
			int len1 = _maxBytes-_inPtr ;
			memcpy( _buffer+_inPtr, buf, len1 ) ;
			memcpy( _buffer, buf+len1, n_bytes-len1 ) ;
		}
		_inPtr = end % _maxBytes ;

		static max_used=10 ;
		int n_used = nData()/1024 ;
		if( max_used < n_used )
		{
			if( max_used/50 != n_used/50 )
				TRACE( "\nUdp buffer uses %dK", max_used ) ;
			max_used = n_used ;
		}
		ret = TRUE ;
	}
	return ret ;
}

BOOL UdpInpQueue::InitInstance()
{
	int cnt=0 ;
	while( WaitForSingleObject( _eventKill, cnt ? 0 : 1) == WAIT_TIMEOUT )
	{
		cnt = nData() ;
		if( cnt > 0 )
		{
			if( _inPtr < _outPtr )
				cnt = _maxBytes - _outPtr ;
			_baseComInp->_acceptTsData( _buffer+_outPtr, cnt ) ;
			_outPtr = (_outPtr + cnt) % _maxBytes ;
		}
	}
	return FALSE;		// avoid entering standard message loop by returning FALSE
}


//-----------------------------------------------------------------------------
//	TcpInpDriver
//-----------------------------------------------------------------------------


#define stopRequested()	_baseComInp->stopRequested()


TcpInpDriver::TcpInpDriver( BaseComInp *baseComInp )
{
	_baseComInp = baseComInp ;
	_sock = INVALID_SOCKET ;
	_port = 0 ;
	_proto= 0 ;
	out_sock = INVALID_SOCKET ;
	if( !sockConnectionStartup() )
		throw (int)TcpErr_Open ;
}

TcpInpDriver::~TcpInpDriver ()
{
	if( _sock != INVALID_SOCKET )
		closesocket( _sock ) ;
	sockConnectionCleanup() ;
}

BOOL TcpInpDriver::getUserId( GlobalUserID *id )
{
	*id = _setup._userId ;
	return TRUE ;
}

int TcpInpDriver::close()
{

	// make socket nonblocking ... does not work
	//ULONG par ;
	//if( ioctlsocket( _sock, FIONBIO, &par) != 0 )
	//	reportWinSockError();

	// needed to terminate listening socket which would otherwise block infinitelly
	if( _sock != INVALID_SOCKET )
	{
		shutdown( _sock, SD_BOTH ) ;
		closesocket( _sock ) ;
		_sock = INVALID_SOCKET ;
	}

	// needed to terminate recv() which would otherwise block infinitelly
	if( out_sock != INVALID_SOCKET )
		shutdown( out_sock, SD_BOTH ) ;
	return 0 ;
}


// Open & bind socket for given connection string.
// connectStr =	"123/tcp"		... port 123, using TCP
//				"456/udp"		... port 456, using udp
//				"xyz"			... port & protocol taken from etc\services
//
int TcpInpDriver::open( const char *connectStr )
{
	const char *syntaxPrompt = 
		"    connectString = port/tcp\n"
		"        or\n"
		"    connectStr = service\n"
		"\n"
		"where\n"
		"   - port       \t is the same number (1..65535) as used by the Server\n"
		"   - service \t is the name defined e.g. in system...\\etc\\services\n"
		"\n"
		"Example:\n"
		"  12345/tcp" ;

	// decode connectStr
	char buf[1024] ;
	strcpy( buf, connectStr ) ;

	{
		int  syntaxErr=0 ;
		char syntaxErrBuf[256] ;

		char *protocol=NULL ;
		char *service = buf ;
		char *s = strchr( buf, '/' ) ;
		if( s != NULL )
		{
			*s = 0 ;
			protocol = s+1 ;
		}

		_port = atoi( service ) ;
		if( _port > 0 )							// service = port (e.g. "123")
		{
			_port = htons((u_short)_port);
		}
		else
		{
			struct servent *se = getservbyname( service, NULL ) ;
			if( se == NULL )
			{
				strcpy( syntaxErrBuf, "Unknown service name used." ) ;
				syntaxErr = TcpErr_UnknownService ;
				goto labelSyntaxError ;
			}
			_port = se->s_port ;
			protocol= se->s_proto ;
		}

		_proto = IPPROTO_TCP ;
		if( protocol != NULL )
		{
			struct protoent *pe = getprotobyname( protocol ) ;
			if( pe == NULL )
			{
				strcpy( syntaxErrBuf, "Unknown protocol name used." ) ;
				syntaxErr = TcpErr_UnknownProtocol ;
				goto labelSyntaxError ;
			}
			_proto = pe->p_proto ;
		}

	  labelSyntaxError:
		if( syntaxErr != 0 )
		{
			sprintf( buf, "Error in tcp connect string (\"%s\"):\n%s\n      -----------\n\nUse following syntax:\n\n%s",
				connectStr, syntaxErrBuf, syntaxPrompt ) ;
			::MessageBox( NULL, buf, "Error", MB_OK|MB_ICONERROR ) ;
			return syntaxErr ;
		}
	}

	// create socket
	int connectErr = 0 ;

	int type = (_proto==IPPROTO_TCP ? SOCK_STREAM : SOCK_DGRAM) ;
	_sock = socket( AF_INET, type, _proto ) ;
	if( _sock == INVALID_SOCKET )
		connectErr = TcpErr_CreateSocketFailed ;
	else
	{
		struct sockaddr_in sin ;		// INADDR_ANY
		memset( &sin, 0, sizeof(sin) ) ;
		sin.sin_family = AF_INET ;
		sin.sin_port   = _port ;
		if( bind( _sock, (struct sockaddr*)&sin, sizeof(sin)) != 0 )
		{
			closesocket( _sock ) ;
			_sock = INVALID_SOCKET ;
			connectErr = TcpErr_BindSocketFailed ;
		}
	}

	if( connectErr != 0 )
	{
		const char *sockErrText = reportWinSockError();
		sprintf( buf,
			"connectString = %s\n"
			"\n"
			"Failed to open listening socket.\n"
			"Error announced by the system:\n"
			"    \"%s\"",
			connectStr, sockErrText ) ;

		::MessageBox( NULL, buf, "Error", MB_OK|MB_ICONERROR ) ;
	}
	return connectErr ;
}


// Work kernel for tcp connection
// Only single connection at a time accepted. (It's only a demo, anyway.)
// Can be terminated by either stopRequested() or by socket error.
void TcpInpDriver::_workTCP()
{
	char buf[5000] ;
	while( !stopRequested() )
	{
		// Problem!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// Blocking socket ... does not finish until connection made.
		// Under debugger: If Server is not running, then some dead lock occurs
		// when terminating the thread.
		// Should be changed to non-blocking socket, but don't know how...
		if( listen( _sock, SOMAXCONN) != 0 )
			break;
		while( !stopRequested() )
		{
			struct sockaddr adr ;
			int    adr_len = sizeof(adr);

			out_sock= accept( _sock, &adr, &adr_len ) ;		// (...,NULL,NULL)
			if( out_sock == INVALID_SOCKET )
			{
				int err = WSAGetLastError() ;
				if( err == WSAEWOULDBLOCK )		// timeout for nonblocking socket
					continue ;
				break ;			// error
			}

			int     cnt ;
			while( (cnt = recv( out_sock, buf, sizeof(buf), 0)) != 0 && !stopRequested() )
			{
				if( cnt > 0 )
					_baseComInp->_acceptTsData( buf, cnt ) ;
				else
				if( cnt == SOCKET_ERROR )
				{
					closesocket( out_sock ) ;
					return ;		// error
				}
				else
					break ;			// connection closed
			}
			closesocket( out_sock ) ;
			out_sock = INVALID_SOCKET ;
		}
	}
}

//#include "IpHlpApi.h"	//GetUdpStatistics()

// Work kernel for udp connection
void TcpInpDriver::_workUDP()
{
	char buf[20000] ;
	UdpInpQueue queue( _baseComInp ) ;
	queue.CreateThread( ) ;

	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
	while( !stopRequested() )
	{
		struct sockaddr fromAdr ;
		int fromlen = sizeof(fromAdr) ;
		int cnt = recvfrom( _sock, buf, sizeof(buf), 0, &fromAdr, &fromlen ) ;
		if( cnt > 0 )
			queue.put( buf, cnt ) ;
		else
		if( cnt < 0 )
			break ;				// error
		//else
		//if( cnt == 0 )		// connection gracefully closed
		//	break ;

		/*
		static int x=0 ;
		static int out0=0, in0=0 ;
		if( x == 0 )
		{
			MIB_UDPSTATS udp ;
			if( GetUdpStatistics( &udp) != NO_ERROR )
				return ;
			out0 = udp.dwOutDatagrams ;
			in0  = udp.dwInDatagrams ;
		}
		x++ ;
		if( x % 1000 == 0 )
		{
			MIB_UDPSTATS udp ;
			if( GetUdpStatistics( &udp) != NO_ERROR )
				return ;
			int sent = udp.dwOutDatagrams - out0 ;
			int rcv  = udp.dwInDatagrams - in0 ;
			TRACE( "\nUdp: Sent %d, Received %d, Lost %d", sent, rcv, sent-rcv ) ;
		}
		*/
	}

	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );
	queue.shutdown() ;
}

int TcpInpDriver::workKernel()
{
	if( _proto == IPPROTO_TCP )
		_workTCP() ;
	else
		_workUDP() ;

	// Thread finished
	if( stopRequested() )
		return 0 ;
	reportWinSockError() ;			// this is just for testing to put the break point here
	return TcpErr_SocketError ;
}


//-----------------------------------------------------------------------------
//	ComOutTcp
//-----------------------------------------------------------------------------


ComOutTcp::ComOutTcp()
{
	_sock = INVALID_SOCKET ;
	_port = 0 ;
	_proto= 0 ;
	if( !sockConnectionStartup() )
		throw TcpErr_Open ;
}

ComOutTcp::~ComOutTcp()
{
	close() ;
	if( _sock != INVALID_SOCKET )
		closesocket( _sock ) ;
	sockConnectionCleanup() ;
}


BOOL ComOutTcp::hasCapability( ComIOCapability cap )
{
	switch( cap )
	{
		case ComIO_DynamicSetSpeed:
			return TRUE ;
	}
	return FALSE ;
}

int ComOutTcp::setSpeed( float maxSpeed, BaseConfigClass *cfg )
{
	_setup._speed = maxSpeed ;
	_setup.save( cfg ) ;
	return 0 ;
}

int ComOutTcp::close()
{
	if( !isOpened() )
		return 0 ;
	if( _sock != INVALID_SOCKET )
		shutdown( _sock, SD_BOTH ) ;
	return ComOut::close() ;
}


// connectStr = [host//]service
//   where
// (optional) host
//			  = "localhost"		... local computer (this is default host),
//				"ftp.microsoft.com",
//				"127.54.67.32",
//				host name as registrated in system...\etc\hosts
// service	  =	"123/tcp"		... port 123, using TCP
//				"456/udp"		... port 456, using udp
//				"xyz"			... port & protocol taken from etc\services
//
int ComOutTcp::open( const char *connectStr )
{
	//ASSERT( !isOpened() ) ;
	const char *syntaxPrompt = 
		"    connectString = [host//]port/tcp\n"
		"        or\n"
		"    connectString = [host//]service\n"
		"\n"
		"where\n"
		"   - host       \t is any name resolvable by your DNS (optional)\n"
		"   - port       \t is the same number as specified by the Receiver (1..65535)\n"
		"   - service \t is the name defined e.g. in system...\\etc\\services\n"
		"\n"
		"Examples:\n"
		"  12345/tcp                \t... contact Receiver running on the local machine\n"
		"  127.54.67.32//12345/tcp  \t... contact to another machine (no need for DNS)\n"
		"  janos//12345/tcp         \t... contact within LAN (no need for DNS)\n"
		"  www.microsoft.com//23/tcp\t... contact via DNS" ;


	char  buf[1024] ;
	struct sockaddr_in sin ;

	{
		int  syntaxErr=0 ;
		char syntaxErrBuf[256] ;

		// decode connectStr
		char *service, *host=NULL, *protocol=NULL ;
		strcpy( buf, connectStr ) ;

		if (stricmp(buf, "Ethernet")==0)
		{
			// load from cfg
			host	= _setup._host ;		if (!host[0])		host = NULL ;
			service = _setup._port ;		if (!service[0])	service = "4321" ;
			protocol= _setup._protocol ;	if (!protocol[0])	protocol = "udp" ;
		}
		else
		{
			char *s = strstr( buf, "//" ) ;
			if( s != NULL )
			{
				host    = buf ;
				*s      = 0 ;
				service = s+2 ;
			}
			else
				service = buf ;
			s = strchr( service, '/' ) ;
			if( s != NULL )
			{
				*s = 0 ;
				protocol = s+1 ;
			}
		}

		memset( &sin, 0, sizeof(sin) ) ;
		sin.sin_family = AF_INET ;
		//if( host == NULL )
		//	host = "localhost" ;

		// 1. convert host -> IP address
		struct hostent *he = gethostbyname( host ) ;					// host = NULL or "hostName"
		if( he != NULL )
			memcpy( (char*)&sin.sin_addr, he->h_addr, he->h_length ) ;
		else															// host unknown 
		if( (sin.sin_addr.s_addr = inet_addr( host)) == INADDR_NONE )	// try IP address (e.g. "123.124.125.126")
		{
			strcpy( syntaxErrBuf, "Host name not resolved." ) ;
			syntaxErr = TcpErr_UnknownHost ;
			goto labelSyntaxError ;
		}

		// host found
		// 2. find port number and protocol
		_port  = atoi( service ) ;
		if( _port > 0 )													// service = port (e.g. "123")
		{
			_port = htons((u_short)_port);
		}
		else															// port given as service
		{
			struct servent *se = getservbyname( service, protocol ) ;	// look for port# in hosts file
			if( se != NULL )
				_port = se->s_port ;
			if( _port <= 0 )
			{
				strcpy( syntaxErrBuf, "Unknown service name used." ) ;
				syntaxErr = TcpErr_UnknownService ;
				goto labelSyntaxError ;
			}
			if( protocol == NULL )										// if no protocol given...
				protocol = se->s_proto ;								// ...take it from the hosts file, too
		}
		sin.sin_port = _port ;

		// 3. protocol
		_proto = IPPROTO_TCP ;		// IPPROTO_UDP ...
		if( protocol != NULL )
		{
			struct protoent *pe = getprotobyname( protocol ) ;
			if( pe == NULL )
			{
				strcpy( syntaxErrBuf, "Unknown protocol name used." ) ;
				syntaxErr = TcpErr_UnknownProtocol ;
				goto labelSyntaxError ;
			}
			_proto = pe->p_proto ;
		}

	  labelSyntaxError:
		if( syntaxErr != 0 )
		{
			sprintf( buf, "Error in tcp connect string (\"%s\"):\n%s\n      -----------\n\nUse following syntax:\n\n%s",
				connectStr, syntaxErrBuf, syntaxPrompt ) ;
			::MessageBox( NULL, buf, "Error", MB_OK|MB_ICONERROR ) ;
			return syntaxErr ;
		}
	}

	int connectErr = 0 ;

	// 4. create socket
	int type = ((_proto==IPPROTO_TCP) ? SOCK_STREAM : SOCK_DGRAM) ;
	_sock = socket( AF_INET, type, _proto ) ;

	if( _sock == INVALID_SOCKET )
		connectErr = TcpErr_CreateSocketFailed ;
	else
	// 5. connect socket
	if( connect( _sock, (struct sockaddr*)&sin, sizeof(sin)) == 0 )
		return ComOut::open(connectStr) ;
	else
		connectErr = TcpErr_ConnectSocketFailed ;

	const char *sockErrText = reportWinSockError();
	sprintf( buf,
		"connectString = %s\n"
		"\n"
		"Failed to establish contact with Receiver program.\n"
		"Error announced by the system:\n"
		"    \"%s\"\n\n"
		"Make sure that Receiver program is running at this time, and\n"
		"it uses the same connect parameters (port & protocol) as the Server.\n"
		"\n"
		"Remark:\n"
		"   Although it may seem a bit confusing, from the tcp point of view\n"
		"   the roles of Server and Receiver are exchanged and Receiver must be\n"
		"   started before Server.",
		connectStr, sockErrText ) ;

	::MessageBox( NULL, buf, "Error", MB_OK|MB_ICONERROR ) ;

	closesocket( _sock ) ;
	_sock = INVALID_SOCKET ;

	return connectErr ;
}


//
// Udp characteristics:
// - Need to transfer whole TSPacket's. Otherwise missing packets are interpreted
//	 as corrupted.
// - Sending 1 TSPacket at a time is worse (slower, more missing packets) than sending
//	 20 packets.
//
int ComOutTcp::write( const char *p, int n_bytes, int *written )
{
	int total_cnt = n_bytes ;

	while( total_cnt > 0 )
	{
		int val = __min( total_cnt, 40*188 ) ;
		int cnt = send( _sock, p, val/*total_cnt*/, 0 ) ;
		if( cnt == SOCKET_ERROR )
		{
			reportWinSockError() ;
			*written = n_bytes - total_cnt;
			return TcpErr_SocketError ;
		}
		total_cnt -= cnt ;
		p += cnt ;
	}
	*written = n_bytes;
	return 0;
}

