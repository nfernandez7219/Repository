#ifndef __INC_MUX_HPP__
#define __INC_MUX_HPP__

#ifndef __INC_DVBERROR_H__
	#include "DvbError.hpp"
#endif
#ifndef __INC_COMIO_HPP__
	#include "ComIO.hpp"
#endif

#include "serialNum.hpp"
#include "setup.hpp"
#include "MuxPacket.hpp"

#ifdef _DUMP_RECEIVED_BAD_PACKETS
	#define DUMP_RCVPACKET(packet,flag)		dumpPacket(packet,flag)
#else
	#define DUMP_RCVPACKET(packet,flag)
#endif

#ifdef _PRIVATETRACE
	void writeTrace( char *pFormat, ... );
	#ifdef TRACE
		#undef TRACE
	#endif
	#define TRACE	writeTrace
#endif

// For the version without channels, providers and users uncomment the following line
//#define EMPTY_VERSION

class Mux ;
class MuxChannel ;
class Inbox ;
class FileSender ;
class DataSender ;
class MuxOutput ;
class MuxOutputTimer ;
class MuxTimer ;
class SchedulerManager;
struct DSMCC_section ;
class PidStreamAttrib ;

typedef sTemplateArray<MuxChannel*> MuxChannelPtrArray ;


class MuxSetup
{
  public:
	long  absPriorityPart ;		// [%]
	long  relPriorityPart ;		// 100 - absPriorityPart
	MuxSetup()
	{
		absPriorityPart = 30;
		relPriorityPart = 70;
	}
	int operator ==( const MuxSetup &src) const	{ return absPriorityPart==src.absPriorityPart  &&  relPriorityPart==src.relPriorityPart; }
} ;


//
// Intermediate buffer accepting MuxPacket's and outputting (onto ComOut) TSPacket's.
//
class MuxOutputQueue
{
	CRITICAL_SECTION _bufferLock;		// synchronization of the buffer access
	CRITICAL_SECTION _flushLock;		// synchronization of the buffer access

	char   *_buffer ;					// buffer for packets
	int		_maxPackets ;				// buffer size in packets
	int		_maxBytes ;					// buffer size in Bytes
	int		_inPtr ;					// index of the next packet to be put
	int		_outPtr ;					// index of the next packet to be written
	int		_outPtrByteOff ;			// part of the _outPtr packet written (0 for write on the packet boundary)

	uchar	_sectionEnd	;				// end of the data in the last TS packet
	ULONG	_numFlushedBytes ;			// count of Bytes successfully sent onto output
	BOOL	_useMultiSection ;
	ComOut *_comOut ;					// write device

	void sendFillPackets( int nPackets ) ;

  public:
	MuxOutputQueue (ComOut *c) ;
   ~MuxOutputQueue () ;

	void put	( MuxPacket *p, PidStreamAttrib *pidStrAttr ) ;
	void put	( DSMCC_section *s, int n_tsPackets, PidStreamAttrib *pidStrAttr ) ;
	int	 flush	( int &written, int maxBytes=0 );		// -> error code

	void defineBuffer	( int maxNumOfPackets );		// packets
	void freeBuffer		( );

	void resetComOut	( ComOut *c=NULL) ;
	int  countWaitingTestPackets() ;

	inline int numFlushedBytes()		{ return _numFlushedBytes ; }
	inline int maxInBuffer	  ()		{ return _maxPackets ; }
	inline int waiting		  ()		{ return _maxPackets ? ((_inPtr+_maxPackets-_outPtr) % _maxPackets) : 0 ; }
	inline int hasFree		  ( int n )	{ return (_inPtr+_maxPackets-_outPtr) % _maxPackets ; }
	
	inline void setMultiSectionMode ( BOOL useMultiSection )	{ _useMultiSection = useMultiSection ; }
} ;


class MuxOutput
{
	friend class Mux ;
	friend class MuxOutputTimer ;
	friend class TestRateThread ;

	MuxOutputQueue	_dataQueue ;
	MuxOutputQueue	_inetQueue ;

	ComOut *_comOut ;
	int		_byteRate ;					// amount of data to be transfered per second

	// for testing output rate
	int		_testCapacity ;				// min. capacity needed during testOutputRate()
	long	_testPacketIndex ;			// packet counter in the test mode
	HANDLE	_hNumTestPacketAvailable ;	// new space for packets available
	int		_numTestLoops ;				// loop counter

	MuxOutputTimer *_muxOutputTimer;
	void _delOutputTimer() ;

