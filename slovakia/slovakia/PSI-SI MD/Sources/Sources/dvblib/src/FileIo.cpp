/*
 *	Filename:		fileio.cpp
 *
 *	Version:		1.00
 *
 *	Description:	This file contains implementation of various data sender and receiver classes
 *					
 *
 *	History:
 *		Jan 17, 2000
 *			FEC added
 *			checkSum used in the file send protocol changed (faster computation)
 *			file send protocol not downward compatible
*/

//
// Namety na optimalizaciu:
// ------------------------
//
//- redukovat volania msgFun(); optimalizovat vytvaranie sprav
//- redukovat volania getSystemTime() - vypocet case v MfxPostMessage() je radovo efektivnejsi
//- ak je cely FEC blok OK, tak odhadzovat FEC pakety v tom bloku bez analyzy;
//  to je ciastocne urobene v push() (hladaj _lastPacketWasFec) - mam pochybnosti, ci to pracovalo
//- FEC: pre kazdy blok evidovat max. neznamy interval; ak je prilis velky, kaslat na FEC
//- atof() sa uplne zbytocne vola v ProgressList.cpp
//
// Vypoctova narocnost:
//		atof()	computeCrcAndCheckSum()	getSystemTime()	new
//		  2				1					0.5			0.5
//
// ToDo:
// -----
//
//	- presunut FEC spravy z TRACE do logu
//

#include "tools2.hpp"
#include "mux.hpp"

#include <errno.h>
#include "inbox.hpp"
#include "ClientCfg.hpp"
#include "ServCfg.hpp"


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

#pragma warning(disable:4083)	// disable 'expected string' warning

#define MAX_BUFFER		4096
#define MAX_JOB_NUMB	0xff
#define MY_ERRNO		DvbErrno( errno )
#define UPDATE_TIME		0.5f		// delay between 2 progress reports [sec]

// uncoment to enable agregation of MUX packets into 1 UDP packet
// Has sense only for channels using MPE stream format
//#define MUX_PACKET_AGREGATION

// uncomment next define if you want to read files in sendFile with overlapped
// if you work lot with disks this enable to you end this process along reading
//#define READ_FILE_OVERLAPPED

// Use to simulate failures in sending packets
//#define FORCE_SINGLELOSTPACKETS			// 1 packet per FEC block
//#define FORCE_GROUPLOSTPACKETS			// 80 packets each 5th FEC block

// On closeFile(): Outputs info about FEC success into debug output window.
#define FEC_TRACE

#ifdef FEC_TRACE
	#define	ON_FEC_OPENFILE()	{		\
		_numFecPackets = 0 ;			\
		_numBadFecPackets = 0 ;			\
		_numUndeterminedFecPackets = 0;	\
		_numUnneededFecPackets = 0 ;	\
		_numFecReadErrors = 0 ;			\
		_numUnknownFecPackets  = 0 ;	\
		_numRecoveredPackets = 0 ;		\
		_numLateRecoveredPackets = 0 ;	\
		_maxUnknownInterval = -1 ;		\
	}

	#define ON_FEC_CLOSEFILE(txt)	{	\
		if( _numRecoveredPackets > 0 )	\
		{								\
			TRACE( "\n%s; FEC statistics for %s> Max. unknown interval=%d"											\
				   "\n    All=%d, Bad=%d, Undetermined=%d, Unneeded=%d, ReadErr=%d, Unknown=%d Recovered=%d+%d",\
				txt, fileInfo->fileName, _maxUnknownInterval,														\
				 _numFecPackets, _numBadFecPackets, _numUndeterminedFecPackets,									\
				_numUnneededFecPackets, _numFecReadErrors, _numUnknownFecPackets,								\
				_numRecoveredPackets, _numLateRecoveredPackets ) ;												\
		}								\
		else							\
			TRACE( "\n%s; %s> no FEC packets recovered", txt, fileInfo->fileName ) ;	\
	}

	#define ON_FECPACKET_ARRIVED()		_numFecPackets++ ;
	#define ON_BAD_FECPACKET()			_numBadFecPackets++ ;
	#define ON_UNDETERMINED_FECPACKET()	_numUndeterminedFecPackets++ ;
	#define ON_UNNEEDED_FECPACKET()		_numUnneededFecPackets++ ;
	#define ON_GETPACKET_ERROR()		_numFecReadErrors++ ;
	#define ON_UNKNOWN_FECPACKET()		_numUnknownFecPackets++ ;
	#define ON_RECOVER_FECPACKET()		_numRecoveredPackets++ ;
	#define ON_LATERECOVER_FECPACKET()	_numLateRecoveredPackets++ ;

	#define ON_REBROADCAST()			\
		if( _maxUnknownInterval == -1 )	\
			_maxUnknownInterval = maxUnknownInterval() ;
#else
	#define	ON_FEC_OPENFILE()
	#define ON_FEC_CLOSEFILE(txt)
	#define ON_FECPACKET_ARRIVED()
	#define ON_BAD_FECPACKET()
	#define ON_UNDETERMINED_FECPACKET()
	#define ON_UNNEEDED_FECPACKET()
	#define ON_GETPACKET_ERROR()
	#define ON_UNKNOWN_FECPACKET()
	#define ON_RECOVER_FECPACKET()
	#define ON_LATERECOVER_FECPACKET()
	#define ON_REBROADCAST()
#endif


//-------------------------------------------------------------------------------
// Utilities
//-------------------------------------------------------------------------------


//////// for moving file with replace of existing under win9x platform
static BOOL renameFile( const char *source, const char *dest )
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

// moves a file to the new location
static BOOL moveFile( const char *src, const char *dst )
{
	char path[1024], drive[256], dir[1024];
	_splitpath( dst, drive, dir, NULL, NULL );
	_makepath( path, drive, dir, NULL, NULL );
	if( !dirExist( path ) )
		makeDir( path );
	return renameFile( src, dst );
}

// delete a file
static void removeFile( const char *fname )
{
	DWORD atr = GetFileAttributes( fname ) ;
	if( atr & FILE_ATTRIBUTE_READONLY )
		SetFileAttributes( fname, atr & ~FILE_ATTRIBUTE_READONLY ) ;
	DeleteFile( fname );
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


//-------------------------------------------------------------------------------
// structures holding varius informations for data receiving/sending and messaging
//-------------------------------------------------------------------------------


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
} ;

float MuxMsgSendStatus::outputRate( MuxMsgSendStatus *st2 )
{
	LONGLONG time      = timeStamp - st2->timeStamp ;	// elapsed time
	if ( time == 0 ) time = 1;
	LONGLONG sentBytes = (nPacketsSent - st2->nPacketsSent)*MUXDATASIZE ;
	/*
	float s = (10000000.f/1024.f) * sentBytes/time;
	TRACE( "\nSPEED %5.4f : %02d:%02d:%02d:%03d -> %02d:%02d:%02d:%03d = %I64d            %I64d -> %I64d", 
		s, st2->st.wHour, st2->st.wMinute, st2->st.wSecond, st2->st.wMilliseconds,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, time, st2->nPacketsSent, nPacketsSent ) ;
	*/
	return (10000000.f/1024.f) * sentBytes/time;
}

	
//---------------------------------------------------------------------------------------
// FileSender - sender of files
// Provides file reading, packetizing of the file data and packet sending.
// One file is one job, jobs can be mixed (file's rebroadcasts must not be sent in one run)
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

