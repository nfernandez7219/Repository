/*
 *	Filename:		comdvb.cpp
 *
 *	Version:		1.00
 *
 *	Description: Low level functions accessing the card.
 *	Contains implementation of following classes:
 *		ComOutDvbAsi - data output
 *		ComInpDvbAsi - data input
 *		CommandReceiver - used by ComInpDvbAsi to process command packets
 *
 *	History:
*/

#include "tools2.hpp"
#include "loadRes.hpp"
#include "ComDvbAsi.hpp"
#include "setup.hpp"
#include "Except.hpp"
#include "SerialNum.hpp"

// DvbAsi API uses 8 By alignment without setting it explicitly. (Error in the header.)
#pragma pack( push,8 )
#include "dvbioctl.h"
#pragma pack(pop)


#define		stopRequested()	_baseComInp->stopRequested()
#define		MAX_BYTERATE	27000000 // Maximum ByteRate of DVB ASI card

#define		WRITE_SLEEP		5	// in ms
#define		WRITE_TIMEOUTS	5	// multiply with WRITE_SLEEP to get total writing timeout
								// after (WRITE_SLEEP*WRITE_TIMEOUTS) msecs write operation
								// is canceled
#define		READ_SLEEP		5	// in ms
#define		READ_TIMEOUTS	5	// multiply with READ_SLEEP to get total reading timeout
								// after (READ_SLEEP*READ_TIMEOUTS) msecs READ operation
								// is canceled

//#define _OVELAPPEDOUTPUT

//--------------------------------------------------------------------------------//
#define DVB_LS_DRVNTSysName		"\\\\.\\Dvb1"
// 64K buffer will fail when locking in the memory

//-------------------------------------------------------------------------------
//	Error utilities
//-------------------------------------------------------------------------------


const char* DvbAsiErrorAsText( int errorCode, char *buf )
{
	const char *msg ;
	switch ( errorCode )
	{
		case ER_CONFIGFILE:             msg = "Can't open the config file"; break ;
		case ER_INVALID_HANDLE:         msg = "Can't open the device. (Driver not installed?)"; break ;
		case ER_CLOSE_HANDLE:			msg = "Invalid close handle operation"; break ;
		case ER_WRITE_OPERATION:		msg = "DvbAsi write operation failed"; break ;
		case ER_READ_OPERATION:			msg = "DvbAsi read operation failed"; break ;
		case ER_IOCTRL:					msg = "DeviceIoControl operation failed" ; break ;
		case ER_INSUFFICIENT_MEMORY:	msg = "Insufficient memory" ; break ;
	
		default : msg = "Unknown DvbAsi error" ;
	}
	strcpy( buf, msg ) ;
	return msg ;
}


//-------------------------------------------------------------------------------
//	ComOutDvbAsi
//  Wrapper class for data output - implementation for DVB driver.
//  The only needed functionality is open, write and close.
//-------------------------------------------------------------------------------


ComOutDvbAsi::ComOutDvbAsi( BaseConfigClass *cfg )
{
	_dvbAsi = new DvbAsi( cfg ) ;	
}

ComOutDvbAsi::~ComOutDvbAsi( )
{
	close( ) ;			// must be called before base class destructor; otherwise 
						// base class destructor will call base class close() !!!
	delete	_dvbAsi ;			
}

BOOL ComOutDvbAsi::hasCapability( ComIOCapability cap )
{
	switch( cap )
	{
		case ComIO_SetupDialog:
			return TRUE ;
	}
	return FALSE ;
}

int	ComOutDvbAsi::open( const char *connectStr )
{
	int err = _dvbAsi->openDvbAsiTransmitter( );
	if( err != 0 ) 
		return err ;
	return ComOut::open( connectStr ) ;
}

int	ComOutDvbAsi::close( )
{
	if( !isOpened( ) )
		return 0 ;
	int dvbErr = _dvbAsi->closeDvbAsi( ) ;
	int comErr = ComOut::close( ) ;
	return dvbErr ? dvbErr : comErr ;
}


int	ComOutDvbAsi::write( const char *buf, int n_bytes, int *written )
{
	return _dvbAsi->writeData( buf, n_bytes, written ) ;
}


int ComOutDvbAsi::setSpeed( float maxSpeed, BaseConfigClass *cfg )
{
	return 1;
}


