
#include "tools2.hpp"
#include "Dlg_tool.hpp"
#include "ComDvb.hpp"
#include "card.h"
#include "resource.h"


static ResDllDesc *resMod=0 ;
int resModCounter=0 ;

ResDllDesc *getRcModule()
{
	if( resMod == NULL )
		resMod = new ResDllDesc( "DVB card", theApp.m_hInstance ) ;
	resModCounter++ ;
	return resMod ;
}

void releaseRcModule()
{
	resModCounter-- ;
	if( resModCounter <= 0 )
	{
		delete resMod ;
		resMod = NULL ;
	}
}

ComDVBStatistics _stat ;

BOOL readDrvStatistics( )
{
	DvbDrvStatistics drvStatistic;
	if( dvbDrvStatistics( &drvStatistic) != 0 )
		return FALSE ;

	if( drvStatistic.isValid )
	{
		_stat._numPacketsAcceptedDrv += drvStatistic.numPacketsArrived;
		_stat._numInternetPacketsDrv += drvStatistic.numPacketsHNet;
	}
	return TRUE ;
}


//----------------------------------------------------------------------
//	StatPageBaseDVB
//----------------------------------------------------------------------


class StatPageBaseDVB : public sDllDialog
{
  public:
	void DoDataExchange( CDataExchange* pDX ) ;
	StatPageBaseDVB() : sDllDialog( getRcModule(), sClntD_DVBStatistics, NULL )	{}
   ~StatPageBaseDVB()
	{
	   releaseRcModule() ;
	}
};

void StatPageBaseDVB::DoDataExchange( CDataExchange* pDX )
{
	CString numPacketsAcceptedDrv, numInternetPacketsDrv ;

	if( !readDrvStatistics() )
	{
		numPacketsAcceptedDrv = "Communication error" ;
		DDX_Text(pDX, sClntC_numPacketsAcceptedDrv	, numPacketsAcceptedDrv );
		return ;
	}

	numPacketsAcceptedDrv.Format( "%lu", _stat._numPacketsAcceptedDrv );
	//if( isHNetAllowed() )
		numInternetPacketsDrv.Format( "%lu", _stat._numInternetPacketsDrv );
	//else
	//	numInternetPacketsDrv.Format( "N / A" );

	DDX_Text(pDX, sClntC_numPacketsAcceptedDrv	, numPacketsAcceptedDrv );
	DDX_Text(pDX, sClntC_numInternetPacketsDrv	, numInternetPacketsDrv );

	CString str ;

	str.Format( "%lu", _stat._numPacketsCorruptedHW );
	DDX_Text(pDX, sClntC_numPacketsCorruptedHW	, str );

	str.Format( "%lu", _stat._numPacketsAcceptedHW );
	DDX_Text(pDX, sClntC_numPacketsAcceptedHW	, str );

	str.Format( "%lu", _stat._numPacketsFilteredHW );
	DDX_Text(pDX, sClntC_numPacketsFilteredHW	, str );

	str.Format( "%lu", _stat._numPacketsLostHW );
	DDX_Text(pDX, sClntC_numPacketsLostHW		, str );

	int flag = _stat.isTimingClockSignalOK();
	DDX_Check(pDX, sClntC_TimingClockSignal		, flag );

	flag = _stat.isCardSynchronizationOK();
	DDX_Check(pDX, sClntC_CardSynchronization	, flag );
}


//----------------------------------------------------------------------
//	ComInpDVB utilities
//----------------------------------------------------------------------


BOOL ComInpDVB::getStatisticsLog( char *txt )
{
	if( !readDrvStatistics() )
	{
		MfxMessageHook( EMsg_DvbError, (long)0, (long)"Dvb driver error: wrong statistics." );
		return FALSE ;
	}

	int len = sprintf( txt, "Driver packets:\t%lu ; ", _stat._numPacketsAcceptedDrv );

	//if( isHNetAllowed() )
		len += sprintf( txt+len, "IP: %lu\r\n" , _stat._numInternetPacketsDrv );
	//else
	//	len += sprintf( txt+len, "IP: N/A\r\n" );

	// Card packets
	len += sprintf( txt+len, "Card packets:\tcorrupted: %lu ; accepted: %lu ; filtered: %lu ; lost : %lu\r\n",
					_stat._numPacketsCorruptedHW, _stat._numPacketsAcceptedHW, 
					_stat._numPacketsFilteredHW , _stat._numPacketsLostHW );
	return TRUE ;
}

