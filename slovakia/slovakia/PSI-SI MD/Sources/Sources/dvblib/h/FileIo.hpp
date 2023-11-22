#ifndef __INC_FILEIO_HPP__
#define __INC_FILEIO_HPP__

#ifndef __INC_MUX_HPP__
	#include "mux.hpp"
#endif

class ComOut ;
class MuxOutput ;
class PidStreamAttrib ;


struct MuxMsgSendStatus
{
	LONGLONG	nPacketsSent;
	LONGLONG	timeStamp;
	SYSTEMTIME  st;
	MuxMsgSendStatus( LONGLONG np=0 )
	{
		nPacketsSent = np;
		FILETIME		ft;
		LARGE_INTEGER	li;
		GetLocalTime( &st );
		SystemTimeToFileTime( &st, &ft );
		li.LowPart	= ft.dwLowDateTime;
		li.HighPart	= ft.dwHighDateTime;
		timeStamp   = li.QuadPart;
	}
	// compute kilobyte per sec from previous status
	float outputRate( MuxMsgSendStatus *st2 ) ;
} ;


class DvbProtocolSetup
{
	ushort	_mpeUdpPort ;
	uchar	_mpeHnetIpProtocol ;
  public:
	DvbProtocolSetup() ;
	BOOL set( ushort  udpPort, uchar  ipProtocol ) ;	// -> TRUE iff there was some change
	void get( ushort &udpPort, uchar &ipProtocol ) ;

	// Returned values use network byte order;
	// For the presentation purposes use get/set()
	inline ushort mpeUdpPort		()		{ return _mpeUdpPort ; }
	inline uchar  mpeHnetIpProtocol	()		{ return _mpeHnetIpProtocol ; }
} ;

extern DvbProtocolSetup dvbProtocolSetup ;


//---------------------------------------------------------------------------
//	Forward Error Correction
//---------------------------------------------------------------------------


struct FECDesc
{
	int numCycles ;
	int cycles[4] ;
} ;

BOOL getFecParams(			// FALSE iff blkSize is too small to effectivelly define FEC
	int blkSize,			// IN : num packets per FEC block
	int redundancyClass,	// IN : 1-10%, 2-25%, 3-40%
	FECDesc *fd				// OUT
) ;


//---------------------------------------------------------------------------
//	BaseSender/Receiver
//---------------------------------------------------------------------------


class BaseSender
{
  protected:
	MuxOutput	*muxOutput ; 
	ushort		 channel ;
	uchar		 streamFormat ;
	int			 rebroadcastIndex ;
	HANDLE		 hKillEvent;				// external termination
	HANDLE		_hNumFreePacketAvailable ;	// new space for packets available
	HANDLE		 handleArray[2];
	long	   *_numfreepack ;
	GlobalUserID usrId ;					// usrId != 0 for Unicast
	float		 speed ;
	long		 packets ;
	MuxMsgSendStatus oldSendStatus ;
	uchar		_jobId ;

	ulong		 packetInd ;
	LONGLONG	 numPacketsSent;			// # packets sent since creating this class

	inline void nextJob()				{ if( ++_jobId == 0 ) ++_jobId ; packetInd = 0; };

	inline BOOL isInternetChannel()		{ return channel == 0xffff; }

	// Sending single packet.
	// Can be used in 2 modes:
	//	- data != NULL : muxPacket will be filled with data and sent
	//	- data == NULL : muxPacket will be sent without any change
	int sendPacket(
		MuxPacket *muxPacket,		// IN/OUT 
		ushort flg,					// IN:  flags to be used
		char *data,					// IN:  data to be sent or NULL
		int len,					// IN:  data length
		PidStreamAttrib *pidStrAttr	// IN:  attributes to be used
	) ;

	int sendAgregatedPackets( ushort flg, char *data, int nPackets, int packetSize, int &sentPackets, PidStreamAttrib *pidStrAttr ) ;
	int sendMPESection		( DSMCC_section *section, PidStreamAttrib *pidStrAttr ) ;

	BaseSender( MuxOutput *o, HANDLE hKillEvent, HANDLE hFreePacketAvailable, long *numfreepack ) ;
   ~BaseSender()						{ }

