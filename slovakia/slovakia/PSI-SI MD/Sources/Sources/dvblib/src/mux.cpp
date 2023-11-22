/*
 *	Filename:		mux.cpp
 *
 *	Version:		1.00
 *
 *	Description: implementation of following clases:
 *		MuxPacket
 *		Mux
 *		MuxChannel
 *		MuxOutput
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"

#include "DvbUser.hpp"
#include "inbox.hpp"
#include "MfxGlobals.hpp"
#include "ServCfg.hpp"

// a = no aliasing
// g = global
// t = fast machine code
// y = no stack frame
//#define OPTOPTIONS	"agt"
//#define OPTOPTIONS	"agty"
#pragma warning(disable:4083)	// disable 'expected string' warning

// Uncomment to get info about multiplexor.
//#define MUXTRACE

// Uncomment to get info about output speed.
//#define FLUSHTRACE

// Minimal number of flushed packets
// MuxOutput will send fill packets if small amount of internet data is transmited
// Comment to disable this function
//#define MIN_FLUSHED_INTERNET_PACKETS		6

// uncomment to obtain short statistics about output stream
// statistics are traced into output window at the end of the session
//#define ANALYZE_OUTPUT_STREAM

// uncomment to avoid breaking of the MPE section
// Whole MPE section will be sent in 1 flush
// For testing only - may cause problems with MuxOutput
//#define DO_NOT_DIVIDE_MPE_SECTION

static MuxPacket fillPacket;


#ifdef _PRIVATETRACE

// Debugging tool - redirects TRACE macros to the file.
void writeTrace( char *pFormat, ... )
{
	static FILE *traceFile = NULL;
	static BOOL  traceFileOpened=FALSE ;

	if ( !traceFile )
	{
		if( traceFileOpened )
			return ;
		traceFileOpened = TRUE ;

		char buf[1024], drv[10], dir[1024] ;

		GetModuleFileName( NULL, buf, sizeof(buf) ) ;
		_splitpath( buf, drv, dir, 0, 0 ) ;
		_makepath ( buf, drv, dir, "TraceFile.dump", 0 ) ;

		traceFile = fopen( buf, "wt" );
	}
	vfprintf( traceFile, pFormat, va_arglist(pFormat) );
}

#endif



//--------------------------------------------------------------------------------
//	MuxChannelSetup
//--------------------------------------------------------------------------------

MuxChannelSetup::~MuxChannelSetup()			
{ 
	free( inboxDir) ; /*free( connectStr);*/ 
	PidStreamAttribManager *man = MfxServerSetup()->pidStreamManager() ;
	if (man && channelPID)
		man->releasePidStreamAttrib(channelPID) ;
}

const MuxChannelSetup &MuxChannelSetup::operator=( const MuxChannelSetup & src )
{
	memcpy( (char*)this, (char*)&src, sizeof( MuxChannelSetup ) - 2*sizeof(char*) );
	free( inboxDir	); 
	inboxDir = strdup( src.inboxDir   );

	PidStreamAttribManager *man = MfxServerSetup()->pidStreamManager() ;
	if (channelPID)
		man->releasePidStreamAttrib(channelPID) ;
	channelPID = man->getPidStreamAttrib(src.channelPID->pid()) ;

	return *this;
}

//--------------------------------------------------------------------------------
//	MuxChannel
//--------------------------------------------------------------------------------

CRITICAL_SECTION MuxChannel::_distrLock;		// packet distribution synchronization

// Constructor - initialize variables, create senders (based on channelType)
MuxChannel::MuxChannel( int channelType, Mux *mux, const MuxChannelSetup *s, BOOL online )
{
	_statusString[0] = 0;
	_fileSender = NULL ;
	_dataSender = NULL ;
	_maxErrors	= 5 ;
	_numErrors	= 0 ;
	_status		= online ? Online : 0 ;
	_setup		= *s ;
	_mux		= mux ;
	//_isStarted  = FALSE ;
	_hKillEvent	 			   =NULL ;
	_hNumFreePacketAvailable   =NULL ;
	_hNumFreePacketAvailableSrv=NULL ;
	_hNumFreePacketAvailable = CreateEvent( NULL, FALSE, FALSE, NULL );
	_hKillEvent  = CreateEvent( NULL,  TRUE, FALSE, NULL );
	_numfreepack = 0;//100000 ;
	_assigned_numfreepack = 0 ;
	_speed = 0 ;

	if( channelType & FileType )
		_fileSender = new FileSender( _mux->output(), _hKillEvent, _hNumFreePacketAvailable, &_numfreepack );
	if( channelType & ServiceType )
	{
		// We have to use different event because manual event cannot serve to 2 threads.
		_hNumFreePacketAvailableSrv = CreateEvent( NULL, FALSE, FALSE, NULL );
		_dataSender = new DataSender( _mux->output(), _hKillEvent, _hNumFreePacketAvailableSrv, &_numfreepack );
		_dataSender->start() ;
	}
}

void MuxChannel::setOnline( BOOL yes)
{
	if( yes )
	{
		if( !isOnline() )
			setStatusBit( Online ) ;
	}
	else
	{
		if( isOnline() )
			delStatusBit( Online ) ;
		clearNumFreePackets() ;
	}
}

void MuxChannel::reset( const MuxChannelSetup *s )
{
	EnterCriticalSection( &_distrLock);
	_setup = *s ;
	LeaveCriticalSection( &_distrLock);
}


// -> status in textual form
void MuxChannel::reportStatus( )
{
	char _oldStatus[256];

	strcpy( _oldStatus, _statusString );
	if( !isOnline() )
		strcpy( _statusString, "Offline" ) ;
	else
		strcpy( _statusString, (_status & Started) ? "Active" : "Inactive" ) ;
	if( _status & VolumeExc )
		strcat( _statusString, "; VolumeExceeded" ) ;
	if( _status & StopDueToError )
		strcat( _statusString, "; StopDueToError" ) ;
	if( strcmp( _statusString, _oldStatus ) )
		MfxPostMessage( EMsg_InboxStatus, _setup.channel, _statusString );
}

void MuxChannel::setFatalError( const char *str, DWORD err )
{
	char buf[1024], txt[256] ;
	sprintf( buf, "%s - %s", str, DvbEventText(err,txt) );
	MfxPostMessage( EMsg_InboxError, _setup.channel, buf );

	setStatusBit( StopDueToError );
}

void MuxChannel::setError( const char *str, DWORD err )
{
	char buf[1024], txt[256] ;
	sprintf( buf, "%s - %s", str, DvbEventText(err,txt) );
	MfxPostMessage( EMsg_InboxError, _setup.channel, buf );

	if( ++_numErrors > _maxErrors )
	{
		setStatusBit( StopDueToError );
		SetEvent( _hKillEvent );
	}
}

// post the message to the global message handler
void MuxChannel::msgFun( long code, void *param )
{
	char buf[1024];

	msgAsText( code, param, buf );
	MfxPostMessage( code, _setup.channel, buf );
}

// initiate stop() process
BOOL MuxChannel::initiateStop()
{
	if( !isStarted() )
		return TRUE ;
	if( isSending() )
	{
		SetEvent( _hKillEvent );
	}
	return TRUE ;
}

MuxChannel::~MuxChannel( )
{
	initiateStop() ;
	delete _dataSender ;
	delete _fileSender ;
	if ( _hNumFreePacketAvailable!=NULL )
		CloseHandle( _hNumFreePacketAvailable );
	if ( _hNumFreePacketAvailableSrv!=NULL )
		CloseHandle( _hNumFreePacketAvailableSrv );
	if ( _hKillEvent			!=NULL )
		CloseHandle( _hKillEvent );
}


// File services only pass the data to the underlying sender.
int MuxChannel::broadcastFile( const char *fullFileName, ushort flags )
{
	if( _fileSender == NULL )
		return 0 ;
	return _fileSender->sendFile( fullFileName, 0, _setup.streamFormat, _setup.fecLevel, _setup.numRebroadcasts,
		channelPID(), GlobalUserID(), flags ) ;
}


int MuxChannel::unicastFile( const char *fullFileName, const GlobalUserID& usrId, ushort flags )
{
	if( _fileSender == NULL )
		return 0 ;
	return _fileSender->sendFile( fullFileName, 0, _setup.streamFormat, _setup.fecLevel, _setup.numRebroadcasts,
		channelPID(), usrId, MuxPacket::Unicast | flags ) ;
}