int FileSender::sendPacket( int len, PidStreamAttrib *pidStrAttr )
{
	// construct and send packet (constructed packet will be returned)
	MuxPacket muxPacket ;
	#ifdef FORCE_SINGLELOSTPACKETS
		static int nnn=0 ;
		int err ;
		if( packetInd == 0 )
			nnn = 0 ;
		nnn++ ;
		// Don't send some packets
		if( nnn % 1024 == 20 )
		{
			// Construct packet the same way as in BaseSender, but do not send it.
			err = 0 ;
			packetInd++ ;
			if( !usrId.isValid() )
				muxPacket.makeDataPacket( channel, _flags, len, rebroadcastIndex, packetInd, data+bottom, _jobId ) ;
			else
				muxPacket.makeUnicastPacket( channel, _flags, len, rebroadcastIndex, packetInd, data+bottom, usrId, _jobId ) ;
		}
		else
			err = BaseSender::sendPacket( &muxPacket, _flags, data+bottom, len, pidStrAttr ) ;
	#else
	#ifdef FORCE_GROUPLOSTPACKETS
		static int nnn=0 ;
		int err ;
		if( packetInd == 0 )
			nnn = 0 ;
		nnn++ ;
		// Don't send some packets
		int kkk = nnn%5000 ;
		if( kkk >= 20  &&  kkk <= 100 )
		{
			// Construct packet the same way as in BaseSender, but do not send it.
			err = 0 ;
			packetInd++ ;
			if( !usrId.isValid() )
				muxPacket.makeDataPacket( channel, _flags, len, rebroadcastIndex, packetInd, data+bottom, _jobId ) ;
			else
				muxPacket.makeUnicastPacket( channel, _flags, len, rebroadcastIndex, packetInd, data+bottom, usrId, _jobId ) ;
		}
		else
			err = BaseSender::sendPacket( &muxPacket, _flags, data+bottom, len, pidStrAttr ) ;
	#else
		int err = BaseSender::sendPacket( &muxPacket, _flags, data+bottom, len, pidStrAttr ) ;
	#endif
	#endif

	if( packetInd > 0 )
		checkSum ^= muxPacket.dataCheckSum() ;
	if( fecBlkSize == 0  ||  err != 0  ||  !fecLevel )
		return err ;

	for( int c=0 ; c < fecDesc.numCycles ; c++ )
	{
		int cycleLength = fecDesc.cycles[c] ;
		int rowInd = fecPacketIndex % cycleLength ;
		MuxPacket *fp = fecPackets[c] + rowInd ;
		if( fecBlkIndex == 0  &&  rowInd == 0  &&  c == 0 )
			c = c ;
		if( fp->flags() == 0 )
			*fp = muxPacket ;
		else
			fp->xorData( muxPacket ) ;
	}

	++fecPacketIndex ;
	if( fecPacketIndex == fecBlkSize )
	{
		// FEC block completed; send all FEC packets
		for( c=0 ; c < fecDesc.numCycles ; c++ )
		{
			int cycleLength = fecDesc.cycles[c] ;
			for( int rowInd=0 ; rowInd < cycleLength ; ++rowInd )
			{
				MuxPacket *fp = fecPackets[c] + rowInd ;
				fp->setFecInfo( cycleLength, rowInd, fecBlkIndex, fecBlkSize ) ;
				if( fp->isUnicastPacket() )
					err = 0 ;
				err = BaseSender::sendPacket( fp, 0,0,0, pidStrAttr ) ;
				if( err != 0 )
					return err ;
				if( fp->isUnicastPacket() )
					err = 0 ;
			}
		}

		// setup next FEC block
		initFEC() ;
	}
	return err ;
}


//----------------------------------------
#ifndef MUX_PACKET_AGREGATION

// send all full packets
inline int FileSender::sendPackets( PidStreamAttrib *pidStrAttr )
{
	int  len = pos - bottom ;
	while( len >= packetSize  &&  (WaitForSingleObject(hKillEvent,0) == WAIT_TIMEOUT) )
	{
		int err = sendPacket( packetSize, pidStrAttr ) ;
		bottom += packetSize ;
		len	   -= packetSize ;
		if( err != 0 )
			return err ;
	}
	squeeze( );
	return 0 ;
}

#else
//----------------------------------------

inline int FileSender::sendPackets( PidStreamAttrib *pidStrAttr )
{
	int nPackets = (pos-bottom) / packetSize ;

	while ( nPackets && (WaitForSingleObject(hKillEvent,0) == WAIT_TIMEOUT) )
	{
		int sentPackets ;
		int err = 
			BaseSender::sendAgregatedPackets( 
				_flags, data+bottom, 
				nPackets, packetSize, sentPackets, 
				pidStrAttr 
			) ;

		bottom += sentPackets*packetSize ;
		nPackets -= sentPackets ;
		if (err)
			return err ;
	}
	squeeze() ;
	return 0 ;
}

#endif
//----------------------------------------

inline int FileSender::flushPackets( PidStreamAttrib *pidStrAttr )
{
	int err = sendPackets(pidStrAttr) ;

	int remLength = pos-bottom ;
	if (err==0 && remLength>0)
	{
		err = sendPacket( remLength, pidStrAttr);

		bottom = pos = 0 ;
	}
	return err ;
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
	fecPackets[0] = fecPackets[1] = fecPackets[2] = fecPackets[3] = 0 ;
	fecPacketsLength[0] = fecPacketsLength[1] = fecPacketsLength[2] = fecPacketsLength[3] = 0 ;
	fecLevel = 0 ;
}

FileSender::~FileSender()
{
	free( data);
	free( fecPackets[0] ) ;
	free( fecPackets[1] ) ;
	free( fecPackets[2] ) ;
	free( fecPackets[3] ) ;
}


//---------------------------------------------------------------------------------------
//	Forward Error Correction
//---------------------------------------------------------------------------------------


void FileSender::initFEC( int nPackets )
{
	if( !fecLevel )
		return ;
	if( nPackets != 0 )
	{
		fecTotalPackets = nPackets ;		// first call
		fecBlkIndex = 0 ;
	}
	else
		fecBlkIndex += fecBlkSize/512 ;

	if( fecTotalPackets <= 1024 )
		fecBlkSize = fecTotalPackets ;
	else
	if( fecTotalPackets <= 1024+512 )
		fecBlkSize = 512 ;
	else
		fecBlkSize = 1024 ;

	fecTotalPackets -= fecBlkSize ;
	fecPacketIndex = 0 ;

	if( !getFecParams( fecBlkSize, fecLevel, &fecDesc) )
		fecBlkSize = 0 ;
	else
	{
		// setup FEC buffers
		for( int j=0 ; j < fecDesc.numCycles ; ++j )
		{
			if( fecPacketsLength[j] < fecDesc.cycles[j] )
			{
				fecPacketsLength[j] = fecDesc.cycles[j] ;
				fecPackets[j] = (MuxPacket*)realloc( fecPackets[j], fecPacketsLength[j]*sizeof(MuxPacket) ) ;
			}
			memset( fecPackets[j], 0, fecDesc.cycles[j]*sizeof(MuxPacket) ) ;
		}
	}
}


//---------------------------------------------------------------------------------------
//	FileHeader
//---------------------------------------------------------------------------------------


// # of additional bytes in the file protocol
#define EXTRABYTES	sizeof(char)

struct FileHeader
{
	char	buf[MUXPACKETDATASIZE - sizeof(GlobalUserID)] ;

	FileHeader() { memset( buf, 0, sizeof(buf) ) ; }
	long set( const char *fullFileName, const char *file_name, HANDLE hFile, int numRebroadcasts )
	{
		long	fSize;
		time_t	fTime;
		DWORD	fAttr;

		fSize = GetFileSize( hFile, NULL ) ;
		FILETIME ft;
		GetFileTime( hFile, NULL, NULL, &ft );
		CTime ct( ft );
		fTime = ct.GetTime();
		fAttr = GetFileAttributes( fullFileName );

		int off=0 ;
		#define ADD( data, length )	{ memcpy( buf+off, data, length) ; off += length ; }
		ADD( file_name , strlen(file_name) + 1 );
		ADD( &fSize	, sizeof(long)  );
		ADD( &fTime	, sizeof(time_t));
		ADD( &fAttr	, sizeof(DWORD) );
		ADD( &numRebroadcasts, sizeof(int) );

		return fSize ;
	}

