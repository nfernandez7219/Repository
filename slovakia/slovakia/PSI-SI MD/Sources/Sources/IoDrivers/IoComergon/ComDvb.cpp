/*
 *	Filename:		comdvb.cpp
 *
 *	Version:		1.00
 *
 *	Description: Low level functions accessing the card.
 *	Contains implementation of following classes:
 *		ComOutDVB - data output
 *		ComInpDVB - data input
 *		CommandReceiver - used by ComInpDVB to process command packets
 *
 *	History:
 */

#include "tools2.hpp"
#include "loadRes.hpp"
#include "Card.h"
#include "ComDvb.hpp"
#include "setup.hpp"

#define _MAXPACKETS				100
#define _MAXSIZE				_MAXPACKETS * TSPACKET_SIZE	/* size of buffer for card i/o */

#define stopRequested()	_baseComInp->stopRequested()


static char *_buf=NULL ;		// locked _mem

BOOL MyCWinApp::InitInstance( )
{
	_buf = (char*)GlobalAlloc( GMEM_FIXED, _MAXSIZE ) ;
	return _buf != NULL ;
}

int  MyCWinApp::ExitInstance( )
{
	if( _buf != NULL )
		GlobalFree( (HGLOBAL)_buf ) ;
	return 0 ;
}


//-------------------------------------------------------------------------------
//	Error utilities
//-------------------------------------------------------------------------------


const char* ComergonErrorAsText( int errorCode, char *buf )
{
	// WIN32 codes
	if( Event_IsWinCode(errorCode) )
	{
		FormatMessage
		( 
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			errorCode,
			0,					// default language
			buf,				// buffer
			256,				// size
			NULL				// args (va_list)
		);
		char *ch = buf;
		while ( ch && *ch != 0 )
		{
			if( *ch == '\r' || *ch == '\n' )
				*ch = ' ';
			ch++;
		}
		return buf ;
	}

	const char *msg ;
	switch ( errorCode )
	{
		case DVBERR_INVALID_DRV_VERSION:msg = "Invalid DVB driver version."; break;
		case DVBERR_PROGRAM_NOT_LOADED:	msg = "DVB card program not loaded."; break;
		case DVBERR_OPEN_ERROR:			msg = "DVB card open error."; break;
		case DVBERR_DRVHANDSHAKEFAILED:	msg = "Handshake with DVB driver failed." ; break ;
 		case DVBERR_CARDNOTDETECTED	:	msg = "DVB Card not detected." ; break ;
 		case DVBERR_CARDPRGNOTEXIST	:	msg = "DVB Card program not found." ; break ;
 		case DVBERR_INVALIDCARDPRG	:	msg = "Invalid DVB Card program." ; break ;
 		case DVBERR_CANTOPENCARDPRG	:	msg = "Can't open DVB Card program." ; break ;
 		case DVBERR_CANTREADCARDPRG	:	msg = "Can't read DVB Card program." ; break ;
 		case DVBERR_CMDNOTACCEPTED	:	msg = "Command refused by the DVB driver." ; break ;
		case DVBERR_CARDHANDSHAKEFAILED:msg = "DVB card does not respond." ; break ;
		default : msg = "Unknown DVB error" ;
	}
	strcpy( buf, msg ) ;
	return msg ;
}


//-------------------------------------------------------------------------------
//	To aid the card debugging following section contains implementation of
//  DUMP_OUT_PACKET and DUMP_INP_PACKET macros.
//  These are used to copy handshake packets to "cardSrv/Rcv.dmp".
//  This is working only if _DUMP_DVBCARD_HANDSHAKE macro is defined.
//-------------------------------------------------------------------------------


#ifdef _DUMP_DVBCARD_HANDSHAKE
	#define PACKET_DUMP_NEEDED
#endif
#ifdef _DUMP_DVBCARD_RCVPACKETS
	#define PACKET_DUMP_NEEDED
#endif