int MuxChannel::multicastFile( const char *fullFileName, ushort ch, ushort flags )
{
	if( _fileSender == NULL )
		return 0 ;
	return _fileSender->sendFile( fullFileName, ch, _setup.streamFormat, _setup.fecLevel, _setup.numRebroadcasts,
		channelPID(), GlobalUserID(), MuxPacket::Multicast | flags ) ;
}


//--------------------------------------------------------------------------------
//	MuxTimer
//	This is the timer which calls multiplexor (distributeRecordRequests())
//  in intervals specified by _mux->_checkTime. (Default = 1sec)
//--------------------------------------------------------------------------------


class MuxTimer : public CWinThread
{
	Mux		*_mux ;
	HANDLE	 _eventKill;		// SetEvent() to kill this process
  public:

	void shutdown( )
	{
		VERIFY( SetEvent(_eventKill) );
		SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
		WaitForSingleObject(m_hThread, INFINITE);
	}
	MuxTimer( Mux *mux )
	{
		m_bAutoDelete = FALSE;
		_mux = mux ;
		_eventKill = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
   ~MuxTimer()
	{
		CloseHandle( _eventKill);
	}

  protected:
	virtual BOOL InitInstance();
};


BOOL MuxTimer::InitInstance()
{
	// loop but check for kill notification
	while (WaitForSingleObject(_eventKill, _mux->_checkTime) == WAIT_TIMEOUT)
		_mux->distributeRecordRequests() ;
	return FALSE;		// avoid entering standard message loop by returning FALSE
}


//--------------------------------------------------------------------------------
//	Mux
//  Combines inputs from different channels into single stream and sends them to MuxOutput.
//--------------------------------------------------------------------------------


// If multiplexor does not receive any packets -> probably error.
// These 3 variables monitor this state.
static int  timeNotProcessedRecs= 0;		// how many times no data sent
static BOOL msgPosted			= FALSE;	// was it announced already?
static BOOL packetsSent			= TRUE;		// TRUE if in the last multiplexor round some packets were sent

Mux::Mux( const MuxSetup *s, MuxOutput *o )
{
	InitializeCriticalSection( &MuxChannel::_distrLock );
	_muxTimer		= NULL ;
	_checkTime		= 1000 ;			// 1 sec
	_outputRate		= 0 ;
	_output			= o ;
	_changeOfOutputRateRequested = FALSE ;
	reset( s ) ;
}

// # of packets to be sent during multiplor period
inline int Mux::numRecsDuringCheckTime( float rate )	// Mb/s
{
	// Mb -> B: 1024 * 128		// 128 = 1024/8
	// _checkTime in msec, that's way divide by 1000
	//
	// 131.072 = 1024 * 128 / 1000
	if( rate < 0 )
		rate = _outputRate ;
 	//return (int)(rate * _checkTime * 131.072 /*+ PACKETSIZE - 1*/) / MUXPACKETSIZE ;
 	return (int)( rate * _checkTime * (1000/8) / TSPACKET_SIZE ) ;
}

// change of the output rate
void Mux::setOutputRate( float rate )
{
	_changeOfOutputRateRequested = TRUE ;
	_outputRate	= rate ;
}

// change of setup
void Mux::reset( const MuxSetup *s )
{
	if( _setup == *s )
		return ;
	EnterCriticalSection( &MuxChannel::_distrLock);
	_setup			= *s ;
	setOutputRate( _outputRate ) ;
	LeaveCriticalSection( &MuxChannel::_distrLock);
}

Mux::~Mux( )
{
	DeleteCriticalSection( &MuxChannel::_distrLock );
}

// add new channel to multiplexor
void Mux::registerChannel( MuxChannel *x )
{
	EnterCriticalSection( &MuxChannel::_distrLock);
	if ( _channels.find( x ) < 0 )
		_channels.add( x );	
	LeaveCriticalSection( &MuxChannel::_distrLock);
}

// remove channel from multiplexor
void Mux::unregisterChannel( MuxChannel *x )
{
	EnterCriticalSection( &MuxChannel::_distrLock);
	_channels.delObj( x );
	LeaveCriticalSection( &MuxChannel::_distrLock);
}

// initialize and create multiplexor timer
BOOL Mux::start( )
{
	timeNotProcessedRecs= 0;
	packetsSent			= TRUE;
	msgPosted			= FALSE;

	ASSERT( _muxTimer == NULL ) ;
	_muxTimer = new MuxTimer( this);
	if( _muxTimer == NULL)
		return FALSE;

	ASSERT_VALID(_muxTimer);
	_muxTimer->m_pThreadParams = NULL;

	// Create Thread in a suspended state so we can set the Priority before it gets away from us
	if (!_muxTimer->CreateThread(CREATE_SUSPENDED))
	{
		delete _muxTimer;
		_muxTimer = NULL ;
		return FALSE;
	}

	VERIFY( _muxTimer->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL) );
	_muxTimer->ResumeThread();
	return TRUE ;
}

// stop multiplexor
void Mux::stop() 
{
	if( _muxTimer != NULL )
	{
		_muxTimer->shutdown() ;
		delete _muxTimer ;
		_muxTimer = NULL ;

		int numOfChannels	= _channels.count();
		for( int i=0 ; i < numOfChannels ; i++ )
		{
			MuxChannel *ch = _channels[i];
			ch->_speed = 0 ;
			ch->_assigned_numfreepack = 0 ;
		}
	}
}


//-------------------------------------------------------------------------------
//	Mux::distributeRecordRequests()
//-------------------------------------------------------------------------------


// Variables for the computation of the max. achieved rate.
// Used to rapidly increase transfer speed after longer low transfer.
static int achievedRates[5] ;
static int achievedRatesCounter=0 ;
static int maxAchievedRate=0 ;

void Mux::distributeRecordRequests()
{
	#ifdef MUXTRACE
		static int muxCounter=0 ;
		muxCounter++ ;
		TRACE( "\n\n%4d>>>", muxCounter ) ;
	#endif

	EnterCriticalSection( &MuxChannel::_distrLock );

	try
	{
		#ifdef MUXTRACE
			TRACE( ">>>>>>>>>\nResults from prev. round:\n"
				" Chan unused Mb/s" ) ;
			float tot_speed=0 ;
		#endif
		int   numOfChannels	= _channels.count();
		int   i;
		for( i = 0; i<numOfChannels; i++ )
		{
			MuxChannel *ch = _channels[i];
			ch->_speed = (ch->_assigned_numfreepack - ch->_numfreepack) * TSPACKET_SIZE * 8 / 1000000.f ;
			ch->_assigned_numfreepack = ch->_numfreepack ;
			#ifdef MUXTRACE
				TRACE( "\n%5d %6d %.3f", ch->channelID(), ch->_numfreepack, ch->_speed ) ;
				tot_speed += ch->_speed ;
			#endif
		}
		#ifdef MUXTRACE
			TRACE( "\n Totally --> %.3f\n", tot_speed ) ;
		#endif

		_distributeRecordRequests() ;

		for( i = 0; i < numOfChannels; i++ )
		{
			MuxChannel *ch = _channels[i];
			ch->_assigned_numfreepack = ch->_numfreepack ;
		}
		_output->onDistributeRecordRequests( ) ;
	}
	catch( ... ) {}

	LeaveCriticalSection( &MuxChannel::_distrLock);
}

