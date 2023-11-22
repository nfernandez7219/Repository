#ifndef __INC_MUXPACKET_HPP__
#define __INC_MUXPACKET_HPP__

#ifndef __SERIALNUM_HPP__
#include "SerialNum.hpp"
#endif

extern BOOL	USE_PESHEADER;

#define MUXPACKETSIZE		184
#define MUXPACKETHDRSIZE	14
#define MUXPACKETDATASIZE	(MUXPACKETSIZE-MUXPACKETHDRSIZE)
#define MUXPACKETUNICASTDATASIZE	(MUXPACKETDATASIZE-sizeof(UnicastUserID))

#define PESPACKETSIZE		184
#define PESPACKETHDRSIZE	6
#define PESPACKETDATASIZE	(PESPACKETSIZE-PESPACKETHDRSIZE)

#define	MUXDATASIZE			(USE_PESHEADER ? PESPACKETDATASIZE - MUXPACKETHDRSIZE : MUXPACKETDATASIZE )

#define TSPACKET_SIZE		188
#define TSPAYLOAD_SIZE		184

#define isPESPacket()		USE_PESHEADER


#pragma pack(1)


#define MuxPacketSyncByte	(uchar)0xB3
#define TsPacketSyncByte	(uchar)0x47

struct MuxPacket
{
	friend void dumpPacket( MuxPacket *mp, int why );

  private:
	union
	{
		//------------ Mux ---------------------
		struct
		{
			// header = MUXPACKETHDRSIZE By (incl. CRC and checksum)
			uchar	_syncByte ;			// 0xB3
			ushort 	_channel ;			// 0 = service channel
			uchar	_numDataBytes ;		// length of 'data' member (different meaning for FEC)
			uchar	_flags2 ;			// upper 4 bits= version,
										// lower 4 bits= rebroadcast index (...,3,2,1 = last or 0=infinite broadcast)
			ushort	_flags ;			// bits from enum Flags
			ulong	_packetInd ;		// 0,1,...  indexing within job (wraps around 0); M.S.B. = jobId

			union
			{
				uchar	_data[MUXPACKETDATASIZE] ;
				struct
				{
					UnicastUserID	_userId ;
					uchar			_data[MUXPACKETUNICASTDATASIZE] ;
				} unicast ;
				struct
				{
					UnicastUserID	_userId ;
					uchar			_data[MUXPACKETUNICASTDATASIZE] ;
				} internet ;
			} ;

			ushort _crc16 ;				// computed for the data before _crc16
			uchar  _checkSum ;			// computed for the data before _checkSum (incl. _crc16)
		}
		muxStructure;

		//------------ PES ---------------------
		struct
		{
			uchar	_PESHdrBytes[PESPACKETHDRSIZE];

			// header = MUXPACKETHDRSIZE By (incl. CRC and checksum)
			uchar	_syncByte ;			// 0xB3
			ushort 	_channel ;			// 0 = service channel
			uchar	_numDataBytes ;		// length of 'data' member
			uchar	_flags2 ;			// upper 4 bits= version,
										// lower 4 bits= rebroadcast index (...,3,2,1 = last or 0=infinite broadcast)
			ushort	_flags ;			// bits from enum Flags
			ulong	_packetInd ;		// 0,1,...  indexing within job (wraps around 0); M.S.B. = jobId

			union
			{
				uchar	_data[MUXPACKETDATASIZE-PESPACKETHDRSIZE] ;
				struct
				{
					UnicastUserID	_userId ;
					uchar			_data[MUXPACKETDATASIZE-PESPACKETHDRSIZE-sizeof(UnicastUserID)] ;
				} unicast ;
				struct
				{
					UnicastUserID	_userId ;
					uchar			_data[MUXPACKETDATASIZE-PESPACKETHDRSIZE-sizeof(UnicastUserID)] ;
				} internet ;
			} ;

			ushort _crc16 ;				// computed for the data before _crc16
			uchar  _checkSum ;			// computed for the data before _checkSum (incl. _crc16)
		}
		pesStructure;
		//--------------------------------------
	} ;

  public:
	void	computeCrc16AndCheckSum();
	void	computeCheckSum();