	void defineBuffer	( int maxNumOfPackets );	// packets
	inline int waiting  ()	{ return _dataQueue.waiting() + _inetQueue.waiting(); }
	void onDistributeRecordRequests( ) ;

	BOOL _testOutputRate( Mux *mux, int testDuration, HANDLE hKillEvent, float *estimSpeed ) ;
	BOOL _testOutputRate( Mux *mux, HANDLE hKillEvent, HWND hWnd, int msg ) ;

	int	flush( int nBytes=0, int *written=NULL ) ;	// nBytes==0 means all waiting packets

  public:
	// set maximal amount of data sent during 1 flush interval
	// - rate is total amount of data that can be sent in 1 second in TS packets
	// - distribInterval is capacity distribution interval in msec
	void setMaxFlushedData ( float rate, int distribInterval ) ;

	void freeBuffer	();								// flush + stop the timer

	inline void	put	
		( MuxPacket *p, PidStreamAttrib *pidStrAttr, BOOL isInternet=FALSE )					
				{ (isInternet?&_inetQueue:&_dataQueue)->put(p,pidStrAttr) ; }
	inline void put	
		( DSMCC_section *s, int n_tsPackets, PidStreamAttrib *pidStrAttr, BOOL isInternet=FALSE )
				{ (isInternet?&_inetQueue:&_dataQueue)->put(s,n_tsPackets,pidStrAttr) ; }

	// Use e.g. to suppress the sending temporarily (by passing NULL)
	void resetComOut	( ComOut *c=NULL) ;

	enum TestOutputRateMsg {			// used as message wParam
		StartTest	= 1,				// lParam = 0
		NewSolution	= 2,				// *(float*)lParam = speed [Mb/s]
		Progress	= 3,				// (const char *)lParam = "text"
		EndTest		= 4,				// lParam = 0
		Interrupted = 5,				// lParam = 0s
	} ;
	void testOutputRate(
			Mux		*mux,				// multiplexor using this MuxOutput
			HANDLE	hKillEvent,			// kill event or 0 (cannot be killed prematurally)
			HWND    hWnd,				// handle of the window receiving messages
			int		msg					// message, e.g. WM_APP+1
		) ;

	MuxOutput			( ComOut *c, const MuxSetup *setup );
   ~MuxOutput			();
} ;


// Multiplexor combines inputs from different channels into single stream and sends
// them to MuxOutput.
class Mux
{
	friend class MuxTimer ;

	MuxSetup	 _setup ;
	MuxOutput   *_output ;
	float		 _outputRate ;							// [Mb/s]
	MuxChannelPtrArray _channels ;						// dyn. array of MuxChannel's
	MuxTimer   *_muxTimer ;								// calls distributeRecordRequests() at 1 sec interval
	long		_checkTime ;							// [ms] time interval between consecutive runs of distributeRecordRequests()
	int			_reqNumOutRecs ;						// max. requested # of recs acc. to output rate
	int			_actNumOutRecs ;						// actualy requested # of recs acc. to _outputRate
	int			_numOfDistributedRecs;					// # of distr. rec. in last distr. round
	double		_numOfProcessedRecs[5];					// percent of processed recs in respect to distr recs in last 5 rounds
	int			_roundCounter;							// 0 - 4
	BOOL		_changeOfOutputRateRequested ;			// signal for multiplexor that output rate changed

	// multiplexing packet streams from different channels
	void	   _distributeRecordRequests();
	void		distributeRecordRequests();

  public:
	int  numRecsDuringCheckTime( float rate=-1 ) ;		// Mb/s; by default current output rate taken

	// start/stop
	BOOL		start( ) ;
	inline BOOL	isRunning()			{ return _muxTimer != NULL ; }
	void		stop ( ) ;

	// channels
	void		registerChannel  ( MuxChannel *x ) ;	// add new channel to multiplexor
	void		unregisterChannel( MuxChannel *x ) ;	// remove channel from multiplexor

	// output
	inline MuxOutput  *output()		{ return _output;}	// output device
	inline float	   outputRate()	{ return _outputRate; }

	// setup
	void		reset		 ( const MuxSetup *s ) ;	// change multiplexor setup
	void		setOutputRate( float rate ) ;			// [Mb/s]

	// construction/destruction
	Mux( const MuxSetup *s, MuxOutput *o ) ;
   ~Mux() ;
} ;