void Mux::_distributeRecordRequests()
{
	int numOfChannels = _channels.count();
	int i ;

	// If there was a change in output rate
	if( _changeOfOutputRateRequested )
	{
		// First we clear packet counters so that no new packets will be generated by the channels.
		// However there might be some packets on the way. (max. 1 per channel)
		// They will be written after this function exits.
		for ( i = 0; i<numOfChannels; i++ )
		{
			MuxChannel *ch		= _channels[i];
			ch->clearNumFreePackets();
		}

		// We calculate the number of packets
		_reqNumOutRecs = numRecsDuringCheckTime( _outputRate );
		memset( achievedRates, 0, sizeof(achievedRates) ) ;
		maxAchievedRate = __min( maxAchievedRate, _reqNumOutRecs) ;

		// Next we flush the output.
		// However, the danger remains that the # of packets exceeds _reqNumOutRecs.
		// This can happen only when # of channels is comparable to _reqNumOutRecs. (small rates)
		_output->flush() ;

		// We simulate the situation as if in previous steps packets were supplied
		// acc. to the requested output rate.
		_actNumOutRecs			= _reqNumOutRecs;
		_numOfDistributedRecs	= 0;
		_roundCounter			= 4;
		_output->defineBuffer( _reqNumOutRecs );
		_output->setMaxFlushedData(_outputRate, _checkTime) ;

		for ( i = 0; i<5; i++ )
			_numOfProcessedRecs[i] = 1.;
		_changeOfOutputRateRequested = FALSE ;
	}

	// compute:
	//	minReqRecs = sum of min. output rates (expressed in packets) over all working channels
	//	numFreeRecsFromPrevRound = total number of free packets in all channels + #of packets in output buffer
	int	  numOfMinInternetRecs   	= 0;
	int   numFreeRecsFromPrevRound	= 0;
	int	  minReqRecs				= 0;	// needed to achieve min. channel capacity
	int	  absPriorityAll			= 0;
	double relPriorityAll			= 0;
	int   absPriorityOne[10];
	memset( absPriorityOne, 0, sizeof(absPriorityOne) );

	MuxChannel *IPchannel=NULL ;		// internet channel

	for ( i = 0; i<numOfChannels; i++ )
	{
		MuxChannel *ch		= _channels[i] ;
		int			freep	= ch->numFreePackets();
		if ( ch->isSending() )
		{
			int recsReq = numRecsDuringCheckTime( ch->minRate() ) ;

			// leave only minimal Internet capacity
			if ( ch->isInternetChannel() )
			{
				IPchannel = ch ;
				numOfMinInternetRecs = recsReq;
				// leave some packets for other channels, too
				numOfMinInternetRecs = __min( numOfMinInternetRecs, _reqNumOutRecs-10 ) ;
				numOfMinInternetRecs = __min( numOfMinInternetRecs, (int)(_reqNumOutRecs*0.95) ) ;
				numOfMinInternetRecs = __max( numOfMinInternetRecs, 0 ) ;
				if( freep > numOfMinInternetRecs )
				{
					ch->incNumFreePackets( numOfMinInternetRecs-freep ) ;
					freep = numOfMinInternetRecs ;
				}
			}

			// compute leftover from previous round and min. requested capacity
			numFreeRecsFromPrevRound += freep;
			recsReq -= freep;
			if( recsReq > 0 )
				minReqRecs += recsReq;

			// make preparations for distribution acc. to priorities
			int prior = 11 - ch->absPriority();
			if ( prior < 11 )
			{
				absPriorityAll += prior;
				absPriorityOne[prior-1]++;
			}
			int rprior = ch->relPriority();
			if ( rprior <= 0 )
				rprior = 10 ;
			relPriorityAll += 1./rprior;
		}
		else
		if ( freep>0 )
		{
			if ( ch->hasDataToSend() )
				numFreeRecsFromPrevRound += freep;
			else
				ch->clearNumFreePackets();
		}
	}

	int _numOfDistritibutedPackets = numFreeRecsFromPrevRound ;
	int nWaiting = _output->waiting() ;
	numFreeRecsFromPrevRound  += nWaiting;

	// check if some packet was sent since the previous redistribution
	if ( !packetsSent  &&  nWaiting>0 )
	{
		timeNotProcessedRecs++;
		if ( timeNotProcessedRecs > 2 )
		{
			if ( !msgPosted )
			{
				char buf[512];
				sprintf( buf, "Data in buffer, but no data leaves. Possible hardware error." );
				MfxPostMessage( EMsg_CommunicationError, 0, buf );
				msgPosted = TRUE;
			}
			BEEP_ERROR;
		}
	}
	else
	{
		timeNotProcessedRecs = 0;
		packetsSent = FALSE;
		msgPosted = FALSE;
	}

	//-----------------------------------------------------------------------------
	// Compute how many packets should be distributed among the channels.
	// Target is _reqNumOutRecs as specified by the current output rate.
	// However, the real number is adjusted taking into account the achieved 
	// results in the previoius 5 steps (rounds).

	_roundCounter = (_roundCounter + 1) % 5;
	int achievedRate ;
	double coef;
	if ( _numOfDistributedRecs == 0 )
	{
		coef = 0.9;
		achievedRate = 0 ;
		_numOfProcessedRecs[_roundCounter]	 = 1.;
	}
	else
	{
		coef = 1.0 - ((double)numOfMinInternetRecs / (double)_numOfDistributedRecs);
		coef = __min( 0.9, coef );
		achievedRate = _numOfDistributedRecs - numFreeRecsFromPrevRound ;
		if( IPchannel )							// simulate that Internet used full capacity
			achievedRate += IPchannel->numFreePackets() ;
		_numOfProcessedRecs[_roundCounter]	 = achievedRate / (double)_numOfDistributedRecs;
	}
	achievedRates[achievedRatesCounter] = achievedRate ;
	achievedRatesCounter = (achievedRatesCounter+1)%5 ;
	int meanAchievedRate = (achievedRates[0] + achievedRates[1] + achievedRates[2] + achievedRates[3] + achievedRates[4]) / 5 ;
	maxAchievedRate = __max( maxAchievedRate, meanAchievedRate ) ;

	double averageProcessed = 0.;
	for ( i = 0; i<5; i++ )
		averageProcessed += _numOfProcessedRecs[i];
	averageProcessed /= 5.;

	if ( averageProcessed < coef )
	{
		int newNum = (int)(_actNumOutRecs * averageProcessed / 0.9);
		int oldNum = (int)(_actNumOutRecs * _numOfProcessedRecs[_roundCounter]);
		_actNumOutRecs = __max( newNum, (int)(_actNumOutRecs * 0.85) ) ;
		_actNumOutRecs = __max( _actNumOutRecs, 10 ) ;
		_actNumOutRecs = __max( _actNumOutRecs, oldNum ) ;
	}
	else
	if ( averageProcessed >= 0.95 )
	{
		_actNumOutRecs = (int)((_actNumOutRecs + 2) * 1.05f);
		if( maxAchievedRate > _actNumOutRecs )
			_actNumOutRecs = (maxAchievedRate + _actNumOutRecs) / 2 ;
	}

	// The capacity must cover min. requirements (this includes Internet)
	// but must not exceed available capacity.
	_actNumOutRecs = __max( _actNumOutRecs, minReqRecs+numFreeRecsFromPrevRound ) ;
	_actNumOutRecs = __min( _actNumOutRecs, _reqNumOutRecs ) ;
	//-----------------------------------------------------------------------------

	// Subtract free packets from previous round.
	int recsToDistr	= _actNumOutRecs - numFreeRecsFromPrevRound;

	if ( ( recsToDistr <= 0 ) || ( minReqRecs + absPriorityAll + relPriorityAll == 0 ) )
	{
		#ifdef MUXTRACE
		TRACE( "\n>>>QUIT recsToDistr=%d minReqRecs=%d absPriorityAll=%d relPriorityAll=%.3f",
			recsToDistr, minReqRecs, absPriorityAll, relPriorityAll ) ;
		#endif

		_numOfDistributedRecs = 0;
		_numOfDistritibutedPackets = _actNumOutRecs ;
		setNumfreepack( _actNumOutRecs, numOfMinInternetRecs,
			IPchannel ? IPchannel->numFreePackets() : numOfMinInternetRecs );
		return;
	}

	// 1. distribute records acc. to min. guarrantied bandwidths
	// Test if min. requested recs is greater than recs to distribute.
	// If not, distribute packets acc. to priorities only.
	if ( minReqRecs > 0  &&  minReqRecs <= recsToDistr )
	{
		for ( i = 0; i<numOfChannels; i++ )
		{
			MuxChannel *ch = _channels[i] ;
			if ( ch->isSending() )
			{
				int recsToAdd = numRecsDuringCheckTime(ch->minRate()) - ch->numFreePackets();
				if ( recsToAdd > 0 )
				{
					ch->incNumFreePackets( recsToAdd );
					recsToDistr				   -= recsToAdd;
					_numOfDistritibutedPackets += recsToAdd;
					#ifdef MUXTRACE
						TRACE( "\nChannel %d got %d global", ch->channelID(), recsToAdd );
					#endif
				}
			}
		}
	}

	// 2. distribute records acc. to abs. priorities
	int recsToDistrAbs	  = (int)( ((float)(_setup.absPriorityPart) / 100.f) * recsToDistr );
	if ( ( recsToDistrAbs > 0 ) && ( absPriorityAll > 0 ) )
	{
		int priority = 10;
		while ( priority > 0 && recsToDistrAbs > 0 )
		{
			if ( absPriorityOne[priority-1] > 0 )
			{
				int recsPerChannel = recsToDistrAbs / absPriorityOne[priority-1];
				if( recsPerChannel > 0 )
				for ( i = 0; i<numOfChannels; i++ )
				{
					MuxChannel *ch = _channels[i] ;
					if ( ch->isSending() && (11 - ch->absPriority()) == priority )
					{
						int maxReqRecs	= numRecsDuringCheckTime(ch->maxRate()) - ch->numFreePackets();
						int add			= __min( recsPerChannel, maxReqRecs ) ;
						ch->incNumFreePackets( add );
						recsToDistr				   -= add;
						recsToDistrAbs			   -= add;
						_numOfDistritibutedPackets += add;
						#ifdef MUXTRACE
							TRACE( "\nChannel %d got %d abs", ch->channelID(), add );
						#endif
					}
				}
			}
			priority--;
		}
	}

	// 3. distribute records acc. to rel. priorities
	//	1. pass = all channels participate
	//	2. pass = capacity not used due to maxRate is distributed again
	//	3. pass = (max-min) Internet capacity is distributed among non-Internet channels
	//	4. pass = leftover from pass 3 redistributed

	// Exclude first channels which used up the whole capacity already
	for( i = 0; i<numOfChannels; i++ )
	{
		MuxChannel *ch = _channels[i] ;
		if( !ch->isSending() )
			continue ;
		
		int maxReqRecs = numRecsDuringCheckTime(ch->maxRate()) - ch->numFreePackets();
		if( maxReqRecs <= 0 )
		{
			int rprior = ch->relPriority();
			if( rprior <= 0 )
				rprior = 10 ;
			double rpriorFactor = 1. / rprior ;
			relPriorityAll -= rpriorFactor ;
		}
	}

	for( int pass=1 ; pass < 5 ; ++pass )
	{
		if( pass == 3 )
		{
			if( IPchannel == NULL )
				break ;
			recsToDistr += IPchannel->numFreePackets() - numOfMinInternetRecs ;
		}
		if ( recsToDistr <= 0   ||   relPriorityAll <= 0.001 )
			continue ;
	
		double recsPerRelPriority = recsToDistr / relPriorityAll;

		for ( i = 0; i<numOfChannels; i++ )
		{
			MuxChannel *ch = _channels[i] ;
			if( pass >= 3  &&  ch == IPchannel )
				continue ;
			if( !ch->isSending() )
				continue ;
			
			int maxReqRecs = numRecsDuringCheckTime(ch->maxRate()) - ch->numFreePackets();
			if( maxReqRecs > 0 )
			{
				int rprior = ch->relPriority();
				if( rprior <= 0 )
					rprior = 10 ;
				double rpriorFactor = 1. / rprior ;
				int recsToAdd = (int)(rpriorFactor * recsPerRelPriority);
				if( recsToAdd >= maxReqRecs )	// channel capacity used up
				{
					recsToAdd = maxReqRecs ;
					relPriorityAll -= rpriorFactor ;
				}
				ch->incNumFreePackets( recsToAdd );
				recsToDistr				   -= recsToAdd;
				_numOfDistritibutedPackets += recsToAdd;
				#ifdef MUXTRACE
					TRACE( "\nChannel %d got %d rel", ch->channelID(), recsToAdd );
				#endif
			}
		}
	}

	int	  numOfDistrInternetPackets	= IPchannel ? IPchannel->numFreePackets() : numOfMinInternetRecs ;

	_numOfDistributedRecs = _actNumOutRecs - recsToDistr;
	#ifdef MUXTRACE
		int np = _numOfDistritibutedPackets+nWaiting ;
		TRACE( "\n>>>Distributed(%d) + waiting for flush(%d)= %d= %.1f%% capacity(%d)",
			_numOfDistritibutedPackets, nWaiting, np,
			(double)np/_reqNumOutRecs*100., _reqNumOutRecs );
	#endif
	int maxAvailablePackets = _reqNumOutRecs - nWaiting ;
	if( _numOfDistritibutedPackets > maxAvailablePackets )
	{
		_numOfDistritibutedPackets = maxAvailablePackets ;
		#ifdef MUXTRACE
			TRACE( " ---> decreased to %d", maxAvailablePackets ) ;
		#endif
	}
	#ifdef MUXTRACE
		TRACE( "\n>>>Internet:%d..%d", numOfMinInternetRecs, numOfDistrInternetPackets );
	#endif

	// set parameters for packet pool
	setNumfreepack( _numOfDistritibutedPackets, numOfMinInternetRecs, numOfDistrInternetPackets ) ;
}


