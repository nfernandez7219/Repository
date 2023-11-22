
#include "tools2.hpp"
#include "mux.hpp"

// a = no aliasing
// g = global
// t = fast machine code
// y = no stack frame
#define OPTOPTIONS	"agty"
//#define OPTOPTIONS	"agt"


#define MuxPacketVersion	1


//--------------------------------------------------------------------------------
//	MuxPacket
//		time critical procedures are written in assembler
//--------------------------------------------------------------------------------


#pragma optimize( OPTOPTIONS, on )

void MuxPacket::computeCheckSum()
{
	__asm {
		mov		esi,[this]
		// whole MuxPacket, exclude CheckSum and PESHdrBytes if PES packets are used
		mov		ecx,MUXPACKETSIZE  - ( TYPE uchar )

// Support for PES packet - cancelled
//		cmp		[esi]MuxPacket.muxStructure._syncByte, 0xB3
//		jz		__jump1
//		sub		ecx, PESPACKETHDRSIZE
//		add		esi, PESPACKETHDRSIZE
//	}
//__jump1:
//	__asm
//	{
		xor		eax,eax					// checksum
	}
__loop1:
	__asm {
		mov		bl,byte ptr [esi]

		xor		al,bl					// checksum

		inc		esi

		loop	__loop1

		mov		esi,[this]

// Support for PES packet - cancelled
//		cmp		[USE_PESHEADER], 0
//		jz		__jump2
//		mov		[esi]MuxPacket.pesStructure._checkSum,al
//	}
//__jump2:
//	__asm
//	{
		mov		[esi]MuxPacket.muxStructure._checkSum,al
	}
}

uchar MuxPacket::dataCheckSum() const
{
	uchar c = muxStructure._checkSum ;
	c ^= ((uchar*)&muxStructure._crc16)[0] ;
	c ^= ((uchar*)&muxStructure._crc16)[1] ;

	int len = isUnicastPacket() ? MUXPACKETHDRSIZE+sizeof(UnicastUserID)-4 : MUXPACKETHDRSIZE-4 ;
	while( len >= 0 )
	{
		c ^= ((uchar*)this)[len] ;
		len-- ;
	}
	return c ;
}


// for the algorithm see usrlib\utils.cpp - crc16()
void MuxPacket::computeCrc16AndCheckSum()
{
	static ushort crctable[256] = {
	    0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
	    0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
	    0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
		0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
		0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
		0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
		0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
		0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
		0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
		0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
		0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
		0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
		0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
		0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
		0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
		0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
		0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
		0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
		0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
		0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
		0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
		0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
		0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
		0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
		0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
		0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,

		0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
		0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
		0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
		0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
		0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
		0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040
	};

	__asm {
		mov		esi,[this]
		// whole MuxPacket, exclude CheckSum and PESHdrBytes if PES packets are used
		mov		ecx,MUXPACKETSIZE  - ( TYPE ushort + TYPE uchar )

// Support for PES packet - cancelled
//		cmp		[esi]MuxPacket.muxStructure._syncByte, 0xB3
//		jz		__jump1
//		sub		ecx, PESPACKETHDRSIZE
//		add		esi, PESPACKETHDRSIZE
//	}
//__jump1:
//	__asm
//	{
		xor		eax,eax					// checksum
		xor		edx,edx					// crc
	}
__loop1:
	__asm {
		xor		ebx,ebx
		mov		bl,byte ptr [esi]

		xor		al,bl					// checksum

		xor		bx,dx					// crc
		and		ebx,0xff
		shr		dx,0x08
		xor		dx,word ptr [crctable + 2 * ebx]

		inc		esi

		loop	__loop1

		xor		al,dh
		xor		al,dl

		mov		esi,[this]

// Support for PES packet - cancelled
//		cmp		[USE_PESHEADER], 0
//		jz		__jump2
//		mov		[esi]MuxPacket.pesStructure._checkSum,al
//		mov		[esi]MuxPacket.pesStructure._crc16,dx
//	}
//__jump2:
//	__asm
//	{
		// store results
		mov		[esi]MuxPacket.muxStructure._checkSum,al
		mov		[esi]MuxPacket.muxStructure._crc16,dx
	}
}