// channel setup
class MuxChannelSetup
{
  public:
	char		name[32] ;				// service name or "HybridNet" or "Service channel"
	int			absPriority,			// 0-10
				relPriority;			// 0-10
	float		minRate, maxRate ;		// limits for channel rate
	ushort		channel ;				// channel id
	short		fecLevel ;				// 0-don't use, 1-low, 2-medium, 3-high
	int			fecRebrSize ;			// [K]; if fecLevel!=0 then smaller files are rebroadcasted
	int 		numRebroadcasts ;		// 0=single broadcast, -1=infinite rebroadcast
	uchar		streamFormat ;			// has values defined in CfgChannel class

	PidStreamAttrib	  *channelPID ;

	// Inbox
	int			fileSendDelay ;			// [s] how many secs the file must not change to be sent
	int			volumeLimitPerDay ;		// [K]; max. size per day (incl. rebroadcasts)
	int			rebroadcastDelay ;		// [s] 0=no delay, else delay between rebroadcasts
	char	   *inboxDir ;				// inbox directory; unique among channels

	MuxChannelSetup()			{ memset( this, 0, sizeof(MuxChannelSetup) ) ; }
   ~MuxChannelSetup() ;
	BOOL		isEmpty()		{ return name[0]==0; }
	const MuxChannelSetup &operator=( const MuxChannelSetup & src ) ;
} ;

// Generic output channel:
// Reading of the data is organized through separate thread (MuxChannelInputThread()).
class MuxChannel
{
	friend class Mux ;
  protected:
	MuxChannelSetup  _setup ;
	Mux				*_mux ;
	HANDLE			 _hKillEvent;				// channel termination

	// senders
	FileSender		*_fileSender ;				// sending files
	DataSender		*_dataSender ;				// sending service data

	// status
	//CRITICAL_SECTION _statusLock;
	ulong			 _status;					// Status bits
	char			 _statusString[40];
		   void		  reportStatus( ) ;			// -> status in textual form
	inline void		  setStatusBit( ulong bit ) ;
	inline void		  delStatusBit( ulong bit ) ;
	inline BOOL		  hasStatusBit( ulong bit ) ;

	long			 _numErrors;				// error counter
	long			 _maxErrors;				// max number of errors allowed (then stop())
	void			  setFatalError( const char *str, DWORD err ) ;		// immediate stop
	void			  setError     ( const char *str, DWORD err ) ;		// ++_numErrors

	float			 _speed ;					// Mb/s; speed in last multiplexor step
	long			 _assigned_numfreepack ;

	// packet pool
  private:
	long			 _numfreepack;				// # packets allowed to output
	HANDLE			 _hNumFreePacketAvailable;	// set by incNumFreePackets() if waitingForFreePackets
	HANDLE			 _hNumFreePacketAvailableSrv;	// the same for data sender
  public:
	static CRITICAL_SECTION _distrLock;			// packet distribution synchronization
  protected:
	inline long	 numFreePackets			()			{ return _numfreepack;}
	inline long	 incNumFreePackets		( long cnt ) ;
	inline void	 clearNumFreePackets	() ;

  public:
	enum ChannelType {
		ServiceType=1,
		FileType=2,
	} ;
	// create channel with file sender or service sender or both
	// channelType = FileType, ServiceType, FileType | ServiceType
	MuxChannel( int channelType, Mux *mux, const MuxChannelSetup *s, BOOL online=TRUE ) ;
	virtual ~MuxChannel( ) ;

    inline  ushort		channelID()				{ return _setup.channel; }
    inline  PidStreamAttrib*
						channelPID()			{ return _setup.channelPID; }
	inline  uchar		streamFormat()			{ return _setup.streamFormat; }
	inline  BOOL		isInternetChannel()		{ return _setup.channel == 0xFFFF ; }
	inline  Mux		   *mux      ()				{ return _mux ; }
	inline  float		speed	 ()				{ return _speed ; }

	inline  DataSender *dataSender()			{ return _dataSender ; }

	// ---- sending messages ----
	virtual void	    msgFun	 ( long code, void *param );
	virtual const char *msgAsText( long code, void *param, char *buf )	{ *buf=0 ; return buf; }

	// ---- start/stop ----
	// initiateStop() is used to start stop process without waiting for result; call stop() afterwards
	virtual BOOL start		  ()				{ ResetEvent(_hKillEvent) ; return TRUE; }
	inline  BOOL isStarted	  ()				{ return hasStatusBit(Started);}
	virtual BOOL initiateStop ()=0;
	virtual BOOL stop		  ()=0;

