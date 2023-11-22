#ifndef __INC_MESSAGEQUEUE_HPP__
#define __INC_MESSAGEQUEUE_HPP__

////////////////////////////////////////////////////////////////////////////////////
//
//  Microsoft Message Queue (MSMQ) support
//
// you should :
//		- import library "mqrt.lib"
//		- include /delayload:mqrt.dll parameter to link settings
//
// you needn't have mqrt.dll to run program (MessageQueue functions throw exceptions in this case)
//
//
// Message queue specification:
//		- path :
//			machine_name\path_name		( e.g. tester\amaqueue )
//
//		- format name : 
//			PUBLIC=GUID					( e.g. PUBLIC=2769FF5C-E623-11D3-B348-00A0242D8FA3 )
//			used to quicker access to the queue
//
//		- handle
//			assigned by open
//

struct MSMQSendMode;

#define MAX_Q_FORMATNAME_LEN	256		// ensure that format name string is not longer than this
#define MAX_Q_PATHNAME_LEN		256		// ensure that path string is not longer than this

////////////////////////////////////////////////////////////////////////////////////

struct MQSpecification
{
	char  szPathName[MAX_Q_PATHNAME_LEN];		// Queue path name.
	char  szFormatName[MAX_Q_FORMATNAME_LEN];	// Queue format name.
	HANDLE hHandle;								// handle for an open Queue.
	DWORD  dwAccess;							// access for the queue.
	inline MQSpecification(){ hHandle=0 ; dwAccess=0 ; }
} ;

class MQSpecificationPtrArray : public sTemplateArray<MQSpecification*>
{
  public:
	MQSpecificationPtrArray( ) ;
};

////////////////////////////////////////////////////////////////////////////////////

class MessageQueue
{
	HRESULT	 _lastResult ;
	HANDLE	 _hQueue ;

	char	 _formatName[MAX_Q_FORMATNAME_LEN] ;	// queue id (assigned during creation or open)

public:
	enum { NoAccess = 0, SendAccess, ReceiveAccess, PeekAccess };	// used by open functions

	MessageQueue	( );
	~MessageQueue	( );

	BOOL	_create	( char *pathName, char *label );		// creates queue and sets label
	BOOL	_delete	( char *formatName ) ;					// deletes specified queue

	///////////////////////////////////////////////////////
	// Queue opening / closing
	// -----------------------
	//
	// - you can open the queue for sending,
	//   peeking or receiving messages
	// - you cannot combine any of these types of access
	//
	// - opening with known format name is quicker
	// - you must have permission to access the queue

	BOOL	open	( char *pathName, DWORD dwAccess = SendAccess ) ;
	BOOL	_open	( char *formatName, DWORD dwAccess = SendAccess ) ;
	BOOL	close	( ) ;

	////////////////////////////////////////////////////////////
	// Sending
	// ---------
	//
	// - you can set all supported MSMQ parameters filling
	//   sendMode structure
	
	BOOL	send	(	UCHAR			*msg,				// IN: contains message body
						ULONG			 nMsgBytes,			// IN: message body size
						const char		*label = NULL,		// IN: message label
						MSMQSendMode	*sendMode = NULL	// IN: send mode - see below
					) ;

	////////////////////////////////////////////////////////////
	// Receiving
	// ---------
	//
	// - remember that default timeout is INFINITE
	// - if buffer size is insufficient receiving fails
	//   and message leaves in the queue (buffer is filled
	//   by its prefix)
	// - if message filter is set to true you will receive only
	//   messages with "normal" class type (no acknowledgements,...)
	// - you can filter messages yourself using classType parameter

	BOOL	receive	(	UCHAR		*msg,					// OUT	: buffer for message body
						ULONG		 &maxMsgBytes,			// INOUT: buffer size [bytes]
						char		*label = NULL,			// OUT  : buffer for message label (label is <= 250 chars)
						BOOL		 msgFilter = TRUE,		// IN   : allow only messages of class type MQMSG_CLASS_NORMAL - filters ack,... messages
						DWORD		 dwTimeout = INFINITE,	// IN   : timeout in msec; can be INFINITE
						UINT		*classType = NULL		// OUT  : filled by class type
					) ;

	///////////////////////////////////////////////////////////////
	// Peeking
	// -------
	//
	// - reads the message and leaves it in the queue
	//   (another peek will get same message)

