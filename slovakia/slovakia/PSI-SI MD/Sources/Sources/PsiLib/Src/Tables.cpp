
#include "stdafx.h"
#include "Tables.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void PrivateSection::computeCRC( )
{
	int crcPos = getTotalLength() - 4 ;
	DWORD *crc = (DWORD*)(((uchar*)this) + crcPos) ;
	*crc = 0 ;
}

//------------------------------------------------------------------------------------//
//
//							Program Association Table
//
//------------------------------------------------------------------------------------//

void PATable::create( ushort nPrograms, ushort transportStreamId, BOOL bApplicable )
{
	table_id = TABLE_ID_PAT ;

	setSectSyntaxIndicator	( );
	resetPrivateIndicator	( );

	ushort newSize = nPrograms*4 + 12 ;
	ASSERT( newSize < 0x3FD ) ;
	setSectionLength(newSize) ;

	transport_stream_id = ntols(transportStreamId) ;

	if (bApplicable)
		setCurrentNextIndicator( );
	else
		resetCurrentNextIndicator( );

	setVersionNumber(0) ;

	section_number = last_section_number = 0 ;

	computeCRC() ;
}

void PATable::setProgram ( ushort index, ushort prgNum, ushort prgPID )
{
	programs[index].program_number  = ntols(prgNum) ;
	programs[index].program_map_PID = ntols(prgPID) ;
}

//------------------------------------------------------------------------------------//
//
//							Program Map Table
//
//------------------------------------------------------------------------------------//

void PmtStream::streamTypeName( int streamType, char *buf )
{
	switch(streamType)
	{
	case video_MPEG1			: strcpy(buf, "Video MPEG1") ;			break ;
	case video_MPEG2			: strcpy(buf, "Video MPEG2") ;			break ;
	case audio_MPEG1			: strcpy(buf, "Audio MPEG1") ;			break ;
	case audio_MPEG2			: strcpy(buf, "Audio MPEG2") ;			break ;
	case private_section		: strcpy(buf, "Private section") ;		break ;
	case PES_with_private_data	: strcpy(buf, "PES with private data") ;break ;
	case MHEG					: strcpy(buf, "MHEG") ;					break ;
	case DSMCC					: strcpy(buf, "DSM CC") ;				break ;
	case ITU_T_Rec_H_221_1		: strcpy(buf, "ITU-T Rec. H.221.1") ;	break ;
	case DSMCC_type_A			: strcpy(buf, "DSM CC type A") ;		break ;
	case DSMCC_type_B			: strcpy(buf, "DSM CC type B") ;		break ;
	case DSMCC_type_C			: strcpy(buf, "DSM CC type C") ;		break ;
	case DSMCC_type_D			: strcpy(buf, "DSM CC type D") ;		break ;
	case auxiliary				: strcpy(buf, "Auxiliary") ;			break ;
	default						: strcpy(buf, "Unknown") ;				break ;
	}

	char num[20] ;
	sprintf(num, "(0x%x)", streamType) ;
	strupr(num+3) ;
	strcat(buf, num) ;
}

int PMTable::getNumDescriptors( )
{
	int descLength = getProgramInfoLength() ;
	int p = 0 ;
	int nDesc = 0 ;
	uchar *descStart = descriptors ;
	
	while ( p < descLength )
	{
		BaseDescriptor *desc = (BaseDescriptor*)(descStart+p) ;
		p += desc->getLength() ;
		nDesc++ ;
	}

	ASSERT(p==descLength) ;

	return nDesc ;
}

BaseDescriptor *PMTable::getDescriptor( int index )
{
	int descLength = getProgramInfoLength() ;
	int p = 0 ;
	int nDesc = 0 ;
	uchar *descStart = descriptors ;
	
	while ( p < descLength )
	{
		BaseDescriptor *desc = (BaseDescriptor*)(descStart+p) ;
		if (nDesc==index)
			return desc ;
		p += desc->getLength() ;
		nDesc++ ;
	}

	return NULL ;
}

int PMTable::getNumStreams( )
{
	int size = getStreamsSize() ;
	uchar *streams = pmtStreams() ;
	int p = 0 ;
	int nStreams = 0 ;

	while ( p < size )
	{
		PmtStream *stream = (PmtStream*)(streams+p) ;
		p += stream->getLength() ;
		nStreams++ ;
	}

	ASSERT(p==size) ;

	return nStreams ;
}

PmtStream *PMTable::getPmtStream( int index )
{
	int size = getStreamsSize() ;
	uchar *streams = pmtStreams() ;
	int p = 0 ;
	int nStreams = 0 ;

	while ( p < size )
	{
		PmtStream *stream = (PmtStream*)(streams+p) ;
		if (nStreams==index)
			return stream ;

		p += stream->getLength() ;
		nStreams++ ;
	}

	return NULL ;
}

