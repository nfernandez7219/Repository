#ifndef __INC_TRANSPORTPRIVATE_HPP__
#define __INC_TRANSPORTPRIVATE_HPP__

//
// Internal declarations which should not be used outside of the transport*.cpp files
//

#ifndef _MESSAGE_QUEUE_HEADER_
#include "MessageQueue.hpp"
#endif

// error codes
enum TransportErrorCode
{
	ErrNoError			=0,		// 

	// TransportMedium errors
	ErrTimeout			=5,		//
	ErrInvalidData		=6,		//
	ErrInvalidParameter	=7,		//
	ErrNoMemory			=9,		//
	ErrUnknownError		=10,	//
	ErrInvalidAddress	=11,	//
	ErrUnknownAddress	=12,	//
	ErrHandshakeFailed	=17,	//

	// TransportMedium errors with additional medium-specific information
	ErrMediumOpenFailed	=13,	//
	ErrMediumCloseFailed=14,	//
	ErrMediumSndFailed	=15,	//
	ErrMediumRcvFailed	=16,	//
} ;

//
// generic class to send data using specific transport medium
//
class TransportMedium
{
	friend DWORD WINAPI rcvThreadFnc( LPVOID param ) ;

  protected:
	DWORD						_mediumError ;		// medium specific error
	int							_error ;			// general error
	BOOL						_isOpen ;			// TRUE if opened
	BOOL						_isServer ;			// TRUE if act as server
	char					   *_connectString ;	// string used for last open...()

	// server specific
	TransportMediumRcvFun		_callbackFnc ;
	LPARAM						_callbackFncParam ;
	BOOL						_terminated ;
	TransportMediumMsg			_rcvData ;

	// Client specific
	TransportMediumHandshakeFun _handshakeFnc ;
	LPARAM						_handshakeParam ;
	TransportMediumMsg		   *_handshakeSendMsg ;

  private:
	// server specific
	HANDLE						_rcvThread ;
	LPTHREAD_START_ROUTINE		_rcvThreadFnc ;		// pointer to the thread funcion, default is rcvThreadFnc
	BOOL						_wantDisconnect ;
	BOOL __open( const char *openString ) ;

  protected:
	// set/clear the last error
	inline	void	 setLastError	( int e, DWORD me=0 )				{ _error = e ; _mediumError = me ; }
	inline	void	 clearLastError	( )									{ setLastError( ErrNoError ) ; }

	// converts medium specific error code to string, NULL if no specific string is available
	virtual	const char*	mediumError2String( ) const						{ return NULL ; }

	// virtual functions to perform medium specific operations
	virtual	BOOL	_open			( const char *openString )			{ return FALSE ; }
	virtual	BOOL	_close			( )									{ return TRUE  ; }
	virtual void	prepareKillRcvThread()								{ }

	void	_setRcvThreadFunction	( LPTHREAD_START_ROUTINE fnc )		{ _rcvThreadFnc = fnc ; }
	BOOL	_startRcvThread			( ) ;

  public:
	TransportMedium					( ) ;
	virtual	~TransportMedium		( ) ;
	virtual BOOL connectionOriented ( ) const = 0 ;

	// returns peer computer name or IP
	// locationBuffer must be large enough
	virtual void getPeerLocation	( char *locationBuffer ) const = 0 ;

	inline  BOOL isOpened			( ) const							{ return _isOpen ; }
	inline  BOOL isServer			( ) const							{ return _isServer ; }
	virtual BOOL isInstalled		( ) const							{ return TRUE; }
	inline  BOOL closeRequested		( ) const							{ return _wantDisconnect ; }

	// openString is medium specific, the name of the computer, host, etc.
	BOOL		openAsClient		( const char *openString ) ;
	BOOL		openAsServer		( const char *openString, TransportMediumRcvFun f, LPARAM param ) ;
	BOOL		close				( ) ;

	// Set handshake paramaters (client only)
	void		setHandshake		( TransportMediumHandshakeFun f,
									  LPARAM param, TransportMediumMsg *handshakeSendPacket ) ;

	// data sending/receiving - client specific
	virtual BOOL sendData			( const TransportMediumMsg *x )		{ return FALSE ; }
	virtual BOOL receiveData		( TransportMediumMsg *x )			{ return FALSE ; }

	// errors
	inline BOOL	hasError			( ) const							{ return _error != 0 ; }
	inline int	error				( ) const							{ return _error ; }

	const char*	getErrorString		( char *buffer, int maxLen=1024 ) const ;

	// Call to refresh internal status of the medium.
	// Returns FALSE if the medium is unable to perform communication.
	virtual BOOL refresh			( ) ;

