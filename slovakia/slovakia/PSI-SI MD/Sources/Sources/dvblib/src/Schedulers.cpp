#include "tools2.hpp"
#include "mux.hpp"
#include  <math.h>
#include "servCfg.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//------------------------------------------------------------------------------------
//	ChannelScheduler
//  Background thread which takes care about multiplexor output rate.
//------------------------------------------------------------------------------------

ChannelScheduler::ChannelScheduler()
{
	m_bAutoDelete   = FALSE;
	_eventKill		= CreateEvent(NULL, TRUE, FALSE, NULL);
	_eventReset		= CreateEvent(NULL, TRUE, FALSE, NULL);
	_handlers[0]    =_eventKill ;
	_handlers[1]    =_eventReset;
}

// timeEps		= time epsilon (-1 means accept start)
// iSwitchFlag == -1 not valid interval
// iSwitchFlag ==  0 sweech off
// iSwitchFlag ==  1 switch on
int ChannelScheduler::getNextTimeout( CfgOutputSchedulerPtrArray *scheduler, tm &tNow, int &iSwitchFlag, BOOL bStartUp )
{
	CfgOutputScheduler *foundSch= NULL, *sch;
	int					iCount  = scheduler->count();

	iSwitchFlag = -1;
	
	if( iCount==0 )
		return (23 - tNow.tm_hour)*3600 + (59 - tNow.tm_min)*60 + (60 - tNow.tm_sec)+1;

	// Find scheduler acc. to the date
	for( int i=0; i<iCount; i++ )
	{
		sch = (*scheduler)[i];
		if( sch->_viaDayName==TRUE )
		{
			time_t  tDate = sch->_date;
			tm     *tmDate= localtime( &tDate );

			if( tNow.tm_year == tmDate->tm_year &&
				tNow.tm_mon  == tmDate->tm_mon  &&
				tNow.tm_mday == tmDate->tm_mday )
			{
				foundSch = sch ;
				break ;
			}
		}
	}

	// If not found, try via day name.
	if( foundSch == NULL )
	{
		int day = tNow.tm_wday;
		for( i=0; i<iCount; i++ )
		{
			sch = (*scheduler)[i];
			if( sch->_viaDayName==FALSE )
			{
				int schDay = (int)sch->dayName();
			
				if( schDay <= sch->Sat && day != schDay ||
					((schDay == sch->Mon_Fri && (day <1   || day > 5) ) || 
					(schDay == sch->Weekend &&  day != 0 && day !=6)))
					continue;
				foundSch = sch ;
				break ;
			}
		}
	}

	// find time interval
	if( foundSch != NULL )
	{
		int				now = tNow.tm_hour*60 + tNow.tm_min;
		CfgOutputRate*	rt  = NULL ;

		iCount = foundSch->_outputRates.count();
		for( int j=0 ; j < iCount; ++j )
		{
			rt= &foundSch->_outputRates.item(j) ;
			time_t from = rt->timeFrom(), to = rt->timeTo();

			if( now<from || now>to )
			{
				if( now<from )
					return (60*from - (60*now+tNow.tm_sec))+1;
				continue;
			}

			if( to==now )			// SWITCH OFF
			{
				iSwitchFlag	= 0;
				if( j+1<iCount )
				{
					// next period
					rt	= &foundSch->_outputRates.item(j+1) ;
					from= rt->timeFrom();
					return (60*from - (60*now+tNow.tm_sec))+1;
				}
				// next day
				break;
			}
			else					// SWITCH ON
			{
				if( bStartUp || from==now )
					iSwitchFlag	= 1;
				return (60*to - (60*now+tNow.tm_sec))+1;
			}
		}
	}

	// wait for tomorrow
	return (23 - tNow.tm_hour)*3600 + (59 - tNow.tm_min)*60 + (60 - tNow.tm_sec)+1;
}

#define TIMEOUT_EPS		5
int ChannelScheduler::refreshChannelsState( BOOL bOnStartUp )
{
	CfgChannelsSetup *pStp	= MfxChannelsSetup();
	int				  iNum	= pStp->numChannels();
	int				  iSwitchFlag, iMinTimeout=0;
	time_t			  now	= time(NULL);
	tm				 *_tm	= localtime( &now ), tmNow;

	if( _tm==NULL )
		return 60;
	tmNow = *_tm;
	TRACE( "   [%02d:%02d] ", tmNow.tm_hour, tmNow.tm_min );
	for( int i=iNum-1; i>=0; i-- )
	{
		const CfgChannel*			ch = pStp->channel(i);
		CfgOutputSchedulerPtrArray*	sch= (CfgOutputSchedulerPtrArray*)&ch->_schedulers;

		if( ch->serviceID()==0 || ch->serviceID()==0xFFFF )
			continue;
		if( sch->ignoreSchedular() )
			continue;
		int t = getNextTimeout( sch, tmNow, iSwitchFlag, bOnStartUp );
		if( iMinTimeout==0 || t<iMinTimeout)
			iMinTimeout = t;

		if( iSwitchFlag>=0 && MfxServerSetup() )
		{
			MuxChannel *muxCh = MfxServerSetup()->getMuxChannel( ch->serviceID() );
			if( muxCh )
			{
				if( iSwitchFlag==0 )
				{
					TRACE( "STOPPED channel <%d>\n", ch->serviceID() );
					muxCh->stop();
				}
				else
				{
					TRACE( "STARTED channel <%d>\n", ch->serviceID() );
					muxCh->start();
				}
			}
		}
	}
	if( iMinTimeout==0 )
		iMinTimeout =3600;

  #ifdef _DEBUG
	now += iMinTimeout;
	_tm	 = localtime( &now );
	TRACE( "   [next timeout =%02d:%02d]\n", _tm->tm_hour, _tm->tm_min );
  #endif
	return iMinTimeout;
}

BOOL ChannelScheduler::InitInstance( )
{
	int timeInterval;
	
	timeInterval = refreshChannelsState( TRUE );
	while(1)
	{
		if( timeInterval<60 )
			timeInterval=60;

		// loop but check for kill notification
		// Wait until either killed or output rate changed.
		switch( WaitForMultipleObjects( 2, _handlers, FALSE, timeInterval*1000) )
		{
			case WAIT_OBJECT_0+1:			// reset event
				ResetEvent( _eventReset ) ;
			case WAIT_TIMEOUT:				// scheduler timeout
				break;
			case WAIT_OBJECT_0:				// kill event
				return FALSE;
		}
		timeInterval = refreshChannelsState( FALSE );
	}
	return FALSE;
}

int ChannelScheduler::ExitInstance( )
{
	CloseHandle(_eventKill );
	CloseHandle(_eventReset);
	return CWinThread::ExitInstance();
}

void ChannelScheduler::kill()
{
	VERIFY( SetEvent( _eventKill ) );
	SetThreadPriority	( THREAD_PRIORITY_ABOVE_NORMAL );
	WaitForSingleObject	( m_hThread, 500);
}