int	ComOutDvbAsi::resetCard( )
{
	return _dvbAsi->resetCard( ) ;
}

//-------------------------------------------------------------------------------
//	ComInpDvbAsi
//  Wrapper class for data input - implementation for DVB driver.
//-------------------------------------------------------------------------------


ComInpDvbAsi::ComInpDvbAsi( BaseComInp *x, BaseConfigClass *cfg )
{
	_dvbAsi			= new DvbAsi( cfg, x ) ;
}


ComInpDvbAsi::~ComInpDvbAsi( )
{
	close( ) ;			// must be called before base class destructor
	delete _dvbAsi ;
}

BOOL ComInpDvbAsi::getUserId( GlobalUserID *id )
{
	char	path[1024], drive[20];
	ULONG	ulId;

	memset( drive, 0, 20 );
	GetModuleFileName( NULL, path, 1024 );
	_splitpath( path, drive, NULL, NULL, NULL );
	drive[strlen( drive )] = '\\';
	GetVolumeInformation( drive, NULL, 0, (LPDWORD)&ulId, NULL, NULL, NULL, 0 );
	id->set( ulId );
	return TRUE ;
}

BOOL ComInpDvbAsi::hasCapability( ComIOCapability cap )
{
	switch( cap )
	{
		case ComIO_SetupDialog:
			return TRUE ;
	}
	return FALSE ;
}


int	ComInpDvbAsi::open( const char *connectStr )
{
	return _dvbAsi->openDvbAsiReceiver( ) ;
}


int ComInpDvbAsi::close( )
{
	return _dvbAsi->closeDvbAsi( ) ;
}


int	ComInpDvbAsi::workKernel( ) 
{
	return _dvbAsi->workKernel( ) ;
}


int	ComInpDvbAsi::resetCard( )
{
	return _dvbAsi->resetCard( ) ;
}

//------------------------------------------------------------------
//						Statistic Thread
//------------------------------------------------------------------

static BOOL		stopStatisticThread		= FALSE;
static HANDLE	statisticThreadHandle	= NULL;

static DWORD WINAPI StatisticsThreadFn( LPVOID param )
{
	while ( !stopStatisticThread )
	{
		if( comInp != NULL )
			comInp->_dvbAsi->getStats( RxDir ) ;
		Sleep(1000) ;
	}

	return 0 ;
}


//------------------------------------------------------------------
//						DvbAsi Config Class
//------------------------------------------------------------------

BOOL DvbAsiConfig::load( BaseConfigClass * cfg )
{
	if (!cfg)
		return FALSE ;

	if( !cfg->getFloat( DVBASI_CFG_SECTION, "Speed",			      (float*)&_speed       ) )
		_speed = 1.0 ;

	if( !cfg->getInt  ( DVBASI_CFG_SECTION, "204 bytes frames",		  (BOOL*)&_204Bytes     ) )
		_204Bytes = FALSE ;

	if( !cfg->getInt  ( DVBASI_CFG_SECTION, "Packet sync",			  (BOOL*)&_pacSynth     ) )
		_pacSynth = TRUE ;

	if( !cfg->getInt  ( DVBASI_CFG_SECTION, "RF High",				  (BOOL*)&_rfHigh       ) )
		_rfHigh = TRUE ;

	if( !cfg->getInt  ( DVBASI_CFG_SECTION, "No Inter-Byte Stuffing", (BOOL*)&_NoIbStuffing ) )
		_NoIbStuffing = FALSE ;
	

	return TRUE ;
}

BOOL DvbAsiConfig::save( ConfigClass * cfg )
{
	if ( !cfg )
		return FALSE ;

	cfg->setFloat( DVBASI_CFG_SECTION, "Speed"					 , _speed			  ) ;
	cfg->setInt	 ( DVBASI_CFG_SECTION, "204 bytes frames"		 , (BOOL)_204Bytes	  ) ;
	cfg->setInt  ( DVBASI_CFG_SECTION, "Packet sync"			 , (BOOL)_pacSynth	  ) ;
	cfg->setInt  ( DVBASI_CFG_SECTION, "RF High"				 , (BOOL)_rfHigh	  ) ;
	cfg->setInt  ( DVBASI_CFG_SECTION, "No Inter-Byte Stuffing"  , (BOOL)_NoIbStuffing) ;

	cfg->save( FALSE ) ;

	return TRUE ;
}