	void get( char *file_name, long *fSize, time_t *fTime, DWORD *fAttr, int *numOfRebroadcast )
	{
		long offset = 0;

		strncpy( file_name, buf, sizeof(buf));	offset += strlen( file_name) + 1 ;
		*fSize  = *(long  *)( buf+offset);		offset += sizeof(long);
		*fTime  = *(time_t*)( buf+offset);		offset += sizeof(time_t);
		*fAttr  = *(DWORD *)( buf+offset);		offset += sizeof(DWORD);
		*numOfRebroadcast = *(int *)( buf+offset );
	}
} ;


//---------------------------------------------------------------------------------------
// FileSender - sendFile() - send a file only once (one rebroadcast)
// - send a file job header with file name and attributes
// - read file data to a buffer, if file do not fit to the buffer read it in more stepps
// - send readed data, make from them packets and put then\m to the output
// can read files in two modes : overlapped and normal mode
//---------------------------------------------------------------------------------------

#pragma optimize( OPTOPTIONS, on )

int FileSender::sendFile( const char *fullFileName, const char *file_name, HANDLE hFile,
		ushort _channel, uchar streamFmt, int _fecLevel, int _rebroadcastIndex, int _numRebroadcasts,
		PidStreamAttrib *pidStrAttr, const GlobalUserID& _usrId, uchar &jobId, ushort flags )
{
	_flags = flags | MuxPacket::File ;
	fecLevel = _fecLevel ;
	uchar savedJobId;
	if ( jobId == 0 )
	{
		nextJob();
		jobId = _jobId;
		savedJobId = _jobId;
	}
	else
	{
		savedJobId = _jobId;
		_jobId = jobId;
	}
	#ifdef READ_FILE_OVERLAPPED
		OVERLAPPED	overlapped;
		overlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
		if ( overlapped.hEvent==NULL )
		{
			_jobId = savedJobId;
			return GetLastError();
		}
		HANDLE handles[2];
		handles[0] = hKillEvent;
		handles[1] = overlapped.hEvent;
		long  fPos = 0;
	#endif
	
	channel			 = _channel ;
	streamFormat	 = streamFmt ;
	rebroadcastIndex = _rebroadcastIndex ;
	usrId			 = _usrId ;

	packetSize		 = usrId.isValid() ? MUXPACKETUNICASTDATASIZE : MUXPACKETDATASIZE ;
	bottom			 = 0;
	pos				 = 0;
	int sendError	 = 0 ;
	checkSum		 = 0;

	__try
	{
		FileHeader hdr ;
		long fSize = hdr.set( fullFileName, file_name, hFile, _numRebroadcasts ) ;

		initFEC( 1 + (fSize-1)/packetSize + 1 ) ;
		packetInd = 0 ;
		addBuffer( &hdr, sizeof(hdr) ) ;
		sendError = flushPackets( pidStrAttr );		// send packet with header

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
					pos += bytesRead ;
					if( sendError == 0 )
						sendError = sendPackets( pidStrAttr );
					if( bytesToRead != bytesRead )
						break;
				}
				else
					sendError = DvbErr_InboxKilled ;
		}

		if( sendError == 0 )
		{
			// Add to the check sum remainder of data
			for( int j=0 ; j < pos ; ++j )
				checkSum ^= data[pos] ;
			addBuffer( &checkSum, EXTRABYTES ) ;
			ASSERT( EXTRABYTES == sizeof(checkSum) );
			sendError = flushPackets( pidStrAttr );
		}
	}
	CATCH_EXCEPTION_CODE
	{
  		sendError = EXCEPTION_CODE ;
	}

	#ifdef READ_FILE_OVERLAPPED
		CancelIO( hFile );
		WaitForSingleObject( overlapped.hEvent, INFINITE );
		CloseHandle( overlapped.hEvent );
	#endif
	_jobId = savedJobId;
	return sendError ;
}

#pragma optimize( "", on )			// restore original optimization options

// send a file - all rebroadcasts in one run, one by one
int FileSender::sendFile( const char *fullFileName, ushort channel, uchar streamFmt, int _fecLevel, int numRebroadcasts,
						  PidStreamAttrib *pidStrAttr, const GlobalUserID& _usrId, ushort flags)
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

				exc = sendFile( fullFileName, shortFileName, hFile, channel, streamFmt, _fecLevel,
					numRebroadcasts, _numRebroadcasts, pidStrAttr, _usrId, _jobId, flags );
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


//------------------------------------------------------------------------------
//	FileReceiver
//------------------------------------------------------------------------------


// holds information on actually receiving file
struct FileRcvFileSent
{
	char	fileName[_MAX_PATH];// "???" for unknown name (packet lost)
	long	fileSize ;			// [By]
	time_t	fileTime ;			// file last access time
	DWORD	fileAttr ;
	long	totalBytesSent ;	// only bytes successfully received
	BOOL	infoCorrect;		// all informations in this structure are valid
	uint	numberOfRebrNeeded;	// how many rebr. received for this file
	int		maxPackets ;
	uchar	checkSum;

	inline BOOL equal( const char *path, long fsize, time_t ftime, DWORD fattr )
	{
		return	infoCorrect && 
			 strcmp( fileName, path ) == 0 &&
			 fileSize == fsize &&
			 // File time resolution on NT is 2 secs!
			 abs(fileTime - ftime) <= 1 &&
			 fileAttr == fattr;
	}
	inline void set( const char *path, long fsize, time_t ftime, DWORD fattr, int packet_size )
	{
		strcpy( fileName, path ) ;
		fileSize = fsize ;
		fileTime = ftime ;
		fileAttr = fattr ;
		infoCorrect= TRUE ;
		maxPackets = 2 + (fileSize + EXTRABYTES - 1) / packet_size ;
	}
	inline void reset( )	{ memset( this, 0, sizeof(FileRcvFileSent) ) ; }
	FileRcvFileSent()		{ reset() ; }
} ;

// create a receiver with known outbox directory
// initialize the receiver, make the receiving directory
FileReceiver::FileReceiver( ushort channel, const char *dir ) : BaseReceiver()
{
	_channel		= channel;
	_status			= Inactive;
	_running		= FALSE ;
	_overwriteMode  = TRUE ;
	fileInfo		= NULL;

	_ignoreData		= FALSE;
	_lastPacketWasFec = FALSE ;
	_fecBlocked		= FALSE ;
	_lastPacketInd	= -1 ;

	_prepareForStart() ;
	openFecQueue() ;
	initFileData() ;

	if( dir != NULL )
	{
		_isOutdirOK = TRUE;
		strcpy( _dir, dir ) ;
	}
	else
		_isOutdirOK = validateOutbox();

	int   len  = strlen( _dir ) ;
	if( _dir[ --len] != '\\'  &&  _dir[len] != '/' )
		strcat( _dir, "\\" );
	if( makeDir(_dir) != 0 )
	{
		FileRcvMsgError msg( "Can't create directory.", GetLastError() );
		msgFun( RcvError, &msg );
		_status = StopDueToError;
	}

	fileInfo = new FileRcvFileSent;	// holds information of actualy receiving file
	InitializeCriticalSection( &_fileReceiverLock );
}