	enum Flags {
		// broadcast flags; !Unicast && !Multicast means broadcast
		Unicast		=0x01,
		Multicast	=0x02,

		// attributes
		Crypted		=0x04,
		AliveSignal	=0x08,
		CrcComputed =0x10,
		Fec			=0x1000,	// Forward error correction; used with File packets

		// jobs
		Internet	=0x20,		// stream of Internet packets for 1 HNET request
		Message		=0x40,		// message to the client
		UserLog		=0x80,		// user permissions (unicast)
		UserTable	=0x100,		// table of permissions for all users (broadcast)
		Upgrade		=0x200,		// upgrade of the receiver SW
		File		=0x400,		// sending a file

		// others
		Ctrl		=0x800,		// for communication with the driver
		Fill		=0x8000,	// for card synchronization
	} ;

	inline uchar	 syncByte			() const		{ return muxStructure._syncByte ;		}
	inline ushort	 channel			() const		{ return muxStructure._channel ;		}
	inline uchar	 numDataBytes		() const		{ return muxStructure._numDataBytes ; }
	inline uchar	 flags2				() const		{ return muxStructure._flags2 ;			}
	inline ushort	 flags				() const		{ return muxStructure._flags ;			}
	inline void		 setFlags			( ushort f )	{ muxStructure._flags = f ;				}

	inline ulong	 packetInd			() const		{ return muxStructure._packetInd ;		}
	inline void		 setPacketInd		( int packetIdx )	// preserves old JobId
	{
		muxStructure._packetInd = (packetIdx & 0x00FFFFFF) + (muxStructure._packetInd & 0xFF000000) ;
	}

	inline void		 setDataLength		( int len )
	{
		muxStructure._numDataBytes = len ;
	}

	// unicast || internet only
	inline UnicastUserID	userId		()				{ return muxStructure.unicast._userId ;	}

	inline const uchar *data			() const
	{
		return (muxStructure._flags & (Unicast|Internet)) ? muxStructure.unicast._data : muxStructure._data ;
	}
	inline ushort	 crc16				() const		{ return muxStructure._crc16 ;			}
	inline uchar	 checkSum			() const		{ return muxStructure._checkSum ;		}

	// Computes data check sum out of previously computed packet check sum.
		   uchar	 dataCheckSum		() const ;

	/*
	inline uchar	&syncByte			() const		{ return isPESPacket() ? pesStructure._syncByte		: muxStructure._syncByte ;		}
	inline ushort	&channel			() const		{ return isPESPacket() ? pesStructure._channel		: muxStructure._channel ;		}
	inline uchar	&numDataBytes		() const		{ return isPESPacket() ? pesStructure._numDataBytes	: muxStructure._numDataBytes ;	}
	inline uchar	&flags2				() const		{ return isPESPacket() ? pesStructure._flags2		: muxStructure._flags2 ;		}
	inline ushort	&flags				() const		{ return isPESPacket() ? pesStructure._flags		: muxStructure._flags ;			}
	inline ulong	&packetInd			() const		{ return isPESPacket() ? pesStructure._packetInd	: muxStructure._packetInd ;		}

	// unicast || internet only
	inline UnicastUserID	&userId		()				{ return isPESPacket() ? pesStructure.unicast._userId: muxStructure.unicast._userId ;	}

	inline const uchar *data			() const
	{
		if ( isPESPacket() )
			return (flags() & (Unicast|Internet)) ? pesStructure.unicast._data : pesStructure._data ;
		else
			return (flags() & (Unicast|Internet)) ? muxStructure.unicast._data : muxStructure._data ;
	}
	inline ushort	&crc16				() const		{ return isPESPacket() ? pesStructure._crc16			: muxStructure._crc16 ;			}
	inline uchar	&checkSum			() const		{ return isPESPacket() ? pesStructure._checkSum		: muxStructure._checkSum ;		}
	*/

	inline int		 version			() const		{ return flags2()/16 ; }
	inline int		 rebroadcastIndex	() const		{ return ((unsigned int)flags2()) & 0x0f ; }
	inline int		 packetIndex		() const		{ return packetInd() & 0x00FFFFFF ; }
	inline uchar	 jobId				() const		{ return (uchar)(packetInd() >> 24) ; }