void ComInpDVB::clearStatistics( )
{
	memset( &_stat, 0, sizeof(_stat) ) ;
}

sDllDialog *ComInpDVB::createStatisticsPage()
{
	StatPageBaseDVB *dlg = new StatPageBaseDVB() ;
	return dlg ;
}


//----------------------------------------------------------------------
//	ComDVBSetup
//----------------------------------------------------------------------


void ComDVBSetup::save( BaseConfigClass *cfg )
{
	cfg->printf( "CardState"  , "Flags", "0x%02x", _flagsCard ) ;
	if( !runningAsServer )
		cfg->setInt( "CardState"  , "ACKtimeout", _ackTimeoutCard );
	cfg->setInt( "CardState"  , "Speed", _cardSpeed );
	cfg->setInt( "DriverState", "DriverMemory", _dvbMemoryDrv );
	cfg->setInt( "DriverState", "Flags", _flagsDrv & ~(DVBDRVFLAG_IsSender | DVBDRVFLAG_UsePESHeader) );
	cfg->setInt( "",			"PID",	(unsigned short)_PID );
}

void ComDVBSetup::load( BaseConfigClass *cfg )
{
	const char *str;

	cfg->getInt( "CardState"  , "ACKtimeout"  , &_ackTimeoutCard );
	cfg->getInt( "CardState"  , "Speed"		  , &_cardSpeed		 );
	cfg->getInt( "DriverState", "DriverMemory",	&_dvbMemoryDrv   );

	_PID = 0;
	if( (str = cfg->get( "", "PID" )) )
	{
		char *pExt = strpbrk( str, ",;" );
		int	  pid;

		if( pExt )
		{
			pExt[0] = 0;
			pid     = atoi( str );
			pExt[0] = ';';
		}
		else
			pid     = atoi( str );
		if( pid > 8191 )
			pid = 8191;
		_PID = (unsigned short)pid;
	}
	_ackTimeoutCard = __max( 0, _ackTimeoutCard ) ;
	_ackTimeoutCard = __min( _ackTimeoutCard, 1000 ) ;

	// Take care about these flags:
	//		(R):   DVBFLAG_StreamFormatPES
	//		(S,R): DVBFLAG_IsSender

	str = cfg->get( "", "Format" );
	BOOL usePesHeader  = stristr( str, "Stream") ? 1 : 0 ;	// TSStream, MDStream

	ushort defaultFlags ;
	if( !runningAsServer )
		defaultFlags = ( usePesHeader ? DVBFLAG_StreamFormatPES : 0x00 );
	else
		defaultFlags = 0 ;

	str = cfg->get( "CardState", "Flags" );
	if( str )
	{
		int n_char;
		if( sscanf( str, "%x%n", &_flagsCard, &n_char ) == EOF || n_char != 4 )
		{
			char warning[256] ;
			sprintf( warning, "[CardState]Flags: invalid value (\"%s\"). Default value taken (0x%02x).", str, defaultFlags );
			MessageBox( NULL, warning, "Config error", MB_OK | MB_ICONERROR ) ;
			_flagsCard = defaultFlags ;
		}
	}

	if( !runningAsServer )
	{
		_flagsCard &= ~(DVBFLAG_IsSender | DVBFLAG_StreamFormatPES) ;
		_flagsCard |= defaultFlags;
	}
	else
	{
		_flagsCard |= DVBFLAG_IsSender ;
	}

	cfg->getInt( "DriverState", "Flags", &_flagsDrv );
	if( !runningAsServer )
		_flagsDrv &= ~DVBDRVFLAG_IsSender ;
	else
		_flagsDrv |= DVBDRVFLAG_IsSender ;
	if( usePesHeader )
		_flagsDrv |= DVBDRVFLAG_UsePESHeader ;
}