  public:
	inline void zeroSpeed( )			{ speed=0 ; }

	int sendServicePacket( 
		ushort flg, uchar _streamFormat, char *data, int len, PidStreamAttrib *pidStrAttr,
		const GlobalUserID& _usrId=GlobalUserID()	// unicast only
	) ;
} ;


class BaseReceiver
{
	static uchar bits[8] ;
	// packet map
	uchar  *_packetMap;		// mask of packets, bit i = 1 - packet i arrived in good condition
	int		_mapSize ;
	int		_maxPackets ;
	int		_numPackets ;
	int		_highestPacket ;

  protected:
	enum Status {
			Inactive		= 0,		// not started
			Ready			= 1,		// ready to receive
			Receiving		= 2,		// receiving
			ReceivingBadData= 3,
			StopDueToError  = 4,		// fatal error occured
	};
	int		_status ;
	uchar	_jobId;						// currently processed job (0=none)

			void resizeMap	( int numPackets ) ;
			void resetMap   ( int numPackets ) ;		// resize + clear all
	inline  BOOL isMapSet	( int packetNumber )
	{
		if( packetNumber >= _maxPackets )
			return 0 ;
		int byteNumber = packetNumber / 8;
		int bitNumber  = packetNumber % 8;
		return _packetMap[byteNumber] & bits[bitNumber];
	}
	inline  void setMap		( int packetNumber )
	{
		ASSERT( packetNumber < _maxPackets ) ;
		int byteNumber = packetNumber / 8;
		int bit		   = bits[packetNumber % 8];
		if( (_packetMap[byteNumber] & bit) == 0 )
		{
			_packetMap[byteNumber] |= bit;
			_numPackets++ ;
			_highestPacket = __max( _highestPacket, packetNumber ) ;
		}
	}
	inline  void clearMap	()
	{
		memset( _packetMap, 0, _mapSize ) ;
		_numPackets= 0 ;
		_highestPacket = 0 ;
	}

			BOOL isReady	( int lastPacketNumber );	// not include the 0. packet ( the header packet )
	inline	BOOL isReady	()		{ return _numPackets >= _maxPackets ; }
	inline	int	 numPackets ()		{ return _numPackets ; }
	inline	int	 maxPackets ()		{ return _maxPackets ; }
	inline  int  highestPacket()	{ return _highestPacket ; }

	int			 maxUnknownInterval() ;
	BOOL		 isIntervalReady( int i1, int i2 ) ;

	// read/write bitmap from actual position <_mapSize><_maxPackets><_numPackets><_packetMap>
	BOOL		 readBitmap	( FILE *bmpFp );			// read bitmap from file
	void		 writeBitmap( FILE *bmpFp );			// write bitmap to file
	
	inline BaseReceiver( )
	{
		_packetMap  = NULL ;
		_mapSize    = 0 ;
		_numPackets = 0 ;
		_maxPackets = 0 ;
		_status		= Inactive;
		_jobId		= 0;
		_highestPacket = 0 ;
	}
	inline ~BaseReceiver( )
	{
		if ( _packetMap )
			free( _packetMap ) ;
	}
  public:
	inline	BOOL isStarted		()						{ return ( _status != Inactive ) && ( _status != StopDueToError ); }
	inline	BOOL isReceiving	()						{ return _status == Receiving; };
	virtual void push			( MuxPacket *packet )	{};	// receiver input
} ;


//---------------------------------------------------------------------------
//	DataSender/Receiver
//---------------------------------------------------------------------------


class DataSender : public BaseSender
{
	int			_status ;
	enum Status {
			Inactive	= 0,			// not started
			Ready		= 1,			// ready to send
			Sending		= 2,			// sending
	};

  public:
	DataSender( MuxOutput *o, HANDLE hKillEvent, HANDLE hNumFreePacketAvailable, long *n_freepack ) ;
	inline ~DataSender()				{ if( isStarted() ) stop(); }

	inline	void start()				{ ASSERT( !isStarted() ); _status= Ready; }
	inline	BOOL isStarted()			{ return _status != Inactive ; }
	inline  BOOL isSending()			{ return _status == Sending ; }
	inline	void stop()					{ ASSERT(  isStarted() ); _status= Inactive; }