	inline BOOL		isUnicastPacket		() const		{ return flags() & Unicast ; }
	inline BOOL		isInternetPacket	() const		{ return flags() & Internet; }
	inline BOOL		isMessagePacket		() const		{ return flags() & Message ; }
	inline BOOL		isUserLogPacket		() const		{ return flags() & UserLog ; }
	inline BOOL		isUserTablePacket	() const		{ return flags() & UserTable ; }
	inline BOOL		isUpgradePacket		() const		{ return flags() & Upgrade ; }
	inline BOOL		isFilePacket		() const		{ return flags() & File    ; }
	inline BOOL		isAliveSignalPacket	() const		{ return flags() & AliveSignal ; }
	inline BOOL		isCommandPacket		() const		{ return flags() & Ctrl ; }
	inline BOOL		isFillPacket		() const		{ return flags() == Fill ; }

	void makeDataPacket(
			ushort dvbChannel,			// channel used for sending (0 for service channel)
			ushort flg,					// job flag from enum Flags
			int    nDataBytes,			// max. MUXPACKETDATASIZE
			int    rebroadcastInd,		// rebroadcast index 0,1...
			ulong  packetIdx,			// packet index within job
			char  *data,				// data[nDataBytes]
			uchar  job=0				// optional jobId
		) ;

	void makeUnicastPacket(
			ushort dvbChannel,			// channel used for sending (0 for service channel)
			ushort flg,					// job flag from enum Flags
			uchar  nDataBytes,			// max. MUXPACKETDATASIZE
			int    rebroadcastInd,		// rebroadcast index 0,1...Ind,
			ulong  packetIdx,			// packet index within job
			char  *datas,				// data[nDataBytes]
			const  GlobalUserID& usrId,	// user which will receive this packet
			uchar  job=0				// optional jobId
		) ;

	// return # bytes written from datas to this packet
	int  makeCommandPacket(
			ushort command,
			char  *datas,
			int    nDataBytes,
			ulong  packetIdx=0,
			uchar  job=0
		) ;

	void makeFillPacket() ;

	void xorData( const MuxPacket &src ) ;

	BOOL isFillPacketOk		() ;
	BOOL isCrcAndCheckSumOk	() ;
	BOOL isCheckSumOk		() ;

	void setFecInfo( 
		uchar  numBlockRows,		// 2..125
		uchar  rowInd,				// 0..124
		ushort blockInd,			// from file start; in multiples of 512
		ushort blockSize			// 1..1024
	) ;
	BOOL getFecInfo(				// FALSE if FEC packet invalid
		uchar  &numBlockRows,		//
		uchar  &rowInd,				//
		ushort &blockInd,			//
		ushort &blockSize			//
	) ;
} ;

//---------------------------------------------------------------------------------//
//	PID stream attributes
//
// Contains attributes of the PID stream necessary to ensure PID stream continuity.
// Each TS packet have "continuity counter" one bigger (modulo 16) than previous
// TS packet with the same PID.
//---------------------------------------------------------------------------------//

class PidStreamAttrib
{
	friend class PidStreamAttribManager ;

	ushort	_pid ;					// PID of the stream
	ushort	_convertedPid ;			// PID with swapped bytes (as it is in the TS packet header)
	uchar	_continuityCounter ;	// number of the next TS packet in the PID stream
	long	_nReferences ;			// number of channels which referenced this class

  public:
	PidStreamAttrib( ushort pid ) ;

	inline uchar	getContinuityCounter ()	{ return (_continuityCounter++)&0x0F; }
	inline ushort	getConvertedPid		 () { return _convertedPid ; }
	inline ushort	pid					 () { return _pid ; }
} ;


			//////////////////////////////////////////////
			//	Transport stream packet - Data piping	//
			//////////////////////////////////////////////


struct DSMCC_section ;

struct TsPacket
{
	uchar	sync ;		// 0x47

	// transport_error_indicator:1=0
	// payload_unit_start_indicator:1=0
	// transport_priority:1=0
	// pid:13 = pid
	ushort	pid ;

