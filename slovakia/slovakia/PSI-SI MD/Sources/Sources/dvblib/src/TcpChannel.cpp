#include "tools2.hpp"
#include "mux.hpp"
#include  <math.h>

#include "clientCfg.hpp"
#include "servCfg.hpp"
#include "TcpChannel.hpp"
#include "BaseRegistry.hpp"
#include "FileIO.hpp"
#include <WinSock2.h>


DWORD WINAPI TcpSendThreadFunc( LPVOID arg ) ;
#define MAXDATA  1024 
#define PORT	 5050 

TcpChannel::TcpChannel( Mux *mux, const MuxChannelSetup *s, BOOL online )
: MuxChannel( MuxChannel::ServiceType, mux, s, online )
{
	_sendingThread = NULL ;
	_isKilled = FALSE ;
}


TcpChannel::~TcpChannel( )
{
	stop() ;
}


// Start sending the data
BOOL TcpChannel::start()
{
	if( isStarted() )
		return TRUE ;
	if( !isOnline() )
		return FALSE ;

	MuxChannel::start() ;
	MfxPostMessage( EMsg_InboxStart, _setup.channel );

	// create sending thread
	DWORD threadId ;
	_sendingThread = ::CreateThread( NULL, NULL, TcpSendThreadFunc, this, 0, &threadId ) ;

	setStatusBit( Started ) ;
	return TRUE ;
}

// issue signal to stop the channel
BOOL TcpChannel::initiateStop()
{
	if( !isStarted() )
		return TRUE ;
	if( isSending() )
	{
		SetEvent( _hKillEvent );
		_isKilled = TRUE ; ///////////////////////////////////////////
	}
	return TRUE ;
}