//--------------------------------------------------------------------------------
//	MuxPacket - FEC
//--------------------------------------------------------------------------------


// Coding described in doc file fec.report

void MuxPacket::setFecInfo( uchar numBlockRows, uchar rowInd, ushort blockInd, ushort blockSize )
{
	ASSERT( rowInd <= 125 ) ;
	ASSERT( numBlockRows <= 125 ) ;

	if( numBlockRows == 2 )
		numBlockRows = 0 ;
	else
	if( numBlockRows == 4 )
		numBlockRows = 63 ;
	else
		numBlockRows = (numBlockRows-1) / 2 ;
	muxStructure._flags2 = (muxStructure._flags2 & 0x0F) | ((numBlockRows & 0x0F)<<4) ;

	muxStructure._numDataBytes = rowInd ;		// rowInd(7 bits)

	ulong packInd =
		(muxStructure._packetInd&0xFF000000) |	// preserve _jobId
		(numBlockRows>>4) ;						// 0-1; numBlockRows(upper 2 bits)

	if( blockSize == 1024  ||  blockSize == 512 )
	{
												// restFlag==0
												// 2 (smallFlag==0)
		packInd |= (blockInd<<3) ;				// 3-15
		if( blockSize == 512 )
			packInd |= (1<<16) ;				// 16 (blockSize: 0-1024, 1-512)
	}
	else
	if( blockSize > 512 )
	{
		muxStructure._numDataBytes |= (1<<7) ;	// restFlag==1
		packInd |= (blockInd<<2) ;				// 2-14
		packInd |= ((blockSize-512) << 15) ;	// 15-23
	}
	else	// blockSize < 512
	{
												// restFlag==0
		packInd |= (1<<2) ;						// 2 (smallFlag==1)
		packInd |= (blockSize << 3) ;			// 3-11
	}

	muxStructure._packetInd = packInd ;
	muxStructure._flags |= (Fec | CrcComputed) ;
	computeCrc16AndCheckSum();
}

BOOL MuxPacket::getFecInfo( uchar &numBlockRows, uchar &rowInd, ushort &blockInd, ushort &blockSize )
{
	ulong packInd = muxStructure._packetInd ;

	numBlockRows = (uchar)( (muxStructure._flags2 >> 4) + ((packInd & 0x3) << 4) ) ;
	if( numBlockRows == 0 )
		numBlockRows = 2 ;
	else
	if( numBlockRows == 63 )
		numBlockRows = 4 ;
	else
		numBlockRows = numBlockRows*2 + 1 ;

	rowInd = (muxStructure._numDataBytes & 0x7F) ;
	if( muxStructure._numDataBytes & (1<<7) )	// restFlag
	{
		blockInd  = (ushort)((packInd>> 2) & 0x1FFF) ;
		blockSize = (ushort)(((packInd>>15) & 0x1FF) + 512) ;
	}
	else
	if( packInd & (1<<2) )						// smallFlag
	{
		blockInd  = 0 ;
		blockSize = (ushort)((packInd>>3) & 0x1FF) ;
	}
	else
	{
		blockInd  = (ushort)((packInd>>3) & 0x1FFF) ;
		blockSize = (packInd & (1<<16)) ? 512 : 1024 ;
	}
	return TRUE ;
}


//--------------------------------------------------------------------------------
//	MuxPacket - other procedures
//--------------------------------------------------------------------------------

/*
void MuxPacket::xorData( const MuxPacket &srcPacket )
{
	uchar len ;
	uchar *src, *dst ;
	if( isUnicastPacket() )
	{
		dst = muxStructure.unicast._data ;
		src = (uchar*)srcPacket.muxStructure.unicast._data ;
		len = MUXPACKETDATASIZE - sizeof(UnicastUserID) ;
	}
	else
	{
		dst = muxStructure._data ;
		src = (uchar*)srcPacket.muxStructure._data ;
		len = MUXPACKETDATASIZE ;
	}
	while( 1 )
	{
		--len ;
		dst[len] ^= src[len] ;
		if( len == 0 )
			break ;
	}
}
*/