//------------------------------------------------------------------
//						Low-Level DvbAsi class
//------------------------------------------------------------------


DvbAsi::DvbAsi( BaseConfigClass *cfg, BaseComInp *baseComInp ) 
{
	_cfg		= cfg ;
	_baseComInp = baseComInp ;
}

DvbAsi::~DvbAsi()
{
	DWORD Length = _numBufs*_bufSize + _numBufs*sizeof(OVERLAPPED);

	VirtualUnlock(_buf, Length) ;
	VirtualFree( _buf, 0, MEM_DECOMMIT|MEM_RELEASE );
}

int DvbAsi::allocateBuffers( )
{
	DWORD Length = _numBufs*_bufSize + _numBufs*sizeof(OVERLAPPED);

	DWORD minSize = 0, maxSize = 0;

	HANDLE hProcess = GetCurrentProcess();
	if( !GetProcessWorkingSetSize(hProcess,&minSize,&maxSize) )
		return ER_INSUFFICIENT_MEMORY ;

	if( !SetProcessWorkingSetSize(hProcess, Length+minSize, Length+maxSize ) )
		return ER_INSUFFICIENT_MEMORY ;

	_buf = (char *)VirtualAlloc( NULL,Length,MEM_RESERVE,PAGE_READWRITE );
	_buf = (char *)VirtualAlloc( _buf,Length,MEM_COMMIT ,PAGE_READWRITE  ); 
	
	BOOL ret = VirtualLock(_buf,Length) ;
	if( !ret )
	{
		ULONG err = GetLastError( ) ;
		return ER_INSUFFICIENT_MEMORY ;
	}

	_ov = (OVERLAPPED*)( _buf+_numBufs*_bufSize );

	for( int j=0; j<_numBufs; j++ )
	{
		_ov[j].hEvent			= (void*)1 ;
		_ov[j].Internal			= 0 ;
		_ov[j].InternalHigh		= 0 ;
		_ov[j].Offset			= 0 ;
		_ov[j].OffsetHigh		= 0 ;
	}

	return 0 ;
}


int DvbAsi::openDvbAsiReceiver( ) 
{
	ULONG	n ;
	DVBCfg	Cfg;
	int		ret ;

	#define OPEN_INPUT	_handle = CreateFile(				\
			DVB_LS_DRVNTSysName,							\
			GENERIC_READ | GENERIC_WRITE,					\
			0,												\
			NULL,											\
			OPEN_EXISTING,									\
			FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING,  \
			NULL ) 

	OPEN_INPUT ;
	if ( _handle == INVALID_HANDLE_VALUE )
		return ER_INVALID_HANDLE ;

	// read cfg from card
	if( !DeviceIoControl(_handle,IOCTL_DVB_RD_CFG,NULL,0,&Cfg,sizeof(DVBCfg),(ULONG*)&n,NULL) )
		goto drvOpenFailedLabel ;
	
	// read cfg from file and change cfg
	DvbAsiConfig config ;
	if( !config.load( _cfg ) )
		return ER_CONFIGFILE ;

	if( config._pacSynth ) 
		Cfg.OptionFlags |= PktSyncEnbl ;
	else
		Cfg.OptionFlags &= ~PktSyncEnbl ;

	if( config._rfHigh )   
		Cfg.OptionFlags |= RLSetHigh ;
	else
		Cfg.OptionFlags &= ~RLSetHigh ;

	_numBufs = Cfg.MaxBuffers[RxDir] ;
	// buSize must be multiple of TSPACKET_SIZE and should not be larger than 1000 packets
	_bufSize = __min( Cfg.MaxTransferSize[RxDir], TSPACKET_SIZE*1000 )/TSPACKET_SIZE*TSPACKET_SIZE ;
	
	// write cfg to card
	if( !DeviceIoControl(_handle,IOCTL_DVB_SET_CFG,&Cfg,sizeof(Cfg),NULL,0,(ULONG*)&n,NULL) )
	{
		goto drvOpenFailedLabel ;
	}

	if( !DeviceIoControl(_handle,IOCTL_DVB_RESET_REFRAME,NULL,0,NULL,0,(ULONG*)&n,NULL) )
		goto drvOpenFailedLabel ;

	CloseHandle( _handle ) ;     //Must reopen to accept the changes
	OPEN_INPUT ;

	if( _handle == INVALID_HANDLE_VALUE )
		return ER_INVALID_HANDLE ;

	ret = allocateBuffers( ) ;
	if( ret )
	{
		CloseHandle( _handle ) ;
		_handle = INVALID_HANDLE_VALUE ;
		return ret ;
	}

	stopStatisticThread = FALSE;
	DWORD dw;
	statisticThreadHandle = CreateThread( NULL, 0, StatisticsThreadFn, 0, 0, &dw );

	return 0 ;

  drvOpenFailedLabel:
	CloseHandle( _handle ) ;
	return ER_IOCTRL ;

}