//-------------------------------------------------------------------------------
//	MuxOutputTimer
//-------------------------------------------------------------------------------


// send error to the error log
inline void outputErrorOccured( int err )
{
	static int    lastError=0 ;
	static time_t lastBeep=0 ;
	if( err == 0 )
	{
		lastError = 0 ;
		return ;
	}
	if( lastError == 0  ||  lastError != err )
	{
		char buf[512];
		const char *msg = DvbEventText( err, buf );
		MfxPostMessage( EMsg_CommunicationError, 0, msg );
	}
	time_t timeNow = time(NULL) ;
	if( timeNow-lastBeep > 3 )
	{
		BEEP_WARNING ;
		lastBeep = timeNow ;
	}
	lastError = err ;
}

// this timer regularly calls MuxOutput::flush() in MUXOUTPUT_FLUSHINTERVAL intervals
class MuxOutputTimer : public CWinThread
{
	MuxOutput		*_muxOutput ;
	HANDLE			 _eventKill;		// SetEvent() to kill this process

  public:
	inline void shutdown( )
	{
		VERIFY( SetEvent(_eventKill) );
		SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
		if ( WaitForSingleObject( m_hThread, 500 ) == WAIT_TIMEOUT )
			TerminateThread( m_hThread, 0 );
	}
	inline MuxOutputTimer( MuxOutput *muxOutput )
	{
		m_bAutoDelete = FALSE;
		_muxOutput = muxOutput ;
		_eventKill = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	inline ~MuxOutputTimer()
	{
		CloseHandle( _eventKill);
	}

  protected:
	virtual BOOL InitInstance();
};

// wraps around in 50 days ... therefore unsigned
static DWORD get_time()	// [msecs]
{
	static BOOL first_time=TRUE ;

	static DWORD tick0=0 ;

	static LARGE_INTEGER HR_freq ;		// HR = HighResolution
	static BOOL			 HRTimerAvailable ;
	static LARGE_INTEGER HRtick0 ;
	static BOOL			 HRtick0_defined=FALSE ;

	if( first_time )
	{
		HRTimerAvailable = QueryPerformanceFrequency( &HR_freq) ;
		first_time = FALSE ;
		tick0 = GetTickCount() ;
	}

	DWORD msecs ;
	if( HRTimerAvailable )
	{
		LARGE_INTEGER cnt ;
		if( QueryPerformanceCounter( &cnt) )
		{
			if( !HRtick0_defined )
			{
				HRtick0_defined = TRUE ;
				HRtick0 = cnt ;
			}
			LONGLONG d = (cnt.QuadPart - HRtick0.QuadPart) * 1000 / HR_freq.QuadPart ;
			msecs = ((LARGE_INTEGER*)&d)->LowPart ;
			return msecs ;
		}
	}

	msecs = GetTickCount() - tick0 ;
	return msecs ;
}

BOOL MuxOutputTimer::InitInstance()
{
	// Loop but check for kill notification.
	// In case no data pass through the output, time interval between
	// successive writes is increased.

	UINT  numOfSuccessiveProblems = 0 ;

	// Estimate suitable flush step
	DWORD dt0=MUXOUTPUT_FLUSHINTERVAL ;
	while( dt0 < 500 )
	{
		int nBytes = _muxOutput->_byteRate * dt0/1000 ;
		if( nBytes > 5*TSPACKET_SIZE )
			break ;
		dt0 *= 2 ;
	}

	DWORD t0 = get_time() ;

	//
	// There are 2 problems we have to handle here:
	// 1. We must assure good responsivness of IP.
	//	  Therefore we send only small amount of data in each loop.
	//	  (File channels are able to produce data very fast - usuallly much faster than
	//	  the output speed. If we send all this data at once, IP data which are being
	//	  produced more uniform, would have to wait.)
	// 2. Different device behavior.
	//	  Some devices wait until data are sent, some not.
	//	  Therefore we measure how long flush() really takes and wait the rest of the time
	//	  the loop should take.
	//	  There is an additional problem in this respect - resolution with which time
	//	  can be measured. (See the discussion in MfxGlobals.cpp.)
	//
	while( 1 )
	{
		try
		{
			int waiting = _muxOutput->waiting() ;
			int err ;
			DWORD flushTime ;
			int written ;

			if( waiting > 0 )				// if some data to send
			{
				int nBytes = _muxOutput->_byteRate * dt0/1000 ;

				flushTime = get_time() ;
				err = _muxOutput->flush( 2*nBytes, &written ) ;
				flushTime = get_time() - flushTime;

				if( written == 0 )
					numOfSuccessiveProblems++ ;		// nothing sent
				else
					numOfSuccessiveProblems = 0 ;
			}
			else
			{
				err = 0 ;
				numOfSuccessiveProblems = 0 ;
				flushTime = 0 ;
			}

			DWORD extra_dt ;
			if( numOfSuccessiveProblems <= 10 )
				extra_dt = 0 ;
			else
			{
				if( numOfSuccessiveProblems > 100 )
					extra_dt = 10 * dt0 ;
				else
					extra_dt = 5 * dt0 ;
				extra_dt = __min( extra_dt, 500 ) ;
			}

			outputErrorOccured( err ) ;

/*			DWORD t1 = get_time() ;
			DWORD output_dt = t1 - t0 ;
			t0 = t1 ;
			DWORD dt = (output_dt < dt0) ? dt0-output_dt : 0 ;
*/
			flushTime += 2 ;
			DWORD dt = (flushTime < dt0) ? dt0-flushTime : 0 ;

//			TRACE("Flush: %4d (%6d b) => Sleep: %4d\n", flushTime, written, dt );

			if( WaitForSingleObject(_eventKill, dt+extra_dt) != WAIT_TIMEOUT )
				break ;
		}
		catch ( ... )
		{
		}
	}
	return FALSE;		// avoid entering standard message loop by returning FALSE
}

//-------------------------------------------------------------------------------
//	Transport stream analyzer.
//	Displays summary of the transmitted streams into the output windows
//	at the end of the session
//-------------------------------------------------------------------------------

#ifdef ANALYZE_OUTPUT_STREAM