#ifdef PACKET_DUMP_NEEDED

	static FILE *fp_dump=NULL ;

	static void openDumpFile( const char *filename )
	{
		char path[1024], drive[20], dir[1024];
		GetModuleFileName( NULL, path, 1024 );
		_splitpath( path, drive, dir, NULL, NULL );
		_makepath ( path, drive, dir, filename, NULL );
		fp_dump = fopen( path, "wb" ) ;
	}

	#define DUMP_PACKET(p,str)  if( fp_dump != NULL )  { fwrite( str,1,1,fp_dump) ; fwrite( (p),sizeof(MuxPacket),1,fp_dump ) ; }
	#define DUMP_OPEN()			openDumpFile( runningAsServer ? "cardSrv.dmp" : "cardRcv.dmp" )
	#define DUMP_CLOSE()		if( fp_dump != NULL ) fclose( fp_dump )

#else

	#define DUMP_PACKET(p,str)
	#define DUMP_OPEN()
	#define DUMP_CLOSE()

#endif

#define DUMP_OUT_PACKET(p)  DUMP_PACKET( (p), ">" )
#define DUMP_INP_PACKET(p)  DUMP_PACKET( (p), "<" )


///////////////////////////////////////////////////////////////////////////////
// Schema of the command protocol:
///////////////////////////////////////////////////////////////////////////////

/*		
	CMD							IN						RET
	----------------------------------------------------------------------
	DVB_OPEN					DvbOpen					DvbCardInfo
	DVB_START					--						--
	DVB_STOP					--						--
	DVB_CLOSE					--						--
	DVB_SETSPEED				DvbSetSpeed				--

    DVB_CARDSTATISTICS			--						DvbCardStatistics

	DVB_DRVOPEN					DvbDrvMode				DvbDrvInfo
*/


//-------------------------------------------------------------------------------
//	utilities
//-------------------------------------------------------------------------------

// hardcoded DvbOpen
/*
			BYTE buff[188];
			memset (buff,0,188);
			buff[0]  = 0x00; buff[1] = 0x00;
			buff[2]  = 0x00; buff[3] = 0x00;
			buff[4]  = 0xb3;						// sync
			buff[5]  = 0x00; buff[6] = 0x00;		// channel 0
			buff[7]  = 0x08;						// 8 By
			buff[8]  = 0x00;						// flags2 (version 0, rebroadcast 0)
			buff[9]  = 0x00; buff[10] = 0x08;		// flags  (Ctrl)
			buff[11] = 0x00; buff[12] = 0x00; buff[13] = 0x00; buff[14] = 0x00; // DWORD index (0)

			// data (DvbOpen)
			buff[15] = 0x01; buff[16] = 0x01;		// appVersion
			buff[17] = 0x02; buff[18] = 0x02;		// cardPrgVersion
			buff[19] = 0x00; buff[20] = 0x01;		// PID
			buff[21] = 0x01;						// flags (Sender, MuxPacket)
			buff[22] = 184 ;						// packetLength

			buff[181]= 0x01;						// lobyte crc16 (DVB_OPEN)
			buff[183]= 0x0b;

			err = dvbDrvWrite (buff,1,&n_packets);
*/

static uchar DvbOpenPacket[] = {
	//0    1     2     3     4     5     6     7     8     9    10    11    13    14    15    16
	0x47, 0x00, 0x00, 0x00, 0xb3, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x00, 0x00, 0x01, 0x01, 0xf3, 0xb8, 0x50, 0x05, 0x10, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x9c, } ;

// make and send packet(s) for one command
static int makePacketAndWrite( int command, char *cmd, int bytes )
{
	int offset = 0;
	ulong packetInd = 0;

	do
	{
		MuxPacket p;

		int result = p.makeCommandPacket( command, cmd + offset, bytes, packetInd );
		#ifdef _DUMP_DVBCARD_HANDSHAKE
			DUMP_OUT_PACKET(&p) ;
		#endif

		int err;
		int n_packets ;

		TsPacket tp ;
		memcpy( &tp.data, &p, TSPAYLOAD_SIZE );

		//TSPacket ts( 0, 0, TSPacket::SCRAMBLING_CONTROL_NOT_SCRAMBLED, 0, (uchar*)&p, buf );

		err = dvbDrvWrite( &tp, 1, &n_packets );
		if( err != 0 )
			return err ;
		if ( n_packets != 1  )
			return DVBERR_CMDNOTACCEPTED;
		bytes -= result;
		offset+= result;
		packetInd++;
	}
	while ( bytes > 0 );
	return 0;
}


