#ifndef __TRANSPORT_HPP__
#define __TRANSPORT_HPP__

class TransportMedium ;


//
// Low level support for data exchange between remote machines.
//
// UniversalTransportMedium (UTM) provides medium independent functionality to send
// and receive data.
//
// You can open the UTM to use a specific communication channel.
// (Currently connection oriented tcp or connectionless MSMQ.)
//
// UTM can run in
// - client mode (with send/receive functions)
// - server mode (Here direct receive is forbidden; instead server is listening and returns
//   received data via callback function.)
//
// TransportMediumMsg is the data structure used in UTM send/receive functions.
// Message is defined as (cmd, label, data).
// Max. size of data is 4 MB and of the label 40 By.
//

#pragma pack(1)

struct TransportMediumMsgHdr
{
  protected :
	enum Flags {
		CrcComputed	= 1,
		DataCrcComputed	= 2,
	} ;
	DWORD	_cmd ;
	DWORD	_flags ;
	DWORD	_packetId ;
	ULONG	_dataLength ;
	char	_label[40] ;
	ULONG	_reserved ;			// set to 0
	ULONG	_dataCrc ;
	ULONG	_crc ;				// computed for all preceding members up to _reserved (incl.)

	void	setCrc() ;
  public:
	BOOL	isHdrValid() ;
} ;

struct TransportMediumMsg : protected TransportMediumMsgHdr
{
	long	_allocated ;
	char*	_data ;

	inline clear()								{ memset( this, 0, sizeof(TransportMediumMsg) ) ; }
	void   setPacketId( DWORD id=-1 ) ;

  public:
	// To increase the reliability of the data transfer, call setDataCrc().
	// isDataValid() performs data crc check.
	// UniversalTransportMedium and TransportClass perform data checks automatically.
	void setDataCrc() ;
	BOOL isDataValid() ;

	// Data access functions
	inline DWORD		cmd		  () const		{ return _cmd ; }
	inline ULONG		dataLength() const		{ return _dataLength ; }
	inline const char  *data	  () const		{ return _data ; }		// NULL means no data transfered
	inline const char  *label	  () const		{ return _label ; }

	// packetId is used in reply packets which should have identical id as the send packet.
	inline DWORD		packetId  () const		{ return _packetId ; }

	// setData() replaces old data by a copy of the data passed
	// (For setData(cmd) the packet contains only cmd; data is empty)
	// By default packetId is not changed.
	// Returns pointer to data().
		   char *setData	( DWORD cmd, const char *data=NULL, int len=0, DWORD packetId=-1 ) ;

	// Appends data[len] to existing data contents.
		   void addData		( const char *data, int len ) ;

	// Optional label (empty by default)
		   void setLabel	( const char *lbl )	;

	// Use this constructor to set data pointer directly.
	// <data> pointer must either be NULL or live until data contents is changed or destructor is called.
	// By default the framework generates packetId.
	TransportMediumMsg ( DWORD cmd, ULONG dataLen=0, char *data=0, DWORD packetId=-1 ) ;

	// Construction of an empty packet. The data must be set explicitly.
	inline TransportMediumMsg()					{ clear() ; }

	~TransportMediumMsg() ;

	TransportMediumMsg &operator= ( const TransportMediumMsg &src ) ;

	// allocData() allocates requested memory for the data() member; it is then allowed
	// to fill (char*)data() directly.
	// Return value: same pointer as returned by data() function
		   char *allocData	( int len ) ;
} ;

#pragma pack()


//
// Callback function type (used in UTM server) called when the data was received.
// (The function should not throw any exception.)
//
typedef BOOL (*TransportMediumRcvFun)(		// returns TRUE iff reply packet was generated and is to be sent
				TransportMediumMsg *rcv,	// received packet
				TransportMediumMsg *reply,	// reply packet to be filled (can be NULL)
				LPARAM param				// parameter passed to the class together with callback function
			 ) ;

typedef BOOL (*TransportMediumHandshakeFun)(// TRUE=success, FALSE=failure
				TransportMediumMsg *reply,	// packet received in reply to handshake
				LPARAM param				// parameter passed to the class together with callback function
			 ) ;

class UniversalTransportMedium
{
  public:
	// supported transport medium types
	enum TransportMediumType {
		UnknownMedium	= -1,
		SocketMedium	= 0,
		MSMQMedium		= 1,
	} ;

  private:
	TransportMedium*		_medium ;	// specialized transport class (tcp, MSMQ...)
	TransportMediumType		_mediumType ;

	BOOL		_openObject			( TransportMediumType type ) ;

  public:
	UniversalTransportMedium		( ) ;
   ~UniversalTransportMedium		( ) ;

	// Attributes - meaningfull only for opened medium
	BOOL		isServer			( ) const ;
	BOOL		isConnectionOriented( ) const ;
	BOOL		isConnected			( ) ;
	BOOL		isLocalConnection   ( ) ;

	const char *connectString		( ) const ;
	BOOL        connectStringEqual  ( const char *newConnectString ) const ;
	TransportMediumType	mediumType	( ) const ;
	
	// returns peer computer name or IP
	// locationBuffer must be large enough
	void		getPeerLocation		( char *locationBuffer ) const ;

	// open/close
	// (connectString may change if the user interactivelly specifies the medium)
	BOOL		openAsClient		( TransportMediumType type, const char *connectString, char *errReason,
									  TransportMediumHandshakeFun f=0, LPARAM param=0, TransportMediumMsg *handshakeSendPacket=0 ) ;
	BOOL		openAsServer		( TransportMediumType type, const char *connectString, TransportMediumRcvFun f, LPARAM param,
									  char *errReason, BOOL *isMediumModuleInstalled ) ;
	BOOL		isOpened			( ) const ;
	void		close				( ) ;

	// send/receive
	BOOL		sendData			( const TransportMediumMsg *data ) ;
	BOOL		receiveData			( TransportMediumMsg *data ) ;

	// error functions
	BOOL		hasError			( ) const ;
	BOOL		hasTimeoutError		( ) const ;
	const char*	errorString			( char *buffer, int maxLen=1024 ) const ;

	// Call to refresh internal status of the medium.
	// Returns FALSE if the medium is unable to perform communication.
	BOOL		refresh				( ) ;

	// the medium name or NULL if type is not a valid medium type
	static	const char*			mediumType2String	( TransportMediumType type ) ;
	// the medium type or UnknownMedium if name is not recognized
	static	TransportMediumType	mediumName2Type		( const char* name ) ;

	//
	// These functions replace calling WSAStartup()/WSACleanup().
	// It is up to the programmer which set of functions will be used to initialize
	// sockets; the only important thing is that they are initialized.
	//
	static BOOL socketInitialize	( char *buf ) ;
	static void socketDeinitialize	( ) ;
} ;

typedef UniversalTransportMedium::TransportMediumType UTM_TYPE ;

#endif // __TRANSPORT_HPP__