	struct TsAnalyzerStream
	{
		ushort	pid ;
		uchar	lastContCounter ;
		ulong	nPackets ;
		ulong	nContErrors ;

		inline void update( TsPacket *tp )
		{
			uchar newCont = tp->flags & 0x0F ;
			if ( ((lastContCounter+1)&0x0F) != newCont )
				nContErrors++ ;
			lastContCounter = newCont ;
			nPackets++ ;
		}
	} ;

	class TsAnalyzer
	{
		sTemplateArray<TsAnalyzerStream>	streams ;

		inline void analyzePacket	( TsPacket *tp )
		{
			int		iStream		= streams.count() ;
			ushort	packetPid	= tp->pid & 0xFF1F ;
			
			while (iStream--)
			{
				TsAnalyzerStream *stream = &streams[iStream] ;
				if ( stream->pid == packetPid )
				{
					stream->update(tp) ;
					return ;
				}
			}

			TsAnalyzerStream stream ;
			stream.lastContCounter = tp->flags&0x0F ;
			stream.pid = packetPid ;
			stream.nPackets = 1 ;
			stream.nContErrors = 0 ;
			streams.add(stream) ;
		}

	public:
		void analyzePackets( const void *buf, int nPackets )
		{
			TsPacket *analyzedTP = (TsPacket*)buf ;
			while ( nPackets-- )
			{
				analyzePacket(analyzedTP) ;
				analyzedTP++ ;
			}
		}

		~TsAnalyzer()
		{
			TRACE("\nOutput streams statistics\n"
				  " PID          Packets    CErrors\n"
				  "---------------------------------\n") ;
			for ( int i = 0; i < streams.count(); i++ )
			{
				TsAnalyzerStream *stream = &streams[i] ;
				ushort pid = ((stream->pid) >> 8) | ((stream->pid&0x1F)<<8);
				TRACE("%4d(%4x) %10d %10d\n", pid, pid, stream->nPackets, stream->nContErrors) ;
			}
			TRACE("---------------------------------\n") ;
		}
	} ;

	TsAnalyzer s_analyzer ;