// read DvbCardInfo from the driver (part of handshake)
static int readCardInfo( DvbCardInfo *infoCard )
{
	int err ;
	for( int j=0 ; j < 2/*1000*/ ; ++j )
	{
		int  n_packets ;
		char buf[1024] ;
		err = dvbDrvRead( buf, 1, &n_packets );
		if( err != 0 )
			continue ;
		if( n_packets != 1 )
		{
			Sleep( 2 ) ;
			continue ;
		}

		TsPacket *ts = (TsPacket*)buf ;
		if ( ts->dataLength() == MUXPACKETSIZE )
		{
			MuxPacket *mp = &ts->data;

			#ifdef _DUMP_DVBCARD_HANDSHAKE
				DUMP_INP_PACKET( mp ) ;
			#endif
			if( !mp->isCommandPacket()  ||
				 mp->crc16() != DVB_CARDINFO  ||
				 mp->numDataBytes() != sizeof(DvbCardInfo) + 1/*due to Card programmer's mistake*/ )
				continue ;

			memcpy( infoCard, mp->data(), sizeof(DvbCardInfo) );
			if( stricmp( (const char*)infoCard->name, DVB_CARDPASSWORD) == 0 )
				return 0 ;

			/*// for testing only !!!!!!
			if ( mp->crc16() != DVB_CARDINFO )
				continue;

			mp->flags() = MuxPacket::Ctrl;

			memcpy( infoCard, mp->data() + 2, sizeof(DvbCardInfo) );
			if( stricmp( (const char*)infoCard->name, DVB_CARDPASSWORD) == 0 )
				return 0 ;*/
		}
	}
	return err ? err : DVBERR_CARDHANDSHAKEFAILED;	// no connection
}


//-------------------------------------------------------------------------------
//	openDVB() / closeDVB()
//-------------------------------------------------------------------------------


// send DvbCardInfo to the driver (part of the handshake)
static int sendDVBOpenCommand( DvbCardInfo &infoCard )
{
	//TSPacket t( DvbOpenPacket ) ;
	//MuxPacket *m = (MuxPacket*)t.getData() ;
	//DvbOpen *o = (DvbOpen*)m->data() ;

	DvbOpen dvbOpen;
	dvbOpen.appVersion			= 200;
	dvbOpen.flags				= _setup._flagsCard;
	dvbOpen.PID					= _setup._PID ;
	dvbOpen.speed				= _setup._cardSpeed ;
	dvbOpen.ackTimeout			= _setup._ackTimeoutCard ;		// 16
	dvbOpen.statisticsTimeout	= 1000 ;
	dvbOpen.cardPrgVersion		= 0;				//dvbQueryCardPrgVersion();
	dvbOpen.reserved			= 0;

	int err = makePacketAndWrite( DVB_OPEN, (char*)&dvbOpen, sizeof(DvbOpen) );
	if ( err )
		return err;
	Sleep (1000);

	//#pragma message("    Docasne vypnuty handshake s kartou")
	//return 0 ;
	return readCardInfo( &infoCard ) ;
}