	// ---- status ----
	//		online/offline	... channel cannot be started (ignores start() and incNumFreePackets())
	//		sending			... means channel is in a sending loop (usu. in a thread) and
	//							is subject to Mux::distributeRecordRequests()
	//		pending requests... means data are waiting to be sent
	enum Status {
			Started			= 0x01,
			PendingRequests	= 0x02,
			Sending			= 0x04,
			VolumeExc		= 0x08,
			StopDueToError  = 0x10,
			Online			= 0x20,
	};
	inline ulong	status		 ()		{ return _status; }
	inline BOOL		isSending	 ()		{ return hasStatusBit(Sending) ; }
	inline void		setSending	 ()		{ setStatusBit(Sending) ; }
	inline void		clearSending ()		{ delStatusBit(Sending) ; }
	inline BOOL		hasDataToSend()		{ return hasStatusBit(PendingRequests) ; }
	inline BOOL		isOnline	 ()		{ return hasStatusBit(Online) ; }
		   void		setOnline	 ( BOOL yes) ;

	// ---- channel setup ----
	inline  float	minRate		()		{ return _setup.minRate; }
	inline  float	maxRate		()		{ return _setup.maxRate; }
	inline  int		absPriority	()		{ return _setup.absPriority; }
	inline  int		relPriority	()		{ return _setup.relPriority; }
	virtual void	reset( const MuxChannelSetup *s ) ;					// change channel setup
	virtual	void	setInboxDirectory( const char *newDir )
	{
		if ( !newDir )
			return;
		BOOL startAfterChange = isStarted();
		stop();
		if ( _setup.inboxDir )
			FREE( _setup.inboxDir );
		_setup.inboxDir = STRDUP( newDir );
		if ( startAfterChange )
			start();
	}

	// ---- send services ----
	// Send (synchroneously) file <_setup.numRebroadcasts> times without any delay.
	// Send file protocol will contain only short file name (name+extension).
	// flags = 0 | MuxPacket::Upgrade
	int  broadcastFile	( const char *fullFileName, ushort flags=0 ) ;
	int  unicastFile	( const char *fullFileName, const GlobalUserID& usrId, ushort flags=0) ;
	int  multicastFile	( const char *fullFileName, ushort channel, ushort flags=0) ;
} ;

// set parameters for packet pool
void setNumfreepack( long totalPackets, long minIPPackets, long distrIPPackets );

//---------------------------------------------------------------------------------


inline void MuxChannel::setStatusBit( ulong bit )
{
	if( (_status & bit) == 0 )
	{
		//EnterCriticalSection( &_statusLock);
		EnterCriticalSection( &_distrLock);
		_status |= bit ;
		reportStatus() ;
		LeaveCriticalSection( &_distrLock);
		//LeaveCriticalSection( &_statusLock);
	}
}

inline void MuxChannel::delStatusBit( ulong bit )
{
	if( (_status & bit) != 0 )
	{
		//EnterCriticalSection( &_statusLock);
		EnterCriticalSection( &_distrLock);
		_status &= ~bit ;
		reportStatus() ;
		LeaveCriticalSection( &_distrLock);
		//LeaveCriticalSection( &_statusLock);
	}
}

inline BOOL MuxChannel::hasStatusBit( ulong bit )
{
	return _status & bit ;
}

inline long MuxChannel::incNumFreePackets( long val )
{
	long ret = InterlockedExchange(&_numfreepack,val+_numfreepack) ;
	//if ( ret <= 0  &&  _numfreepack > 0 )
	if( _numfreepack > 0 )		// MPE section may wait for more packets so original value may be positive
	{
		SetEvent( _hNumFreePacketAvailable );
		if( _hNumFreePacketAvailableSrv != NULL )
			SetEvent( _hNumFreePacketAvailableSrv );
	}
	return ret ;
}

inline void	MuxChannel::clearNumFreePackets( )
{
	ResetEvent( _hNumFreePacketAvailable );
	if( _hNumFreePacketAvailableSrv != NULL )
		ResetEvent( _hNumFreePacketAvailableSrv );
	InterlockedExchange(&_numfreepack,0) ;
}

//-----------------------------------------------------------------------------------
//	PidStreamAttribManager
//	Manages the assignment of the channels to the PID streams
//  Ensures continuity of the PID stream
//-----------------------------------------------------------------------------------

class PidStreamAttribManager
{
	sTemplateArray<PidStreamAttrib*>	_streamAttribs ;

  public:
	PidStreamAttrib  *getPidStreamAttrib		( ushort pid ) ;
	void			  releasePidStreamAttrib	( PidStreamAttrib *streamAttrib ) ;

	~PidStreamAttribManager() ;
} ;

#endif