void MuxPacket::xorData( const MuxPacket &srcPacket )
{
	uint *src, *dst ;
	if( isUnicastPacket() )
	{
		dst = (uint*)muxStructure.unicast._data ;
		src = (uint*)srcPacket.muxStructure.unicast._data ;
		for( int len = MUXPACKETUNICASTDATASIZE/sizeof(uint)-1 ; len>=0 ; len-- )
			*dst++ ^= *src++ ;
	}
	else
	{
		dst = (uint*)muxStructure._data ;
		src = (uint*)srcPacket.muxStructure._data ;
		for( int len = MUXPACKETDATASIZE/sizeof(uint)-1 ; len>=0 ; len-- )
			*dst++ ^= *src++ ;
		uchar *s = (uchar*)srcPacket.muxStructure._data + (MUXPACKETDATASIZE-2) ;
		uchar *d = (uchar*)			 muxStructure._data + (MUXPACKETDATASIZE-2) ;
		d[0] ^= s[0] ;
		d[1] ^= s[1] ;
	}
}


#define setPESHdr()		{				\
	pesStructure._PESHdrBytes[0] = 0;	\
	pesStructure._PESHdrBytes[1] = 0;	\
	pesStructure._PESHdrBytes[2] = 1;	\
	pesStructure._PESHdrBytes[3] = 0xBF;\
	pesStructure._PESHdrBytes[4] = 0;	\
	pesStructure._PESHdrBytes[5] = 0xB2;\
	computeCheckSum();					\
}

void MuxPacket::makeDataPacket( ushort dvbChannel, ushort flg, int nDataBytes, int rebroadcastInd,
				ulong packetIdx, char *datas, uchar job )
{
	//memset( this, 0, sizeof(MuxPacket) ) ;
	//ASSERT( nDataBytes <= MUXDATASIZE ) ;
	uchar nBytes = (uchar)nDataBytes ;

	/*
	if( USE_PESHEADER )
	{
		setPESHdr();
		pesStructure._syncByte		= MuxPacketSyncByte;
		pesStructure._channel		= dvbChannel ;
		pesStructure._numDataBytes	= nBytes ;
		pesStructure._flags2		= (uchar)(rebroadcastInd + (MuxPacketVersion<<4)) ;
		pesStructure._flags			= flg | CrcComputed ;
		pesStructure._packetInd		= (packetIdx & 0x00FFFFFF) + (job<<24) ;
		memcpy( pesStructure._data, datas, nBytes ) ;
	}
	else
	*/
	{
		muxStructure._syncByte		= MuxPacketSyncByte;
		muxStructure._channel		= dvbChannel ;
		muxStructure._numDataBytes	= nBytes ;
		muxStructure._flags2		= (uchar)(rebroadcastInd + (MuxPacketVersion<<4)) ;
		muxStructure._flags			= flg | CrcComputed ;
		muxStructure._packetInd		= (packetIdx & 0x00FFFFFF) + (job<<24) ;
		memcpy( muxStructure._data, datas, nBytes ) ;

		// Because of FEC the unused portion of the packet must be known, too.
		// Therefore we set those Bytes to 0.
		if( nDataBytes < MUXPACKETDATASIZE )
			memset( muxStructure._data+nDataBytes, 0, MUXPACKETDATASIZE-nDataBytes ) ;
	}
	computeCrc16AndCheckSum();
}