	// transport_scrambling_control:2=0
	// AF:1 = 0
	// data:1 = 1
	// counter:4=0,1,2... incremented for subsequent packets with the same PID
	uchar	flags ;

	MuxPacket data ;

	int dataLength()
	{
		if( flags & 0x20)
			return 0 ;				// simplification
		return sizeof(MuxPacket) ;
	}

	void create( const uchar *mp, PidStreamAttrib *pidStrAttr, size_t size=TSPAYLOAD_SIZE )
	{
		sync = TsPacketSyncByte ;
		pid = pidStrAttr->getConvertedPid();
		flags = 0x10 | pidStrAttr->getContinuityCounter() ;
		memcpy( &data, mp, size );
	}

	// Creates the TS packet containing DSMCC section
	// Sets "Payload unit start" flag and adds section start pointer
	// If the section is smaller than payload size,
	// rest of the packet is filled with 0xFF stuffing bytes
	int createSection	( const uchar *dataPtr, int size, PidStreamAttrib *pidStrAttr ) ;

	// Adds the DSMCC section into packet
	// Use only if packet has enough size to contain the section header
	// You must know offset where the data ends in the TS packet
	// If not already set addSection sets "Payload unit start" flag
	// and adds section start pointer
	int addSection		( const uchar *dataPtr, uchar &offset, int size ) ;

	TsPacket( )		
	{ 
		sync = TsPacketSyncByte ;
		pid = 0;
		flags = 0x10 ;
	}

	TsPacket( const uchar *mp, PidStreamAttrib *pidStrAttr )
	{
		create( mp, pidStrAttr ) ;
	}

	inline isPayloadUnitStart	( )								{ return pid&0x40 ; }
	inline setPayloadUnitStart	( )								{ pid = 0x40|pid ; }
	inline isCompatible			( PidStreamAttrib *pidStrAttr )	{ return (pid&0xFF1F)==pidStrAttr->getConvertedPid() ; }
} ;


		//////////////////////////////////////////////////
		//	Multiprotocol Encapsulation	- DSMCC section	//
		//////////////////////////////////////////////////


typedef unsigned char MAC_ADDRESS[6];

#define MPE_HEADERSIZE		12		// does not contain data, LLC_SNAP and checksum
#define MAX_SECTIONLENGTH   0x0FFF	// maximal value of section_length

// maximal size of the IP datagram which could be contained in 1 DSMCC section
// If the datagram is bigger it can be contained in more than 1 section
#define MAX_IP_DATA_SIZE		(MAX_SECTIONLENGTH - MPE_HEADERSIZE + 3 - 4)

#define LLC_SNAP_HEADER_SIZE	8
#define MAX_LLC_SNAP_DATA		(MAX_IP_DATA_SIZE - LLC_SNAP_HEADER_SIZE)

// DMSCC section according to official ISO/IEC 13818-6 standard using LLC/SNAP
// encapsulation for IP datagrams:
struct LLC_SNAP
{
	uchar DSAP;				// 0xaa - K1
	uchar SSAP;				// 0xaa - K1
	uchar LLC_Control;		// 0x03 unnumbered information
	uchar ProtocolID[3];	// big endian network order, but we are using only 0 as value
	ushort EtherType;		// this again works only for specianl value 0x008
							// which in reality means 0x800 as IP
	char datagram[MAX_LLC_SNAP_DATA];
};

/*
DSMCC_section is embedded in the MPEG-2 Transport Stream using private section syntax.
One TS packet can contain more DSMCC_section's and one DSMCC_section can span over
more TS packets.
Portion of the TS packet past the end of the DSMCC_section may be used either for the
next DSMCC_section or filled by the stuffing Bytes (0xFF).

When the payload of the Transport Stream packet contains PSI data (or private streams of stream type 5):
If payload_unit_start_indicator==1 then 1st By of the payload is pointer containing offset
of 1st Byte of the PSI section. If indicator==0, there is no pointer.

payload of the TS packet:
	[pointer]			Optional: # of Bytes following this field until 1st section starts.
						Present iff TS_packet.payload_unit_start_indicator==1.
	DSMCC_section(s)...
	[stuffing]			0xFF Bytes (any other value is interpreted as next section)

Remark:
	For sect_syntax_indicator==1 different format of DSMCC_section is used.
	This fact will be for the time being ignored.
*/