// stop the receiver, rename the temp dir to the correct if needed or remove the whole outbox dir if needed
FileReceiver::~FileReceiver()
{
	if ( _running )
		stop();
	closeFecQueue() ;
	destroyFileData() ;
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

	sprintf( buff2, "Received\\Channel%u.temporary", _channel );
	if( channelName == NULL )
	{
		_makepath( _dir, drive, dir, buff2, NULL );
		return FALSE;
	}

	_makepath( buff1, drive, dir, buff2, NULL );
	sprintf( buff2, "Received\\%s", channelName );
	_makepath( _dir, drive, dir, buff2, NULL );

	if( dirExist( buff1 ) )
		return renameDir( buff1, _dir, TRUE );

	return TRUE;
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


//-----------------------------------------------------------
//	start/stop()
//-----------------------------------------------------------


void FileReceiver::_prepareForStart( )
{
	_dataReady  = FALSE;
	_status		= Ready;
	_jobId		= 0;
	_rebroadcastInd = 0;
	_numOfRebroadcast = -1;
}

// start the receiving (set the flags to the default state)
void FileReceiver::start( BOOL ovrMode )
{
	ASSERT( !_running );
	if ( _status == StopDueToError )
		return;
	_running	= TRUE ;
	_overwriteMode = ovrMode ;
	fileInfo->reset() ;

	_prepareForStart( ) ;

	msgFun( StartProcess, _dir );
}

// stop the receving
void FileReceiver::stop( )
{
	if( !_running )
		return;

	// Probably better if this flag is before criticval section -
	// to interrupt eventual lengthy FEC processing.
	_running = FALSE ;

	EnterCriticalSection( &_fileReceiverLock );
	closeFile();
	_status = Inactive;
	LeaveCriticalSection( &_fileReceiverLock );

	msgFun( EndProcess, _dir );
}


//-----------------------------------------------------------
//	bitmap utilities
//-----------------------------------------------------------


#define INFOVERSION	1

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

	char buf[10] ;
	sprintf( buf, "BMP%02d", INFOVERSION ) ;
	fwrite( buf, 5, 1, _bmpFp ) ;
	fwrite( fileInfo			, sizeof(FileRcvFileSent  )	, 1, _bmpFp );
	fwrite( &_numOfRebroadcast  , sizeof(_numOfRebroadcast) , 1, _bmpFp);
	writeBitmap( _bmpFp );

	fclose( _bmpFp );
	TRACE( "\nSaved file bitmap for %s(%s) (CS=%x)", _rcvFileName, fileInfo->fileName, fileInfo->checkSum ) ;
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
	
	char buf[10] ;
	int ver=0 ;
	fread( buf, 5, 1, _bmpFp ) ; buf[5] = 0 ;
	sscanf( buf, "BMP%d", &ver ) ;
	BOOL err = (ver != INFOVERSION) ;

	if( !err )
	{
		fread( fileInfo				, sizeof(FileRcvFileSent  )	, 1, _bmpFp );
		ASSERT( fileInfo->numberOfRebrNeeded < 100 ) ;
		fread( &_numOfRebroadcast	, sizeof(_numOfRebroadcast)	, 1, _bmpFp );
		if( !readBitmap( _bmpFp) )
			err = TRUE ;
		else
			err = ferror( _bmpFp );
	}

	fclose( _bmpFp );

	if( err )
		removeFile( bmpFileName );
	else
		TRACE( "\nLoaded file bitmap for %s (%s)", _rcvFileName, fileInfo->fileName ) ;

	return !err;
}

// remove information file for current job
void FileReceiver::deleteInformationFile( )
{
	char bmpFileName[_MAX_PATH];
	sprintf( bmpFileName, "%s.bitmap", _rcvFileName ) ;
	if( fileExist(bmpFileName) )
	{
		removeFile( bmpFileName );
		TRACE( "\nDeleted file bitmap for %s (%s)", _rcvFileName, fileInfo->fileName ) ;
	}
}


//-----------------------------------------------------------
//	open/closeFile()
//-----------------------------------------------------------


// Makes file "receiving.*" and opens it.
// On failure:
//	- sends a message
//	- sets _ignoreData
void FileReceiver::openFile( BOOL openWithBitmap )
{
	ON_FEC_OPENFILE() ;

	char path[1024] ;
	sprintf( path, "receiving.%u", _jobId ) ;
	_makepath( _rcvFileName, NULL, _dir, path, NULL ) ;

	int io_err=0 ;
	if( openWithBitmap )
	{
		if( readInformationFile() )	// open saved bitmap
		{
			_makepath( path, NULL, NULL, fileInfo->fileName, "incomplete" ) ;
			if ( fileExist( path ) )
			{
				renameFile( path, _rcvFileName );
				SetFileAttributes( _rcvFileName, FILE_ATTRIBUTE_NORMAL );
				io_err = openFileData( _rcvFileName, TRUE ) ;
			}
		}
		else
			fileInfo->reset();
	}

	if( !isFileOpened() )
	{
		io_err = openFileData( _rcvFileName, FALSE ) ;
		fileInfo->totalBytesSent = 0 ;
		fileInfo->checkSum = 0 ;

		if( !isFileOpened() )
		{
			sprintf( path, "Can't open file %s.", _rcvFileName );
			FileRcvMsgError msg( path, io_err );
			msgFun( RcvError, &msg );
			_ignoreData = TRUE;
			return;
		}
	}

	_lastPacketInd	 = -1 ;
	_lastPacketWasFec= FALSE ;
	_fecBlocked		 = FALSE ;
	_progressTime	 = getSystemTime();
	_nextProgress	 = fileInfo->totalBytesSent;
	_progressCounter = 0 ;
	_status			 = Receiving;
	msgFun( StartReceiving, NULL );
	if( !_ignoreData && fileInfo->infoCorrect )
	{
		FileRcvMsgReceiving msg( fileInfo->fileName, fileInfo->fileSize, 0, _numOfRebroadcast );
		msgFun( FileName, &msg );
	}
}

// Decorate filename by "~nnn" suffix
void FileReceiver::makeFileVersion( char *fn )
{
	char path[1024] ;
	for( int j=0 ; ; j++ )
	{
		sprintf( path, "%s~%03d", fn, j ) ;
		if ( !fileExist( path ) )
			break ;
	}
	if ( !renameFile( fn, path ) )
	{
		FileRcvMsgError msg( "File I/O error.", GetLastError() );
		msgFun( RcvError, &msg );
	}
}


