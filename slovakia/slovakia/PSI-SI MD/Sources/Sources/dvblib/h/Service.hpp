#ifndef			__SERVICE_HPP__
#define			__SERVICE_HPP__

#ifndef __INC_FILEIO_HPP__
#include "fileIO.hpp"
#endif

struct ServiceEvent ;
class ServiceManager;

class ServiceChannel : public MuxChannel
{
	friend DWORD WINAPI sendThreadFunc( LPVOID arg ) ;
	friend class ServiceManager;
	ServiceEvent  *_ev ;
	HANDLE		   _sendingThread ;
	void		  sendData( ServiceEvent *ev, ushort flag ) ;

  public:
	ServiceChannel( Mux *mux, const MuxChannelSetup *s, BOOL online=TRUE ) ;
	inline  ~ServiceChannel( )					{ }

	HANDLE		  sendData	( ServiceEvent *ev ) ;			// send data in thread; -> thread handle
	inline ServiceEvent *event()				{ return _ev ; }

	virtual BOOL  start		  () ;
	virtual BOOL  initiateStop() ;
	virtual BOOL  stop		  () ;
} ;


// unicast  : (data, userID , TRUE )
// multicast: (data, channel, FALSE)
// broadcast: (data, 0      , FALSE)
BOOL sendFileToServiceChannel	  ( const char *file, ulong channel );
BOOL sendFileToServiceChannel	  ( const char *file, const GlobalUserID& userId );
BOOL sendUpdateToServiceChannel	  ( const char *file, ulong channel );
BOOL sendUpdateToServiceChannel	  ( const char *file, const GlobalUserID& userId );
// msg contains <message>0<title>0
BOOL sendMsgToServiceChannel	  ( const char *msg , ulong channel, ulong dataLength );
BOOL sendMsgToServiceChannel	  ( const char *msg , const GlobalUserID& userId, ulong dataLength );
BOOL sendUserTableToServiceChannel( );
BOOL sendUserTableToServiceChannel( const GlobalUserID& userId );

#endif