	// support for receiving thread
	inline BOOL callback			( TransportMediumMsg *snd, TransportMediumMsg *reply) const
																		{ return _callbackFnc ? _callbackFnc( snd, reply, _callbackFncParam) : FALSE; }
		   void onThreadTerminated	( ) ;
	inline const char *connectString( ) const							{ return _connectString ; }
	virtual BOOL  connectStringEqual( const char *newConnectString ) const	
																		{ return FALSE; }
	virtual	BOOL  testConnection	( const char *openString, char *reason ) ;
	virtual BOOL  isConnected		( )									{ return _isOpen ; }
	virtual BOOL  isLocalConnection ( )									{ return FALSE ; }
} ;


class SocketTransport : public TransportMedium
{
	friend DWORD WINAPI newSocketConnectionAcceptThread( LPVOID param ) ;

	SOCKET				_sock ;

	sTemplateArray<SocketTransport*> _allSockets ;	// server: all child sockets

	BOOL	_sendData		( const char* data, ULONG dataLength ) ;
	BOOL	_receiveData	( char* data, ULONG dataLength ) ;

	static BOOL decodeConnectString		( const char *openString, char *computer, int *port ) ;
	inline BOOL	startReceiveUserPackets	( SOCKET userSocket )
	{
		_sock = userSocket ;
		return _startRcvThread() ;
	}

	SocketTransport			( const SocketTransport &src ) ;

  protected:
	// ConnectString syntax: [host]:port
	//		host = IP-adress or machine name or name defined in the host file;
	//			   compulsory for client, ignored for server
	//		port = port number
	// Example:
	//		If the machine "tester" has IP 129.87.64.2 and the host file contains following line
	//			129.87.64.2  CuttexAmaServer
	//		then following connect strings are equivalent:
	//			tester:12346
	//			129.87.64.2:12346
	//			CuttexAmaServer:12346
	virtual	BOOL		_open		( const char *openString ) ;
	virtual	BOOL		_close		( ) ;
	virtual BOOL connectionOriented ( ) const		{ return TRUE ; }
	virtual void getPeerLocation	( char *locationBuffer ) const ;
	virtual BOOL sendData			( const TransportMediumMsg *userData ) ;
	virtual BOOL receiveData		( TransportMediumMsg *userData ) ;
	virtual BOOL refresh			( ) ;
	virtual void prepareKillRcvThread() ;

  public:
	SocketTransport					( ) ;
   ~SocketTransport					( ) ;

	virtual	const char*	mediumError2String( ) const ;
	virtual BOOL		connectStringEqual( const char *newConnectString ) const ;
	virtual BOOL		isConnected		  ( ) ;
	virtual BOOL		isLocalConnection ( ) ;
	
	static  BOOL		isLocalConnection ( const char *connectStr ) ;
} ;


class MSMQTransport : public TransportMedium
{
	MessageQueue	*_sendQueue;
	MessageQueue	*_receiveQueue;

	DWORD			 _timeout;		// timeout for receiving

	char			 _queueFormatName[MAX_Q_FORMATNAME_LEN];

	enum ErrorCode { erNoError = 0, erFailedToLoadLib, erInvalidPath } ;	// non MSMQ error codes

	BOOL		_sendData	( const char *dataPtr, ULONG dataLength, const char *label ) ;					// low-level sending
	BOOL		_receiveData( char* dataPtr, ULONG &dataLength, char *label=NULL, BOOL peek=FALSE ) ;	// low-level receiving

  protected:

	// OpenString syntax: machine\queue_name[format_name]
	//		machine		= name of computer containing the queue
	//		queue_name	= queue name (you can find it using MSMQ Explorer - see documentation)
	//					- it is case insensitive
	//		format_name	= GUID of the queue 
	//					- it is optional (can be empty)
	//					- increases the speed of opening queue
	// Example:
	//			tester\AmaQueue[PUBLIC=823b40d1-e3c7-11d3-b346-00a0242d8fa3]

	virtual	BOOL		_open		( const char *openString ) ;
	virtual	BOOL		_close		( ) ;

	virtual BOOL		sendData	( const TransportMediumMsg *x ) ;
	virtual BOOL		receiveData	( TransportMediumMsg *x ) ;

	virtual BOOL		connectionOriented	( ) const						{ return FALSE ; }
	virtual void		getPeerLocation		( char *locationBuffer ) const ;
	virtual BOOL		connectStringEqual	( const char *newConnectString ) const;	// compares format name from connect string
																					// and format name of actually opened queues

  public:
	// Returns FALSE if open failed due to the absence of MSMQ library (mqrt.dll).
	// Otherwise return TRUE.
	virtual BOOL	isInstalled	( ) const			{ return _mediumError != erFailedToLoadLib; }

	void			setTimeout	( DWORD timeout )	{ _timeout = timeout; }		// set receive timeout - can be INFINITE
	DWORD			getTimeout	( )					{ return _timeout; }		// get receive timeout

	MSMQTransport	( ) ;
   ~MSMQTransport	( ) ;

	virtual	const char*	mediumError2String		( ) const ;						// returns string containing description of last error
};

#endif