// stop sending data
BOOL TcpChannel::stop()
{
	if( !isStarted() )
		return TRUE ;
	BOOL ret = TRUE ;
	if( isSending() )
	{
		SetEvent( _hKillEvent );
		_isKilled = TRUE ; ////////////////////////////////////////////////
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
	delStatusBit( Started | PendingRequests ) ;
	MfxPostMessage( EMsg_InboxEnd, _setup.channel );
	return ret ;
}

void tError( int err, TcpChannel* ch, SOCKET* sock ) 
{
	closesocket( *sock ) ;
	*sock = INVALID_SOCKET ;

	//if fatal error!
	ch->initiateStop() ;
	ch->clearSending( ) ;
	//MfxPostMessage( EMsg_TcpError, 0xFFFF, expl ) ;
	BEEP_WARNING ;
}

static DWORD WINAPI TcpSendThreadFunc( LPVOID arg )
{
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
	TcpChannel *ch = (TcpChannel *)arg ;

	ch->setSending( ) ;

	static char			dataBuf[MAXDATA] ;
	static int			inactiveCounter=0 ;
	char				textBuf[128] ;
	int					err=0 ;
	DWORD				cnt ;
	CardSerialNumber	usrId ;
	BOOL				bAccepted = FALSE ;
	struct sockaddr		adr ;
 	int					adr_len = sizeof(adr);
	SOCKET				inSock = INVALID_SOCKET ;
	SOCKET				sock = INVALID_SOCKET ;
	DWORD				flags = 0 ;
	WSABUF				bufs[2] ;
	WSANETWORKEVENTS	netEvents;
	WSAEVENT			eve ;
	struct sockaddr_in	sin ;
	WSADATA				wsadata ;

	if( (WSAStartup( MAKEWORD(2,1), &wsadata )) != 0 )
		tError( WSAGetLastError(), ch, &sock ) ;

	sock = WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL , 0, WSA_FLAG_OVERLAPPED ) ;
	if( sock == INVALID_SOCKET )
		tError( WSAGetLastError(), ch, &sock ) ;

	memset( &sin, 0, sizeof(sin) ) ;
	sin.sin_family = AF_INET ;
	sin.sin_port   = PORT ;
	if( bind( sock, (struct sockaddr*)&sin, sizeof(sin)) != 0 )
		tError( WSAGetLastError(), ch, &sock ) ;	
	
	if( listen( sock, 5 ) != 0 )
		tError( WSAGetLastError(), ch, &sock ) ;

	bufs[0].buf	= dataBuf ; /////////////////
	bufs[0].len = MAXDATA ; ////////////////
	
	if( ( eve = WSACreateEvent()) == WSA_INVALID_EVENT )   
	    tError( WSAGetLastError(), ch, &sock ) ;
	
	if( (WSAEventSelect( sock, eve, FD_ACCEPT | FD_CLOSE | FD_READ )) != 0 )
		tError( WSAGetLastError(), ch, &sock ) ;

	while( !(ch->_isKilled) )
	{
		if( (WSAWaitForMultipleEvents( 1, &eve, FALSE, 100, FALSE )) == WSA_WAIT_TIMEOUT )
		{
			if( ++inactiveCounter > 100 )
			{
				WSAResetEvent( eve ) ;
				AfxMessageBox( "Connection time-out.", MB_OK ) ; 
				inactiveCounter = 0 ;
				bAccepted = FALSE ;
			}
			continue ; ////////////////
		}
		else
		{	
			inactiveCounter = 0 ;
			if( bAccepted )
				err = WSAEnumNetworkEvents( inSock, NULL, &netEvents ) ;
			else
				err = WSAEnumNetworkEvents( sock, NULL, &netEvents ) ;

			switch( netEvents.lNetworkEvents )
			{
				case FD_ACCEPT:
				{
					inSock = WSAAccept( sock, &adr, &adr_len, NULL, NULL ) ;
					if( inSock == INVALID_SOCKET )
							err = WSAGetLastError() ;
					else
					bAccepted = TRUE ;
					
					WSAResetEvent( eve ) ;
					AfxMessageBox( "Connection accepted.", MB_OK ) ; 
					break ;
				}			
				case( FD_CLOSE ):
				{
					AfxMessageBox( "Closing socket", MB_OK ) ; 
					WSAResetEvent( eve ) ;
					bAccepted = FALSE ;
					break ;
				}
				case( FD_READ ):
				{
					if( (WSARecv( inSock, bufs, 1, &cnt, &flags, NULL, NULL )) != 0 ) 
					{
		 				err = WSAGetLastError() ;
						if( err = WSAEWOULDBLOCK )
							continue ;
						else
							tError( WSAGetLastError(), ch, &sock ) ;
					}
					else
					{
						sprintf( textBuf, " Dorazilo %d bajtov", cnt ) ; 
						AfxMessageBox( textBuf, MB_OK ) ; 
						//Senddata
						ch->sendData( dataBuf, cnt, usrId ) ;
					}
					WSAResetEvent( eve ) ;
					break ;
				}
				default:
					WSAResetEvent( eve ) ;
					AfxMessageBox( "default", MB_OK ) ; 
				break ;
			}
		}
	}
	return 0 ;
}


///////////prerobit!
inline int TcpChannel::sendData( const char *buf, int cnt, const CardSerialNumber &usrId )
{
	//usrId = 0xc46deeb9 ; //0x80941dd8 ;
	//#pragma message("InternetChannel::sendData(): USER_ID set to c46deeb9")
	//ASSERT( usrId.isValid() ) ;
	return _dataSender->sendInternetData( MuxPacket::Internet | MuxPacket::Unicast, buf, cnt, _setup.channel,
		_setup.numRebroadcasts, channelPID(), *(GlobalUserID*)&usrId ) ;
}



/*			

			HNSrv_readPackets( err, dataBuf, cnt, usrId ) ;
			//usrId = staticUsrId ;
			if( err != 0 )
			{
				terminateInternetChannel( ch, err ) ;
				inactiveCounter   = 0 ;
			}

			if( cnt <= 0 )
			{
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

*/