// closes the received file - send ending messages depending on state of receiving
// state can be - completted, incompletted with known file name, etc.
void FileReceiver::closeFile()
{
	if ( !isFileOpened() )
		return;

	int io_err = closeFileData() ;

	// Bitmap is saved only if known file was partially received without any error.
	// In all other cases the bitmap is deleted.
	if( !io_err  &&  fileInfo->infoCorrect  &&  !_dataReady  &&  _status != ReceivingBadData )
		writeInformationFile();
	else
		deleteInformationFile();

	if( io_err )
	{
		FileRcvMsgError msg( "File refused due to I/O error.", io_err );
		msgFun( RcvError, &msg );
		removeFile( _rcvFileName );
	}
	else
	if ( fileInfo->infoCorrect )
	{
		FileReceiverMsgCode mcode;
		if ( _status == ReceivingBadData )		// damaged
		{
			strcat( fileInfo->fileName, ".damaged" );

			renameFile( _rcvFileName, fileInfo->fileName );
			setFileTime( fileInfo->fileName, fileInfo->fileTime );
			SetFileAttributes( fileInfo->fileName, fileInfo->fileAttr );
			
			mcode = FileDamagedData;
			TRACE( "\nSaving %s", fileInfo->fileName ) ;
		}
		else
		if ( !_dataReady )						// incomplete
		{
			strcat( fileInfo->fileName, ".incomplete" );
			renameFile( _rcvFileName, fileInfo->fileName );
			mcode = FileIncompleteData;
			TRACE( "\nSaving %s", fileInfo->fileName ) ;
		}
		else
		if ( _ignoreData )						// allready exist
		{
			removeFile( _rcvFileName );
			mcode = FileAllreadyExisting;
		}
		else									// ok
		{
			// remove old file if exist
			if( fileExist( fileInfo->fileName) )
			{
				if ( !_overwriteMode )	// remove old file
					removeFile( fileInfo->fileName ) ;
				else					// rename old file to version fileName~nnn
					makeFileVersion( fileInfo->fileName );
			}
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
	else										// no file header received
	{
		removeFile( _rcvFileName );
		clearMap() ;
	}
	_status = Ready;
	fileInfo->checkSum = 0 ;
}


//-----------------------------------------------------------
//	File i/o
//-----------------------------------------------------------


//
// All file access is done via these functions.
// Win32 interface is used instead of stream or low-level i/o functions, because Win32
// functions are much faster for our purposes.
//
// File buffer is adjusted to FEC needs, i.e. it is generally 1024 packets big.
// However, there is an exception at the file start. (File protocol uses 1st packet as the header.)
// Therefore 1st file buffer is only 1023 packets big.
//

#define USE_WIN32

#ifdef USE_WIN32
	#define _rcvFp	*((HANDLE*)&_file)
#else
	#define _rcvFp	*((FILE**)&_file)
#endif

void FileReceiver::initFileData()
{
	_rcvFp		 = NULL ;
	_fileBuf	 = NULL ;
	_fileBufPtr  = 0 ;
	_fileWrPtr1  = 1024*MUXPACKETDATASIZE ;
	_fileWrPtr2  = 0 ;
	_fileBufSize = 0 ;
	_fileChanged = FALSE ;
	_fileErr	 = 0 ;
	_filePgSize  = 0 ;
	_flushingFileData = FALSE ;
	_progressCounter  = 0 ;
}

int FileReceiver::openFileData( const char *file_name, BOOL preserveOldData )
{
	_rcvFp		 = NULL ;
	_fileErr	 = 0 ;
	_fileChanged = FALSE ;
	#ifdef USE_WIN32
		DWORD attr = preserveOldData ? OPEN_EXISTING : CREATE_ALWAYS ;
		_rcvFp = CreateFile( file_name, GENERIC_READ|GENERIC_WRITE, 0, NULL, attr, FILE_ATTRIBUTE_NORMAL, 0 ) ;
		if( _rcvFp == NULL )
			_fileErr = GetLastError() ;
	#else
		const char *attr = preserveOldData ? "r+b" : "w+b" ;
		_rcvFp = fopenRetry( file_name, attr, sFILESHARE_NOSHARE );
		if( _rcvFp == NULL )
			_fileErr = MY_ERRNO ;
	#endif
	return _fileErr ;
}

int FileReceiver::closeFileData()
{
	_flushFileData() ;

	if( !_fileErr && fileInfo->infoCorrect && !_ignoreData && _dataReady )
	{
		// Remove check sum from the file
		uchar cs;
		ASSERT( EXTRABYTES == sizeof(cs) ) ;

		#ifdef USE_WIN32
			SetFilePointer( _rcvFp, fileInfo->fileSize, 0, FILE_BEGIN ) ;

			DWORD len ;
			ReadFile( _rcvFp, &cs, EXTRABYTES, &len, NULL ) ;

			int sizeFile = GetFileSize( _rcvFp, NULL ) ;
		#else
			fseek( _rcvFp, fileInfo->fileSize, SEEK_SET );
			fread( &cs, EXTRABYTES, 1, _rcvFp );

			SetFilePointer( _rcvFp, 0, 0, FILE_END ) ;
			fseek( _rcvFp, 0, SEEK_END );

			int sizeFile = ftell( _rcvFp );
		#endif

		if( fileInfo->fileSize + EXTRABYTES != sizeFile )
		{
			_status	= ReceivingBadData ;
			ON_FEC_CLOSEFILE( "Bad file size" ) ;
		}
		else
		if( cs != fileInfo->checkSum )
		{
			_status	= ReceivingBadData ;
			char buf[256] ;
			sprintf( buf, "\nBad CRC %x/%x", fileInfo->checkSum, cs );
			ON_FEC_CLOSEFILE( buf ) ;
		}
		else
		{
			#ifdef USE_WIN32
				SetFilePointer( _rcvFp, fileInfo->fileSize, 0, FILE_BEGIN ) ;
				SetEndOfFile( _rcvFp ) ;
			#else
				_chsize( _fileno( _rcvFp ), fileInfo->fileSize );
			#endif
			ON_FEC_CLOSEFILE( "OK" ) ;
		}
	}
	else
		ON_FEC_CLOSEFILE( _fileErr ? "I/Oerror" : "?" ) ;

	#ifdef USE_WIN32
		CloseHandle( _rcvFp );
	#else
		fclose( _rcvFp );
	#endif
	_rcvFp = NULL;
	return _fileErr ;
}

BOOL FileReceiver::_setFilePage( long pos )
{
	if( _fileBuf == NULL )
		_fileBuf = (char*)malloc( 1024*MUXPACKETDATASIZE ) ;

	if( pos >= _fileBufPtr  &&  pos < _fileBufPtr+_filePgSize )
		return !_fileErr ;

	_flushFileData() ;
	if( _fileErr )
		return FALSE ;

	int packet_ind = pos/packetSize ;
	if( packet_ind < 1023 )
	{
		_fileBufPtr = 0 ;
		_filePgSize = 1023 * packetSize ;
	}
	else
	{
		_fileBufPtr = ((packet_ind+1)/1024*1024 - 1) * packetSize ;
		_filePgSize = 1024 * packetSize ;
	}
	_fileWrPtr1 = _filePgSize ;
	_fileWrPtr2 = 0 ;

	#ifdef USE_WIN32
		if( SetFilePointer( _rcvFp, _fileBufPtr, 0, FILE_BEGIN) == -1 )
			_fileErr = GetLastError() ;
	#else
		if( fseek( _rcvFp, _fileBufPtr, SEEK_SET) != 0 )
			_fileErr = MY_ERRNO ;
	#endif
	else
	{
		//TRACE( "\nfread  %d .. %d", _fileBufPtr, _fileBufPtr+_filePgSize-1 ) ;

		// Because of FEC unused portion of the packet must be set to 0.
		#ifdef USE_WIN32
			ulong n ;
			if( !ReadFile( _rcvFp, _fileBuf, _filePgSize, &n, 0 ) )
				_fileErr = GetLastError() ;
			else
			if( n != _filePgSize )
			{
				memset( _fileBuf+n, 0, _filePgSize-n ) ;
			}
		#else
			int n = fread( _fileBuf, 1, _filePgSize, _rcvFp) ;
			if( n != _filePgSize )
			{
				if( ferror( _rcvFp) )
					_fileErr = MY_ERRNO ;
				else
					memset( _fileBuf+n, 0, _filePgSize-n ) ;
			}
		#endif
	}
	return !_fileErr ;
}

BOOL FileReceiver::readFileData( long pos, uchar *bytes, int n_bytes )
{
	if( !_setFilePage( pos) )
		return FALSE ;
	memcpy( bytes, _fileBuf+pos-_fileBufPtr, n_bytes ) ;

	return TRUE ;
}

BOOL FileReceiver::writeFileData( long pos, const uchar *bytes, int n_bytes )
{
	if( !_setFilePage( pos) )
		return FALSE ;

	int pos1 = pos - _fileBufPtr ;
	int pos2 = pos1 + n_bytes ;
	memcpy( _fileBuf+pos1, bytes, n_bytes ) ;

	_fileWrPtr1  = __min( _fileWrPtr1, pos1 ) ;
	_fileWrPtr2  = __max( _fileWrPtr2, pos2 ) ;
	_fileChanged = TRUE ;
	return TRUE ;
}

void FileReceiver::_flushFileData()
{
	if( _flushingFileData )
		return ;
	_flushingFileData = TRUE ;
	try
	{
		applyAllFecPackets() ;

		if( !_fileErr  &&  _fileChanged )
		{
			DWORD n ;
			#ifdef USE_WIN32
				if( SetFilePointer( _rcvFp, _fileBufPtr+_fileWrPtr1, 0, FILE_BEGIN) == -1  ||
					!WriteFile( _rcvFp, _fileBuf+_fileWrPtr1, _fileWrPtr2-_fileWrPtr1, &n, 0) )
			#else
				if( fseek( _rcvFp, _fileBufPtr+_fileWrPtr1, SEEK_SET) != 0  ||
					fwrite( _fileBuf+_fileWrPtr1, _fileWrPtr2-_fileWrPtr1, 1, _rcvFp) != 1 )
			#endif
			{
				_fileErr = GetLastError() ;
				//_fileErr = MY_ERRNO ;
				FileRcvMsgError msg( "File write error.", _fileErr );
				msgFun( RcvError, &msg );
			}
			_fileChanged = FALSE ;
		}
	}
	catch(...) {}
	_flushingFileData = FALSE ;
}

void FileReceiver::destroyFileData()
{
	free( _fileBuf ) ;
	initFileData() ;
}


//-----------------------------------------------------------
//	Fec packet queue
//	Fec packets which have more than 2 unknown base packets
//	are stored here to be decoded later.
//-----------------------------------------------------------


struct FecPacketHdr
{
	int packetInd0 ;			// Base packets used in the construction of fp are
	int packetInd1 ;			//	for( int j=packetInd0 ; j < packetInd1 ; j+=cycleLength )
	int cycleLength;			//
	int packetIndToCorrect ;	// Index of the first undefined base packet
	inline FecPacketHdr( int p0, int p1, int cycleLen )
	{
		packetInd0 = p0 ;
		packetInd1 = p1 ;
		cycleLength= cycleLen ;
		packetIndToCorrect = p0 ;
	}
	FecPacketHdr() {}
} ;

struct FecPacket
{
	FecPacketHdr hdr ;
	MuxPacket	 packet ;
	FecPacket *next ;
	FecPacket *prev ;
	FecPacket( ) {}
} ;

void FileReceiver::openFecQueue( )
{
	usedFecList = unusedFecList = NULL ;
}

void FileReceiver::applyAllFecPackets( )
{
	if( _ignoreData  ||  _dataReady )
		goto labelEnd ;
	try
	{
		MuxPacket recoveredPacket ;
		while( 1 )
		{
			BOOL quit=TRUE ;
			FecPacket *fp, *next ;
			for( fp=usedFecList ; fp != NULL ; fp=next )
			{
				if( !_running )
					goto labelEnd ;
				next = fp->next ;
				switch( testBaseFecPackets( &fp->hdr) )
				{
					case 1 :
						if( processFecPacket( &fp->hdr, &fp->packet, &recoveredPacket) )
						{
							ON_LATERECOVER_FECPACKET() ;
							acceptPacket( &recoveredPacket ) ;
							quit = FALSE ;
						}
						// fall through
					case 0 :
						releaseFecPacket( fp ) ;
						break ;
				}
			}
			if( quit )
				break ;
		}
	}
	catch(...)
	{
	}
  labelEnd:
	releaseAllFecPackets() ;
}

void FileReceiver::storeFecPacket( FecPacketHdr *fp_hdr, MuxPacket *mp )
{
	// Test if stored packets are from the same packet block.
	if( usedFecList != NULL  &&  usedFecList->hdr.packetInd1 != fp_hdr->packetInd1 )
	{
		// Different block; try to apply all packets and release the rest of them.
		applyAllFecPackets() ;
	}

	// construct packet (take from unused or create a new one)
	FecPacket *fp ;
	if( unusedFecList != NULL )
	{
		fp = unusedFecList ;
		FecPacket *next = unusedFecList->next ;
		unusedFecList = next ;
		if( next != NULL )
			next->prev = NULL ;
	}
	else
	{
		fp = new FecPacket( ) ;
	}

	// initialize packet contents
	fp->hdr		= *fp_hdr ;
	fp->packet	= *mp ;

	// add to used
	fp->next	= usedFecList ;
	fp->prev	= NULL ;
	if( usedFecList != NULL )
		usedFecList->prev = fp ;
	usedFecList = fp ;
}

void FileReceiver::releaseFecPacket( FecPacket *fp )
{
	// release from used
	FecPacket *prev = fp->prev ;
	FecPacket *next = fp->next ;
	if( prev != NULL )
		prev->next = next ;
	else
		usedFecList = next ;
	if( next != NULL )
		next->prev = prev ;

	// add to unused
	fp->next	= unusedFecList ;
	fp->prev	= NULL ;
	if( unusedFecList != NULL )
		unusedFecList->prev = fp ;
	unusedFecList = fp ;
}

void FileReceiver::releaseAllFecPackets( )
{
	if( usedFecList == NULL )
		return ;

	// Find last used
	FecPacket *last_used ;
	int cnt=0 ;
	for( last_used=usedFecList ; last_used->next != NULL ; last_used = last_used->next )
		cnt++ ;
	TRACE( "\nReleasing %d FEC packets from %d..%d", cnt, last_used->hdr.packetInd0, last_used->hdr.packetInd1 ) ;

	// Connect last used to the unused list
	last_used->next = unusedFecList ;
	if( unusedFecList != NULL )
		unusedFecList->prev = last_used ;
	unusedFecList = usedFecList ;

	// clear used list
	usedFecList = NULL ;
}

void FileReceiver::closeFecQueue( )
{
	// Release used list
	releaseAllFecPackets() ;

	// Destroy unused list
	FecPacket *fp, *next ;
	for( fp=unusedFecList ; fp != NULL ; fp=next )
	{
		next = fp->next ;
		delete fp ;
	}
	unusedFecList = NULL ;
}


//-----------------------------------------------------------
//	applyFec()
//-----------------------------------------------------------


#pragma optimize( OPTOPTIONS, on )

BOOL FileReceiver::applyFec( MuxPacket *fp, MuxPacket *recoveredPacket )
{
	ON_FECPACKET_ARRIVED() ;

	uchar  cycleLength, rowInd ;
	ushort blockInd, blockSize ;
	if( !fp->getFecInfo( cycleLength, rowInd, blockInd, blockSize) )
	{
		ON_BAD_FECPACKET() ;
		return FALSE ;
	}

	// fp = XOR{ p[i] ; i%cycleLength==rowInd }
	int  packetInd0 = blockInd*512 ;
	int  packetInd1 = packetInd0 + blockSize ;
	if( packetInd1 > maxPackets() )		// Last block?
	{
		if( !fileInfo->infoCorrect )
			// real value of maxPackets() is unknown
			return FALSE ;
		packetInd1 = maxPackets() ;
	}
	packetInd0 += rowInd ;

	FecPacketHdr hdr( packetInd0, packetInd1, cycleLength ) ;
	switch( testBaseFecPackets( &hdr) )
	{
		case 2 :
			ON_UNDETERMINED_FECPACKET() ;
			storeFecPacket( &hdr, fp ) ;
			return FALSE ;				// cannot correct more than 1 packet
		case 0 :
			ON_UNNEEDED_FECPACKET() ;
			return FALSE ;				// nothing to correct
	}

	return processFecPacket( &hdr, fp, recoveredPacket ) ;
}

int FileReceiver::testBaseFecPackets( FecPacketHdr *fpd )		// # unknown base packets: 0, 1, 2 (2 and more)
{
	int j = fpd->packetIndToCorrect ;
	fpd->packetIndToCorrect = -1 ;
	while( j < fpd->packetInd1 )
	{
		if( !isMapSet(j) )
		{
			if( fpd->packetIndToCorrect != -1 )
				return 2 ;
			fpd->packetIndToCorrect = j ;
		}
		j += fpd->cycleLength ;
	}
	return fpd->packetIndToCorrect == -1 ? 0 : 1 ;
}

BOOL FileReceiver::processFecPacket( FecPacketHdr *fpd, MuxPacket *fp, MuxPacket *recoveredPacket )
{
	int packetIndToCorrect = fpd->packetIndToCorrect ;
	int packet_size = fp->isUnicastPacket() ? MUXPACKETUNICASTDATASIZE : MUXPACKETDATASIZE ;

	MuxPacket mp ;
	mp.setFlags( fp->flags() ) ;	// tmp packet; actually we only need correct Unicast flag

	*recoveredPacket = *fp ;		// header is needed for subsequent operations
	for( int j=fpd->packetInd0 ; j < fpd->packetInd1 ; j+=fpd->cycleLength )
	{
		if( j == packetIndToCorrect )
			continue ;
		if( j == 0 )
		{
			mp = _hdrPacket ;
		}
		else
		if( !readFileData( (j-1)*packet_size, (uchar*)mp.data(), packet_size) )
		{
			ON_GETPACKET_ERROR() ;
			return FALSE ;
		}
		recoveredPacket->xorData( mp ) ;
	}

	recoveredPacket->setPacketInd( packetIndToCorrect ) ;

	// _numDataBytes is used by FEC ... must be corrected
	if( packetIndToCorrect == 0 )
	{
		recoveredPacket->setDataLength( sizeof(FileHeader) ) ;
	}
	else
	if( fileInfo->infoCorrect )
	{
		// All packets except the last one occupy all available Bytes.
		int  data_size   = fileInfo->fileSize + EXTRABYTES ;		// CRC is appended to the file data!
		BOOL last_packet = ( packetIndToCorrect*packet_size > data_size ) ;
		if( last_packet )
			recoveredPacket->setDataLength( data_size % packet_size ) ;
		else
			recoveredPacket->setDataLength( packet_size ) ;
	}
	else
	if( packetIndToCorrect < highestPacket() )
		// The packet must have max. size because it is not the last one
		recoveredPacket->setDataLength( packet_size ) ;
	else
	{
		// We don't know the packet size
		ON_UNKNOWN_FECPACKET() ;
		return FALSE ;
	}

	recoveredPacket->computeCheckSum() ;		// needed for fileInfo->checkSum
	ON_RECOVER_FECPACKET() ;

	return TRUE ;
}

#pragma optimize( "", on )			// restore original optimization options


//-----------------------------------------------------------
//	push()
//-----------------------------------------------------------


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

void FileReceiver::_startReceiving( void *msg )
{
	_ignoreData = !isFileOpened();		// ignore data if can't open file
	if ( !_ignoreData )
	{
		msgFun( FileOpened, (FileRcvMsgReceiving*)msg );	// signalize opening
		msgFun( StartReceiving, NULL );						// signalize start of receiving
		msgFun( FileName, (FileRcvMsgReceiving*)msg );		// set file name in progress
	}
}

#define CLEAR_MAP()	{ clearMap(); fileInfo->checkSum = 0 ; }

void FileReceiver::push( MuxPacket *packet )
{
	if( !_running )
		return ;

	EnterCriticalSection( &_fileReceiverLock );
	try
	{
		BOOL sameJob = (packet->jobId() == _jobId) ;
		if( !sameJob )
		{
			closeFile();								// another job, close this file
			CLEAR_MAP() ;
			_jobId = packet->jobId() ;
		}

		if( packet->flags() & MuxPacket::Fec )
		{
			if( !sameJob )
				goto labelEnd ;

			if( !isFileOpened()  ||  _ignoreData  ||  _dataReady )
				goto labelEnd ;

			/*
			if( !_lastPacketWasFec )
			{
				// Test if the FEC block is ready.
				// If yes, we'll ignore all FEC packets until next FEC block.
				if( _lastPacketInd >= 0 )
				{
					int i1 = _lastPacketInd/1024*1024 ;
					_fecBlocked = isIntervalReady( i1, i1+1023 ) ;
					_lastPacketWasFec = TRUE ;
				}
				else
					_fecBlocked = TRUE ;

			}
			if( _fecBlocked )
				goto labelEnd ;
			*/

			MuxPacket mp ;
			if( !applyFec( packet, &mp) )
				goto labelEnd ;

			// some packet was recovered; lets continue as if it was received in a standard way
			*packet = mp ;
		}
		else
			_lastPacketWasFec = FALSE ;
		acceptPacket( packet, sameJob ) ;
	}
	catch( ... )
	{
	}
  labelEnd:
	LeaveCriticalSection( &_fileReceiverLock );
}

void FileReceiver::acceptPacket( MuxPacket *packet, BOOL sameJob )
{
	DvbClientSetup *cSetup = MfxClientSetup();

	int rebroadcastInd ;
	if( packet->flags() & MuxPacket::Fec )
		rebroadcastInd	= _rebroadcastInd ;
	else
		rebroadcastInd	= packet->rebroadcastIndex();	// get the current rebr. number
	int packetInd		= packet->packetIndex() ;		// get the current packet index
	_lastPacketInd = packetInd ;
	packetSize = packet->isUnicastPacket() ? MUXPACKETUNICASTDATASIZE : MUXPACKETDATASIZE ;

	#define INC_REBR_NUM()	{							\
		int x = _rebroadcastInd - rebroadcastInd ;		\
		fileInfo->numberOfRebrNeeded += __max( x, 1 ) ;	\
		_rebroadcastInd = rebroadcastInd;				\
	}
	#define RESET_REBR_NUM()	{						\
		fileInfo->numberOfRebrNeeded = 1 ;				\
		_rebroadcastInd = rebroadcastInd;				\
	}

	//
	//----------------------- Header packet -----------------------
	//
	if ( packetInd == 0 )
	{
		char fn[256], path[_MAX_PATH];

		FileHeader *hdr = (FileHeader *)packet->data() ;
		long   fsize ;
		time_t ftime ;
		DWORD  fattr ;
		hdr->get( fn, &fsize, &ftime, &fattr, &_numOfRebroadcast ) ;

		_makepath( path, NULL, _dir, fn, NULL ) ;

		FileRcvMsgReceiving msg( path, fsize, rebroadcastInd, _numOfRebroadcast );

		// File time resolution on NT is 2 secs!
		if ( fileExist( path ) && abs(ftime - fileTime(path)) <= 1 )
		{	// if file exist in this version ignore packets data ( new job, if old
			_dataReady  = TRUE;
			_ignoreData = TRUE;
			closeFile() ;
			if( !sameJob )
				msgFun( FileAllreadyExisting, &msg );	// send message only if new job arrives
			return ;
		}

		_ignoreData = FALSE ;
		if( !sameJob )
		{
			_dataReady	= FALSE;

			// Try to open old file, resp. create a new one.
			openFile( TRUE );

			if( fileInfo->infoCorrect )
			{
				// previously received file of this job was incomplete
				if ( fileInfo->equal( path, fsize, ftime, fattr ) )
				{
					// the same file as received in prev. rebr. of this job, no modifications needed
					INC_REBR_NUM() ;
					msgFun( FileName, &msg );
					msgFun( FileOpenedToAppend, &msg );
					return ;		// continue to completize file from this rebroadcast
				}

				closeFile();		// close incomplete file and
				CLEAR_MAP();
				openFile( FALSE );	// open a new one for this job
			}

			// fill fileInfo structure
			fileInfo->set( path, fsize, ftime, fattr, packetSize ) ;
			fileInfo->totalBytesSent = 0;
			RESET_REBR_NUM() ;

			cSetup->incNumFilesTransferred();				// new file arriving
			_startReceiving( &msg ) ;
		}
		else
		{	// same job
			if ( !fileInfo->infoCorrect )
			{
				// fill fileInfo structure
				fileInfo->set( path, fsize, ftime, fattr, packetSize ) ;

				if ( highestPacket() >= fileInfo->maxPackets )
				{	// previous file can't be the same
					closeFile();							// close(delete) noname file and
					CLEAR_MAP();
					openFile( FALSE );						// open a new one for this job

					RESET_REBR_NUM() ;
					cSetup->incNumFilesTransferred();		// new file arriving
				}
				else
				{	// the same file as prev. (prev. has not fileInfo completed)
					INC_REBR_NUM() ;
				}
				_startReceiving( &msg ) ;
			}
			else
			{
				if ( !fileInfo->equal( path, fsize, ftime, fattr ) )
				{
					// File differs from previous rebroadcast.
					fileInfo->set( path, fsize, ftime, fattr, packetSize ) ;
					RESET_REBR_NUM() ;
					closeFile();							// close incomplete file
				}
				if( !isFileOpened() )
				{
					RESET_REBR_NUM() ;
					CLEAR_MAP();
					openFile( FALSE );						// open a new one for this job

					cSetup->incNumFilesTransferred();		// new file arriving
					_startReceiving( &msg ) ;
				}
				else
				{
					INC_REBR_NUM() ;
					msgFun( FileName, &msg );
					ON_REBROADCAST() ;
				}
			}
		}

		resizeMap( fileInfo->maxPackets ) ;
		setMap( packetInd );
		_hdrPacket = *packet ;
		_dataReady = isReady( );	// update ready flag ( if only header missing from previous rebroadcast )
		if ( _dataReady )			// if ready, close the file
			closeFile();

		return ;					// no other data in header packet, continue receiving with next packet
	}

	//
	//----------------------- Packet from another job -----------------------
	//
	if( !sameJob )
	{
		_dataReady	= FALSE;
		_ignoreData	= FALSE;

		// Try to reopen old partially received file.
		openFile( TRUE );

		if ( !fileInfo->infoCorrect )
		{
			// Invalid file info, new file was opened
			RESET_REBR_NUM() ;
			cSetup->incNumFilesTransferred();			// new file arriving
			// don't need to send message
		}
		else
		{
			// Valid file info; test if it belongs to this packet
			if ( packetInd > fileInfo->maxPackets )
			{
				// It does not - open a new file.
				closeFile();
				CLEAR_MAP();
				openFile( FALSE );

				RESET_REBR_NUM() ;
				cSetup->incNumFilesTransferred();		// new file arriving

				// don't know how many rebroadcast can arrive
				FileRcvMsgReceiving msg( fileInfo->fileName, fileInfo->fileSize, 0, -1 );
				_startReceiving( &msg ) ;
			}
			else
			{
				// It does - continue receiving old file.
				INC_REBR_NUM() ;

				// update progress
				FileRcvMsgReceiving msg( fileInfo->fileName, fileInfo->fileSize, rebroadcastInd, _numOfRebroadcast );
				msgFun( FileName, &msg );		// set file name in progress

				resizeMap( fileInfo->maxPackets ) ;
				_dataReady = isReady( );	// update ready flag ( if only header missing from previous rebroadcast )
				if ( _dataReady )			// if ready, close the file
					closeFile();
			}
		}
	}

	//
	//----------------------- check -----------------------
	//
	if( _dataReady || !isFileOpened() || _ignoreData || isMapSet(packetInd) )	// nothing to do
		return ;

	if( rebroadcastInd != _rebroadcastInd )
	{
		INC_REBR_NUM() ;
		if ( fileInfo->infoCorrect )
		{	// update file rebr. number
			FileRcvMsgReceiving msg( fileInfo->fileName, fileInfo->fileSize, _rebroadcastInd, _numOfRebroadcast );
			msgFun( FileName, &msg );		// set file name in progress
		}
	}
	// allow accepting packets without header (i.e. unknown size)
	if( maxPackets() <= packetInd )
	{
		if( !fileInfo->infoCorrect )
			resizeMap( packetInd+10 ) ;
		else
		{	// error in data, more data than possible
			_status = ReceivingBadData ;
			TRACE ( "\nMore packets - Bad Data" );
			DUMP_RCVPACKET( packet, 0x08 );

			_ignoreData = TRUE;					// ignore the rest of this rebroadcast
			return ;
		}
	}

	//
	//----------------------- write -----------------------
	//

	// test for error only if file is not correctly received and header information is correct
	// if header exist the info is correctly set and temp file must be created

	long filePos = (packetInd-1)*packetSize ;

	int numBytes = packet->numDataBytes() ;
	if ( fileInfo->infoCorrect )
	{
		int numMissing = fileInfo->fileSize - filePos + EXTRABYTES;
		if( numBytes > numMissing  ||  ( numMissing>packetSize && numBytes<packetSize) )
		{
			_status = ReceivingBadData ;
			TRACE ( "\n%s data - Bad Data", numBytes > numMissing ? "More" : "Less" );
			DUMP_RCVPACKET( packet, 0x08 );
			_ignoreData = TRUE;					// ignore the rest of this rebroadcast
			return ;
		}
	}

	if( !writeFileData( filePos, packet->data(), numBytes) )
		_ignoreData = TRUE;			// ignore the rest
	else
	{
		fileInfo->checkSum ^= packet->dataCheckSum() ;
		setMap( packetInd );
		_dataReady = isReady( );	// update ready flag
		fileInfo->totalBytesSent += numBytes;
		if ( _dataReady )			// if ready, close the file
		{
			closeFile();
		}
	}

	//
	//----------------------- progress -----------------------
	//
	if( !_dataReady )
	{
		// This is just to save some time.
		// Should be programmed better - eg. getSystemTime() is extremally inefficient

		int wantMsg = 0 ;	// 0=not, 1=yes if not too often, 2=always
		#define STEP	10
		if( ++_progressCounter % STEP == 0 )
			wantMsg = 1 ;
		if( fileInfo->infoCorrect )
		{
			if( maxPackets() - packetInd < 3 )
				wantMsg = 2 ;
			else
			if( maxPackets() - numPackets() < 3 )
				wantMsg = 2 ;
		}

		if( wantMsg )
		{
			LONGLONG elapsedTime = getSystemTime() - _progressTime;
			if( (wantMsg == 2 && elapsedTime > 0)  ||  elapsedTime > UPDATE_TIME * 10000000.0f )
			{
				msgFun( RcvProgress, NULL );

				long size = fileInfo->totalBytesSent - _nextProgress;
				double speed;
				_nextProgress = fileInfo->totalBytesSent;
				if( size > 0 )
				{
					speed = (double)size / (double)elapsedTime;
					#ifdef REPORT_KB_SPEED
						speed *= 10000000.0f / 1024.0f;				// KB/s
					#else
						speed *= 10000000.0f * 8.f / 1000000.f * (float)TSPACKET_SIZE / (float)MUXPACKETDATASIZE ;	// Mb/s
					#endif
				}
				else
					speed = 0.0f;
				_progressTime += elapsedTime;
				msgFun( RcvSpeed, &speed );
			}
		}
	}
}


//-----------------------------------------------------------
//	messages
//-----------------------------------------------------------


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
		case RcvProgress:
		{
			float f ;
			if( fileInfo && fileInfo->infoCorrect )
				f = 100.f*fileInfo->totalBytesSent/fileInfo->fileSize;
			else
				f = 0 ;
			sprintf( buf, "%.2f", f );
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
				sprintf( buf, "(%d/infinit) %s  (size  %li bytes).", fileInfo->numberOfRebrNeeded, msg->fileName, msg->fileSize );
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