void MuxPacket::makeUnicastPacket( ushort dvbChannel, ushort flg, uchar nDataBytes, int rebroadcastInd,
				ulong packetIdx, char *datas, const GlobalUserID& usrId, uchar job )
{
	//memset( this, 0, sizeof(MuxPacket) ) ;
	//ASSERT( nDataBytes <= MUXDATASIZE ) ;
	uchar nBytes = (uchar)nDataBytes ;

	/*
	if( USE_PESHEADER )
	{
		setPESHdr();
		pesStructure._syncByte		= MuxPacketSyncByte;
		pesStructure._channel		= dvbChannel ;
		pesStructure._numDataBytes	= nBytes ;
		pesStructure._flags2		= (uchar)(rebroadcastInd + (MuxPacketVersion<<4)) ;
		pesStructure._flags			= flg | Unicast | CrcComputed ;
		pesStructure._packetInd		= (packetIdx & 0x00FFFFFF) + (job<<24) ;
		pesStructure.unicast._userId= usrId ;
		memcpy( pesStructure.unicast._data, datas, nBytes ) ;
	}
	else
	*/
	{
		muxStructure._syncByte		= MuxPacketSyncByte;
		muxStructure._channel		= dvbChannel ;
		muxStructure._numDataBytes	= nBytes ;
		muxStructure._flags2		= (uchar)(rebroadcastInd + (MuxPacketVersion<<4)) ;
		muxStructure._flags			= flg | Unicast | CrcComputed ;
		muxStructure._packetInd		= (packetIdx & 0x00FFFFFF) + (job<<24) ;
		muxStructure.unicast._userId= usrId ;
		memcpy( muxStructure.unicast._data, datas, nBytes ) ;

		// Because of FEC the unused portion of the packet must be known, too.
		// Therefore we set those Bytes to 0.
		if( nDataBytes < MUXPACKETUNICASTDATASIZE )
			memset( muxStructure._data+nDataBytes, 0, MUXPACKETUNICASTDATASIZE-nDataBytes ) ;
	}
	computeCrc16AndCheckSum();
}

#pragma optimize( "", on )			// restore original optimization options


int MuxPacket::makeCommandPacket( ushort command, char *datas, int nDataBytes,
				ulong packetIdx, uchar job )
{
	memset( this, 0, sizeof(MuxPacket) ) ;
	muxStructure._syncByte = MuxPacketSyncByte;
	uchar nBytes = (uchar)( nDataBytes <= MUXPACKETDATASIZE ? nDataBytes : MUXPACKETDATASIZE );
	muxStructure._numDataBytes = nBytes;
	muxStructure._flags = Ctrl;
	muxStructure._packetInd = (packetIdx & 0x00FFFFFF) + (job<<24) ;
	if ( nBytes )
		memcpy( (void*)data(), datas, nBytes ) ;
	muxStructure._crc16 = command;
	computeCheckSum();
	return nBytes;
}

void MuxPacket::makeFillPacket()
{
	memset( this, 0, sizeof(MuxPacket) );
	//if( USE_PESHEADER )
	//	setPESHdr();
	muxStructure._syncByte = MuxPacketSyncByte;
	muxStructure._flags = Fill;
}

#pragma optimize( OPTOPTIONS, on )

BOOL MuxPacket::isFillPacketOk()
{
	/*
	if ( isPESPacket() )
	{
		if ( !USE_PESHEADER )
			return FALSE ;

		BOOL ok = pesStructure._syncByte == MuxPacketSyncByte;
		ok = ok && (pesStructure._flags == Fill);
		if ( ok )
		{
			pesStructure._syncByte = 0;
			pesStructure._flags = 0;
			char *packet = (char*)this;
			int sum = 0;
			for ( int i = PESPACKETHDRSIZE ; i < sizeof(MuxPacket); i++ )
				sum += packet[i];
			ok = ok && (sum == 0);
			pesStructure._syncByte = MuxPacketSyncByte;
			pesStructure._flags = Fill;
		}
		return ok;
	}
	else
	*/
	{
		//if ( USE_PESHEADER )
		//	return FALSE ;
		if( muxStructure._syncByte		!= MuxPacketSyncByte  ||
			muxStructure._channel		!= 0  ||
			muxStructure._numDataBytes	!= 0  ||
			muxStructure._flags2		!= 0  ||
			muxStructure._flags			!= Fill  ||
			muxStructure._packetInd		!= 0 )
			return FALSE ;

		uchar *p = muxStructure._data ;
		for( int i=MUXPACKETDATASIZE ; i > 0 ; i--, p++ )
			if( *p != 0 )
				return FALSE ;
		return TRUE;
	}
}