void PMTable::create( ushort programNumber, ushort pcrPid, BOOL bApplicable )
{
	table_id = TABLE_ID_PMT ;

	setSectSyntaxIndicator	( );
	resetPrivateIndicator	( );

	ushort newSize = 16 ;
	setSectionLength(newSize) ;

	program_number = ntols(programNumber) ;

	setVersionNumber(0) ;

	if (bApplicable)
		setCurrentNextIndicator( );
	else
		resetCurrentNextIndicator( );

	section_number = last_section_number = 0 ;

	PCR_PID = ntols(pcrPid) ;

	program_info_length = ntols(0) ;

	computeCRC() ;
}

void PMTable::addDescriptor( BaseDescriptor *descriptor )
{
	int totalDescLen = getProgramInfoLength() ;
	uchar *descEnd = descriptors + totalDescLen ;

	int descLen = descriptor->getLength() ;
	int bytesToMove = getTotalLength() - ((int)descEnd-(int)this) ;

	// prepare a place for new descriptor
	memmove( descEnd+descLen, descEnd, bytesToMove ) ;
	// copy new descriptor
	memmove( descEnd, descriptor, descLen ) ;

	int newLen = getTotalLength() + descLen ;
	setSectionLength(newLen) ;

	computeCRC() ;
}

void PMTable::addStream( PmtStream* stream )
{
}

void PMTable::addStreamDescriptor( int streamIndex, BaseDescriptor *descriptor )
{
}

//------------------------------------------------------------------------------------//
//
//							Service Description Table
//
//------------------------------------------------------------------------------------//

void SdtService::runningStatusAsText( int status, char *buf )
{
	switch(status)
	{
	case NotRunning			: strcpy(buf,"Not running"); break ;
	case StartsInFewSeconds	: strcpy(buf,"Starts in a few seconds"); break ;
	case Pausing			: strcpy(buf,"Pausing"); break ;
	case Running			: strcpy(buf,"Running"); break ;
	default					: strcpy(buf,"Undefined"); break ;
	} ;
}

void SDTable::create( ushort tsId, ushort origNtwId, BOOL bApplicable )
{
	table_id = TABLE_ID_SDT ;

	setSectSyntaxIndicator	( );
	resetPrivateIndicator	( );

	ushort newSize = 11 ;
	setSectionLength(newSize) ;

	transport_stream_id = ntols(tsId) ;

	setVersionNumber(0) ;

	if (bApplicable)
		setCurrentNextIndicator( );
	else
		resetCurrentNextIndicator( );

	section_number = last_section_number = 0 ;

	original_network_id = ntols(origNtwId) ;

	computeCRC() ;
}

int SDTable::getNumServices( )
{
	int size = getServicesSize() ;
	int p = 0 ;
	int nServices = 0 ;

	while ( p < size )
	{
		SdtService *service = (SdtService*)(services+p) ;
		p += service->getLength() ;
		nServices++ ;
	}

	ASSERT(p==size) ;

	return nServices ;
}

//------------------------------------------------------------------------------------//
//
//							Network Information Table
//
//------------------------------------------------------------------------------------//

void NITable::create( ushort networkId, BOOL bApplicable )
{
	table_id = TABLE_ID_NIT;

	setSectSyntaxIndicator	( );
	resetPrivateIndicator	( );

	ushort newSize = 16 ;
	setSectionLength(newSize) ;

	network_id = ntols(networkId) ;

	setVersionNumber(0) ;

	if (bApplicable)
		setCurrentNextIndicator( );
	else
		resetCurrentNextIndicator( );

	section_number = last_section_number = 0 ;

	network_descriptors_length = 0 ;

	*((ushort*)descriptors) = 0 ;

	computeCRC() ;
}

int NITable::getNumStreams( )
{
	int size = getStreamsSize() ;
	uchar *streams = transpStreams() ;
	int p = 0 ;
	int nStreams = 0 ;

	while ( p < size )
	{
		NitTransportStream *stream = (NitTransportStream*)(streams+p) ;
		p += stream->getLength() ;
		nStreams++ ;
	}

	ASSERT(p==size) ;

	return nStreams ;
}


//------------------------------------------------------------------------------------//
//
//							Conditional Access Table
//
//------------------------------------------------------------------------------------//

void CATable::create( BOOL bApplicable )
{
	table_id = TABLE_ID_CAT;

	setSectSyntaxIndicator	( );
	resetPrivateIndicator	( );

	ushort newSize = 12 ;
	setSectionLength(newSize) ;

	setVersionNumber(0) ;

	if (bApplicable)
		setCurrentNextIndicator( );
	else
		resetCurrentNextIndicator( );

	section_number = last_section_number = 0 ;

	computeCRC() ;
}
