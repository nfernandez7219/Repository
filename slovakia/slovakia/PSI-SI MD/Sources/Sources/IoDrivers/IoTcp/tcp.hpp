#ifndef __INC_TCP_HPP__
#define __INC_TCP_HPP__


#ifdef _IOTCPDLL
    #define IotcpDll	__declspec( dllexport )
#else
    #define IotcpDll	__declspec( dllimport )
#endif


#define IOTCP_VERSION		320

#pragma warning( disable: 4275 )

#include <winsock2.h>
#include "ComIo.hpp"
#include "DvbError.hpp"
#include "SerialNum.hpp"

BOOL MfxMessageHook( UINT msg, long wParam=0, long lParam=0 ) ;

const char *tcpEventAsText( int code, char *buf )	;	// max 256 chars

#define TcpErr_Open					(Event_IOErrorFlag | 1)
#define TcpErr_UnknownHost			(Event_IOErrorFlag | 2)
#define TcpErr_UnknownService		(Event_IOErrorFlag | 3)
#define TcpErr_UnknownProtocol		(Event_IOErrorFlag | 4)
#define TcpErr_CreateSocketFailed	(Event_IOErrorFlag | 5)
#define TcpErr_ConnectSocketFailed	(Event_IOErrorFlag | 6)
#define TcpErr_BindSocketFailed		(Event_IOErrorFlag | 7)
#define TcpErr_SocketError			(Event_IOErrorFlag | 8)


struct ComTCPSetup
{
	float		 _speed ;		// Mb/s
	GlobalUserID _userId ;
	char		 _host[64] ;
	char		 _protocol[5] ;
	char		 _port[7] ;

	void load( BaseConfigClass *cfg ) ;
	void save( BaseConfigClass *cfg ) ;

	ComTCPSetup()
	{
		_speed = 8;
		_host[0]	 =
		_protocol[0] =
		_port[0]	 = '\x0' ;
	}
} ;

extern ComTCPSetup _setup ;
extern CWinApp theApp ;
extern BOOL bOutputOpened ;
#define CFGSECT	"Ethernet"

class IotcpDll ComOutTcp : public ComOut
{
	SOCKET  _sock ;
	int		_port ;
	int		_proto;			// IPPROTO_TCP, IPPROTO_UDP

  public :
	ComOutTcp () ;
   ~ComOutTcp () ;

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
	// Defaults:
	//		open( "123")		// TCP connection on local computer, port=123
	//		open( "abc")		// local computer, port and protocol defined in ...\services
	//
	virtual int		open ( const char *connectStr) ;
	virtual int		close( ) ;
	virtual int		write( const char *p, int n_bytes, int *written ) ;
	virtual	int		setSpeed( float maxSpeed, BaseConfigClass *cfg ) ;	// Mb/s
	virtual BOOL	hasCapability( ComIOCapability cap ) ;
};


// Implementation of ComInp class for Tcp/Udp connection.
class IotcpDll TcpInpDriver : public BaseInpDriver
{
	BaseComInp *_baseComInp ;
	SOCKET  _sock ;
	int		_port ;
	int		_proto;			// IPPROTO_TCP, IPPROTO_UDP
	void    _workUDP() ;
	void    _workTCP() ;
	SOCKET  out_sock ;

  public :
	TcpInpDriver ( BaseComInp * x ) ;
   ~TcpInpDriver () ;

	// connectStr =	"123/tcp"		... port 123, using TCP
	//				"456/udp"		... port 456, using udp
	//				"xyz"			... port & protocol taken from etc\services
	//
	virtual int		open ( const char *connectStr) ;
	virtual int		close( ) ;
	virtual int		workKernel() ;

	virtual BOOL	getUserId( GlobalUserID *id ) ;
} ;

void TcpSetupDialog( CWnd* wnd, ConfigClass *cfg, BOOL asServer ) ; // Runs setup dialogs

#endif
