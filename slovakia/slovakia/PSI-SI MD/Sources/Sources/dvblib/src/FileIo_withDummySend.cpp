/*
 *	Filename:		fileio.cpp
 *
 *	Version:		1.00
 *
 *	Description:	This file contains implementation of various data sender and receiver classes
 *					
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"

#include <afxconv.h>
#include <errno.h>
#include "inbox.hpp"
#include "ClientCfg.hpp"
#include "ServCfg.hpp"
#include "BaseRegistry.hpp"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// a = no aliasing
// g = global
// t = fast machine code
// y = no stack frame
#define OPTOPTIONS	"agt"
//#define OPTOPTIONS	"agty"


#define MAX_BUFFER		4096
#define MAXSERVICESIZE	(65536+sizeof(ServiceHdr))
#define MY_ERRNO		DvbErrno( errno )
#define MAX_JOB_NUMB	0xff

//To suppres writing to the file uncomment following line
//#define DUMMY_WRITE	  

//#define DUMMY_SEND  

#define UPDATE_TIME		0.5f		// delay between 2 progress reports [sec]

// uncomment next define if you want to read files in sendFile with overlapped
// if you work lot with disks this enable to you end this process along reading
//#define READ_FILE_OVERLAPPED

//-------------------------------------------------------------------------------
// structures holding varius informations for data receiving/sending and messaging
//-------------------------------------------------------------------------------


//////// for moving file with replace of existing under win9x platform
static BOOL moveOrReplace( const char *source, const char *dest )
{ 
	if( rename( source, dest ) != 0 )  // error
	{
		if( errno == EACCES )
		{
			remove( dest ) ;
			return ( rename( source, dest ) == 0 ) ;
		}
		else
			return FALSE ;
	}
	return TRUE ;
}


// service header holds information for data sent on service channel
struct ServiceHdr
{
	uchar	version ;
	ushort	flags ;			// same as packet flags
	ulong	reserved ;
	ulong	length ;		// length of data
	ulong	crc ;
	const char *name()
	{
		if( flags & MuxPacket::Message )
			return "" ;
		if( flags & MuxPacket::Upgrade )
			return "Upgrade" ;
		if( flags & MuxPacket::UserLog )
			return "UserLog" ;
		//if( flags & MuxPacket::UserTable )
		//	return "UserTable" ;
		return "Service" ;
	}
	int operator== ( const ServiceHdr &src )  { return memcmp(this,&src,sizeof(ServiceHdr)) == 0 ; }
	int operator!= ( const ServiceHdr &src )  { return !(*this == src) ; }

	ServiceHdr() { }
	ServiceHdr( ushort _flags, ulong _length )
	{
		version = 1 ;
		flags	= _flags ;
		reserved= 0 ;
		length	= _length ;
	}
} ;

// file reiving status - used by messages
struct FileRcvStatus
{
	long		totalKbRec ;		// Kb from this file
	const char *currentFile ;
	float		percentFileRec ;
	FileRcvStatus( const char *f, long tkb, float p )
	{
		currentFile = f;
		totalKbRec = tkb;
		percentFileRec = p;
	}
} ;

// file receiving error report - message structure
struct FileRcvMsgError
{
	const char  *text ;
	DWORD		 error ;			// error code - dvb or win32
	FileRcvMsgError( const char *txt, DWORD err )
	{
		text = txt ; error = err ;
	}
} ;

// message struct. used to notify which file is receiving
struct FileRcvMsgReceiving
{
	const char  *fileName ;
	long		 fileSize ;
	int			 rebrInd ;				// for infinit always 0
	int			 numOfRebroadcast;		//	-1 unknown, 0 infinit
	FileRcvMsgReceiving( const char *fn, long fs, int ri, int nr )
	{
		fileName = fn; fileSize = fs; rebrInd = ri; numOfRebroadcast = nr;
	}
} ;

// different messages sent by receivers/senders and message data are of which type
enum FileReceiverMsgCode {
	StartProcess				=1,		// char *dir		receiving started
	EndProcess					=5,		// NULL
	RcvError					=6,		// FileRcvMsgError*
	RcvStatus					=9,		// FileRcvStatus*
	RcvProgress					=10,	// NULL
	RcvSpeed					=11,	// NULL
	StartReceiving				=12,	// NULL		receiving file
	EndReceiving				=13,	// NULL
	FileOpened					=100,	// FileRcvMsgReceiving*	copying of new file to outbox started
	FileCompleted				=101,	// FileRcvMsgReceiving*
	FileCompletedWithoutHeader	=102,	// FileRcvMsgReceiving*	data saved to file "unknown.*"
	FileIncompleteData			=103,	// FileRcvMsgReceiving*
	FileDamagedData				=104,	// FileRcvMsgReceiving*
	FileRefused					=105,	// FileRcvMsgReceiving* 
	FileAllreadyExisting		=106,	// FileRcvMsgReceiving*
	FileName					=107,	// FileRcvMsgReceiving*
	FileOpenedToAppend			=108,	// FileRcvMsgReceiving*
	// debug
} ;


//------------------------------------------------------------------------------
//	BaseSender - base class for sender classes
//------------------------------------------------------------------------------


BaseSender::BaseSender( MuxOutput *o, HANDLE _hKillEvent, HANDLE hNumFreePacketAvailable,
					    long *n_freepack )
{
	_jobId			= 0 ;
	muxOutput		= o ;
	channel			= -1 ;
	rebroadcastIndex= -1 ;
	hKillEvent		= _hKillEvent ;
	_hNumFreePacketAvailable = hNumFreePacketAvailable ;
	handleArray[0]	= hKillEvent;
	handleArray[1]	= hNumFreePacketAvailable;

	_numfreepack	= n_freepack ;
	usrId.makeInvalid() ;
	speed			= 0 ;
	packetInd		= 0;
	numPacketsSent	= 0 ;
}

static long	_total_numfreepack	 =0;	// total # available packets
static long _IP_min_numfreepack	 =0;	// # packets guaranteed for internet if not working
static long _IP_distr_numfreepack=0;	// # packets guaranteed for internet if working
extern BOOL isInternetWorking ;

// set the # of free packets
void setNumfreepack( long total, long IP, long dIP )
{
	_total_numfreepack		= total ;
	_IP_min_numfreepack		= IP;
	_IP_distr_numfreepack	= dIP;
}


#pragma optimize( OPTOPTIONS, on )

// take one packet from packet pool
inline BOOL takePacketFromPacketPool( BOOL takeForInternet )
{
	if( _total_numfreepack < 1 )
		return FALSE ;

	if ( takeForInternet )
	{
		if( _total_numfreepack > 0 )			// negative value could happen if 2nd thread decrements the value
		{
			InterlockedDecrement( &_total_numfreepack ) ;
			_IP_distr_numfreepack--;
			_IP_min_numfreepack = __min( _IP_min_numfreepack, _IP_distr_numfreepack );
		}
		return _total_numfreepack >= 0 ;
	}

	if ( isInternetWorking )
	{
		if ( _total_numfreepack <= _IP_distr_numfreepack )
			return FALSE;
	}
	else
	{
		if (_total_numfreepack <= _IP_min_numfreepack)
			return FALSE;
		else
		if (_total_numfreepack <= _IP_distr_numfreepack)
			_IP_distr_numfreepack-- ;
	}

	if( _total_numfreepack > 0 )			// negative value could happen if 2nd thread decrements the value
		InterlockedDecrement( &_total_numfreepack ) ;
	return _total_numfreepack >= 0 ;
}


// main data sending function
// - guaranties the # of distributed packets
// - makes packet from given data with given flag and send it to the output
// - updates the speed of channel
static int howMany = 0;

int BaseSender::sendPacket( ushort flg, char *data, int len )
{
	ASSERT( len < MAXSERVICESIZE ) ;

	// _numfreepack can only be changed on this place or by distributeRecordRequests()
	EnterCriticalSection( &MuxChannel::_distrLock );
	if( *_numfreepack <= 0 )
	{	// no free packet wait for it
		//TRACE( "\nWaiting when sent = %d", howMany );
		ResetEvent( _hNumFreePacketAvailable ) ;
		LeaveCriticalSection( &MuxChannel::_distrLock );
		zeroSpeed() ;
		if( WaitForMultipleObjects( 2, handleArray, FALSE, INFINITE) == WAIT_OBJECT_0 )
			return FALSE ;			// kill
		EnterCriticalSection( &MuxChannel::_distrLock );
	}
	BOOL isHNet = isInternetChannel() ;
	while ( !takePacketFromPacketPool( isHNet) )
	{	// not available packet wait for it
		//TRACE( "\nSent = %d", howMany );
		howMany = 0;
		LeaveCriticalSection( &MuxChannel::_distrLock );
		zeroSpeed() ;
		Sleep( isHNet ? DELAY_WAITING_FOR_FREE_IP_PACKETS : DELAY_WAITING_FOR_FREE_PACKETS );
		EnterCriticalSection( &MuxChannel::_distrLock );
	}

	howMany++;
	VERIFY( InterlockedDecrement( _numfreepack ) >= 0 );

	MuxPacket muxPacket ;
	if( !usrId.isValid() )
		muxPacket.makeDataPacket( channel, flg, len, rebroadcastIndex, packetInd, data, _jobId ) ;
	else
		muxPacket.makeUnicastPacket( channel, flg, len, rebroadcastIndex, packetInd, data, usrId, _jobId ) ;

	int err = muxOutput->put( &muxPacket ) ;
	LeaveCriticalSection( &MuxChannel::_distrLock );
	if( ++packetInd > 0xFFFFFF )
		packetInd = 0 ;
	numPacketsSent++;
	if (numPacketsSent % 10 == 0)
	{
		MuxMsgSendStatus st( numPacketsSent );
		if( speed == 0 )
		{
			oldSendStatus = st ;
			speed = -1 ;
		}
		else
		{
			//speed = st.outputRate( &oldSendStatus ) ;

			//TRACE( "\n%d: SPEED %5.4f : %02d:%02d:%02d:%03d -> %02d:%02d:%02d:%03d = %I64d            %I64d -> %I64d", 
			//	channel, speed, oldSendStatus.st.wHour, oldSendStatus.st.wMinute, oldSendStatus.st.wSecond, oldSendStatus.st.wMilliseconds,
			//	st.st.wHour, st.st.wMinute, st.st.wSecond, st.st.wMilliseconds, st.timeStamp - oldSendStatus.timeStamp, oldSendStatus.nPacketsSent, st.nPacketsSent ) ;

			packets = (long)(st.nPacketsSent - oldSendStatus.nPacketsSent);
			oldSendStatus = st ;
			//MfxPostMessage( EMsg_InboxSpeed, channel, *(long*)(&speed) );
			if(!MfxDvbSetup() || MfxDvbSetup()->notShowProgress()==FALSE )
				MfxPostMessage( EMsg_InboxSpeed, channel, packets );
		}
	}
	return err ;
}

#pragma optimize( "", on )			// restore original optimization options


//------------------------------------------------------------------------------
//	BaseReceiver - base class for receiver classes
//------------------------------------------------------------------------------


static uchar bits	 [8]   = { 1, 2, 4, 8, 16, 32, 64, 128 };
static uchar bitMasks[8]   = { 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff} ;

// test wheter the packet with given number is already received
inline BOOL BaseReceiver::isMapSet( int packetNumber )
{
	if( packetNumber >= _maxPackets )
		return 0 ;
	int byteNumber = packetNumber / 8;
	int bitNumber  = packetNumber % 8;
	return _packetMap[byteNumber] & bits[bitNumber];
}

// set the packet with given number to be received
// set the packetNumber-th bit in the receiving map to 1
inline void BaseReceiver::setMap( int packetNumber )
{
	ASSERT( packetNumber < _maxPackets ) ;
	int byteNumber = packetNumber / 8;
	int bit		   = bits[packetNumber % 8];
	if( (_packetMap[byteNumber] & bit) == 0 )
	{
		_packetMap[byteNumber] |= bit;
		_numPackets++ ;
	}
}

// TRUE iff all bits from 1. to the lastPacketNumber-th ( including ) are set
BOOL BaseReceiver::isReady( int lastPacketNumber )
{
	int  lastByteNumber= lastPacketNumber / 8;
	int  numLastBits   = lastPacketNumber % 8;
	if ( lastByteNumber > 0 )
	{
		if( _packetMap[0] != 0xfe )
			return FALSE ;
		for( int i=1; i < lastByteNumber; i++ )
			if( _packetMap[i] != 0xff )
				return FALSE ;
		if( numLastBits > 0 )
			if( _packetMap[lastByteNumber] != bitMasks[numLastBits] )
				return FALSE ;
	}
	else
	{
		if( numLastBits > 0 )
			if( _packetMap[lastByteNumber] != bitMasks[numLastBits] - 0x01 )
				return FALSE ;
	}
	return TRUE;
}

// clears the receiving map
inline void BaseReceiver::clearMap()
{
	memset( _packetMap, 0, _mapSize ) ;
	_numPackets= 0 ;
}

// reset the size of the receiving map to be able to hold informations on nPackets packets
// clear the map contents
inline void BaseReceiver::resetMap( int nPackets )
{
	int newSize = (nPackets+7)/8 ;
	if( newSize > _mapSize )
	{
		_mapSize = newSize + 128 ;
		_packetMap = (uchar*)realloc( _packetMap, _mapSize ) ;
	}
	_maxPackets = nPackets ;
	clearMap() ;
}

// resize the receiving map to be able to hold informations on nPackets packets
// map contents are preserved
void BaseReceiver::resizeMap( int nPackets )
{
	if( nPackets == _maxPackets )
		return ;

	if( nPackets > _maxPackets )
	{
		int newSize = (nPackets+7)/8 ;
		if( newSize > _mapSize )
		{
			int oldMapSize = _mapSize ;
			_mapSize = newSize + 128 ;
			_packetMap = (uchar*)realloc( _packetMap, _mapSize ) ;
			memset( _packetMap+oldMapSize, 0, _mapSize-oldMapSize ) ;
		}
	}
	_maxPackets = nPackets ;

	int   lastIndex= (_maxPackets+7)/8 ;
	_packetMap[lastIndex] &= bits[_maxPackets%8] ;
}

// write the map to a file - needed to be able to continue receiving of current file
void BaseReceiver::writeBitmap( FILE *bmpFp )
{
	if( bmpFp != NULL )
	{
		fwrite( &_mapSize   , sizeof(_mapSize)   , 1      , bmpFp);
		fwrite( &_maxPackets, sizeof(_maxPackets), 1      , bmpFp);
		fwrite( &_numPackets, sizeof(_numPackets), 1      , bmpFp);
		fwrite( _packetMap  , sizeof(uchar)      ,_mapSize, bmpFp );
	}
}

// read the previusly saved map from the file
void BaseReceiver::readBitmap( FILE *bmpFp )
{	// TRUE iff bitmap was correctly readed
	if( bmpFp != NULL )
	{
		fread( &_mapSize   , sizeof(_mapSize)   , 1       , bmpFp);
		_packetMap  = (uchar*)realloc( _packetMap, _mapSize ) ;
		clearMap();
		fread( &_maxPackets, sizeof(_maxPackets), 1      , bmpFp);
		fread( &_numPackets, sizeof(_numPackets), 1      , bmpFp);
		fread( _packetMap  , sizeof(uchar)      , _mapSize, bmpFp );
	}
}

//------------------------------------------------------------------------------
//	DataSender - sender of different types of data stored in the memory
// sends data as service - with ServiceHdr
// given data are rebroadcasted on one run - rebroadcast by rebr.
// this do not allows mixing packets for this data with packets of other data
//------------------------------------------------------------------------------


// protocol:
//		<hdr> <data>

DataSender::DataSender( MuxOutput *o, HANDLE _hKillEvent, HANDLE hNumFreePacketAvailable,
					   long *n_freepack ) :
		BaseSender( o, _hKillEvent, hNumFreePacketAvailable, n_freepack )
{
	_status = Inactive ;
}

// send n_bytes bytes from data on channel with num. _channel, with flags and _usrId set
// in all packets
// - data are sent as service
// - appends a service header to the start of data
// - provides dividing and packeting of data and sending of this packets to the output
int DataSender::sendData( ushort flags, const char *data, int n_bytes,
			ushort _channel, int numRebroadcasts, const GlobalUserID& _usrId )
{
	int err = 0 ;
	ASSERT( isStarted() ) ;
	channel = _channel ;
	usrId   = _usrId ;
	_status = Sending ;
	nextJob() ;
	int packetSize = MUXDATASIZE - ((flags & MuxPacket::Unicast) ? sizeof(UnicastUserID) : 0);

	ServiceHdr	hdr(flags|MuxPacket::CrcComputed,n_bytes) ;
	char		rec1[256] ;
	int			rec1_len = packetSize-sizeof(ServiceHdr) ;
				rec1_len = __min( rec1_len, n_bytes ) ;

	hdr.crc = crc32( (const uchar*)data, n_bytes ) ;
	memcpy( rec1, &hdr, sizeof(ServiceHdr) ) ;
	memcpy( rec1+sizeof(ServiceHdr), data, rec1_len ) ;

	for( rebroadcastIndex=numRebroadcasts ; rebroadcastIndex > 0 ; rebroadcastIndex-- )
	{
		packetInd = 0;
		err = sendPacket( flags, rec1, rec1_len+sizeof(ServiceHdr) ) ;
		for( int pos=rec1_len ; pos < n_bytes && err == 0; pos += packetSize )
		{
			int len = __min( packetSize, n_bytes-pos ) ;
			err = sendPacket( flags, (char*)(data+pos), len ) ;
		}
	}
	_status = Ready ;
	if( err != 0 )
	{
		char buf[512];
		const char *msg = DvbEventText( err, buf );
		MfxPostMessage( EMsg_CommunicationError, 0, msg );
	}
	return err ;
}

#pragma optimize( OPTOPTIONS, on )

// send internet data - same as sendData, only packe index is not changed betveen rebroadcasts
int DataSender::sendInternetData( ushort flags, const char *data, int n_bytes,
			ushort _channel, int numRebroadcasts, const GlobalUserID& _usrId )
{
	int err = 0 ;
	ASSERT( isStarted() ) ;
	channel = _channel ;
	usrId   = _usrId ;
	_status = Sending ;
	nextJob() ;
	int packetSize = MUXDATASIZE - ((flags & MuxPacket::Unicast) ? sizeof(UnicastUserID) : 0);

	ServiceHdr	hdr(flags|MuxPacket::CrcComputed,n_bytes) ;
	char		rec1[256] ;
	int			rec1_len = packetSize-sizeof(ServiceHdr) ;
				rec1_len = __min( rec1_len, n_bytes ) ;

	hdr.crc = crc32( (const uchar*)data, n_bytes ) ;
	memcpy( rec1, &hdr, sizeof(ServiceHdr) ) ;
	memcpy( rec1+sizeof(ServiceHdr), data, rec1_len ) ;

	for( rebroadcastIndex=numRebroadcasts ; rebroadcastIndex > 0 ; rebroadcastIndex-- )
	{
		//packetInd = 0;
		err = sendPacket( flags, rec1, rec1_len+sizeof(ServiceHdr) ) ;
		for( int pos=rec1_len ; pos < n_bytes && err == 0; pos += packetSize )
		{
			int len = __min( packetSize, n_bytes-pos ) ;
			err = sendPacket( flags, (char*)(data+pos), len ) ;
		}
	}
	_status = Ready ;
	if( err != 0 )
	{
		char buf[512];
		const char *msg = DvbEventText( err, buf );
		MfxPostMessage( EMsg_CommunicationError, 0, msg );
	}
	return err ;
}

#pragma optimize( "", on )			// restore original optimization options


//------------------------------------------------------------------------------
//	DataReceiver - receiver for different data types, data are received to the memory
// if a job is comming the full job must come before next job starts
// if next job is started before prev. job is finished, this means that prev. job is incomplette
//------------------------------------------------------------------------------

#define MAX_PACKETS	(MAXSERVICESIZE/(MUXDATASIZE - sizeof(long)) + 1)

// start receiving   
void DataReceiver::start( )
{
	if( _isPaused )
	{
		_isPaused = FALSE;
		return;
	}
	ASSERT( !isStarted() );

	_status	= Ready ;
	_jobId	= 0 ;
	// allocate max. possible space
	_data	= (uchar*)calloc( (MAX_PACKETS+1)*MUXDATASIZE, 1 ) ;

	expPacketIndex = 0;
	lostPackets = 0;
	firstPacket = TRUE;
}

// stop receiving
void DataReceiver::stop( )
{
	ASSERT( isStarted() );
	free( _data ) ;
	_data	= NULL ;
	_status	= Inactive ;
	_jobId	= 0 ;
}

// push one packet's data to the job's data
// test the continuity of data and the job index
// if new job index comes start receiving this new job and clear the prev. job
void DataReceiver::push( MuxPacket *packet )
{
	if ( !isStarted() )
		return;

	if( packet->packetIndex() > (int)(MAX_PACKETS) )			// may by corrupted packet
		return;

	if( packet->jobId() != _jobId )			// another job
	{
		if( _jobId != 0 )					// clear previous job
		{
			clearMap() ;
		}
		_jobId      = packet->jobId() ;
	}

	int packetInd = packet->packetIndex() ;
	int packetSize= MUXDATASIZE - (packet->isUnicastPacket() ? sizeof(UnicastUserID) : 0);
	if( packetInd == 0 )					// header packet
	{
		if( !hasHeader() )
		{
			ServiceHdr *hdr = (ServiceHdr *)packet->data() ;
			resizeMap( (hdr->length +packetSize-1 + sizeof(ServiceHdr)) / packetSize ) ;
			if( hdr->flags & MuxPacket::Message )
				MfxClientSetup()->incNumMessagesTransferred();
		}
	}
	else					// allow accepting packets without header (i.e. unknown size)
	if( maxPackets() <= packetInd )
		resizeMap( packetInd+10 ) ;		// we set map size to the forbidden value

	// packet control
	if( firstPacket )
	{
		firstPacket = FALSE;
		expPacketIndex = (ulong)packetInd;
	}
	if( expPacketIndex != (ulong)packetInd )
	{
		if( expPacketIndex > (ulong)packetInd )
			lostPackets += ( 0xffffff - expPacketIndex ) + packetInd;
		else
			lostPackets += packetInd - expPacketIndex;
		expPacketIndex = packetInd;
	}
	expPacketIndex++;
	if( expPacketIndex > 0xffffff )
		expPacketIndex = 0;

	if( !isReady()  &&  packetInd < maxPackets()  &&  !isMapSet(packetInd) )
	{
		_status = Receiving ;
		memcpy( _data+packetInd * packetSize, packet->data(), packet->numDataBytes()) ;

		setMap( packetInd );

		if( isReady() )
		{
			_status = Ready;
			uchar *data   = _data+sizeof(ServiceHdr) ;
			int    n_bytes= ((ServiceHdr*)_data)->length ;

			if( (((ServiceHdr*)_data)->flags & MuxPacket::CrcComputed) == 0  ||
				 ((ServiceHdr*)_data)->crc == crc32( data, n_bytes) )
				processData( ((ServiceHdr*)_data)->flags, data, n_bytes, packet->channel() );
			else
				clearMap() ;		// corrupted data; try again
		}
	}
}

	
//---------------------------------------------------------------------------------------
// FileSender - sender of files
// provides file reading, packeting of the file data and sending of this packets
// one file is one job, jobs can be mixed (file's rebroadcasts must not be sent on one run)
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// FileSender - primitives
//---------------------------------------------------------------------------------------

// merge data from previous read with curently readed data
inline BOOL FileSender::addBuffer( const void *buff, unsigned int size )
{
	ASSERT( pos+size <= bufSize ) ;
	memcpy( data+pos, buff, size ) ;
	pos += size;
	return TRUE;
}

// remove sent data
inline void FileSender::squeeze( )
{
	if( bottom <= 0 )
		return ;
	if( bottom >= pos )
	{
		bottom = pos = 0;
		return ;
	}
	pos -= bottom ;
	memmove( data, data+bottom, pos ) ;
	bottom = 0 ;
}


#pragma optimize( OPTOPTIONS, on )

// send one packet with data readed from the file
inline int FileSender::sendPacket( )
{
	int  len = pos - bottom ;
	len = __min( len, packetSize ) ;
	int ret = BaseSender::sendPacket( _flags, data+bottom, len ) ;
	bottom += len;
	return ret ;
}

#pragma optimize( "", on )			// restore original optimization options


// send all full packets
inline int FileSender::sendPackets( )
{
	while( hasFullPacket()  &&  (WaitForSingleObject(hKillEvent,0) == WAIT_TIMEOUT) )
	{
		int err = sendPacket() ;
		if( err != 0 )
			return err ;
	}
	squeeze( );
	return 0 ;
}

// flushes out all packets
inline int FileSender::flushPackets( )
{
	while( hasData()  &&  (WaitForSingleObject(hKillEvent,0) == WAIT_TIMEOUT) )
	{
		int err = sendPacket() ;
		if( err != 0 )
			return err ;
	}
	return 0 ;
}


FileSender::FileSender( MuxOutput *o, HANDLE _hKillEvent, HANDLE hNumFreePacketAvailable,
					   long *n_freepack ) :
		BaseSender( o, _hKillEvent, hNumFreePacketAvailable, n_freepack )
{
	bufSize				= MAX_BUFFER ;
	data				= (char *)malloc( bufSize );
	bottom				= 0;
	pos					= 0;
	//_numRebroadcasts	= -1;
}


//---------------------------------------------------------------------------------------
// FileSender - sendFile() - send a file only once (one rebroadcast)
// - send a file job header with file name and attributes
// - read file data to a buffer, if file do not fit to the buffer read it in more stepps
// - send readed data, make from them packets and put then\m to the output
// can read files in two modes : overlapped and normal mode
//---------------------------------------------------------------------------------------
#ifndef DUMMY_SEND

int FileSender::sendFile( const char *fullFileName, const char *file_name, HANDLE hFile,
		ushort _channel, int _rebroadcastIndex, int _numRebroadcasts, const GlobalUserID& _usrId, uchar &jobId, ushort flags )
{
	_flags = flags | MuxPacket::File ;
	uchar ji;
	if ( jobId == 0 )
	{
		nextJob();
		jobId = _jobId;
		ji = _jobId;
	}
	else
	{
		ji = _jobId;
		_jobId = jobId;
	}
	#ifdef READ_FILE_OVERLAPPED
		OVERLAPPED	overlapped;
		overlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
		if ( overlapped.hEvent==NULL )
		{
			_jobId = ji;
			return GetLastError();
		}
		HANDLE handles[2];
		handles[0] = hKillEvent;
		handles[1] = overlapped.hEvent;
	#endif
	
	channel			 = _channel ;
	rebroadcastIndex = _rebroadcastIndex ;
	usrId			 = _usrId ;
	packetInd		 = 0;

	packetSize		 = usrId.isValid() ? MUXDATASIZE-sizeof(GlobalUserID) : MUXDATASIZE ;
	bottom			 = 0;
	pos				 = 0;
	int sendError	 = 0 ;
	ulong checkSum	 = 0;
	__try
	{
		const char *fileName = file_name;
		long fSize;
		fSize = GetFileSize( hFile, NULL ) ;
		time_t fTime;
		FILETIME ft;
		GetFileTime( hFile, NULL, NULL, &ft );
		CTime ct( ft );
		fTime = ct.GetTime();
		DWORD fattr = GetFileAttributes( fullFileName );

		#ifdef READ_FILE_OVERLAPPED
			long  fPos		 = 0;
		#endif

//
// send file protocol:
//		packets 0,1,...n:	<fileName>\0<length><fileTime><attribs>
//		packets 0,1... n:	copy of the first packet series with the same packetInd
//		other packets:		<contents>
// where
//		fileName = file path relative to inbox directory (char*)
//		length   = file size in bytes (long)
//		fileTime = file last access time (time_t)
//		attribs  = as returned by GetFileAttributes()
//		contents = file itself as a stream of bytes
//
		int   fileNameLen= strlen(fileName) + 1 ;
		addBuffer( fileName , fileNameLen   );
		addBuffer( &fSize	, sizeof(long)  );
		addBuffer( &fTime	, sizeof(time_t));
		addBuffer( &fattr	, sizeof(DWORD) );
		addBuffer( &_numRebroadcasts, sizeof( int ) );
		sendError = flushPackets();				// send packet with header
		packetInd = 0;
		addBuffer( fileName , fileNameLen   );
		addBuffer( &fSize	, sizeof(long)  );
		addBuffer( &fTime	, sizeof(time_t));
		addBuffer( &fattr	, sizeof(DWORD) );
		addBuffer( &_numRebroadcasts, sizeof( int ) );
		sendError = flushPackets();				// send packet with header

		/*// dummy
		while ( 1 )
		{
			pos += bytesFree();
			sendPackets();
		}
		// dummy*/

		while( sendError == 0 )
		{
			unsigned int bytesToRead = bytesFree();
			DWORD		 bytesRead;
			#ifdef READ_FILE_OVERLAPPED
				overlapped.Offset = fPos;
				overlapped.OffsetHigh = 0;
				ReadFile( hFile, data+pos, bytesToRead, &bytesRead, &overlapped );
				if( WaitForMultipleObjects( 2, handles, FALSE, INFINITE ) == WAIT_OBJECT_0+1 )
				{
					if( !GetOverlappedResult( hFile, &overlapped, &bytesRead, FALSE) )
						sendError = GetLastError() ;
					fPos   += bytesRead;
			#else
				if( !ReadFile( hFile, data+pos, bytesToRead, &bytesRead, NULL) )
					sendError = GetLastError() ;
				else
				if( WaitForSingleObject(hKillEvent,0) == WAIT_TIMEOUT )
				{
			#endif
					for ( uint i = 0; i<bytesRead; i++ )
						checkSum += (unsigned char)(data[pos+i]);
					pos += bytesRead ;
					if( sendError == 0 )
						sendError = sendPackets( );
					if( bytesToRead != bytesRead )
						break;
				}
				else
					sendError = DvbErr_InboxKilled ;
		}

		if( sendError == 0 )
		{
			addBuffer( &checkSum, sizeof(checkSum) );
			sendError = flushPackets( );
		}

		#ifdef READ_FILE_OVERLAPPED
			CancelIO( hFile );
			WaitForSingleObject( overlapped.hEvent, INFINITE );
			CloseHandle( overlapped.hEvent );
		#endif
	}
	CATCH_EXCEPTION_CODE
	{
  		sendError = EXCEPTION_CODE ;
		#ifdef READ_FILE_OVERLAPPED
			CancelIO( hFile );
			WaitForSingleObject( overlapped.hEvent, INFINITE );
			CloseHandle( overlapped.hEvent );
		#endif
		_jobId = ji;
		return 1;
	}

	_jobId = ji;
	return sendError ;
}