	// <data> is formatted (private format) and sent using MuxPacket protocol.
	// On the receiver side packets must be passed to DataReceiver::push(), which
	// after all packets arrive calls virtual function processData().
	// All or nothing delivery, data contents is checked by CRC.
	// Max. 64K <data> allowed.
	// Return value = error code
	int sendData(
		ushort		flags,							// MuxPacket flags
		const char *data,							// data to be sent
		int			n_bytes,						// data length
		ushort		channel,						// MuxPacket channel
		uchar		streamFmt,						// Piping or MPE
		int			numRebroadcasts,				// how many times send; must be >= 1!
		PidStreamAttrib *pidStrAttr,				// PID to be used
		const GlobalUserID& _usrId=GlobalUserID()	// unicast only
	) ;

	// data = HNet data as returned from IoHNet.dll
	// IP packets are extracted and sent as MPE stream.
	// At the receiver side the packets should be received by satellite card and passed
	// to tcp stack. (No application action required.)
	// Return value = error code
	int sendHNetDataAsMPE(
		const char *data,							// data to be sent
		int			n_bytes,						// data length
		int			numRebroadcasts,				// how many times send; must be >= 1!
		PidStreamAttrib *pidStrAttr					// PID to be used
	) ;
} ;


class DataReceiver : public BaseReceiver
{
  protected:
	uchar  *_data ;					// data buffer
	BOOL	_isPaused;	

	ulong	expPacketIndex;
	ulong	lostPackets;
	BOOL	firstPacket;

	inline BOOL		   hasHeader()		{ return isMapSet(0) ; }

  public:
	inline  DataReceiver()				{ _data= NULL ; _status= Inactive ; _isPaused = FALSE; }
    inline ~DataReceiver()				{ if( isStarted() ) stop(); }

			void start		();
	inline 	void pause		()			{ _isPaused = TRUE; };
			void stop		();

	virtual void push( MuxPacket *packet ) ;
	virtual int  processData( ushort flags, uchar *data, int n_bytes, ushort channel ) = 0 ;
};


//---------------------------------------------------------------------------
//	FileSender/Receiver
//---------------------------------------------------------------------------


class FileSender : public BaseSender
{
	unsigned int bufSize;
	char		*data;
	unsigned int bottom;					// data = <bottom,pos)
	unsigned int pos;						// 1-based
	ushort		_flags ;
	uchar		 packetSize ;
	uchar		 checkSum ;

	int			 fecLevel ;					// 0-none, 1-low, 2-medium, 3-high
	long		 fecTotalPackets ;			// number of packets past fecPacketIndex (until EOF)
	long		 fecBlkSize ;				// number of packets to be protected by FEC in one go
	long		 fecPacketIndex ;			// within fecBlkSize
	long		 fecBlkIndex ;				// from file start; in multiples of 512
	FECDesc		 fecDesc ;					// definition of FEC cycles
	MuxPacket	*fecPackets[4] ;			// buffers for up to 4 different cycles
	int			 fecPacketsLength[4] ;		// allocated length of fecPackets[] buffers
	
	void		initFEC( int nPackets=0 ) ;


	inline unsigned int dataCount ()	{ return pos-bottom ; }
	inline unsigned int bytesFree ()	{ return bufSize-pos; }
	inline BOOL		 hasFullPacket()	{ return bottom+packetSize <= pos ; }
	inline BOOL		 hasData	  ()	{ return bottom < pos;}

	inline BOOL addBuffer( const void *buff, unsigned int size ) ;
	inline void squeeze( )	;				// remove sent data
		   int  sendPacket	( int len, PidStreamAttrib *pidStrAttr ) ;
	inline int  sendPackets (PidStreamAttrib *pidStrAttr ) ;	// -> err
	inline int  flushPackets(PidStreamAttrib *pidStrAttr ) ;	// -> err
	inline int	sendAgregatedPackets( int nPackets, PidStreamAttrib *pidStrAttr ) ;// -> err