int DvbAsi::openDvbAsiTransmitter( ) 
{
	int		stuffing ;
	int		DesiredRate ;
	int		InterByte ;
	int		InterFrame ;
	int		packetSize ;
	ULONG	n ;
	DVBCfg	Cfg;
	int		ret ;

	#ifdef _OVELAPPEDOUTPUT
		#define OPEN_OUTPUT	_handle = CreateFile(	\
			DVB_LS_DRVNTSysName,			\
			GENERIC_READ | GENERIC_WRITE,	\
			0,								\
			NULL,							\
			OPEN_EXISTING,					\
			FILE_FLAG_OVERLAPPED,			\
			NULL )
	#else
		#define OPEN_OUTPUT	_handle = CreateFile(	\
			DVB_LS_DRVNTSysName,			\
			GENERIC_READ | GENERIC_WRITE,	\
			0,								\
			NULL,							\
			OPEN_EXISTING,					\
			0,								\
			NULL )
	#endif
	OPEN_OUTPUT ;
	if ( _handle == INVALID_HANDLE_VALUE )
		return ER_INVALID_HANDLE ;

	// read cfg from card
	if(!DeviceIoControl(_handle,IOCTL_DVB_RD_CFG,NULL,0,&Cfg,sizeof(DVBCfg),(ULONG*)&n,NULL))
		goto drvOpenFailedLabel ;

	// read cfg from file and change cfg
	DvbAsiConfig config ;
	if( !config.load( _cfg ) )
		return ER_CONFIGFILE ;

	DesiredRate = int( 1048576/8 * config._speed) ;  // Mbit/sec -> byte/sec 

	if( config._NoIbStuffing ) 
		InterByte = 0 ;	
	else
		InterByte = MAX_BYTERATE/DesiredRate - 1;			

	packetSize  = config._204Bytes ? 204 : 188 ;
	InterFrame	= int ( (double)MAX_BYTERATE*(double)packetSize/(double)DesiredRate ) -packetSize-2-(packetSize-1)*InterByte  ;
	stuffing	= InterByte&0xff |(InterFrame<<8);

	Cfg.Stuffing = stuffing ;

	if( config._204Bytes ) 
		Cfg.OptionFlags |= FrSz204 ;
	else
		Cfg.OptionFlags &= ~FrSz204 ;

	_numBufs = __min( Cfg.MaxBuffers[TxDir], 2 ) ;		
	_bufSize = Cfg.MaxTransferSize[TxDir]	;

	// write cfg to card
	if(!DeviceIoControl(_handle,IOCTL_DVB_SET_CFG,&Cfg,sizeof(Cfg),NULL,0,(ULONG*)&n,NULL))
		goto drvOpenFailedLabel ;
	
	if(!DeviceIoControl(_handle,IOCTL_DVB_RESET_REFRAME,NULL,0,NULL,0,(ULONG*)&n,NULL))
		goto drvOpenFailedLabel ;

	CloseHandle(_handle) ;     //Must reopen to accept the changes
	OPEN_OUTPUT ;
	if (_handle == INVALID_HANDLE_VALUE)
		return ER_INVALID_HANDLE ;

	ret = allocateBuffers( ) ;
	if (ret)
	{
		CloseHandle( _handle ) ;
		_handle = INVALID_HANDLE_VALUE ;
		return ret ;
	}

	return 0 ;

drvOpenFailedLabel:
	CloseHandle( _handle ) ;
	return ER_IOCTRL ;
}