// handshake procedure with the card
int openDVB( char *expl )
{
	int  err=0;
	char buf[1024] ;

	// handshake with driver
	DvbDrvMode modeDrv;
	modeDrv.drvMemory	= _setup._dvbMemoryDrv;
	modeDrv.flags		= _setup._flagsDrv;
	if( runningAsServer )
		modeDrv.flags  |= DVBDRVFLAG_DumpPackets;
	modeDrv.connectToHNet = 0;

	DvbDrvInfo infoDrv;
	err = dvbDrvOpen ( &modeDrv, &infoDrv );
	if ( err != 0 )
	{
		sprintf( expl, "Failed to open DVB Driver due to:\n\"%s\"",
			ComergonErrorAsText(err, buf) ) ;
		return	err;
	}

	if( stricmp( (const char *)infoDrv.name, DVB_DRVPASSWORD) )
	{
		strcpy( expl, "Handshake with the DVB Driver failed.\nRestart the driver and try again." ) ;
		dvbDrvClose();
		return DVBERR_DRVHANDSHAKEFAILED;
	}

	sprintf( _setup._drvVersion, "%d.%02d", infoDrv.drvVersion/100, infoDrv.drvVersion%100 ) ;

	if( !infoDrv.cardDetected )
	{
		strcpy( expl, "DVB Driver could not detect DVB Card.\n"
			"\nCheck the hardware installation and/or adjust card setup." ) ;
		dvbDrvClose();
		return DVBERR_CARDNOTDETECTED;
	}

	// handshake with card
	DvbCardInfo infoCard;
	err = sendDVBOpenCommand( infoCard );

	/*
	DvbDrvStatistics stat ;
	dvbDrvStatistics( &stat ) ;

	MY_ENTRY_INFO dbgInfo[DVB_DRVDumpBufSize] ;
	dvbDrvDbgInfo( dbgInfo ) ;

	char bufPackets[100*188];
	int n_packets;
	err = dvbDrvRead( bufPackets, 100, &n_packets );

	if ( err == 0 )
		for ( int kk = 0; kk < n_packets; kk++ )
		{
			TSPacket ts( (uchar*)(bufPackets + kk*188) );
			if ( ts.dataLength() == MUXPACKETSIZE )
			{
				MuxPacket *mp = (MuxPacket*)ts.getData();

				TRACE( "\nSYNC BYTE = %d, CRC16 = %d, FLAGS = %d, DATA LENGTH = %d",
					mp->syncByte(), mp->crc16(), mp->flags(), mp->numDataBytes() );
			}
		}
	*/

	if( err == 0 )
	{
		_setup._userId = infoCard.serialNum ;
		err = makePacketAndWrite( DVB_START, NULL, 0 );
		if( err == 0 )
			return 0 ;
	}
	
	ComergonErrorAsText( err, buf ) ;
	sprintf( expl, "DVB driver contacted, interrupts work, but the Card handshake failed:\n%s", buf ) ;
	dvbDrvClose();

	//#pragma message("    Tu by sa mala vracat chyba")
	//return 0 ;
	return err ;
}

// Termination of the connection to the DVB driver.
int closeDVB()
{
	if( !dvbDrvOpened() )
		return 0 ;

	int err ;
	if( runningAsServer )
	{
		err = makePacketAndWrite( DVB_STOP, NULL, 0 );
		if( err == 0 )
			err = makePacketAndWrite( DVB_CLOSE, NULL, 0 );
		if( err == 0 )
			err = dvbDrvClose();
	}
	else
		err = dvbDrvClose();

	return err ;
}


//-------------------------------------------------------------------------------
//	Processing statistics
//-------------------------------------------------------------------------------


// Processing of 1 command packet.
BOOL ComInpDVB::processCommandPacket( MuxPacket *packet )
{
	#ifdef _DUMP_DVBCARD_RCVPACKETS
		DUMP_INP_PACKET(packet) ;
	#endif

	if( packet->crc16() != DVB_CARDSTATISTICS )
		return FALSE ;

	// accept card statistics
	DvbCardStatistics stat;
	if ( packet->numDataBytes() != 9/*sizeof(DvbCardStatistics)*/ )
		return TRUE;
	memcpy( (char *)&stat, packet->data(), sizeof(DvbCardStatistics)/*packet->numDataBytes()*/ );

	ushort pc = stat.corrupted ;
	ushort pa = stat.filtered ;
	ushort pf = stat.arrived ;
	ushort pl = stat.lost ;
	pf -= pa ;
	_stat._numPacketsCorruptedHW += pc ;	// # packets corrupted ( bad checksum )
	_stat._numPacketsAcceptedHW  += pa ;	// # packets sent to applic.
	_stat._numPacketsFilteredHW  += pf ;	// # packets filtered out ( come to nonregistered channel )
	_stat._numPacketsLostHW		 += pl ;	// # packets lost due to fifo size ( come but no space to store )
	_stat.setCardFlags( stat.flags );		// # set timing and synchronization flags

	readDrvStatistics() ;
	return TRUE ;
}