// send a file - all rebroadcasts on one run, one by one
int FileSender::sendFile( const char *fullFileName, ushort channel, int numRebroadcasts,
						  const GlobalUserID& _usrId, ushort flags)
{
	HANDLE   hFile = INVALID_HANDLE_VALUE;
	DWORD    exc=0 ;
	char     shortFileName[256], filename[1024], ext[80] ;
	char	*buf = filename ;

	GlobalUserID *uID = NULL ;
	if ( flags & MuxPacket::Unicast )
		uID = new GlobalUserID( _usrId ) ;

	nextJob() ;
	speed = 0 ;
	__try
	{
		_splitpath( fullFileName , NULL, NULL, filename, ext ) ;
		_makepath ( shortFileName, NULL, NULL, filename, ext ) ;

		hFile = CreateFile( fullFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
		if( hFile==INVALID_HANDLE_VALUE )
			exc = GetLastError() ;
		else
		{
			int _numRebroadcasts = numRebroadcasts;
			for( ; numRebroadcasts > 0; numRebroadcasts-- )
			{
				sprintf( buf, "(%d/%d)%s Sending...", _numRebroadcasts - numRebroadcasts + 1, _numRebroadcasts, shortFileName ) ;
				if ( flags & MuxPacket::Unicast )
					MfxPostMessage( EMsg_InboxUnicasting, (long)uID, buf );
				else if ( flags & MuxPacket::Multicast )
					MfxPostMessage( EMsg_InboxMulticasting, channel, buf );
				else
					MfxPostMessage( EMsg_InboxBroadcasting, channel, buf );

				exc = sendFile( fullFileName, shortFileName, hFile, channel, 
								numRebroadcasts, _numRebroadcasts, _usrId, _jobId, flags );
				if( exc != 0 )
					break ;

				sprintf( buf, "(%d/%d)%s Send completed... OK", _numRebroadcasts - numRebroadcasts + 1, _numRebroadcasts, shortFileName ) ;
				MfxPostMessage( EMsg_InboxSendCompleted, channel, buf );

				SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
			}
		}
	}
	CATCH_EXCEPTION_CODE
	{
  		exc = EXCEPTION_CODE ;
	}

	if ( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile );
	if( exc != 0 )
	{
		char txt[256] ;
		sprintf( buf, "(%d)%s Send failed... %s", numRebroadcasts, shortFileName, DvbEventText(exc,txt) ) ;
		MfxPostMessage( EMsg_InboxSendCompleted, channel, buf );
	}
	zeroSpeed() ;
	return exc ;
}

#else

//////////////////
// dummy sending

static 	long fSize = 0;

int FileSender::sendFile( const char *fullFileName, const char *file_name, HANDLE hFile,
		ushort _channel, int _rebroadcastIndex, int _numRebroadcasts, const GlobalUserID& _usrId, uchar &jobId, ushort flags )
{
	int sendError	 = 0 ;

	_flags = flags | MuxPacket::File ;
	uchar ji;
	if ( jobId == 0 )
	{
		nextJob();
		jobId = _jobId;
		ji = _jobId;
	}
	else
	{
		ji = _jobId;
		_jobId = jobId;
	}
	
	channel			 = _channel ;
	rebroadcastIndex = _rebroadcastIndex ;
	usrId			 = _usrId ;
	packetInd		 = 0;

	packetSize		 = usrId.isValid() ? MUXDATASIZE-sizeof(GlobalUserID) : MUXDATASIZE ;
	bottom			 = 0;
	pos				 = 0;
	sendError	 = 0 ;
	ulong checkSum	 = 0;
	try
	{
		srand(GetTickCount());
		char fileName[50];
		sprintf(fileName,"GenFile%d",rand()) ;

		time_t fTime;
		time(&fTime) ;
		DWORD fattr = 0;

		int   fileNameLen= strlen(fileName) + 1 ;
		addBuffer( fileName , fileNameLen   );
		addBuffer( &fSize	, sizeof(long)  );
		addBuffer( &fTime	, sizeof(time_t));
		addBuffer( &fattr	, sizeof(DWORD) );
		addBuffer( &_numRebroadcasts, sizeof( int ) );
		sendError = flushPackets();				// send packet with header
		packetInd = 0;
		addBuffer( fileName , fileNameLen   );
		addBuffer( &fSize	, sizeof(long)  );
		addBuffer( &fTime	, sizeof(time_t));
		addBuffer( &fattr	, sizeof(DWORD) );
		addBuffer( &_numRebroadcasts, sizeof( int ) );
		sendError = flushPackets();				// send packet with header


		unsigned int toSend = fSize ;
		while( sendError == 0 && toSend )
		{
			unsigned int bytes = __min( bytesFree(), toSend ) ;
//			for ( uint i = 0; i<bytes; i++ )
//				checkSum += (unsigned char)(data[pos+i]);
			pos += bytes ;
			toSend -= bytes ;
			if( sendError == 0 )
				sendError = sendPackets( );

			if( WaitForSingleObject(hKillEvent,0)==WAIT_OBJECT_0 )
			{
				sendError = DvbErr_InboxKilled ;
				break ;
			}
		}

		if( sendError == 0 )
		{
			addBuffer( &checkSum, sizeof(checkSum) );
			sendError = flushPackets( );
		}

	}
	catch(...)
	{
  		sendError = -1 ;
		_jobId = ji;
		return 1;
	}

	_jobId = ji;
	return sendError ;
}

// send a file - all rebroadcasts on one run, one by one
int FileSender::sendFile( const char *fullFileName, ushort channel, int numRebroadcasts,
						  const GlobalUserID& _usrId, ushort flags)
{
	DWORD    exc=0 ;

	int rebr = numRebroadcasts ;

	// load cfg
	int nFiles = 0 ;
	char buf[_MAX_PATH] ;
	CWinApp *app = AfxGetApp() ;
	GetModuleFileName( app->m_hInstance, buf,_MAX_PATH);
	char dir[_MAX_PATH], drive[_MAX_DRIVE] ; ;
	_splitpath( buf , drive, dir, NULL, NULL ) ;
	_makepath ( buf , drive, dir, "config\\dummySend.cfg", NULL ) ;
	ConfigClass config(buf) ;
	config.open() ;

	config.getInt( "nFiles", &nFiles) ;
	nFiles = __min(nFiles,50) ;
	for ( int i = 0; i < nFiles; i++ )
	{
		char varName[50] ;
		sprintf( varName,"Size%d", i+1 ) ;
		config.getInt( varName, (int*)&fSize) ;
		fSize = __min(fSize, 50000000 ) ; // max 50 MBytes

		HANDLE   hFile = INVALID_HANDLE_VALUE;
		DWORD    exc=0 ;
		char     shortFileName[256], filename[1024], ext[80] ;
		char	*buf = filename ;

		GlobalUserID *uID = NULL ;
		if ( flags & MuxPacket::Unicast )
			uID = new GlobalUserID( _usrId ) ;

		nextJob() ;
		speed = 0 ;
		try
		{
			_splitpath( fullFileName , NULL, NULL, filename, ext ) ;
			_makepath ( shortFileName, NULL, NULL, filename, ext ) ;

			numRebroadcasts = rebr ;
			int _numRebroadcasts = numRebroadcasts;
			for( ; numRebroadcasts > 0; numRebroadcasts-- )
			{
				sprintf( buf, "(%d/%d)%s Sending...", _numRebroadcasts - numRebroadcasts + 1, _numRebroadcasts, shortFileName ) ;
				if ( flags & MuxPacket::Unicast )
					MfxPostMessage( EMsg_InboxUnicasting, (long)uID, buf );
				else if ( flags & MuxPacket::Multicast )
					MfxPostMessage( EMsg_InboxMulticasting, channel, buf );
				else
					MfxPostMessage( EMsg_InboxBroadcasting, channel, buf );

				exc = sendFile( fullFileName, shortFileName, hFile, channel, 
								numRebroadcasts, _numRebroadcasts, _usrId, _jobId, flags );
				if( exc != 0 )
					break ;

				sprintf( buf, "(%d/%d)%s Send completed... OK", _numRebroadcasts - numRebroadcasts + 1, _numRebroadcasts, shortFileName ) ;
				MfxPostMessage( EMsg_InboxSendCompleted, channel, buf );

				SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
			}
		}
		catch(...)
		{
  			exc = -1 ;
		}

		if ( hFile != INVALID_HANDLE_VALUE )
			CloseHandle( hFile );
		if( exc != 0 )
		{
			char txt[256] ;
			sprintf( buf, "(%d)%s Send failed... %s", numRebroadcasts, shortFileName, DvbEventText(exc,txt) ) ;
			MfxPostMessage( EMsg_InboxSendCompleted, channel, buf );
		}
		zeroSpeed() ;
	}
	config.close() ;
	return exc ;
}
#endif

//------------------------------------------------------------------------------
//	FileReceiver - file receiving, receives data and stores them to a file
//------------------------------------------------------------------------------

// holds information on actually receiving file
struct FileRcvFileSent
{
	char	fileName[_MAX_PATH];// "???" for unknown name (packet lost)
	BOOL	wasAlreadySent ;	// TRUE iff in the previous rebr. was the same file
	long	fileSize ;			// [By]
	time_t	fileTime ;			// file last access time
	DWORD	fileAttr ;
	long	totalBytesSent ;	// only bytes successfully received
	BOOL	infoCorrect;		// all informations in this structure are valid
	ulong	checkSum;
	uint	numberOfRebrNeeded;	// how many rebr. received for this file

	inline BOOL equal( const char *path, long fsize, time_t ftime, DWORD fattr )
	{
		return	infoCorrect && 
			 strcmp( fileName, path ) == 0 &&
			 fileSize == fsize &&
			 fileTime == ftime &&
			 fileAttr == fattr;
	}
	inline void set( const char *path, long fsize, time_t ftime, DWORD fattr )
	{
		strcpy( fileName, path ) ;
		fileSize = fsize ;
		fileTime = ftime ;
		wasAlreadySent = FALSE;
		totalBytesSent = 0;
		fileAttr = fattr ;
	}
	inline void reset( )	{ memset( this, 0, sizeof(FileRcvFileSent) ) ; }
	FileRcvFileSent()		{ reset() ; }
} ;

// create a receiver with known outbox directory
// initialize the receiver, make the receiving directory
FileReceiver::FileReceiver( ushort channel, const char *dir ) : BaseReceiver()
{
	_status = Inactive;

	_channel    = channel;
	_isOutdirOK = TRUE;

	strcpy( _dir, dir ) ;
	int   len  = strlen( _dir ) ;
	if( _dir[ --len] != '\\'  &&  _dir[len] != '/' )
		strcat( _dir, "\\" );
	if( makeDir(_dir) != 0 )
	{
		FileRcvMsgError msg( "Can't create directory.", GetLastError() );
		msgFun( RcvError, &msg );
		_status = StopDueToError;
	}
	_rcvFp			= NULL ;
	_running		= FALSE ;
	_overwriteMode  = TRUE ;
	_ignoreData		= FALSE;
	_hasHeader		= FALSE;
	_packetInd		= 0;
	_usrId.makeInvalid() ;
	fileInfo		= NULL;
	fileInfo		= new FileRcvFileSent;	// holds information of actualy receiving file
	_dataReady		= FALSE;
	_jobId			= 0;

	_numFilesTransferred			= 0;
	_numFilesAcceptedSuccessfully	= 0;
	_numRebroadcastsTransferred		= 0;
	_totalFileSizeTransferred		= 0;

	clearJobMask();

	InitializeCriticalSection( &_fileReceiverLock );
}

// create outbox directory
// if not set by setup create a temporary one
// if previously temporary was created and correct dir is set, rename the temp. to the correct
BOOL FileReceiver::validateOutbox()
{
	if( _channel == 0 || _channel == 0xffff )
		return TRUE;

	DvbClientSetup *cSetup	= MfxClientSetup();
	char *drive				= cSetup->drive();
	char *dir				= cSetup->dir();
	const char *channelName	= cSetup->getChannelNameByID( _channel );
	char buff1[1024], buff2[256];

	if( channelName == NULL )
	{
		sprintf( buff2, "Received\\Channel%u.temporary", _channel );
		_makepath( _dir, drive, dir, buff2, NULL );
		return FALSE;
	}

	sprintf( buff2, "Received\\Channel%u.temporary", _channel );
	_makepath( buff1, drive, dir, buff2, NULL );
	sprintf( buff2, "Received\\%s", channelName );
	_makepath( _dir, drive, dir, buff2, NULL );

	if( dirExist( buff1 ) )
		return renameDir( buff1, _dir, TRUE );

	return TRUE;
}

// create a receiver with temporary directory
FileReceiver::FileReceiver( ushort channel ) : BaseReceiver()
{
	_status = Inactive;

	_channel = channel;
	_isOutdirOK = validateOutbox();

	int   len  = strlen( _dir ) ;
	if( _dir[ --len] != '\\'  &&  _dir[len] != '/' )
		strcat( _dir, "\\" );
	if( makeDir( _dir ) != 0 )
	{
		FileRcvMsgError msg( "Can't create directory.", GetLastError() );
		msgFun( RcvError, &msg );
		_status = StopDueToError;
	}
	_rcvFp			= NULL ;
	_running		= FALSE ;
	_overwriteMode  = TRUE ;
	_ignoreData		= FALSE;
	_hasHeader		= FALSE;
	_packetInd		= 0;
	_usrId.makeInvalid() ;
	fileInfo		= NULL;
	fileInfo		= new FileRcvFileSent;	// holds information of actualy receiving file
	_dataReady		= FALSE;
	_jobId			= 0;
	_rebroadcastInd = 0;
	_numOfRebroadcast = -1;

	_numFilesTransferred			= 0;
	_numFilesAcceptedSuccessfully	= 0;
	_numRebroadcastsTransferred		= 0;
	_totalFileSizeTransferred		= 0;

	clearJobMask();

	InitializeCriticalSection( &_fileReceiverLock );
}

// remove the temporary outbox directory
void FileReceiver::destroyOutbox()
{
	if( _channel == 0 || _channel == 0xffff )
		return;

	DvbClientSetup *cSetup	= MfxClientSetup();
	char *drive			= cSetup->drive();
	char *dir			= cSetup->dir();
	char buff1[1024], buff2[256];

	sprintf( buff2, "Received\\Channel%u.temporary", _channel );
	_makepath( buff1, drive, dir, buff2, NULL );
	if( dirExist( buff1 ) )
		rmWholeDir( buff1 );

	if( _isOutdirOK )
	{
		if( dirExist( _dir ) )
			rmWholeDir( _dir );
	}

	sprintf( buff1, "Outbox for channel %i was removed with all files.", _channel );
	MfxPostMessage( FileInf_OutboxRemoved, _channel, buff1 );
}

// stop the receiver, rename the temp dir to the correct if needed or remove the whole outbox dir if needed
FileReceiver::~FileReceiver()
{
	if ( _running )
		stop();
	delete fileInfo;

	DvbClientSetup *cSetup	= MfxClientSetup();
	if( cSetup->hasChannel( _channel ) )
	{
		if( !_isOutdirOK )
			validateOutbox();
	}
	else
	if( cSetup->deleteSubdirectory() )
	{
		destroyOutbox();
	}

	DeleteCriticalSection( &_fileReceiverLock );
}

// start the receiving (set the flags to the default state)
void FileReceiver::start( BOOL ovrMode )
{
	ASSERT( !_running );
	if ( _status == StopDueToError )
		return;
	_running	= TRUE ;
	_ignoreData = FALSE;
	_overwriteMode = ovrMode ;
	_packetInd	= 0;
	fileInfo->reset() ;
	_dataReady  = FALSE;
	_hasHeader	= FALSE;
	_status		= Ready;
	_jobId		= 0;
	_rebroadcastInd = 0;
	_numOfRebroadcast = -1;

	_numFilesTransferred			= 0;
	_numFilesAcceptedSuccessfully	= 0;
	_numRebroadcastsTransferred		= 0;
	_totalFileSizeTransferred		= 0;

	clearJobMask();

	msgFun( StartProcess, _dir );
}

// stop the receving
void FileReceiver::stop( )
{
	if( !_running )
		return;

	EnterCriticalSection( &_fileReceiverLock );
	_running = FALSE ;
	closeFile();
	_status = Inactive;
	LeaveCriticalSection( &_fileReceiverLock );

	msgFun( EndProcess, _dir );
}

// true if job with given index was received
inline BOOL FileReceiver::isJobMaskSet( int jobId  )
{
	if( jobId > MAX_JOB_NUMB )
		return 0;
	int byteNumber = jobId / 8;
	int bitNumber  = jobId % 8;
	return _jobMask[byteNumber] & bits[bitNumber];
}

// set in the job mask jobId-th bit
inline void FileReceiver::setJobMask( int jobId )
{
	ASSERT( jobId <= MAX_JOB_NUMB ) ;
	int byteNumber = jobId / 8;
	int bit		   = bits[jobId % 8];
	if( (_jobMask[byteNumber] & bit) == 0 )
		_jobMask[byteNumber] |= bit;
}

// clear in the job mask jobId-th bit
inline void FileReceiver::resetJobMask( int jobId )
{
	ASSERT( jobId <= MAX_JOB_NUMB ) ;
	int byteNumber = jobId / 8;
	int bit		   = bits[jobId % 8];
	if( (_jobMask[byteNumber] & bit) != 0 )
		_jobMask[byteNumber] ^= bit;
}

// makes file "receiving.*" and opens it
// (exc) on failure
void FileReceiver::openFile( BOOL openWithBitmap )
{
	_rcvFp = NULL;
	char path[1024] ;
	int err;
	sprintf( path, "receiving.%u", _jobId ) ;
	_makepath( _rcvFileName, NULL, _dir, path, NULL ) ;
	if ( openWithBitmap && readInformationFile() )
	{
		_makepath( path, NULL, NULL, fileInfo->fileName, "incomplete" ) ;
		if ( fileExist( path ) )
		{
			moveOrReplace( path, _rcvFileName );
			SetFileAttributes( _rcvFileName, FILE_ATTRIBUTE_NORMAL );
			_rcvFp = fopenRetry( _rcvFileName, "r+b", sFILESHARE_NOSHARE, &err );
		}
	}
	if ( _rcvFp == NULL )
	{
		_rcvFp = fopenRetry( _rcvFileName, "w+b", sFILESHARE_NOSHARE, &err );
	}
	if( _rcvFp == NULL )
	{
		sprintf( path, "Can't open file %s.", _rcvFileName );
		FileRcvMsgError msg( path, MY_ERRNO );
		msgFun( RcvError, &msg );
		_ignoreData = TRUE;
		return;
	}
	_status = Receiving;
	msgFun( StartReceiving, NULL );
	if( !_ignoreData && fileInfo->infoCorrect )
	{
		FileRcvMsgReceiving msg( fileInfo->fileName, fileInfo->fileSize, 0, _numOfRebroadcast );
		msgFun( FileName, &msg );
	}
}

// create a new version from the file name fn
// filename - original file name
// filename.xxx - new version
void FileReceiver::makeFileVersionName( char *fn, char *fnVersion, BOOL completize )
{
	char fnloc[_MAX_PATH];
	if ( completize )
		_makepath( fnloc, NULL, _dir, fn, NULL );
	else
		strcpy( fnloc, fn );
	for( int j=0 ; ; j++ )
	{
		sprintf( fnVersion, "%s~%03d", fnloc, j ) ;
		if ( !fileExist( fnVersion ) )
			break ;
	}
}

// make from old version of file new version
void FileReceiver::makeFileVersion( char *fn )
{
	char path[1024] ;
	makeFileVersionName( fn, path );
	if ( !moveOrReplace( fn, path ) )
	{
		FileRcvMsgError msg( "File I/O error.", GetLastError() );
		msgFun( RcvError, &msg );
	}
}

// delete a file
void FileReceiver::removeFile( const char *fname )
{
	DWORD atr = GetFileAttributes( fname ) ;
	if( atr & FILE_ATTRIBUTE_READONLY )
		SetFileAttributes( fname, atr & ~FILE_ATTRIBUTE_READONLY ) ;
	DeleteFile( fname );
}

// write information file for continuing receving this file in the future (this job)
void FileReceiver::writeInformationFile()
{
	FILE *_bmpFp = NULL;
	char path[1024], bmpFileName[_MAX_PATH];
	sprintf( bmpFileName, "%s.bitmap", _rcvFileName ) ;
	_bmpFp = fopen( bmpFileName, "wb" ) ;
	if( _bmpFp == NULL )
	{
		sprintf( path, "Can't open file %s.", bmpFileName );
		FileRcvMsgError msg( path, MY_ERRNO );
		msgFun( RcvError, &msg );
		return;
	}
	
	fwrite( fileInfo			, sizeof(FileRcvFileSent)	, 1, _bmpFp );
	fwrite( &_numOfRebroadcast  , sizeof(_numOfRebroadcast) , 1, _bmpFp);
	writeBitmap( _bmpFp );

	fclose( _bmpFp );
}

// read previously saved information file for current job
BOOL FileReceiver::readInformationFile()
{	// TRUE iff bitmap was correctly readed
	FILE *_bmpFp = NULL;
	char path[1024], bmpFileName[_MAX_PATH];
	sprintf( bmpFileName, "%s.bitmap", _rcvFileName ) ;
	if ( !fileExist( bmpFileName ) )
		return FALSE;
	_bmpFp = fopen( bmpFileName, "rb" ) ;
	if( _bmpFp == NULL )
	{
		sprintf( path, "Can't open file %s.", bmpFileName );
		FileRcvMsgError msg( path, MY_ERRNO );
		msgFun( RcvError, &msg );
		return FALSE;
	}
	
	fread( fileInfo				, sizeof(FileRcvFileSent)	, 1, _bmpFp );
	fread( &_numOfRebroadcast	, sizeof(_numOfRebroadcast)	, 1, _bmpFp);
	readBitmap( _bmpFp );

	BOOL err = ferror( _bmpFp );

	fclose( _bmpFp );

	if ( err )
		fileInfo->reset();

	return !err;
}

// remove information file for current job
void FileReceiver::deleteInformationFile( )
{
	char bmpFileName[_MAX_PATH];
	sprintf( bmpFileName, "%s.bitmap", _rcvFileName ) ;
	removeFile( bmpFileName );
}

// moves a file to the new location
static BOOL moveFile( const char *src, const char *dst )
{
	char path[1024], drive[256], dir[1024];
	_splitpath( dst, drive, dir, NULL, NULL );
	_makepath( path, drive, dir, NULL, NULL );
	if( !dirExist( path ) )
		makeDir( path );
	return moveOrReplace( src, dst );
}

// closes the received file - send ending messages depending on state of receiving
// state can be - completted, incompletted with known file name, etc.
void FileReceiver::closeFile()
{	// performs close, rename file and clears packetInd
	if ( !_rcvFp )
		return;
	BOOL err = ferror( _rcvFp );
	if ( err )
	{
		FileRcvMsgError msg( "File refused due to I/O error.", MY_ERRNO );
		msgFun( RcvError, &msg );
	}
	else
		if ( fileInfo->infoCorrect && !_ignoreData && _dataReady )
		{
			ulong cs;
			fseek( _rcvFp, fileInfo->fileSize, SEEK_SET );
			fread( &cs, sizeof(cs), 1, _rcvFp );
			for ( int i = 0; i<sizeof(ulong); i++ )
				fileInfo->checkSum -= *((unsigned char *)&cs + i);
			fseek( _rcvFp, 0, SEEK_END );
			if ( (fileInfo->fileSize + (long)sizeof(ulong) == ftell( _rcvFp )) &&
				 (cs == fileInfo->checkSum) )
			{
				_chsize( _fileno( _rcvFp ), fileInfo->fileSize );
			}
			else
			{
				int sizeInfo = fileInfo->fileSize + (long)sizeof(ulong);
				int sizeFile = ftell( _rcvFp );
				_status		= ReceivingBadData ;
				TRACE ( "\nError in data - Bad Data : size %d / %d - chS %d / %d", sizeInfo, sizeFile, fileInfo->checkSum, cs );
				_ignoreData = TRUE;
			}
			fileInfo->checkSum = 0 ;
		}
	if ( _rebroadcastInd == 1 )	// this was the last rebroadcast
	{
		resetJobMask( _jobId );
	}
	fclose( _rcvFp );
	_rcvFp = NULL;

	if ( err )
	{
		deleteInformationFile();
		removeFile( _rcvFileName );

		//FileRcvMsgError msg( "File refused due to I/O error.", Event_IoErrorFlag );
		//msgFun( RcvError, &msg );
		_status = Ready;
		
		return;
	}
	if ( fileInfo->infoCorrect )
	{
		FileReceiverMsgCode mcode;
		if ( _status == ReceivingBadData )
		{	// damaged
			deleteInformationFile();

			strcat( fileInfo->fileName, ".damaged" );

			moveOrReplace( _rcvFileName, fileInfo->fileName );
			setFileTime( fileInfo->fileName, fileInfo->fileTime );
			SetFileAttributes( fileInfo->fileName, fileInfo->fileAttr );
			
			mcode = FileDamagedData;
		}
		else
		if ( !_dataReady )
		{	// incomplete
			writeInformationFile();

			strcat( fileInfo->fileName, ".incomplete" );

			moveOrReplace( _rcvFileName, fileInfo->fileName );
			//setFileTime( fileInfo->fileName, fileInfo->fileTime );
			//SetFileAttributes( fileInfo->fileName, fileInfo->fileAttr );
			
			mcode = FileIncompleteData;
		}
		else
		if ( _ignoreData )
		{	// allready exist
			deleteInformationFile();
			removeFile( _rcvFileName );

			mcode = FileAllreadyExisting;
		}
		else
		{	// ok
			deleteInformationFile();

			// remove old file if exist
			if( fileExist( fileInfo->fileName) )
			{
				if ( !_overwriteMode )	// remove old file
					removeFile( fileInfo->fileName ) ;
				else					// rename old file to version fileName~nnn
					makeFileVersion( fileInfo->fileName );
			}
			//MoveFileEx( _rcvFileName, fileInfo->fileName, MOVEFILE_REPLACE_EXISTING );
			moveFile( _rcvFileName, fileInfo->fileName );
			setFileTime( fileInfo->fileName, fileInfo->fileTime );
			SetFileAttributes( fileInfo->fileName, fileInfo->fileAttr );
			
			DvbClientSetup *cSetup	= MfxClientSetup();
			cSetup->incNumRebroadcast( fileInfo->numberOfRebrNeeded );
			cSetup->incNumFilesAcceptedSuccessfully();
			cSetup->addToTotalFileSizeTransferred( fileInfo->fileSize );

			mcode = FileCompleted;
		}

		FileRcvMsgReceiving msg( fileInfo->fileName, fileInfo->fileSize, _rebroadcastInd, _numOfRebroadcast );
		msgFun( mcode, &msg );
		msgFun( EndReceiving, &msg );
	}
	else
	{	// informations of file are not valid
		deleteInformationFile();
		removeFile( _rcvFileName );
		clearMap() ;
	}
	_status = Ready;
}

// write data to a file
inline BOOL FileReceiver::writeData( const unsigned char *bytes, int n_bytes )
{
	#ifdef DUMMY_WRITE
		return TRUE;
	#else
		if( fwrite( bytes, n_bytes, 1, _rcvFp) == 1 )
			return TRUE ;
		// error writing file
		FileRcvMsgError msg( "File write error.", MY_ERRNO );
		msgFun( RcvError, &msg );
		return FALSE ;
	#endif
}

// return the number of missing byte from current file
inline int FileReceiver::numMissingDataBytes()
{
	return fileInfo->fileSize - ftell(_rcvFp) + sizeof(ulong) ;
}

// stepp over a given part of the file
inline void FileReceiver::skipData( int n_bytes )
{
	#ifdef DUMMY_WRITE
		return ;
	#else
		if ( fileInfo->wasAlreadySent )
			fseek( _rcvFp, n_bytes, SEEK_CUR );
		else
		{
			char data[256];
			fwrite( data, n_bytes, 1, _rcvFp );
		}
	#endif
}

// set the file pointer to the start of the file
inline void FileReceiver::rewindData( )
{
	rewind( _rcvFp );
}

// return the system time
static LONGLONG getSystemTime()
{
	FILETIME		ft;
	SYSTEMTIME		st;
	LARGE_INTEGER	t;

	GetLocalTime( &st );
	SystemTimeToFileTime( &st, &ft );
	t.LowPart	= ft.dwLowDateTime;
	t.HighPart	= ft.dwHighDateTime;
	return t.QuadPart;
}

/*	The algorithm of packet receiving, how are packets processed and where are modifyed number of rec. files and rebroadcast
if header comes
	if new job
		close previous job's file
		clear the packet map
	extract file info from header packet
	if this file already exist
		nothing to receive, return
	if file is in new job
		inc. # rebroadcasts
		read file info saved previously ( if exist ) and open file
		if this file is same as previous in this job
			completize file ( bitmap and file info readed from saved info file )
		if info file exist ( fileInfo is correct )
			wrong file is opened, close this and open a new one
		inc. # files received
		if file is opened
			send messages
	else
		if file info not exist
			inc. # rebroadcasts
			fill file info
			if this previously rec. part can be a part of it
				completize file
			else
				close previous file and open a new one
				inc. # files received
				if file is opened
					send messages
		else
			if this file is same as previous
				completize file from this rebr.
			else
				fill file info
				close previous file and open a new one
				inc. # rebroadcasts
				inc. # files received
				if file is opened
					send messages
	continue in receiving with next packet, return
else
	if new job
		close previous job's file
		clear the packet map
		read file info saved previously ( if exist ) and open file
		inc. # rebroadcasts
		if file info exist
			inc. # files received
			continue in receiving
		else
			if this packet can be a part of it
				continue in receiving
			else
				close opened file
				open a new one
				inc. # files received
				if opened
					send messages
				continue in receiving
	else
		continue in receiving
inc. # rebroadcasts by number depending on previous rebr. number and the actual rebr. number
*/
void FileReceiver::push( MuxPacket *packet )
{
	DvbClientSetup *cSetup	= MfxClientSetup();
	EnterCriticalSection( &_fileReceiverLock );
	try
	{
		if ( !_running )
		{
			LeaveCriticalSection( &_fileReceiverLock );
			return;
		}

		int rebroadcastInd	= packet->rebroadcastIndex();	// get the current rebr. number
		int packetInd		= packet->packetIndex() ;		// get the current packet index
		int packetSize		= MUXDATASIZE - (packet->isUnicastPacket() ? sizeof(UnicastUserID) : 0);

		if ( packetInd == 0 )
		{	// header packet
			if( packet->jobId() != _jobId )
			{
				closeFile();								// another job, close this file
				clearMap() ;
				fileInfo->checkSum = 0 ;
			}

			_hasHeader = TRUE;

			char fn[256], path[_MAX_PATH];
			long fn_len;
			long fsize;
			time_t ftime;
			DWORD fattr;
			long offset = 0;

			// extract file header
			const uchar *packetData = packet->data() ;
			strncpy( fn, (char *)(packetData+offset), MUXDATASIZE );
			fn_len  = strlen( fn )+1 ;
			offset += fn_len;
			fsize   = *(long *)( packetData+offset );
			offset += sizeof(long);
			ftime   = *(time_t *)( packetData+offset );
			offset += sizeof(time_t);
			fattr	= *(DWORD *)( packetData+offset );
			offset += sizeof(DWORD);

			_numOfRebroadcast = *(int *)( packetData+offset );
			offset += sizeof(int);
			//_rebroadcastInd = rebroadcastInd;

			_makepath( path, NULL, _dir, fn, NULL ) ;

			FileRcvMsgReceiving msg( path, fsize, rebroadcastInd, _numOfRebroadcast );

			if ( fileExist( path ) && ftime == fileTime( path ) )
			{	// if file exist in this version ignore packets data ( new job, if old
				_dataReady  = TRUE;
				_ignoreData = TRUE;
				if ( packet->jobId() != _jobId )
					msgFun( FileAllreadyExisting, &msg );	// send message only if new job arrives

				_jobId		= packet->jobId() ;

				LeaveCriticalSection( &_fileReceiverLock );

				return;						// nothing to do
			}

			if( packet->jobId() != _jobId )
			{	// another job
				_jobId		= packet->jobId() ;

				fileInfo->reset();
				_dataReady	= FALSE;
				_ignoreData	= FALSE;
				_hasHeader	= FALSE;

				openFile( TRUE );			// retrive info on this job if exist and open file for receive data to ( old or create a new one )

				_progressTime = getSystemTime();
				_nextProgress = 0;

				//cSetup->incNumRebroadcast();		// this is the next(first) rebroadcast of this job

				if ( fileInfo->equal( path, fsize, ftime, fattr ) )
				{	// the same file as received in prev. rebr. of this job, no modifications needed
					fileInfo->numberOfRebrNeeded += _rebroadcastInd - rebroadcastInd;	// next rebr.
					_rebroadcastInd = rebroadcastInd;
					LeaveCriticalSection( &_fileReceiverLock );

					msgFun( FileName, &msg );
					msgFun( FileOpenedToAppend, &msg );

					return;		// continue to completize file from this rebroadcast
				}

				if ( fileInfo->infoCorrect )
				{	// previously received file of this job was incomplette
					closeFile();		// close incomplette file and
					clearMap();
					openFile( FALSE );	// open a new one for this job
				}

				// fill fileInfo structure
				fileInfo->set( path, fsize, ftime, fattr ) ;
				fileInfo->numberOfRebrNeeded = 1;					// first rebr.
				_rebroadcastInd = rebroadcastInd;
				fileInfo->infoCorrect = TRUE;

				cSetup->incNumFilesTransferred();	// new file arriving

				_ignoreData = !isFileOpened();		// ignore data if can't open file

				if ( !_ignoreData )
				{
					msgFun( FileOpened, &msg );		// signalize opening
					msgFun( StartReceiving, NULL );	// signalize start of receiving
					msgFun( FileName, &msg );		// set file name in progress
					fseek( _rcvFp, 0, SEEK_SET );	// seek to the start of the file
				}
			}
			else
			{	// same job
				if ( !fileInfo->infoCorrect )
				{
					//cSetup->incNumRebroadcast();		// this is the next rebroadcast of this job

					// fill fileInfo structure
					fileInfo->set( path, fsize, ftime, fattr ) ;
					//fileInfo->numberOfRebrNeeded++;		// next/first rebr.
					fileInfo->infoCorrect = TRUE;

					int maxNumberOfPackets = ( fileInfo->fileSize + sizeof(ulong) + packetSize - 1) / packetSize + 1;
					if ( _packetInd > maxNumberOfPackets )
					{	// previous file can't be the same
						closeFile();		// close(delete) noname file and
						clearMap();
						fileInfo->checkSum = 0 ;
						openFile( FALSE );	// open a new one for this job

						fileInfo->numberOfRebrNeeded = 1;		// first rebr.
						_rebroadcastInd = rebroadcastInd;
						cSetup->incNumFilesTransferred();	// new file arriving

						_ignoreData = !isFileOpened();		// ignore data if can't open file

						if ( !_ignoreData )
						{
							msgFun( FileOpened, &msg );		// signalize opening
							msgFun( StartReceiving, NULL );	// signalize start of receiving
							msgFun( FileName, &msg );		// set file name in progress
							fseek( _rcvFp, 0, SEEK_SET );	// seek to the start of the file
						}
					}
					else
					{	// the same file as prev. (prev. has not fileInfo completted)
						fileInfo->numberOfRebrNeeded = _rebroadcastInd - rebroadcastInd;		// next rebr.
						_rebroadcastInd = rebroadcastInd;
						msgFun( FileOpened, &msg );		// signalize opening
						msgFun( StartReceiving, NULL );	// signalize start of receiving
						msgFun( FileName, &msg );		// set file name in progress
						fseek( _rcvFp, 0, SEEK_SET );	// seek to the start of the file
					}
				}
				else
				if ( !fileInfo->equal( path, fsize, ftime, fattr ) )
				{	// not the same file as received in prev. rebr. of this job
					//cSetup->incNumRebroadcast();		// this is the next rebroadcast of this job

					// fill fileInfo structure with new data
					fileInfo->set( path, fsize, ftime, fattr ) ;
					fileInfo->numberOfRebrNeeded = 1;		// first rebr.
					_rebroadcastInd = rebroadcastInd;
					fileInfo->infoCorrect = TRUE;

					closeFile();		// close incomplette file and
					clearMap();
					fileInfo->checkSum = 0 ;
					openFile( FALSE );	// open a new one for this job

					cSetup->incNumFilesTransferred();	// new file arriving

					_ignoreData = !isFileOpened();		// ignore data if can't open file

					if ( !_ignoreData )
					{
						msgFun( FileOpened, &msg );		// signalize opening
						msgFun( StartReceiving, NULL );	// signalize start of receiving
						msgFun( FileName, &msg );		// set file name in progress
						fseek( _rcvFp, 0, SEEK_SET );	// seek to the start of the file
					}
				}
				//else this is the second header packet
			}

			_packetInd = 1;
			int maxNumberOfPackets = ( fileInfo->fileSize + sizeof(ulong) + packetSize - 1) / packetSize + 1;
			resizeMap( maxNumberOfPackets ) ;
			setMap( packetInd );
			_dataReady = isReady( );	// update ready flag ( if only header missing from previous rebroadcast )
			if ( _dataReady )			// if ready, close the file
				closeFile();

			LeaveCriticalSection( &_fileReceiverLock );
			return;	// no other data in header packet, continue receiving with next packet
		}
		else
		{	// not header packet
			if( packet->jobId() != _jobId )
			{	// another job
				closeFile();								// another job, close this file
				clearMap() ;

				_jobId		= packet->jobId() ;

				fileInfo->reset();
				_dataReady	= FALSE;
				_ignoreData	= FALSE;
				_hasHeader	= FALSE;

				openFile( TRUE );			// retrive info on this job if exist and open file for receive data to ( old or create a new one )

				_progressTime = getSystemTime();
				_nextProgress = 0;

				//cSetup->incNumRebroadcast();		// this is the next rebroadcast of this job
				//fileInfo->numberOfRebrNeeded++;				// next/first rebr.

				if ( !fileInfo->infoCorrect )
				{	// no valid file info, new file is opened
					fileInfo->numberOfRebrNeeded = 1;			// first rebr.
					_rebroadcastInd = rebroadcastInd;
					cSetup->incNumFilesTransferred();	// new file arriving

					_ignoreData = !isFileOpened();		// ignore data if can't open file
					// don't need to send message
				}
				else
				{	// file info is complette, test when it belongs to this packet
					int maxNumberOfPackets = ( fileInfo->fileSize + sizeof(ulong) + packetSize - 1) / packetSize + 1;
					if ( packetInd > maxNumberOfPackets )
					{	// this packet can't be a part of prev. opened file, open a new one
						closeFile();
						clearMap();
						openFile( FALSE );

						fileInfo->numberOfRebrNeeded = 1;			// first rebr.
						_rebroadcastInd = rebroadcastInd;
						cSetup->incNumFilesTransferred();	// new file arriving

						_ignoreData = !isFileOpened();		// ignore data if can't open file

						if ( !_ignoreData )
						{	// don't know how many rebroadcast can arrive
							FileRcvMsgReceiving msg( fileInfo->fileName, fileInfo->fileSize, 0, -1 );

							msgFun( FileOpened, &msg );		// signalize opening
							msgFun( StartReceiving, NULL );	// signalize start of receiving
							msgFun( FileName, &msg );		// set file name in progress
							fseek( _rcvFp, 0, SEEK_SET );	// seek to the start of the file
						}
					}
					else
					{
						fileInfo->numberOfRebrNeeded += _rebroadcastInd - rebroadcastInd;	// next rebr.
						_rebroadcastInd = rebroadcastInd;

						// update progress
						FileRcvMsgReceiving msg( fileInfo->fileName, fileInfo->fileSize, rebroadcastInd, _numOfRebroadcast );
						msgFun( FileName, &msg );		// set file name in progress

						_packetInd = 1;
						int maxNumberOfPackets = ( fileInfo->fileSize + sizeof(ulong) + packetSize - 1) / packetSize + 1;
						resizeMap( maxNumberOfPackets ) ;
						_dataReady = isReady( );	// update ready flag ( if only header missing from previous rebroadcast )
						if ( _dataReady )			// if ready, close the file
							closeFile();
					}
				}
			}
		}

		if ( !_dataReady && ( rebroadcastInd != _rebroadcastInd ) )
		{
			//cSetup->incNumRebroadcast( _rebroadcastInd - rebroadcastInd );
			fileInfo->numberOfRebrNeeded += _rebroadcastInd - rebroadcastInd;
			_rebroadcastInd = rebroadcastInd;
			if ( fileInfo->infoCorrect )
			{	// update file rebr. number
				FileRcvMsgReceiving msg( fileInfo->fileName, fileInfo->fileSize, _rebroadcastInd, _numOfRebroadcast );
				msgFun( FileName, &msg );		// set file name in progress
			}
		}
		// allow accepting packets without header (i.e. unknown size)
		if ( maxPackets() <= packetInd )
			if( !fileInfo->infoCorrect )
				resizeMap( packetInd+10 ) ;
			else
			{	// error in data, more data than possible
				_status = ReceivingBadData ;
				TRACE ( "\nMore packets - Bad Data" );
				DUMP_RCVPACKET( packet, 0x08 );

				_ignoreData = TRUE;					// ignore the rest of this rebroadcast
				LeaveCriticalSection( &_fileReceiverLock );
				return;
			}

		if ( _ignoreData  ||  _dataReady || isMapSet(packetInd) )	// nothing to do
		{
			LeaveCriticalSection( &_fileReceiverLock );
			return;
		}

		// test for error only if file is not correctly received and header information is correct
		// if header exist the info is correctly set and temp file must be created
		if( (packetInd != _packetInd) )
		{	// seek to the correct position
			fseek( _rcvFp, (packetInd-1)*packetSize, SEEK_SET );
		}
		int numBytes = packet->numDataBytes() ;
		if ( fileInfo->infoCorrect )
		{
			int numMissing = numMissingDataBytes();
			if ( numBytes > numMissing )
			{	// error in data, more data than possible
				_status = ReceivingBadData ;
				TRACE ( "\nMore data - Bad Data" );
				DUMP_RCVPACKET( packet, 0x08 );
				_ignoreData = TRUE;					// ignore the rest of this rebroadcast
				LeaveCriticalSection( &_fileReceiverLock );
				return;
			}
			if ( numMissing > packetSize && numBytes < packetSize )
			{	// error in data, less data than possible
				_status = ReceivingBadData ;
				TRACE ( "\nLess Data - Bad Data" );
				DUMP_RCVPACKET( packet, 0x08 );
				_ignoreData = TRUE;					// ignore the rest of this rebroadcast
				LeaveCriticalSection( &_fileReceiverLock );
				return;
			}
		}

		if( !writeData( packet->data(), numBytes) )
			_ignoreData = TRUE;			// ignore the rest
		else
		{
			const unsigned char *d = packet->data();
			for ( int i = 0; i<numBytes; i++ )
				fileInfo->checkSum += d[i];
			setMap( packetInd );
			_dataReady = isReady( );	// update ready flag
			fileInfo->totalBytesSent += numBytes;
			if ( _dataReady )			// if ready, close the file
				closeFile();
			if ( (_packetInd > packetInd) || ( _packetInd + 100 - (_packetInd % 100) <= packetInd ) )
				wantStatus();
			_packetInd = packetInd + 1;
		}

		if( !_dataReady )
		{
			static cnt=0 ;
			// This is just to save some time.
			// Should be programmed better - eg. getSystemTime() is extremally inefficient
			if( ++cnt % 5 == 0 )
			{
				LONGLONG elapsedTime = getSystemTime() - _progressTime;
				if( elapsedTime > UPDATE_TIME * 10000000.0f )
				{
					msgFun( RcvProgress, NULL );

					long size = fileInfo->totalBytesSent - _nextProgress;
					double speed;
					_nextProgress = fileInfo->totalBytesSent;
					if( size > 0 )
					{
						speed = (double)size / (double)elapsedTime;
						speed *= 10000000.0f / 1024.0f;
					}
					else
						speed = 0.0f;
					_progressTime += elapsedTime;
					msgFun( RcvSpeed, &speed );
				}
			}
		}
	}
	catch( ... )
	{
		LeaveCriticalSection( &_fileReceiverLock );
		throw;
	}
	LeaveCriticalSection( &_fileReceiverLock );
}

// send a message with code and param
void FileReceiver::msgFun( long code, void *param )
{
	UINT msgCode;
	char buf[1024];

	switch ( code )
	{
		case StartProcess:	// char *dir		receiving started
			sprintf( buf, "Starting receiving files to directory %s.", (char *)param );
			msgCode = EMsg_FRcvrStartProcess;
			break;
		case EndProcess:	// NULL
			sprintf( buf, "End receiving files to directory %s.", (char *)param );
			msgCode = EMsg_FRcvrEndProcess;
			break;
		case RcvError:			// FileRcvMsgError*
		{
			char txt[256] ;
			FileRcvMsgError *msg = (FileRcvMsgError*)param ;
			sprintf( buf, "%s - %s", msg->text, DvbEventText(msg->error,txt));
			msgCode = EMsg_FRcvrError;
			break;
		}
		case RcvStatus:		// FileRcvStatus*
		{
			FileRcvStatus *msg = (FileRcvStatus*)param ;
			sprintf( buf, "Status : current file %s; totally received %li K (%.1f%%).",
				msg->currentFile, msg->totalKbRec, msg->percentFileRec );
			msgCode = EMsg_FRcvrStatus;
			break;
		}
		case RcvProgress:
		{
			sprintf( buf, "%.2f", percentSent() );
			msgCode = EMsg_FRcvrProgress;
			break;
		}
		case RcvSpeed:
		{
			sprintf( buf, "%.3lf", *((double *)param) );
			msgCode = EMsg_FRcvrSpeed;
			break;
		}
		case StartReceiving:
		{
			msgCode = EMsg_FRcvrStartReceiving;
			break;
		}
		case EndReceiving:
		{
			msgCode = EMsg_FRcvrEndReceiving;
			break;
		}
		case FileOpened:	// FileRcvMsgReceiving*		copying of new file to outbox started
		{
			FileRcvMsgReceiving *msg = (FileRcvMsgReceiving*)param ;
			sprintf( buf, "File opened: %s (size %li By, rebr. #%i).",
				msg->fileName, msg->fileSize, msg->rebrInd );
			msgCode = EMsg_FRcvrFileOpened;
			break;
		}
		case FileOpenedToAppend:	// FileRcvMsgReceiving*		copying of existing inclomplete file to outbox started
		{
			FileRcvMsgReceiving *msg = (FileRcvMsgReceiving*)param ;
			sprintf( buf, "File opened for append: %s (size %li By, rebr. #%i).",
				msg->fileName, msg->fileSize, msg->rebrInd );
			msgCode = EMsg_FRcvrFileOpened;
			break;
		}
		case FileCompleted:	// FileRcvMsgReceiving*
		{
			FileRcvMsgReceiving *msg = (FileRcvMsgReceiving*)param ;
			sprintf( buf, "File completed: %s (size %li By, rebr. #%i).",
				msg->fileName, msg->fileSize, msg->rebrInd );
			msgCode = EMsg_FRcvrFileCompleted;
			break;
		}
		case FileCompletedWithoutHeader:
		{
			FileRcvMsgReceiving *msg = (FileRcvMsgReceiving*)param ;
			sprintf( buf, "Unknown file completelly received (without header) to: %s.",
				msg->fileName );
			msgCode = EMsg_FRcvrFileWithoutHeader;
			break;
		}
		case FileIncompleteData:
		{
			FileRcvMsgReceiving *msg = (FileRcvMsgReceiving*)param ;
			sprintf( buf, "Incomplete file: %s (size %li By, rebr. #%i).",
				msg->fileName, msg->fileSize, msg->rebrInd );
			msgCode = EMsg_FRcvrFileIncompleteData;
			break;
		}
		case FileDamagedData:
		{
			FileRcvMsgReceiving *msg = (FileRcvMsgReceiving*)param ;
			sprintf( buf, "File with damaged data: %s (size %li By, rebr. #%i).",
				msg->fileName, msg->fileSize, msg->rebrInd );
			msgCode = EMsg_FRcvrFileDamagedData;
			break;
		}
		case FileRefused:
		{
			FileRcvMsgReceiving *msg = (FileRcvMsgReceiving*)param ;
			sprintf( buf, "File refused and deleted : %s (size %li By, rebr. #%i).",
				msg->fileName, msg->fileSize, msg->rebrInd );
			msgCode = EMsg_FRcvrFileRefused;
			break;
		}
		case FileAllreadyExisting:
		{
			FileRcvMsgReceiving *msg = (FileRcvMsgReceiving*)param ;
			sprintf( buf, "File already exist: %s (size %li By, rebr. #%i).",
				msg->fileName, msg->fileSize, msg->rebrInd );
			msgCode = EMsg_FRcvrFileAllreadyExisting;
			break;
		}
		case FileName:
		{
			FileRcvMsgReceiving *msg = (FileRcvMsgReceiving*)param;
			switch( msg->numOfRebroadcast )
			{
			case -1:	// unknown
				sprintf( buf, "(?/?) %s  (size  %li bytes).", msg->fileName, msg->fileSize );
				break;
			case  0:	// infinit
				sprintf( buf, "(infinit) %s  (size  %li bytes).", msg->fileName, msg->fileSize );
				break;
			default:
				sprintf( buf, "(%i/%i) %s  (size  %li bytes).", msg->numOfRebroadcast - msg->rebrInd + 1, msg->numOfRebroadcast, msg->fileName, msg->fileSize );
			}
			msgCode = EMsg_FRcvrFileName;
			break;
		}
		// debug
		default:
			sprintf( buf, "Unknown msg code." );
			msgCode = EMsg_FRcvrUnknown;
			break;
	}

	MfxPostMessage( msgCode, _channel, buf );
}

// return the percent of sent data from the current file
float FileReceiver::percentSent()
{
	if ( fileInfo && fileInfo->infoCorrect )
	{
		return 100.f*fileInfo->totalBytesSent/fileInfo->fileSize;
	}
	return 0;
}

// send a status message
void FileReceiver::wantStatus()
{
	if ( fileInfo && fileInfo->infoCorrect )
	{
		FileRcvStatus st( fileInfo->fileName, 
						  fileInfo->totalBytesSent, 
						  100.f*fileInfo->totalBytesSent/fileInfo->fileSize );
		msgFun( RcvStatus, &st );
	}
	else
	{
		FileRcvStatus st("not receiving", 0, 0);
		msgFun( RcvStatus, &st );
	}
}



//---------------------------------------------------------------------------
//	InstallsReceiver	- receiver of new version of the program
//
//	If file "rcv_vvv.exe" (vvv = version) is received, and
//	if vvv is greater than current program version, then
//	a message is posted to the user that new version arrived.
//---------------------------------------------------------------------------

// Test if the update is newer than current version
BOOL InstallsReceiver::isNewVersion( const char *fileName, char *versionStr )
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

   _splitpath( fileName, drive, dir, fname, ext );
	if( _strnicmp( ext, ".exe", strlen( ext ) ) || _strnicmp( fname, "rcv_", 4 ) )
		return FALSE;

	char *pos = &fname[4], *s;
	ulong version = strtoul( pos, &s, 10 );
	if( (size_t)( s - pos ) != strlen( pos ) )
		return FALSE;

	return version > getAppVersion( versionStr);
}

// receive a new packet, push it's data to a file
// if file is completly received show information message that new version is arrived
void InstallsReceiver::push( MuxPacket *packet )
{
	FileReceiver::push( packet );
	if( _dataReady && !_newInstallation )
	{
		char version[25] ;
		if( isNewVersion( fileInfo->fileName) )
		{
			static char *title	=	"New installation arrived";
			static char *text	=	"Version V%s of the Receiver program was received\r\n"
									"and stored as %s.\r\n\r\n"
									"Whenever you do not expect any critical data, You should terminate\r\n"
									"the receiving and install the new version.\r\n\r\n"
									"(You can do so e.g. in Windows Explorer by using double click.)";
			char		buff[1024], *msg;

			sprintf( buff, text, version, fileInfo->fileName );
			msg = (char *)MALLOC( strlen( title ) + strlen( buff ) + 2 );
			strcpy( msg, buff );
			strcpy( msg + strlen( buff ) + 1, title );
			MfxPostMessage( EMsg_NewInstallation, (long)0, (long)msg );
		}
		_newInstallation = TRUE;
	}
	else if( !_dataReady )
		_newInstallation = FALSE;
}