int	 DvbAsi::closeDvbAsi() 
{
	if ( _handle != INVALID_HANDLE_VALUE )
	{
		BOOL res = CloseHandle( _handle ) ;
		_handle = INVALID_HANDLE_VALUE ;

		return res?ER_CLOSE_HANDLE:0 ;
	}

	stopStatisticThread = TRUE;
	if ( WaitForSingleObject( statisticThreadHandle, 1000 ) == WAIT_TIMEOUT )
		TerminateThread( statisticThreadHandle, 0 );
	statisticThreadHandle = NULL;

	return 0 ;
}


VOID CALLBACK WriteComplRoutine(
	DWORD dwErrorCode,                // completion code
	DWORD dwNumberOfBytesTransfered,  // number of bytes transferred
	LPOVERLAPPED lpOverlapped)         // pointer to structure with I/O information
{
	lpOverlapped->Internal = dwErrorCode ;	// save the error code
	lpOverlapped->hEvent   = (LPVOID)1 ;	// set completion flag
}

int	DvbAsi::writeData( const char *buf, int n_bytes, int *n_written ) 
{
#ifndef _OVELAPPEDOUTPUT

	try
	{
		if( WriteFile( _handle, buf, n_bytes, (ULONG*)n_written, NULL) )
			return 0 ;
	}
	catch ( ... )
	{
	}
	return GetLastError();

#else

	n_bytes = __min( n_bytes, _bufSize ) ;
	*n_written = 0 ;

	int iBuf = 0 ;
	int nTimeouts = 0 ;
	
	SleepEx(WRITE_SLEEP, TRUE) ;

	// Wait until some buffer free
	// (hEvent is set to 1 in the completion routine after write has finished.)
	while ( _ov[iBuf].hEvent==NULL )
	{
		if ( ++iBuf == _numBufs )
		{
			iBuf = 0 ;
			SleepEx(WRITE_SLEEP, TRUE);

			if (nTimeouts++ == WRITE_TIMEOUTS)
			{
				//getStats( TxDir ) ;
				TRACE("Write timeout. Operation canceled.\n") ;
				CancelIo( _handle ) ;
				_ov[0].hEvent   = (LPVOID)1 ;
				_ov[1].hEvent   = (LPVOID)1 ;
				ULONG n ;
				DeviceIoControl(_handle,IOCTL_DVB_RESET_REFRAME,NULL,0,NULL,0,&n,NULL) ;
				//*n_written = 0 ;
				//return 0;
			}
		}
	} 

	if ( _ov[iBuf].Internal )
	{
		// last WriteFileEx finished with error code
		TRACE("WriteFileEx failed with code %d", _ov[iBuf].Internal) ;
	}

	// Write into free buffer
	_ov[iBuf].hEvent = NULL ;

	char *bufPos = _buf + iBuf*_bufSize ;
	memcpy( bufPos, buf, n_bytes ) ;

	if( !WriteFileEx( _handle, bufPos, n_bytes, &_ov[iBuf], WriteComplRoutine ) )
		return ER_WRITE_OPERATION ;
	
	*n_written = n_bytes ;

	return 0 ;
#endif
}

VOID CALLBACK ReadComplRoutine(
	DWORD dwErrorCode,						// completion code
	DWORD dwNumberOfBytesTransfered,		// number of bytes transferred
	LPOVERLAPPED lpOverlapped)				// pointer to structure with I/O information
{
	lpOverlapped->Internal   = dwErrorCode;		// save the error code
	lpOverlapped->hEvent     = (void *)1;		// this is used as a flag to show this structure has been COMPLETED !
}