	BOOL	peek	(	 UCHAR		*msg,					// OUT  : buffer for message body
						 ULONG		 maxMsgBytes,			// INOUT: buffer size [bytes]
						 char		*label = NULL,			// OUT  : buffer for message label (label is <= 250 chars)
						 DWORD		 dwTimeout = INFINITE,	// IN   : timeout in msec; can be INFINITE
						 UINT		*classType = NULL		// OUT  : filled by class type
					) ;

	// inlines
	inline BOOL			 isOpened		( )			{ return _hQueue != NULL; }
	inline char			*getFormatName	( )			{ return _formatName; }
	inline HANDLE		 getHandle		( )			{ return _hQueue; }

	// error management
	inline HRESULT		 getLastResult	( )			{ return _lastResult; }		// can be used to find out error reason (use HRESULTasText function to convert HRESULT to text)
	BOOL				 isLastErrBufferSmall ( );								// used to filter expected error (returns TRUE if last error was MQ_ERROR_BUFFER_OVERFLOW) - useful for peeking queue with small buffer
	BOOL				 isLastErrTimeout	  ( );								// returns TRUE if last error was MQ_ERROR_IO_TIMEOUT

	/////////////////////
	// static functions

	// ConnectString management
	// ConnectString syntax: machine\queue_name[format_name]	- format_name can be empty string
	static void composeConnectString	( char *connectString, const char *formatName, const char *path );
	static void decomposeConnectString	( char* formatName, char *path, const char *connectString );
	static BOOL validateConnectString	( char *connString );			// checks existence of the queue and fills format name in connection string

	// Utilities
	static BOOL	findFormatName	( char *formatName, int maxSize, char *path ) ;				// fills formatName with format name of message queue corresponding to path
	static void	locateQueue		( char *label, MQSpecificationPtrArray &locatedQueues ) ;	// fills array locatedQueues with all queues in network with specified label
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SendMode
//
//	defines mode for sending MSMQ messages

struct MSMQSendMode
{
										// values set via setFlags()
	UCHAR bAuthenticated;				// 0/1
	UCHAR bEncrypted;					// 0/1
	UCHAR bJournal;						// 0/1
	UCHAR bDeadLetter;					// 0/1
	UCHAR nAcknowledge ;				// MQMSG_ACKNOWLEDGMENT_{FULL|NACK}_{REACH_QUEUE|RECEIVE}
	UCHAR nDelivery;					// MQMSG_DELIVERY_EXPRESS, MQMSG_DELIVERY_RECOVERABLE

										// Acknowledgments queue 
										// (Default=none; can be any non-transactional queue.)
	char szAdminFormatName[MAX_Q_FORMATNAME_LEN];

	UCHAR nPriority;					// 0-7, highest=7, default=3,
	DWORD dwTimeToReachQueue ;			// seconds; default= LONG_LIVED= usu. 90 days
	DWORD dwTimeToBeReceived ;			// seconds; default= LONG_LIVED= usu. 90 days

	enum Flags {
		Authenticate			=0x001,	// digital signature of the msg required
		Encrypt					=0x002,	// msg is encrypted
		CopyToJournalBeforeSend	=0x004,	// msg is copied to journal before sending
		UseDeadLetterQueue		=0x008,	// for nondelivered/nonreceived msgs
		ExpressDelivery			=0x010,	// else: msg backup-ed in file (always for transactions)

										// acknowledgments (ACK queue must be given)
		FullReachAcknowledge	=0x100,	// +/- ACK after msg reaches the queue
		NegReachAcknowledge		=0x200,	//   - ACK after msg reaches the queue
		FullRcvAcknowledge		=0x400,	// +/- ACK after msg reaches the queue
		NegRcvAcknowledge		=0x800,	//   - ACK after msg reaches the queue

		Defaults				=0xFFFF,// ExpressDelivery
	} ;
			void setFlags( UINT flags ) ;
	inline	void setTimeouts( DWORD t )		{ dwTimeToReachQueue = dwTimeToBeReceived = t; }
	inline	void setPriority( UCHAR p )		{ nPriority = p; }

	MSMQSendMode() ;
} ;

#endif // __INC_MESSAGEQUEUE_HPP__