	#define analyzePackets( buf, nPackets )		s_analyzer.analyzePackets(buf, nPackets) ;
#else
	#define analyzePackets( buf, nPackets )	
#endif


//-------------------------------------------------------------------------------
//	MuxOutputQueue
//	Helper class for MuxOutput.
//-------------------------------------------------------------------------------


//
// (Removed:)
// Because of the problems in card synchronisation, it is assured, that alive packet
// is never the last packet in the buffer.
// Would this happen, an artificial fill packet (constructed so that synchronisation is easier)
// is appended after alive packet. Fill packet will be overwritten by first data packet.
//
//static BOOL		 lastPacketIsFillPacket;

#define OUTPUTPACKETSIZE	TSPACKET_SIZE

MuxOutputQueue::MuxOutputQueue( ComOut *c )
{
	_comOut			= c ;
	_buffer			= NULL;
	_numFlushedBytes= 0 ;
	_maxPackets		= 0 ;
	_maxBytes		= 0 ;
	_inPtr			= 0 ;
	_outPtr			= 0 ;
	_outPtrByteOff	= 0 ;
	_sectionEnd		= TSPAYLOAD_SIZE ;
	//_inLockPtr		= 0 ;
	//_outLockPtr		= 0 ;
	InitializeCriticalSection( &_bufferLock );
	InitializeCriticalSection( &_flushLock );
}

MuxOutputQueue::~MuxOutputQueue()
{
	if ( _buffer )
		FREE( _buffer );
	DeleteCriticalSection( &_bufferLock );
	DeleteCriticalSection( &_flushLock );
}

/*
The code bellow contains (not tested) code for queue locking. These functions could
eventually be used to synchronize put and flush functions.

However, as far it is guarantied that buffer overflow cannot happen, no locking is needed.
Multiplexor assures that for packets sent via MuxChannel packet counters.
However, we must check also other packets - e.g. alive signal and CA table (or fill packets).

Class members:
	int		_inLockPtr ;				// currently flushed region (first packet)
	int		_outLockPtr ;				// packet following the last flushed packet

inline void MuxOutputQueue::lock( int p1, int nPackets )
{
	_inLockPtr = p1 ;
	_outLockPtr= (p1 + nPackets) % _maxPackets ;
}
inline void MuxOutputQueue::unlock()
{
	_inLockPtr = _outLockPtr = 0 ;
}
inline BOOL MuxOutputQueue::isLocked( int p1 )
{
	if( _inLockPtr == _outLockPtr )
		return FALSE ;
	if( _outLockPtr < _inLockPtr )
		return p1 >= _outLockPtr  &&  p1 < _inLockPtr ;
	else
		return p1 >= _outLockPtr  ||  p1 < _inLockPtr ;
}

inline BOOL MuxOutputQueue::isLocked( int p1, int nPackets )
{
	if( _inLockPtr == _outLockPtr )
		return FALSE ;

	int p2 = (p1 + nPackets-1) % _maxPackets ;
	if( _outLockPtr < _inLockPtr )
	{
		if( p1 <= p2 )
			return  p2 >= _outLockPtr  &&  p1 < _inLockPtr ;
		else
			return  p2 >= _outLockPtr  ||  p1 < _inLockPtr ;
	}
	else
	{
		if( p1 <= p2 )
			return  p1 < _inLockPtr  ||  p2 >= _outLockPtr ;
		else
			return  TRUE ;		// common point: (_maxPackets-1)
	}
}
*/

void MuxOutputQueue::defineBuffer( int maxNumOfPackets )
{
	int written ;
	if( _buffer )
		flush( written );

	EnterCriticalSection( &_bufferLock ) ;
	EnterCriticalSection( &_flushLock ) ;

	if( maxNumOfPackets )
	{
		if( _inPtr == _outPtr )
			_inPtr = _outPtr = 0 ;
		else
		{
			/*
			int nWaiting = waiting() ;
			if( nWaiting >= maxNumOfPackets )
			{
				// Too many packets left; discard newest packets
				int nDiscarded = nWaiting - maxNumOfPackets + 1 ;
				_inPtr = (_inPtr + _maxPackets - nDiscarded) % _maxPackets ;
			}
			#define LEN(n)	 ((n)*TSPACKET_SIZE)
			#define OFF(ind) (_buffer+LEN(ind))
			if( _inPtr > _outPtr )
			{
				int nMoved = _inPtr - _outPtr ;
				memmove( _buffer, OFF(_outPtr), LEN(nMoved) ) ;
				_inPtr -= _outPtr ;
				_outPtr = 0 ;
			}
			else
			{
				int nMoved = _maxPackets - _outPtr ;
				memmove( OFF(maxNumOfPackets-nMoved), OFF(_outPtr), LEN(nMoved) ) ;
				_outPtr = maxNumOfPackets-nMoved ;
			}
			*/
			// Above code demonstrates how the buffer can be reduced while preserving
			// waiting packets. However, this situation is dangerous as following
			// put() calls could cause buffer overflow. Therefore we rather clear
			// the whole buffer.
			if( maxNumOfPackets < _maxPackets )
				_inPtr = _outPtr = 0 ;
			else
			{
				// Temporary
				_inPtr = _outPtr = 0 ;
			}
		}

		_maxPackets = maxNumOfPackets;
		_maxBytes   = _maxPackets*OUTPUTPACKETSIZE ;
		_buffer		= (char *)REALLOC( _buffer, _maxBytes ) ;
	}
	else
	{
		FREE( _buffer );
		_buffer = NULL;
		_inPtr = _outPtr = 0 ;
	}

	LeaveCriticalSection( &_flushLock ) ;
	LeaveCriticalSection( &_bufferLock ) ;
}

void MuxOutputQueue::freeBuffer( )
{
	EnterCriticalSection( &_flushLock ) ;
	EnterCriticalSection( &_bufferLock ) ;
	if( _buffer )
	{
		FREE( _buffer );
		_buffer		= NULL;
		_maxPackets = 0 ;
		_maxBytes	= 0 ;
		_inPtr = _outPtr = 0 ;
	}
	LeaveCriticalSection( &_flushLock ) ;
	LeaveCriticalSection( &_bufferLock ) ;
}

#pragma optimize( OPTOPTIONS, on )

void MuxOutputQueue::put( MuxPacket *p, PidStreamAttrib *pidStrAttr )
{
	EnterCriticalSection(&_bufferLock) ;
	if( _buffer != NULL )		// can happen in sendAliveSignal()
	{
		TsPacket *tp =(TsPacket*)(_buffer+_inPtr*TSPACKET_SIZE ) ;
		tp->create( (uchar*)p, pidStrAttr ) ;
		_inPtr = (_inPtr+1) % _maxPackets ;
		#pragma message( "ASSERT( _inPtr != _outPtr ) sa obcas objavi" )
		ASSERT( _inPtr != _outPtr );

		_sectionEnd = TSPAYLOAD_SIZE ;	// disallow other section to occupy rest of TS packet
	}
	LeaveCriticalSection(&_bufferLock) ;
}

void MuxOutputQueue::put( DSMCC_section *s, int n_tsPackets, PidStreamAttrib *pidStrAttr )
{
	EnterCriticalSection(&_bufferLock) ;

	if( _buffer != NULL )
	{
		ASSERT( waiting() < _maxPackets-n_tsPackets );

		int sectionLength = s->getTotalLength() ;
		int sectionOff    = 0 ;
		uchar *sectionPtr = (uchar*)s ;


		int inPtr = _inPtr ;

		#define MOVE_IN_PTR \
			tp =(TsPacket*)(_buffer+inPtr*TSPACKET_SIZE ) ; \
			inPtr = (inPtr+1) % _maxPackets ; \
			ASSERT( inPtr != _outPtr );

		// Analyze last TS packet in a buffer whether it can carry DSMCC section "s" with PID "pid"
		TsPacket *tp ;

		if (_useMultiSection && inPtr!=_outPtr)
		{
			tp =(TsPacket*)(_buffer+(inPtr>0?inPtr-1:_maxPackets-1)*TSPACKET_SIZE ) ;

			if ( _sectionEnd<TSPAYLOAD_SIZE-MPE_HEADERSIZE && tp->isCompatible(pidStrAttr) )
			{
				if (TryEnterCriticalSection(&_flushLock))
				{
					if (inPtr!=_outPtr)
						sectionOff = tp->addSection(sectionPtr, _sectionEnd, sectionLength) ;
					LeaveCriticalSection(&_flushLock) ;
				}
			}

			if (sectionOff==0)
			{
				MOVE_IN_PTR ;
				_sectionEnd = sectionOff = tp->createSection(sectionPtr, sectionLength, pidStrAttr) ;
			}
		}
		else
		{
			MOVE_IN_PTR ;
			_sectionEnd = sectionOff = tp->createSection(sectionPtr, sectionLength, pidStrAttr) ;
		}

		while ( sectionOff+TSPAYLOAD_SIZE <= sectionLength )
		{
			MOVE_IN_PTR ;
			tp->create( sectionPtr+sectionOff, pidStrAttr ) ;
			sectionOff += TSPAYLOAD_SIZE ;
			_sectionEnd = TSPAYLOAD_SIZE ;
		}

		int remSize = sectionLength-sectionOff ;
		if ( remSize )
		{
			MOVE_IN_PTR ;
			tp->create( sectionPtr+sectionOff, pidStrAttr, remSize ) ;
			memset(((uchar*)&tp->data)+remSize, 0xFF, TSPAYLOAD_SIZE-remSize) ;
			_sectionEnd = remSize ;
		}

		_inPtr = inPtr ;
	}

	LeaveCriticalSection(&_bufferLock) ;
}

#ifdef MIN_FLUSHED_INTERNET_PACKETS
void MuxOutputQueue::sendFillPackets( int nPackets )
{
	PidStreamAttrib *servPid = MfxServerSetup()->serviceChannelPid() ;

	char fillBuf[MIN_FLUSHED_INTERNET_PACKETS][TSPACKET_SIZE] ;
	int i = nPackets ;
	while( i-- )
	{
		TsPacket *ts = (TsPacket*)(fillBuf[i]) ;
		ts->create((uchar*)&fillPacket, servPid ) ;
	}
	#pragma message( "Fill pakety obchadzaju distribucny algoritmus ... mozny buffer overflow")
	_comOut->write( fillBuf[0], nPackets*TSPACKET_SIZE, &i ) ;
	analyzePackets( fillBuf[0], i/TSPACKET_SIZE ) ;
	//TRACE("%d packet buffer filled up to %d\n", nPackets, nPackets+nPackets ) ;
}
#endif

int MuxOutputQueue::flush( int &written, int maxBytes )
{
	//if( _comOut != NULL )
	//{
	//	written = 0 ;
	//	return 0 ;
	//}

	// Block the whole flush() because it can be called from different threads!
	EnterCriticalSection( &_flushLock );
	int err	= 0;
	written = 0;

	if( _comOut != NULL )
	{
		int toWrite = waiting() * OUTPUTPACKETSIZE ;
		if( maxBytes )
			toWrite = __min( maxBytes, toWrite ) ;

		// Round to packet multiple
		toWrite -= (toWrite%OUTPUTPACKETSIZE) + _outPtrByteOff ;

		#ifdef DO_NOT_DIVIDE_MPE_SECTION
			if ( toWrite>0 && waiting()*OUTPUTPACKETSIZE>toWrite )
			{
				int lastFlushed =  (_outPtr + toWrite/OUTPUTPACKETSIZE) % _maxPackets ;
				while (toWrite > 0)
				{
					TsPacket *lastPacket = (TsPacket*)_buffer + lastFlushed ;
					if (lastPacket->isPayloadUnitStart())
						break ;
					toWrite -= OUTPUTPACKETSIZE ;
					lastFlushed = (lastFlushed>0)?lastFlushed-1:_maxPackets-1 ;
				}
			}
							
		#endif

		if( toWrite > 0 )
		{
			#define ANALYZE( ind, n_bytes)	\
				analyzePackets( _buffer+ind-_outPtrByteOff, (n_bytes-_outPtrByteOff)/TSPACKET_SIZE )

			//lock( _outPtr, (toWrite+_outPtrByteOff)/OUTPUTPACKETSIZE ) ;

			int ind1 = _outPtr*OUTPUTPACKETSIZE + _outPtrByteOff ;
			if( ind1 + toWrite <= _maxBytes )
			{
				err = _comOut->write( _buffer+ind1, toWrite, &written );
				ANALYZE( ind1, written ) ;
			}
			else
			{
				int len1 = _maxBytes - ind1 ;
				err = _comOut->write( _buffer+ind1, len1, &written );
				ANALYZE( ind1, written ) ;
				if( err == 0  &&  written == len1 )
				{
					err = _comOut->write( _buffer, toWrite-len1, &written );
					ANALYZE( 0, written ) ;
					written += len1 ;
				}
			}
			ind1		   = (ind1 + written) % _maxBytes ;
			_outPtr		   = ind1 / OUTPUTPACKETSIZE ;
			_outPtrByteOff = ind1 % OUTPUTPACKETSIZE ;
			
			#ifdef MIN_FLUSHED_INTERNET_PACKETS
			int nPackets = written/TSPACKET_SIZE ;
			if( nPackets>1  &&  nPackets < MIN_FLUSHED_INTERNET_PACKETS  &&  _outPtrByteOff==0 )
			{
				//
				// Small amount of data - sending fill packets.
				//
				// This is because of some receiver cards which wait until certain amount
				// of packets is received prior to handing them over to the application.
				// This slows down actions such as sending ping packet.
				// In order to prevent this effect we arbitrarily generate fill packets.
				//
				TsPacket *lastPacket = (TsPacket*)_buffer + (_outPtr ? (_outPtr-1) : (_maxPackets-1)) ;
				if( lastPacket->sync == TsPacketSyncByte )
				{
					MuxPacket *lastMux = &lastPacket->data ;
					if( lastMux->isInternetPacket() )
						sendFillPackets( MIN_FLUSHED_INTERNET_PACKETS-nPackets ) ;
				}
			}
			#endif

			//unlock() ;
		}

		if ( written > 0 && !err )
		{
			packetsSent = TRUE;
			InterlockedExchangeAdd( (long*)&_numFlushedBytes, written ) ;
		}
	}
	LeaveCriticalSection( &_flushLock );
	return err;
}

void MuxOutputQueue::resetComOut( ComOut *c )
{
	EnterCriticalSection( &_flushLock );
	EnterCriticalSection( &_bufferLock );
	_comOut = c ;
	LeaveCriticalSection( &_flushLock );
	LeaveCriticalSection( &_bufferLock );
}

int MuxOutputQueue::countWaitingTestPackets()
{
	int cnt=0 ;
	EnterCriticalSection( &_flushLock );
	EnterCriticalSection( &_bufferLock );

	for( int j=waiting()-1 ; j >= 0 ; j-- )
	{
		TsPacket  *tsP  = (TsPacket *)_buffer + (_outPtr+j)%_maxPackets ;
		MuxPacket *muxP = &tsP->data ;
		if( muxP->isFillPacket() )
			cnt++ ;
	}
	LeaveCriticalSection( &_flushLock );
	LeaveCriticalSection( &_bufferLock );
	return cnt ;
}

#pragma optimize( "", on )			// restore original optimization options


//-------------------------------------------------------------------------------
//	MuxOutput
//	Output device for multiplexor.
//	On one side it is filled by the packets coming from multiplexor,
//	on the other side its contents is regularly flushed to ComOut.
//	The flush signal is coming from MuxOutputTimer.
//-------------------------------------------------------------------------------


MuxOutput::MuxOutput( ComOut *c, const MuxSetup *setup )
: _dataQueue(c), _inetQueue(c)
{
	_comOut		 = c ;
	_muxOutputTimer = NULL;
	_byteRate = 0;

	_hNumTestPacketAvailable = 0 ;
	_testCapacity	 = 0 ;
	_testPacketIndex = 0 ;
	_numTestLoops	 = 0 ;

	srand( (unsigned)time(NULL) );

	fillPacket.makeFillPacket();

	BOOL bUseMultiSection = MfxDvbSetup()->useMultiSection() ;
	_dataQueue.setMultiSectionMode(bUseMultiSection) ;
	_inetQueue.setMultiSectionMode(bUseMultiSection) ;
}

void MuxOutput::_delOutputTimer()
{
	if( _muxOutputTimer != NULL )
	{
		_muxOutputTimer->shutdown() ;
		delete _muxOutputTimer ;
		_muxOutputTimer = NULL ;
	}
}

MuxOutput::~MuxOutput()
{
	_delOutputTimer() ;
}

void MuxOutput::setMaxFlushedData( float rate, int distribInterval )	
{ 
	_byteRate = (int)(rate*1024.f*1024.f/8.f/distribInterval*1000.f) ;
}

void MuxOutput::resetComOut( ComOut *c )
{
	_comOut = c ;
	_dataQueue.resetComOut(c);
	_inetQueue.resetComOut(c);
}

int	MuxOutput::flush ( int nBytes, int *written )
{
	int inetWritten ;
	int dataWritten=0 ;
	int err ;
	
	if( nBytes <= 0 )
	{
		err = _inetQueue.flush(inetWritten);
		if( err == 0 )
			err = _dataQueue.flush(dataWritten);
	}
	else
	{
		err = _inetQueue.flush(inetWritten, nBytes);
		nBytes -= inetWritten ;

		if (nBytes>0)
		{
			err = _dataQueue.flush(dataWritten, nBytes);
		}

		#ifdef FLUSHTRACE
			static _lastFlushTime = 0 ;
			DWORD time = get_time();
			int diff = _lastFlushTime ?(time - _lastFlushTime) : MUXOUTPUT_FLUSHINTERVAL ;
			_lastFlushTime = time ;
			TRACE("\nData:%dBy, IP:%dBy, dt=%d Prev.speed=%.3f/%.3f",
				dataWritten, inetWritten, diff,
				dataWritten*8./1000000./diff*1000.,
				inetWritten*8./1000000./diff*1000.);
		#endif
	}

	if( written != NULL )
		*written = inetWritten + dataWritten ;
	return err ;
}

void MuxOutput::freeBuffer()
{
	BOOL ask = TRUE;
	_delOutputTimer() ;
	while ( waiting() )
	{
		if ( ask )
			if ( MessageBox( NULL, "Some packets are in the buffer.\nWait for sending them?",
					 		 "Warning", MB_YESNO | MB_ICONWARNING | MB_TOPMOST ) == IDNO )
				break;
		ask = FALSE;
		packetsSent = FALSE;
		int err = flush();
		if( err || !packetsSent )
		{
			MessageBox( NULL, "Could not send packets due to an error.", "Fatal error", MB_OK | MB_ICONSTOP | MB_TOPMOST );
			break;
		}
	}
	_dataQueue.freeBuffer() ;
	_inetQueue.freeBuffer() ;
}

// Define the buffer size so that OVF cannot  happen.
void MuxOutput::defineBuffer( int maxNumOfPackets )
{
	// Data queue must reserve extra packets which are ignored by the multiplexor.
	int dataSize = maxNumOfPackets ;
	if( maxNumOfPackets )
	{
		// + 1 alive packet (not sent by channel)
		// + 1 fill packet
		// + 1 reserve
		// + packets for CATable (not sent by channel)
		int CATablePackets = 1 + 8192/MUXPACKETSIZE ;
		dataSize += (3 + CATablePackets) ;
	
		// In the test output rate mode <_testCapacity> packets must always be reserved.
		dataSize += _testCapacity ;
	}

	if( _dataQueue.maxInBuffer() == dataSize  &&
		_inetQueue.maxInBuffer() == maxNumOfPackets )
		return ;

	_delOutputTimer() ;

	_inetQueue.defineBuffer( maxNumOfPackets ) ;
	_dataQueue.defineBuffer( dataSize ) ;

	if ( maxNumOfPackets )
	{
		_muxOutputTimer = new MuxOutputTimer( this);
		if( _muxOutputTimer == NULL)
			return;

		ASSERT_VALID(_muxOutputTimer);
		_muxOutputTimer->m_pThreadParams = NULL;

		// Create Thread in a suspended state so we can set the Priority before it gets away from us
		if (!_muxOutputTimer->CreateThread(CREATE_SUSPENDED))
		{
			delete _muxOutputTimer;
			_muxOutputTimer = NULL ;
			return;
		}
		VERIFY( _muxOutputTimer->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL) );
		_muxOutputTimer->ResumeThread();
	}
}