  public:
	// Creates permanent FileSender sending packets to given output device <o> and controlled by:
	// hKillEvent			= event killing the send process (use SetEvent())
	// numfreepack			= counter containing allowed # of packets to be sent
	// hFreePacketAvailable	= event announcing that above counter was increased externally
	//						 (use PulseEvent())
	//
	// After the construction you may call sendFile() repeatedly.
	FileSender( MuxOutput *o, HANDLE hKillEvent, HANDLE hFreePacketAvailable, long *numfreepack ) ;
   ~FileSender() ;

	// This form of sendFile() is used to send single opened file 1 time.
	// fileName  = path relative to the inbox directory (used in the Send file protocol)
	// hFile	 = file handle (open so that modification is prevented)
	// rebrIndex = ...,3,2,1 (rebroadcast index)
	// jobId	 = on 1st rebroadcast set to 0; on next rebroadcasts use value returned from 1st rebroadcast
	// usrId	 = use 0 for multi/broadcast
	// channel	 = virtual channel used
	// broadcastFlag =
	//			   0					(broadcast if <channel>==0; transfer via inbox channel otherwise),
	//			   MuxPacket::Unicast	(unicast via service channel; <channel> must be 0), or
	//			   MuxPacket::Multicast (multicast on <channel> users via service channel)
	// returns DVB error code (no exception)
	int sendFile( const char *fullFileName, const char *fileName, HANDLE hFile,
		ushort channel, uchar streamFmt, int fecLevel, int rebrIndex, int maxNumRebr, PidStreamAttrib *pidStrAttr,
		const GlobalUserID& usrId, uchar &jobId, ushort broadcastFlag=0 ) ;

	// This form of sendFile() is used to send single file <numRebroadcasts> times without
	// any delay. Send file protocol will contain only short file name (name+extension).
	// Other parameters - see above.
	int sendFile( const char *fullFileName, ushort channel, uchar streamFmt, int fecLevel, int numRebroadcasts,
		PidStreamAttrib *pidStrAttr, const GlobalUserID& usrId, ushort broadcastFlag) ;
};


struct FileRcvFileSent;
struct FecPacketHdr ;
struct FecPacket ;

class FileReceiver : public BaseReceiver
{
  protected:
	CRITICAL_SECTION _fileReceiverLock;
	char   _dir[_MAX_PATH];				// directory where received files are copied
	BOOL   _overwriteMode ;				// FALSE: new file version (*~nnn) created
	BOOL   _running ;					// TRUE iff started
	char   _rcvFileName[_MAX_PATH] ;	// temporary file for receiving data

	int				 packetSize ;
	FileRcvFileSent *fileInfo;			// info about the file being received
	MuxPacket		_hdrPacket ;		// header packet

	BOOL	_ignoreData;				// TRUE iff incoming packets have to be ignored
	BOOL	_dataReady ;				// TRUE iff the whole file is received in good condition
	int		_rebroadcastInd ;			// ...,2,1 (1=last) or 0 for infinite broadcast
	int		_numOfRebroadcast;			//	-1 unknown, 0 infinit
	int		_lastPacketInd ;			//  -1 unknown

	// FEC statistics
	int		_numRecoveredPackets ;		// # FEC recovered packets in first trial
	int		_numLateRecoveredPackets ;	// # FEC recovered packets in later trials
	int		_numFecPackets ;			// # all accepted packets
	int		_numBadFecPackets ;			// corrupted packets
	int		_numUndeterminedFecPackets;	// more than 1 base packet unknown
	int		_numUnneededFecPackets ;	// all base packets known
	int		_numFecReadErrors ;			// errors on calling getPacket()
	int		_numUnknownFecPackets ;		// (probably) last packet where we don't know its size
	int		_maxUnknownInterval ;		// max. # of consecutive missing packets after 1st rebroadcast is finished

	ushort	_channel;					// data channel (supplied to the constructor)
	BOOL	_isOutdirOK;				// FALSE: channel name unknown - tmp directory used

	// progress
	long		_nextProgress;
	LONGLONG	_progressTime;
	int			_progressCounter;

			void _prepareForStart	( ) ;
			void _startReceiving	( void *msg ) ;
			void makeFileVersion	( char *fn );
			void acceptPacket		( MuxPacket *packet, BOOL sameJob=TRUE ) ;