BOOL MuxPacket::isCrcAndCheckSumOk()
{
	uchar savedCheckSum = checkSum();

	if( flags() & CrcComputed )
	{
		ushort savedCrc16 = crc16();
		computeCrc16AndCheckSum();
		if( savedCrc16 != crc16() )
		{
			#ifdef _DUMP_RECEIVED_BAD_PACKETS
				int flag = 0x01;				// bad crc
				if( savedCheckSum != checkSum() )
				{
					flag |= 0x02;				// bad crc
					crc16() = savedCrc16;
					checkSum() = savedCheckSum;
				}
				else
					crc16() = savedCrc16;
				DUMP_RCVPACKET( this, flag );
			#endif
			return FALSE ;
		}
	}
	else
		computeCheckSum();

	if( savedCheckSum == checkSum() )
		return TRUE ;

	#ifdef _DUMP_RECEIVED_BAD_PACKETS
		checkSum() = savedCheckSum;
		DUMP_RCVPACKET( this, 0x02 );				// bad checksum
	#endif

	return FALSE ;
}

BOOL MuxPacket::isCheckSumOk()
{
	uchar savedCheckSum  = checkSum();
	computeCheckSum();
	#ifdef _DEBUG
		if( savedCheckSum != checkSum() )
		{
			muxStructure._checkSum = savedCheckSum;
			DUMP_RCVPACKET( this, 0x02 );				// bad checksum
			return FALSE;
		}
	#endif
	return savedCheckSum == checkSum();
}



//-------------------------------------------------------//
//	Multiprotocol Encapsulation (DSMCC section)
//-------------------------------------------------------//


DSMCC_section::DSMCC_section( ) 
{
	table_id = 0x3E ;
	scrambling_and_snap_flags = 0xC1 ;
	checksum = 0 ;

	// These variables are later redefined in setHeader()
	section_length[0] = 0x70 ;
	section_length[1] = MPE_HEADERSIZE - 3 + 4 ;
	
	MAC_address_6 = 
	MAC_address_5 = 
	MAC_address_4 = 
	MAC_address_3 = 
	MAC_address_2 =
	MAC_address_1 = 0 ;

	section_number = 0 ;
	last_section_number = 0;
}

void DSMCC_section::setHeader( ushort ipDataSize, __int64 _usrId, 
							 uchar sectionNumber, uchar lastSectionNumber )
{
	ASSERT(ipDataSize <= MAX_IP_DATA_SIZE) ;

	ushort sectionLength = ipDataSize + MPE_HEADERSIZE - 3 + 4 ;
	section_length[1] = sectionLength & 0xFF ;
	section_length[0] = 0x70 | (sectionLength>>8) ;
	
	section_number		= sectionNumber ;
	last_section_number	= lastSectionNumber;

	MAC_ADDRESS *mac = (MAC_ADDRESS*)&_usrId ;
	MAC_address_6 = (*mac)[5] ;
	MAC_address_5 = (*mac)[4] ;
	MAC_address_4 = (*mac)[3] ;
	MAC_address_3 = (*mac)[2] ;
	MAC_address_2 =	(*mac)[1] ;
	MAC_address_1 = (*mac)[0] ;

	// compute checksum or crc - optional
}

//-------------------------------------------------------//
//	Transport stream (TS packet)
//-------------------------------------------------------//

int TsPacket::createSection( const uchar *dataPtr, int size, PidStreamAttrib *pidStrAttr )
{
	// first packet of the section
	//  - payload_unit_start_indicator is set
	//  - next section pointer (zero byte) inserted before section
	sync = TsPacketSyncByte ;
	pid = pidStrAttr->getConvertedPid();
	setPayloadUnitStart() ;
	flags = 0x10 | pidStrAttr->getContinuityCounter() ;

	char *tpData = (char*)&data ;
	*tpData = '\x0' ; // next section pointer
	if (size >= TSPAYLOAD_SIZE-1)
	{
		memcpy(tpData+1, dataPtr, TSPAYLOAD_SIZE-1) ;
		return TSPAYLOAD_SIZE-1 ;
	}
	else
	{
		memcpy(tpData+1, dataPtr, size) ;
		memset(tpData+1+size, 0xFF, TSPAYLOAD_SIZE-(1+size)) ;
		return size ;
	}
}