//-------------------------------------------------------------------------------
//	ComOutDVB
//  Wrapper class for data output - implementation for DVB driver.
//  The only needed functionality is open, write and close.
//-------------------------------------------------------------------------------


ComOutDVB::ComOutDVB()
{
	DUMP_OPEN() ;
}

ComOutDVB::~ComOutDVB()
{
	close() ;			// must be called before base class destructor; otherwise 
						// base class destructor will call base class close() !!!
	DUMP_CLOSE() ;
}

BOOL ComOutDVB::hasCapability( ComIOCapability cap )
{
	switch( cap )
	{
		case ComIO_DriverStatusDlg:
		case ComIO_DriverDump:
		case ComIO_SetupDialog :
			return TRUE ;
	}
	return FALSE ;
}

int	ComOutDVB::open( const char *connectStr)
{
	char   expl[1024] ;
	int    err = 0 ;

	// test usage of the driver
	if( !lockDVB(expl) )
		err = DVBERR_CMDNOTACCEPTED;
	else
	{
		err = openDVB( expl );
		if( err != 0 )
			unlockDVB() ;
	}
	if( err != 0 ) 
	{
		::MessageBox( NULL, expl, "Fatal error", MB_ICONSTOP | MB_OK | MB_TOPMOST );
		return err ;
	}
	return ComOut::open(connectStr ) ;
}

int	ComOutDVB::close()
{
	if( !isOpened() )
		return 0 ;
	
	int dvbErr = closeDVB();

	unlockDVB() ;

	int comErr = ComOut::close() ;
	return dvbErr ? dvbErr : comErr ;
}


int	ComOutDVB::write( const char *buf, int n_bytes, int *written )
{
	if ( n_bytes < MUXPACKETSIZE )
	{
		*written = 0;
		return 0;
	}

	try
	{
		int toWrite = n_bytes / TSPACKET_SIZE ;
		int writtenPackets;
		USHORT  err = dvbDrvWrite( (void*)buf, toWrite, &writtenPackets ) ;

		*written = writtenPackets * TSPACKET_SIZE;

		return err ;
	}
	catch ( ... )
	{
		return GetLastError();// error
	}
}

// maxSpeed MB/s
int ComOutDVB::setSpeed( float maxSpeed, BaseConfigClass *cfg )
{
	unsigned short kBit = (unsigned short)(maxSpeed * 1000 * 8) ;
	if ( kBit < MIN_CARD_SPEED )
		kBit = MIN_CARD_SPEED ;
	else
	if ( kBit > MAX_CARD_SPEED )
		kBit = MAX_CARD_SPEED ;

	DvbSetSpeed speed ;
	speed.speed = kBit ;

	return makePacketAndWrite( DVB_SETSPEED, (char*)&speed, sizeof(DvbSetSpeed) ) ;
}


//-------------------------------------------------------------------------------
//	ComInpDVB
//  Wrapper class for data input - implementation for DVB driver.
//-------------------------------------------------------------------------------


ComInpDVB::ComInpDVB( BaseComInp *x )
{
	_baseComInp = x ;
	DUMP_OPEN() ;
}

ComInpDVB::~ComInpDVB()
{
	close() ;			// must be called before base class destructor
	DUMP_CLOSE() ;
}


BOOL ComInpDVB::getUserId( GlobalUserID *id )
{
	*id = _setup._userId ;
	return TRUE ;
}