			void openFile			( BOOL openWithBitmap );
	inline	BOOL isFileOpened		( )							{ return _file != NULL ; }
			void closeFile			( );

	// File i/o
	long		 _file ;
	char *		 _fileBuf ;
	int			 _fileBufSize ;
	int			 _fileBufPtr  ;
	int			 _filePgSize  ;
	int			 _fileWrPtr1  ;
	int			 _fileWrPtr2  ;
	BOOL		 _fileChanged ;
	int			 _fileErr ;
	BOOL		 _flushingFileData ;

			BOOL _setFilePage		( long pos );
			void _flushFileData		( );

			void initFileData		( );
			int  openFileData		( const char *file_name, BOOL preserveOldData );
			BOOL writeFileData		( long pos, const unsigned char *bytes, int n_bytes ) ;
			BOOL readFileData		( long filePos, uchar *data, int n_bytes ) ;
			int  closeFileData		( ) ;
			void destroyFileData	( );

	// Fec utilities
	BOOL		 _lastPacketWasFec ;
	BOOL		 _fecBlocked ;
			BOOL applyFec			( MuxPacket *fecPacket, MuxPacket *recoveredPacket );
			BOOL processFecPacket	( FecPacketHdr *f, MuxPacket *fp, MuxPacket *recoveredPacket ) ;
			int  testBaseFecPackets	( FecPacketHdr *f) ;	// # unknown base packets: 0, 1, 2 (2 and more)

	// Fec queue
	FecPacket	*usedFecList ;
	FecPacket	*unusedFecList ;
			void openFecQueue		  ( ) ;
			void storeFecPacket		  ( FecPacketHdr *hdr, MuxPacket *mp ) ;
			void releaseFecPacket	  ( FecPacket *fp ) ;
			void releaseAllFecPackets ( ) ;
			void applyAllFecPackets	  ( ) ;
			void closeFecQueue		  ( ) ;

	// Bitmap
	void		 deleteInformationFile( );		// delete bitmap file with _jobId
	BOOL		 readInformationFile  ( );		// read data from bitmap file with _jobId
	void		 writeInformationFile ( );		// write data to bitmap file with _jobId

  public:
	// For NULL dir directory name is derived from channel name (taken from DvbClientSetup)
	FileReceiver					( ushort channel, const char *dir=NULL ) ;
   ~FileReceiver					( ) ;
	virtual void push				( MuxPacket *packet ) ;		// receiver input

	virtual void msgFun				( long code, void *param );

	void		 start				( BOOL overwriteMode=TRUE );
	void		 stop				( );

	BOOL	validateOutbox();
	void	destroyOutbox();
} ;



//---------------------------------------------------------------------------
//	InstallsReceiver
//---------------------------------------------------------------------------


class InstallsReceiver : public FileReceiver
{
  protected:
	BOOL	_newInstallation;

	BOOL isNewVersion( const char *fileName, char *version=0 );
  public:
	inline InstallsReceiver( const char *dir ) : FileReceiver( 0, dir )
	{
		_newInstallation = FALSE;
	}

	virtual void push( MuxPacket *packet );
};




//---------------------------------------------------------------------------
//	Specialised receivers
//---------------------------------------------------------------------------


class IOHNet ;

class InternetReceiver : public DataReceiver
{
	IOHNet *_ioHNet ;
	uint	timerID ;
	BOOL	_started;

	friend void CALLBACK  internetTimerFunct( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime );
  public:
	InternetReceiver() ;		// throws int exception if loading of HNet Dll failed
   ~InternetReceiver() ;

	BOOL start();
	void stop();
	virtual int processData( ushort flags, uchar *data, int n_bytes, ushort channel ) ;
} ;


class CommandReceiver : public BaseReceiver
{
	void			*_CAUser;
	int				numPakcets;
	int				numBytesInLastPacket;
	int				lastRecievedPacketIndex;
	BOOL			isBitmapOK;

  protected:
	void	clearCAUser();

  public:
	CommandReceiver();
	inline  ~CommandReceiver()					{ if( _CAUser != NULL ) FREE( _CAUser ); };
	virtual void push( MuxPacket *packet ) ;
};

#endif