int TsPacket::addSection( const uchar *dataPtr, uchar &offset, int size )
{
	uchar *dataStart = (uchar*)&data ;

	if (!isPayloadUnitStart())
	{
		setPayloadUnitStart() ;

		//  append next section pointer before the data
		memmove(dataStart+1, dataStart, offset) ;
		*dataStart = offset++ ;
	}

	size = __min(size, TSPAYLOAD_SIZE-offset) ;
	memcpy(dataStart+offset, dataPtr, size) ;

	offset += size ;
	return size ;
}
/*
void TsPacket::createFromMpeSection( DSMCC_section *s, ushort _pid )
{
	int sectionLength = s->getTotalLength() ;
	uchar *dataPtr = (uchar*)s ;
	ushort convertedPID = ((_pid&0x1F00) >> 8) | ((_pid&0xFF)<<8) ;

	// first packet of the section
	//  - payload_unit_start_indicator is set
	//  - next section pointer (zero byte) inserted before section
	sync = TsPacketSyncByte ;
	pid = pidStrAttr->getConvertedPid();
	setPayloadUnitStart() ;
	flags = 0x10 | pidStrAttr->getContinuityCounter() ;

	char *tpData = (char*)&data ;
	tpData[0] = '\x0' ; // next section pointer
	if (sectionLength >= TSPAYLOAD_SIZE-1)
		memcpy(tpData+1, dataPtr, TSPAYLOAD_SIZE-1) ;
	else
	{
		memcpy(tpData+1, dataPtr, sectionLength) ;
		memset(tpData+1+sectionLength, 0xFF, TSPAYLOAD_SIZE-(1+sectionLength)) ;
	}

	TsPacket *p = this+1 ;
	sectionLength -= TSPAYLOAD_SIZE-1 ;
	dataPtr += TSPAYLOAD_SIZE-1 ;

	while( sectionLength>0 )
	{
		p->sync  = TsPacketSyncByte ;
		pid = pidStrAttr->getConvertedPid();
		flags = 0x10 | pidStrAttr->getContinuityCounter() ;

		tpData = (char*)&p->data ;
		if (sectionLength >= TSPAYLOAD_SIZE)
			memcpy(tpData, dataPtr, TSPAYLOAD_SIZE) ;
		else
		{
			memcpy(tpData, dataPtr, sectionLength) ;
			memset(tpData+sectionLength, 0xFF, TSPAYLOAD_SIZE-sectionLength) ;
		}

		sectionLength -= TSPAYLOAD_SIZE ;
		dataPtr += TSPAYLOAD_SIZE ;
		++p ;
	}
}
*/
#pragma optimize( "", on )			// restore original optimization options

PidStreamAttrib::PidStreamAttrib( ushort pid )
{
	_pid=pid&0x1FFF;
	_convertedPid = _pid>>8 | _pid<<8 ;
	_continuityCounter=0; 
	_nReferences=1; 
}

//---------------------------------------------------------------------------------------------//
//	PidStreamAttribManager
//
// uses PidStreamAttrib so it must be located here 
//---------------------------------------------------------------------------------------------//

PidStreamAttribManager::~PidStreamAttribManager()
{
	int nStreamAttribs = _streamAttribs.count() ;
	while(nStreamAttribs--)
	{
		PidStreamAttrib *streamAttrib = _streamAttribs[nStreamAttribs] ;
		if (streamAttrib->_nReferences > 0)
			TRACE("PidStreamAttribManager error: PidStreamAttrib(%x) still in use (%d references).", streamAttrib->_pid, streamAttrib->_nReferences);
		else
			delete streamAttrib ;
	}
}

PidStreamAttrib *PidStreamAttribManager::getPidStreamAttrib( ushort pid )
{
	PidStreamAttrib *streamAttrib ;
	int i = _streamAttribs.count() ;
	while(i--)
	{
		streamAttrib = _streamAttribs[i];
		if (streamAttrib->_pid==pid)
		{
			InterlockedIncrement(&streamAttrib->_nReferences);
			return streamAttrib ;
		}
	}

	streamAttrib = new PidStreamAttrib(pid) ;
	_streamAttribs.add(streamAttrib) ;
	return streamAttrib ;
}

void PidStreamAttribManager::releasePidStreamAttrib ( PidStreamAttrib *streamAttrib )
{
	long nRef = InterlockedDecrement(&streamAttrib->_nReferences) ;
/*	if (nRef==0)
	{
		_streamAttribs.delObj(streamAttrib) ;
		delete streamAttrib ;
	}
*/
}