BOOL ComInpDVB::hasCapability( ComIOCapability cap )
{
	switch( cap )
	{
		case ComIO_DriverStatusDlg:
		case ComIO_CanDoHWFiltering:
		case ComIO_SetupDialog :
			return TRUE ;
	}
	return FALSE ;
}

int	ComInpDVB::open( const char *connectStr )
{
	char expl[1024] ;
	int  err = 0 ;

	// test usage of the driver
	if( !lockDVB(expl) )
		err = DVBERR_CMDNOTACCEPTED;
	else
	{
		err = openDVB( expl );
		if( err != 0 )
			unlockDVB() ;
	}
	if( err != 0 )
		::MessageBox( NULL, expl, "Fatal error", MB_ICONSTOP | MB_OK | MB_TOPMOST );
	return err ;
}

int ComInpDVB::close()
{
	int err = closeDVB();
	unlockDVB() ;
	return err ;
}

// Infinite loop:
//		read packet
//		process packet
int ComInpDVB::normalWorkKernel() 
{
	while( !stopRequested() )
	{
		int n_packets;
		int err = dvbDrvRead( _buf, _MAXPACKETS, &n_packets ) ;
		if( err != 0 )
			return err ;

		if( n_packets > 0 )
		{
			int n_bytes = n_packets * TSPACKET_SIZE ;
			_baseComInp->_acceptTsData( _buf, n_bytes ) ;
		}
		else
			Sleep( NO_INPUT_DATA_DELAY ) ;						// no data, sleep a bit
	}

	return 0 ;
}


//------------------------------------------------------------------------------------
//	dump packet
//  This part only aids in debugging data coming from the driver.
//------------------------------------------------------------------------------------


#ifdef _DEBUG
// variables used in specific tests bellow
static ulong nextPI = 0;
static uchar nextJI = 0;
#endif


#ifdef _DUMP_RECEIVED_BAD_PACKETS
void dumpPacket( MuxPacket *mp, int flag )
{
	#define PACKET_DUMP_NAME		"Packets.dmp"	/* name of file where corrupted packets are dumped */
	static FILE *packet_dump = NULL;
	static BOOL  packet_dumpOpened=FALSE ;
	if( packet_dump == NULL )
	{
		if( packet_dumpOpened )
			return;
		packet_dumpOpened = TRUE ;
		packet_dump = fopen( PACKET_DUMP_NAME, "wt" );
		if( packet_dump == NULL )
			return ;
	}

	if( flag & 0x03 )
		fprintf( packet_dump, "===== crc and checksum =============================================\n");
	else
	{
		if( flag & 0x01 )
			fprintf( packet_dump, "===== crc =============================================\n");
		if( flag & 0x02 )
			fprintf( packet_dump, "===== checksum =============================================\n");
	}
	if( flag & 0x04 )
	{
			fprintf( packet_dump, "===== out of order =================================================\n");
			fprintf( packet_dump, "missing packets: %x, %x, %x - %x, %x, %x    # PI, JI, packInd\n\n", nextPI, nextJI, (nextPI & 0x00FFFFFF) + (nextJI<<24), mp->packetIndex(), mp->jobId(), mp->packetInd() );
			nextPI = mp->packetIndex();
			nextJI = mp->jobId();
	}
	if( flag & 0x08 )
		fprintf( packet_dump, "===== bad data =====================================================\n");

	fprintf( packet_dump, "_syncByte         = %x\n", mp->syncByte() );
	fprintf( packet_dump, "_channel          = %x\n", mp->channel() );
	fprintf( packet_dump, "_numDataBytes     = %x\n", mp->numDataBytes() );
	fprintf( packet_dump, "_flags2           = %x\n", mp->flags2() );
	fprintf( packet_dump, "_flags            = %x\n", mp->flags() );
	fprintf( packet_dump, "_packetInd        = %x\n", mp->packetInd() );

	fprintf( packdt_dump, "_data             = " );
	uchar *data = (uchar *)mp->data();
	for( int i = 0; i < MUXDATASIZE; i++ )
	{
		if( ( i % 10 ) == 0 )
			fprintf( packet_dump, "\n" );
		fprintf( packet_dump, " %2x", *data++ );
	}
	fprintf( packet_dump, "\n" );

	fprintf( packet_dump, "_crc16            = %x\n", mp->crc16() );
	fprintf( packet_dump, "_checkSum         = %x\n", mp->checkSum() );

	dvbDrvDump( 0 );
}
#endif