int DvbAsi::workKernel() 
{
	ULONG	n ;
	
	stats.clear() ;

	for( int j=0; j<_numBufs; j++ )
	{
		_ov[j].hEvent			= (void*)1 ;
		_ov[j].Internal			= 0 ;
		_ov[j].InternalHigh		= 0 ;
		_ov[j].Offset			= 0 ;
		_ov[j].OffsetHigh		= 0 ;
	}
	
	int bytes;
	int index			 = 0 ;
	int nTimeouts		 = 0 ;

	while ( !stopRequested() )
	{
		while ( _ov[index].hEvent )
		{
			if( !_ov[index].Internal )  // no errors
			{
				bytes = _ov[index].InternalHigh;
				if (bytes > 0)
					_baseComInp->_acceptTsData( _buf+index*_bufSize, bytes ) ;
			}
			else 	// error or canceled
			{
				//TRACE( "Read operation error code: %d \n", _ov[index].Internal ) ;
				_ov[index].Internal		 =0 ;
				_ov[index].InternalHigh  =0 ;//when cancelled InternalHigh==PACKETSIZE
			}

			_ov[index].hEvent = (void*)0 ;
			
			if( !ReadFileEx( _handle, _buf+index*_bufSize, _bufSize, &_ov[index], ReadComplRoutine ) )
				return ER_READ_OPERATION ;

			index++ ;
			if ( index == _numBufs ) 
				index = 0 ;
 		}

		if( SleepEx( READ_SLEEP, TRUE ) != WAIT_IO_COMPLETION )
		{
			nTimeouts++ ;
			if ( nTimeouts > READ_TIMEOUTS )
			{
				nTimeouts = 0 ;
				CancelIo( _handle ) ;
				DeviceIoControl( _handle,IOCTL_DVB_RESET_REFRAME,NULL,0,NULL,0,(ULONG*)&n,NULL ) ;
				TRACE( "Read TimeOut, CancelIO \n" ) ;
			}
		}
		else
			nTimeouts = 0 ;
	}
	CancelIo( _handle  ) ;
	SleepEx ( 100,TRUE ) ;
	SleepEx ( 100,TRUE ) ;

	return 0 ;
}

int DvbAsi::resetCard( ) 
{
	ULONG n ;

	if( !DeviceIoControl(_handle,IOCTL_DVB_RESET_REFRAME,NULL,0,NULL,0, &n,NULL) )
		return ER_IOCTRL ;

	return 0;
}

BOOL DvbAsi::getStats( int sendOrReceive )
{
	unsigned long n;

	DVBStats statistics ;

	if(!DeviceIoControl( _handle,IOCTL_DVB_RD_ST,NULL,0,&statistics,sizeof(struct DVBStats),&n,NULL))
	{
		TRACE ("Error %x in DeviceIoControl\n",GetLastError());
		return FALSE ;
	}

	stats.StartDma			= statistics.StartDma[sendOrReceive] ;
	stats.MaxDspIntCount	= statistics.MaxDspIntCount[sendOrReceive] ;
	stats.NumPciAbts		= statistics.NumPciAbts[sendOrReceive] ;
	stats.MinNumPend		= statistics.MinNumPend[sendOrReceive] ;
	stats.NumPend			= statistics.NumPend[sendOrReceive] ;
	stats.NumInts			= statistics.NumInts[sendOrReceive] ;
	stats.NumFifoErrs		= statistics.NumFifoErrs[sendOrReceive] ;
	stats.NumQued			= statistics.NumQued[sendOrReceive] ;

	stats.NumLost= statistics.NumLost ;
	stats.NumExErr= statistics.NumExErr ;

	return TRUE ;
}

//------------------------------------------------------------------
//						DvbAsiStats Methods
//------------------------------------------------------------------

void DvbAsiStats::clear( )
{
	StartDma		= 0 ;
	MaxDspIntCount	= 0 ;
	NumPciAbts		= 0 ;
	MinNumPend		= 0 ;
	NumPend			= 0 ;
	NumInts			= 0 ;
	NumFifoErrs		= 0 ;
	NumQued			= 0 ;
	NumLost			= 0 ;
	NumExErr		= 0 ;
}

BOOL DvbAsiStats::report( char *pBuf )
{
	sprintf(pBuf,
		"StartDma: %d\n"
		"MaxDspIntCount: %d\n"
		"NumPciAbts: %d\n"
		"MinNumPend: %d\n"
		"NumPend: %d\n"
		"NumInts: %d\n"
		"NumFifoErrs: %d\n"
		"NumQued: %d\n\n"
		"NumLost: %d\n"
		"NumExErrs: %d\n",
		StartDma,
		MaxDspIntCount,
		NumPciAbts,	
		MinNumPend,
		NumPend,
		NumInts,
		NumFifoErrs,
		NumQued,
		NumLost,
		NumExErr
		) ;

	return TRUE ;
}