//------------------------

static int g_nPackets=0 ;

void MuxOutput::onDistributeRecordRequests( )
{
	if( _hNumTestPacketAvailable )								// If running test
	{
		long cnt = _testCapacity - _dataQueue.countWaitingTestPackets() ;	// #packets which were not output yet
		if( InterlockedExchange(&_testPacketIndex,cnt) <= 0 )	// If no free packets were available
			SetEvent( _hNumTestPacketAvailable );				// Set signal that there are some again.
		_numTestLoops++ ;
		//TRACE( "\nonDistributeRecordRequests(); _numTestLoops=%d", _numTestLoops ) ;
	}
}

BOOL MuxOutput::_testOutputRate(
		Mux *mux,				// multiplexor using this MuxOutput
		int testDuration,		// [secs]
		HANDLE hKillEvent,		// kill event or 0 (cannot be killed prematurally)
		float *estimSpeed		// estimated speed [Mb/s]
		)
{
	// Allocate extra pool for test packets
	int numStdRecs = mux->numRecsDuringCheckTime( );
	defineBuffer( numStdRecs ) ;

	// Prepare packet which will be sent
	MuxPacket muxPacket ;
	muxPacket.makeFillPacket() ;

	// 0 means no free packet available (they will be added on onDistributeRecordRequests())
	_testPacketIndex = 0 ;
	_numTestLoops	 = 0 ;

	// Create manual nonsignaled event
	_hNumTestPacketAvailable = CreateEvent( NULL, FALSE, FALSE, NULL );

	// Prepare array of events used in sleeping phase
	HANDLE handleArray[2] ;
	handleArray[0]	= hKillEvent;
	handleArray[1]	= _hNumTestPacketAvailable;

	// Prepare measurement
	ResetEvent( _hNumTestPacketAvailable ) ;
	if( WaitForMultipleObjects( 2, handleArray, FALSE, INFINITE) == WAIT_OBJECT_0 )
	{
		CloseHandle( _hNumTestPacketAvailable );
		_hNumTestPacketAvailable = 0 ;
		return FALSE ;
	}

	ULONG old_numFlushedBytes = _dataQueue.numFlushedBytes() ;
	clock_t startT = clock() ;
	BOOL    ret=TRUE ;
	PidStreamAttrib *servChannelPid = MfxServerSetup()->serviceChannelPid() ;

	while( _numTestLoops <= testDuration )
	{
		while( _testPacketIndex <= 0 )
		{
			// No capacity; start sleeping
			ResetEvent( _hNumTestPacketAvailable ) ;
			if( WaitForMultipleObjects( 2, handleArray, FALSE, INFINITE) == WAIT_OBJECT_0 )
			{
				ret = FALSE ;
				goto labelQuit ;			// kill
			}
		}
		put( &muxPacket, servChannelPid ) ;
		if( WaitForSingleObject( hKillEvent,0) != WAIT_TIMEOUT )
			break ;
		InterlockedDecrement( &_testPacketIndex ) ;
	}

  labelQuit:
	clock_t endT = clock() ;
	ULONG numFlushedBytes = _dataQueue.numFlushedBytes() ;
	ULONG n_bytes = (numFlushedBytes > old_numFlushedBytes) ?
					(numFlushedBytes - old_numFlushedBytes) :
					(ULONG_MAX - (old_numFlushedBytes - numFlushedBytes)) ;

	int n_packets = n_bytes / OUTPUTPACKETSIZE ;
	g_nPackets += n_packets ;

	// Critical section makes sure that onDistributeRecordRequests() is not called here.
	EnterCriticalSection( &MuxChannel::_distrLock );
	_testCapacity = 0 ;
	CloseHandle( _hNumTestPacketAvailable );
	_hNumTestPacketAvailable = 0 ;
	LeaveCriticalSection( &MuxChannel::_distrLock);

	float duration = (float)( (double)(endT - startT) / CLOCKS_PER_SEC ) ;
	*estimSpeed = ((float)n_bytes/duration) * 8 / 1000000 ;
	TRACE( "\nDuration: %.2f sec, %d packets, %d By, speed %.2f Mb/s\n",
		duration, n_packets, n_bytes, *estimSpeed ) ;
	return ret ;
}