//------------------------------------------------------------------------------------
//	specific tests
//  Specific implementations of the work kernel prepared for testing the card.
//  Left here only for the case of troubles.
//  Under normal circumstances normalWorkKernel() is used.
//-----------------------------------------,------------------------------------------

/*
#ifdef _DEBUG
static ulong packetInd = 0;
static BOOL  firstPacket = TRUE;

int ComInpDVB::test1WorkKernel()
{
	MuxPacket	*mp;
	int			err, n_packets;
	DvbClientSetup *cSetup	= MfxClientSetup();

	while( !stopRequested() )
	{
		err = dvbDrvRead( _buf, _MAXSIZE / MUXPACKETSIZE, &n_packets ) ;
		if( err != 0 )
		{
			return err ;
		}

		if( n_packets > 0 )
		{
			for( int i = 0; i < n_packets; i++ )
			{
				cSetup->incPackets();
				mp = (MuxPacket*)&_buf[i * MUXPACKETSIZE];
				if( mp->syncByte() != MuxPacketSyncByte )
					continue;

				if( mp->isCrcAndCheckSumOk() )
				{
					if( mp->isCommandPacket() )
					{
						cSetup->decPackets();
						_commandReceiver->push( mp );
					}
					else
					{
						int flag;
						flag = 0;
						//some cheks
						// packet index is groving every time by 1
						ulong pi = mp->packetIndex();
						uchar ji = mp->jobId();
						ulong pInd = mp->packetInd();
						
						if ( firstPacket )
						{
							nextPI = pi;
							nextJI = ji;
							firstPacket = FALSE;
						}
						if ( pi != nextPI || ji != nextJI )
						{
							flag |= 0x04;			// out of order
						}
						packetInd = pInd;
						
						// packet data is set to packetIndex % 0x100
						int i = 0;
						uchar c = (uchar)(pi % 0x100);
						const uchar *data = mp->data();
						while ( i < MUXDATASIZE && data[i] == c ) i++;
						if ( i == MUXDATASIZE )
							cSetup->incSuccPackets();
						else
							flag |= 0x08;				// bad data

						if( flag )
							DUMP_RCVPACKET( mp, flag );

						nextPI++;
						if ( nextPI > 0xffff )
						{
							nextPI = 0;
							nextJI++;
						}
					}
				}
				else
				{
					nextPI++;
					if ( nextPI > 0xffff )
					{
						nextPI = 0;
						nextJI++;
					}
				}
			}
		}
		else
			Sleep( 10 ) ;						// no data, sleep a bit
	}
	return 0 ;
}

int ComInpDVB::test4WorkKernel()
{
	MuxPacket	*mp;
	int			err, n_packets;
	DvbClientSetup *cSetup	= MfxClientSetup();

	while( !stopRequested() )
	{
		err = dvbDrvRead( _buf, _MAXSIZE / MUXPACKETSIZE, &n_packets ) ;
		if( err != 0 )
		{
			return err ;
		}

		if( n_packets > 0 )
		{
			for( int i = 0; i < n_packets; i++ )
			{
				cSetup->incPackets();
				mp = (MuxPacket*)&_buf[i * MUXPACKETSIZE];
				if( mp->syncByte() != MuxPacketSyncByte )
					continue;

				if( mp->isCrcAndCheckSumOk() )
				{
					if( mp->isCommandPacket() )
					{
						cSetup->decPackets();
						_commandReceiver->push( mp );
					}
					else
					{
						int flag;
						flag = 0;
						//some cheks
						// packet index is groving every time by 1
						ulong pi = mp->packetIndex();
						uchar ji = mp->jobId();
						ulong pInd = mp->packetInd();
						
						if ( firstPacket )
						{
							nextPI = pi;
							nextJI = ji;
							firstPacket = FALSE;
						}
						if ( pi != nextPI || ji != nextJI )
						{
							flag |= 0x04;			// out of order
						}
						packetInd = pInd;
						
						// packet data is unknown, don't test

						if( !flag )
							cSetup->incSuccPackets();
						//	DUMP_RCVPACKET( mp, flag );

						nextPI++;
						if ( nextPI > 0xffff )
						{
							nextPI = 0;
							nextJI++;
						}
					}
				}
				else
				{
					nextPI++;
					if ( nextPI > 0xffff )
					{
						nextPI = 0;
						nextJI++;
					}
				}
			}
		}
		else
			Sleep( 10 ) ;						// no data, sleep a bit
	}
	return 0 ;
}

int ComInpDVB::test2WorkKernel()
{
	MuxPacket	*mp;
	int			err, n_packets;
	DvbClientSetup *cSetup	= MfxClientSetup();

	while( !stopRequested() )
	{
		err = dvbDrvRead( _buf, _MAXSIZE / MUXPACKETSIZE, &n_packets ) ;
		if( err != 0 )
		{
			return err ;
		}

		if( n_packets > 0 )
		{
			for( int i = 0; i < n_packets; i++ )
			{
				cSetup->incPackets();
				mp = (MuxPacket*)&_buf[i * MUXPACKETSIZE];
				if( mp->syncByte() != MuxPacketSyncByte )
					continue;

				//if( !mp->isCommandPacket() )
				//	fprintf( packet_dump, "==== packet index = %x\n", mp->packetInd() );
				if( mp->isCrcAndCheckSumOk() )
				{
					if( mp->isCommandPacket() )
					{
						cSetup->decPackets();
						_commandReceiver->push( mp );
					}
					else
					{
						int flag;

						flag = 0;
						//some cheks
						// packet index is groving every time by 1
						ulong pi = mp->packetIndex();
						uchar ji = mp->jobId();
						ulong pInd = mp->packetInd();
						if ( firstPacket )
						{
							nextPI = pi;
							nextJI = ji;
							firstPacket = FALSE;
						}
						if ( pi != nextPI || ji != nextJI )
						{
							flag |= 0x04;			// out of order
						}
						packetInd = pInd;

						// packet data is set to 0 1 2 ... 169
						int i = 0;
						const uchar *data = mp->data();
						uchar c = (uchar)(pi % 0x100);
						while ( i < MUXDATASIZE && data[i] == c++ ) i++;
						if ( i == MUXDATASIZE )
						{
							cSetup->incSuccPackets();
							cSetup->incProcessedPackets();
						}
						else
							flag |= 0x08;

						if( flag )
						{
							DUMP_RCVPACKET( mp, flag );
							//if( flag & 0x08 )
							//	fprintf( packet_dump, "\nbad data index = %i\n", i );
						}

						nextPI++;
						if ( nextPI > 0xffff )
						{
							nextPI = 0;
							nextJI++;
						}
					}
				}
				else
				{
					nextPI++;
					if ( nextPI > 0xffff )
					{
						nextPI = 0;
						nextJI++;
					}
				}
			}
		}
		else
			Sleep( 10 ) ;						// no data, sleep a bit
	}
	return 0 ;
}
#endif
*/

//------------------------------------------------------------------------------------
//	end of tests
//------------------------------------------------------------------------------------

/*
int	ComInpDVB::workKernel()
{
	int retval;

	switch( TESTMODE )
	{
		case 4:	// random data
			retval = test4WorkKernel();
			break;
		case 3:
		case 2:
			retval = test2WorkKernel();
			break;
		case 1:
			retval = test1WorkKernel();
			break;
		case 0:
		default:
			retval = normalWorkKernel();
			break;
	}
	return retval;
}
*/

int	ComInpDVB::workKernel()
{
	return normalWorkKernel();
}