// scrambling_and_snap_flags
#define SCRSNP_LLC_SNAP			2
#define SCRSNP_CURRENT_NEXT		1

struct DSMCC_section
{
	//---------------- DATA ------------------------------------------
	//
	// References:
	//		[1] ISO/IEC 13 818-6
	//		[2] EN 301 192 v1.1.1; Digital Video Broadcasting; DVB specification for data broadcasting
	//
	// Abbreviations:
	//		uimsbf = unsigned int, most significant bit first
	//				 I.e. 1 Byte confirms to Intel, for short, int etc. reverse order of Bytes
	//		bslbf  = bit string, left bit first

	// table_id						B	uimsbf	... 0x3e ... (DSM CC section with private data [1])
	uchar	table_id;

	// sect_syntax_indicator		b1	bslbf	... 0 [1] ?
	// private_indicator			b1	bslbf	... 1 [1] ?
	// reserved						b2	bslbf	... "11" [2]
	// section_length				b12	uimsbf
	// Previous fields are joined into 1 short:
	//		section_length[0] = 0x70 + ((ushort)length>>8) ;
	//		section_length[1] = (ushort)length & 0xff ;
	// - section_length contains length of the whole section (with IP data and checksum field) 
	//   except table_id and section_length fields.
	//   Thus you must add 3 to get the total size in bytes.
	uchar	section_length[2];
	
	// MAC_address_i				B	uimsbf
	uchar	MAC_address_6 ;
	uchar	MAC_address_5 ;

	// reserved						b2	bslbf	... "11"
	// payload_scrambling_control	b2	bslbf	... "00" for unscrambled [2]
	// address_scrambling_control	b2	bslbf	... "00" for unscrambled [2]
	// LLC_SNAP_flag				b1	bslbf	... "0" if IP datagram is carried [2]
	// current_next_indicator		b1	bslbf	... "1"
	// Previous fields are packed into the following:
	uchar	scrambling_and_snap_flags;			// 0xC1
	
	// section_number				B	uimsbf	... 0,1,...last_section_number
	// last_section_number			B	uimsbf
	// These items allow for fragmentation of the IP datagram.
	// If IP datagram is not fragmented, both members are 0.
	uchar	section_number;
	uchar	last_section_number;

	// MAC_address_i				B	uimsbf
	uchar MAC_address_4;
	uchar MAC_address_3;
	uchar MAC_address_2;
	uchar MAC_address_1;			// most significant Byte

	union {
		LLC_SNAP llc_snap;						// if( LLC_SNAP_flag == 1 )
		struct {
			char IP_datagram_data_bytes[MAX_IP_DATA_SIZE];	// else
		};
	};

	// Here stuffing Bytes may be inserted. Value not specified.
	// Number of Bytes should be derived from data alignment requirements 
	// defined in the data_broadcast_descriptor (?).

	// Calculated over the whole datagram section [1]
	// This field is actually not here but follows the data bytes immediately.
	// The correct offset for checksum is section_length.
	union {
		uint checksum;		// if( sect_syntax_indicator==0)
		uint CRC_32;		// otherwise
	};


	// constructor fills the section with default values (no IP data and no MAC address)
	DSMCC_section		( ) ;

	// Creates header information
	// and sets the MAC address from the _usrId
	void setHeader	( ushort ipDataSize, __int64 _usrId, 
					  uchar sectionNumber=0, uchar lastSectionNumber=0 ) ;

	// return the total size of the section with data and checksum field
	// Value is computed from the section_length field
	inline ushort	getTotalLength	( )		{ return 3 + (section_length[1] | (((ushort)(section_length[0]&0x0F))<<8)); }

	// returns TRUE if the LLC_SNAP structure is used
	inline BOOL		usingLLC_SNAP	( )		{ return (scrambling_and_snap_flags & SCRSNP_LLC_SNAP) != 0 ; }

	inline void *data()		{ return IP_datagram_data_bytes ; }
};

#pragma pack()


#endif
