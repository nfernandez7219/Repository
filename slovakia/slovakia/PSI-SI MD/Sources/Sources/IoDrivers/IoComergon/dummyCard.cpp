
#include "Usrlib.hpp"
#include "mux.hpp"

#include "Card.h"

// Error codes have only local meaning.
// Allowed range is 1-255.
// 
#define CARDERR_CMD_FAILED	1

const char *dvbDrvErrorString( int code, char *buf )
{
	const char *msg ;
	switch ( code )
	{
		case CARDERR_CMD_FAILED:		msg = "DVB card command failed."; break ;
		default:					msg = "Unknown DVB card error." ; break ;
	}
	strcpy( buf, msg ) ;
	return buf ;
}


USHORT dvbQueryCardPrgVersion   ()	{ return 100; }
USHORT dvbQueryCardDriverVersion()	{ return 100; }
USHORT dvbQueryMajorVersion()	    { return 100; }

static BOOL opened = FALSE;

int dvbDrvOpen( DvbDrvMode *m, DvbDrvInfo *inf )
{
	strcpy( (char *)(inf->name), DVB_DRVPASSWORD );
	inf->drvVersion = 100;
	inf->drvMemory = m->drvMemory;
	inf->IRQ = 9;
	inf->baseAddress = 0x300;
	inf->cardDetected = 1;

	opened = TRUE;

	return 0;
}

int dvbDrvClose()
{
	if ( !opened )
		return CARDERR_CMD_FAILED;
	return 0;
}

static ushort cmd = 0;		// command request coming from dvbDrvWrite()

int dvbDrvRead( void *buf, int maxPackets2read, int *numPacketsRead )
{
	if ( !opened )
		return CARDERR_CMD_FAILED;
	MuxPacket p;
	switch ( cmd )
	{
		case DVB_OPEN:
		{
			DvbCardInfo cardInfo;
			strcpy( (char *)(cardInfo.name), DVB_CARDPASSWORD );
			cardInfo.cardPrgVersion = 100;
			cardInfo.cardMemory = 100;
			cardInfo.serialNum = 0;
			cardInfo.programOK = 1;
			p.makeCommandPacket( DVB_CARDINFO, (char *)&cardInfo, sizeof(cardInfo) );
			*numPacketsRead = 1 ;
			break;
		}
		case DVB_CARDSTATISTICS:
		{
			DvbCardStatistics cardStat;
			srand( (unsigned)time( NULL ) );
			int cor = rand() % 10;
			int fil = rand() % 20;
			int arr = rand() % 100 + fil + cor;
			cardStat.numPacketsCorrupted = cor;
			cardStat.numPacketsArrived = arr;
			cardStat.numPacketsFiltered = fil;
			cardStat.numPacketsLost = rand() % 10;
			cardStat.isValid = 0xff;
			p.makeCommandPacket( DVB_CARDSTATISTICS, (char *)&cardStat, sizeof(cardStat) );
			*numPacketsRead = 1 ;
			break;
		}
		default:
			*numPacketsRead = 0;
	}
	cmd = 0;
	if( *numPacketsRead > 0 )
		memcpy( buf, &p, *numPacketsRead*MUXPACKETSIZE );
	return 0;
}

int dvbDrvWrite( void *buf, int maxPackets2write, int *numPacketsWritten )
{
	if ( !opened )
		return CARDERR_CMD_FAILED;
	MuxPacket *p = (MuxPacket *)buf;
	if ( p->isCommandPacket() )
	{
		cmd = p->crc();
	}
	*numPacketsWritten = maxPackets2write;
	return 0;
}

int dvbDrvStatistics( DvbDrvStatistics *stat )
{
	if ( !opened )
		return CARDERR_CMD_FAILED;
	srand( (unsigned)time( NULL ) );
	stat->numPacketsArrived = rand() % 10;
	stat->numPacketsHNet = rand() % 10;
	stat->isValid = 0xff;

	return 0;
}