BOOL MuxOutput::_testOutputRate(
		Mux    *mux,			// multiplexor using this MuxOutput
		HANDLE  hKillEvent,		// kill event or 0 (cannot be killed prematurally)
		HWND    hWnd,			// handle of the window receiving messages
		int		msg				// message, e.g. WM_APP+1
	)
{
	g_nPackets = 0 ;

	int numStdRecs = mux->numRecsDuringCheckTime( );

	float max_speed ;
	float max_coef=1 ;
	int	  n_loops=5 ;
	char  msgBuf[80] ;
	int   n_pass=0 ;

	SendMessage( hWnd, msg, StartTest, 0 ) ;

	// First pass will be ignored (filling the card cache)
	_testCapacity = 2 * numStdRecs ;
	if( !_testOutputRate(mux,n_loops,hKillEvent,&max_speed) )
		goto labelInterrupted ;

	max_speed = 0 ;
	SendMessage(hWnd,msg,Progress,(LPARAM)"Starting 1st test...");

	#define TEST( coef, speed) {							\
		n_pass++ ;											\
		_testCapacity = (int)(coef * numStdRecs) ;			\
		if( !_testOutputRate(mux,n_loops,hKillEvent,&speed))\
			goto labelInterrupted ;							\
		TRACE( "\ncoef=%.2f ... %.2f Mb/s", coef, speed );	\
		if( max_speed < speed )								\
		{													\
			max_speed = speed ;								\
			max_coef  = coef ;									\
			SendMessage(hWnd,msg,NewSolution,(LPARAM)&max_speed);\
		}														\
		sprintf(msgBuf, "(%d) try %.2f ... got %.2f Mb/s",		\
			n_pass, mux->outputRate()*coef, speed ) ;			\
		SendMessage(hWnd,msg,Progress,(LPARAM)msgBuf);			\
	}

	/*
	float speed1, speed2, coef, coef1, coef2 ;
	coef1 = 1.f ;
	TEST( coef1, speed1 ) ;

	coef2 = 1.4f ;
	TEST( coef2, speed2 ) ;

	// 1. Try increasing # of free packets as long as the output rate increases, too.
	while( speed2 > 1.1 * speed1 )
	{
		coef1  = coef2 ;
		speed1 = speed2 ;
		coef2  = 1.4*coef1 ;
		TEST( coef2, speed2 ) ;
	}

	coef1 = max_coef / 2.f ;
	coef2 = max_coef * 1.5f ;

	// 2. Try values around best value achieved so far.
	{
		float dc = (coef2-coef1) / 10 ;

		int cnt_failed=0 ;
		for( int j=0 ; j < 10 ; ++j )
		{
			float c = coef2 - j*dc ;
			float s ;
			float saved_max_speed = max_speed ;

			TEST( c, s ) ;
			TRACE( "\nspeed ... %.2f Mb/s", s ) ;
			if( saved_max_speed > 1.1*s )
			{
				cnt_failed++ ;
				if( cnt_failed > 3 )
					break ;
			}
			else
				cnt_failed = 0 ;
		}
	}

	// 3. Last trial - longer tests from best value upwards
	n_loops = 2 ;
	for( coef = max_coef ; ; coef *= 1.05f )
	{
		float old_max_speed = max_speed ;
		float speed ;
		TEST( coef, speed ) ;
		if( old_max_speed >= max_speed )
			break ;
	}
	*/

	{
		float coefs[] = {
			0.1f, 0.25f, 0.5f, 0.6f, 0.7f, 0.8f, 0.85f, 0.9f, 0.95f,
			1.0f, 1.1f , 1.2f, 1.3f
		} ;
		float speed ;
		for( int j=0 ; j < sizeof(coefs)/sizeof(float) ; ++j )
			TEST( coefs[j], speed ) ;
	}

	numStdRecs = mux->numRecsDuringCheckTime( );
	defineBuffer( numStdRecs ) ;

	SendMessage( hWnd, msg, EndTest, 0 ) ;
	TRACE( "\n\n%d packets sent during the test\n", g_nPackets ) ;
	return TRUE ;

  labelInterrupted:
	numStdRecs = mux->numRecsDuringCheckTime( );
	defineBuffer( numStdRecs ) ;

	SendMessage( hWnd, msg, Interrupted, 0 ) ;
	TRACE( "\n\n%d packets sent during the test\n", g_nPackets ) ;
	return FALSE ;							// interrupted
}


class TestRateThread : public CWinThread
{
	MuxOutput		*_muxOutput ;
	Mux				*_mux ;
	HANDLE			 _hKillEvent;
	HWND			 _hWnd ;
	int				 _msg ;

  public:
	inline TestRateThread( MuxOutput *muxOutput, Mux *mux, HANDLE hKillEvent, HWND hWnd, int msg )
	{
		m_bAutoDelete = TRUE ;
		_muxOutput	= muxOutput ;
		_mux		= mux ;
		_hKillEvent = hKillEvent ;
		_hWnd		= hWnd ;
		_msg		= msg ;
	}
	virtual BOOL InitInstance()
	{
		_muxOutput->_testOutputRate( _mux, _hKillEvent, _hWnd, _msg ) ;
		return FALSE;		// avoid entering standard message loop by returning FALSE
	}
};


void MuxOutput::testOutputRate( Mux *mux, HANDLE hKillEvent, HWND hWnd, int msg )
{
	TestRateThread *testThread = new TestRateThread( this, mux, hKillEvent, hWnd, msg);
	if (!testThread->CreateThread())
		delete testThread;
}
