
//------------------------------------------------------------------------------------//
//	PSI/SI tables header
//  --------------------
//
// Version		1.00
// Author		J. Janosik
// Created		24. 1. 2001
// Modified
// Description	PSI/SI tables definition
//
//------------------------------------------------------------------------------------//

#ifndef __INC_TABLES_H__
#define __INC_TABLES_H__

#ifndef __INC_DESCRIPTORS_H__
	#include "Descriptors.h"
#endif

//////////////////////
// PSI/SI Tables PIDs

#define TABLE_PID_PAT			0x0000
#define TABLE_PID_CAT			0x0001
#define TABLE_PID_TSDT			0x0002
#define TABLE_PID_NIT			0x0010
#define TABLE_PID_SDT			0x0011
#define TABLE_PID_BAT			0x0011
#define TABLE_PID_EIT			0x0012
#define TABLE_PID_RST			0x0013
#define TABLE_PID_TDT			0x0014
#define TABLE_PID_TOT			0x0014
#define TABLE_PID_DIT			0x001E
#define TABLE_PID_SIT			0x001F

//////////////////////
// PSI/SI Tables IDs

#define TABLE_ID_PAT			0x00
#define TABLE_ID_CAT			0x01	// conditional_access_section
#define TABLE_ID_PMT			0x02	// program_map_section
#define TABLE_ID_TSDT			0x03	// transport_stream_description_section
#define TABLE_ID_NIT			0x40	// network_information_section - actual_network
#define TABLE_ID_NIT_OTHER_NET	0x41	// network_information_section - other_network
#define TABLE_ID_SDT			0x42	// service_description_section - actual_transport_stream
#define TABLE_ID_SDT_OTHER_TS	0x46	// service_description_section - other_transport_stream
#define TABLE_ID_BAT			0x4A	// bouquet_association_section
#define TABLE_ID_EIT			0x4E	// event_information_section - actual_transport_stream, present/following
#define TABLE_ID_EIT_OTHER_TS	0x4F	// event_information_section - other_transport_stream, present/following
//0x50 to 0x5F event_information_section - actual_transport_stream, schedule
//0x60 to 0x6F event_information_section - other_transport_stream, schedule
#define TABLE_ID_TDT			0x70	// time_date_section
#define TABLE_ID_RST			0x71	// running_status_section
#define TABLE_ID_ST				0x72	// stuffing_section
#define TABLE_ID_TOT			0x73	// time_offset_section
#define TABLE_ID_DIT			0x7E	// discontinuity_information_section
#define TABLE_ID_SIT			0x7F	// selection_information_section


struct PrivateSection
{
	//---------------- DATA ------------------------------------------
	//
	// References:
	//		[1] ISO/IEC 13818-1
	//		[2] EN 300 468; Digital Video Broadcasting; DVB specification for data broadcasting
	//
	// Abbreviations:
	//		uimsbf = unsigned int, most significant bit first
	//				 I.e. 1 Byte confirms to Intel, for short, int etc. reverse order of Bytes
	//		bslbf  = bit string, left bit first

	// table_id						B	uimsbf
	uchar				table_id;

	// sect_syntax_indicator		b1	bslbf
	// private_indicator			b1	bslbf
	// reserved						b2	bslbf	... "11" [2]
	// section_length				b12	uimsbf
	// Previous fields are joined into 1 short:
	//		section_length[0] = 0x70 + ((ushort)length>>8) ;
	//		section_length[1] = (ushort)length & 0xff ;
	// - section_length contains length of the whole section (with IP data and checksum field) 
	//   except table_id and section_length fields.
	//   Thus you must add 3 to get the total size in bytes.
	uchar		section_length[2];

	ushort		table_id_extension ;
	
	// reserved						b2	bslbf	...  [2]
	// version_number				b5	bslbf	...  [5]
	// current_next_indicator		b1	bslbf	... 0 [1] ?
	uchar		version_number ;

	// section_number				B	uimsbf	... 0,1,...last_section_number
	// last_section_number			B	uimsbf
	// These items allow for fragmentation of the IP datagram.
	// If IP datagram is not fragmented, both members are 0.
	uchar		section_number;
	uchar		last_section_number;


	// return the total size of the section with data and checksum field
	// Value is computed from the section_length field
	inline ushort	getTotalLength		( )		{ return 3 + (section_length[1] | (((ushort)(section_length[0]&0x0F))<<8)); }
	inline void		setSectionLength	( ushort totalLength )	
	{
		totalLength -= 3 ;
		section_length[1] = (uchar)totalLength ;
		// clear old bits
		section_length[0] &= 0xF0 ;
		// set new bits
		section_length[0] |= (uchar)(0x0F & (totalLength>>8)) ;
	}

	inline uchar	getVersionNumber	( )					{ return (version_number>>1) & 0x1F; }
	inline void		setVersionNumber	( uchar version )	{ version_number &= 0xC1; version_number |= (version & 0x1F) << 1; }

	inline BOOL		getSectSyntaxIndicator	( )				{ return (section_length[0] & 0x80) != 0 ; }
	inline BOOL		getPrivateIndicator		( )				{ return (section_length[0] & 0x40) != 0 ; }
	inline BOOL		getCurrentNextIndicator	( )				{ return (version_number & 0x01) != 0 ; }

	inline void		setSectSyntaxIndicator	( )				{ section_length[0] |= 0x80 ; }
	inline void		setPrivateIndicator		( )				{ section_length[0] |= 0x40 ; }
	inline void		setCurrentNextIndicator	( )				{ version_number |= 0x01 ; }

	inline void		resetSectSyntaxIndicator	( )			{ section_length[0] &= ~0x80 ; }
	inline void		resetPrivateIndicator		( )			{ section_length[0] &= ~0x40 ; }
	inline void		resetCurrentNextIndicator	( )			{ version_number &= ~0x01 ; }

		   void		computeCRC					( )	;
} ;

struct BaseTable
{
	inline	ushort	getTotalLength		( )						{ return ((PrivateSection*)this)->getTotalLength() ; }
	inline	void	setSectionLength	(ushort totalLength)	{ ((PrivateSection*)this)->setSectionLength(totalLength) ; }

	inline	uchar	getVersionNumber	( )						{ return ((PrivateSection*)this)->getVersionNumber() ; }
	inline void		setVersionNumber	( uchar version )		{ ((PrivateSection*)this)->setVersionNumber(version) ; }

	inline BOOL		getSectSyntaxIndicator	( )		{ return ((PrivateSection*)this)->getSectSyntaxIndicator () ; }
	inline BOOL		getPrivateIndicator		( )		{ return ((PrivateSection*)this)->getPrivateIndicator	 () ; }
	inline BOOL		getCurrentNextIndicator	( )		{ return ((PrivateSection*)this)->getCurrentNextIndicator() ; }

	inline void		setSectSyntaxIndicator	( )		{ ((PrivateSection*)this)->setSectSyntaxIndicator	() ; }
	inline void		setPrivateIndicator		( )		{ ((PrivateSection*)this)->setPrivateIndicator		() ; }
	inline void		setCurrentNextIndicator	( )		{ ((PrivateSection*)this)->setCurrentNextIndicator	() ; }

	inline void		resetSectSyntaxIndicator ( )	{ ((PrivateSection*)this)->resetSectSyntaxIndicator	() ; }
	inline void		resetPrivateIndicator	 ( )	{ ((PrivateSection*)this)->resetPrivateIndicator		() ; }
	inline void		resetCurrentNextIndicator( )	{ ((PrivateSection*)this)->resetCurrentNextIndicator	() ; }

	inline void		computeCRC				 ( )	{ ((PrivateSection*)this)->computeCRC() ; }
} ;

//------------------------------------------------------------------------------------//
//
//							Program Association Table
//
//------------------------------------------------------------------------------------//

struct PatProgram
{
	ushort		program_number ;
	union
	{
		ushort	program_map_PID ;
		ushort	network_PID ;
	} ;

	PatProgram(	ushort programNumber, ushort programMapPID )	{ program_number = programNumber; program_map_PID = programMapPID ; } 
	PatProgram(	ushort networkPID )								{ program_number = 0; network_PID = networkPID ; } 
} ;

#define MAX_PAT_SIZE		1024	// (1021+3)
#define MAX_PAT_PROGRAMS	252		// (1021-12)/4 = 252 programs

struct PATable : public BaseTable
{
	uchar		table_id ;			// must be TABLE_ID_PAT
	// sect_syntax_indicator		b1	bslbf
	// private_indicator			b1	bslbf
	// reserved						b2	bslbf	... "11" [2]
	// section_length				b12	uimsbf
	uchar		section_length[2];		// must be < 1021 (0x3FD)
	ushort		transport_stream_id ;
	// reserved						b2	bslbf	...  [2]
	// version_number				b5	bslbf	...  [5]
	// current_next_indicator		b1	bslbf	... 0 [1] ?
	uchar		version_number ;
	uchar		section_number;
	uchar		last_section_number;

	// now the array of programs following
	// number of programs can be computed from section_length
	 PatProgram	programs[1] ;	
	//

	inline	ushort	getNumberOfPrograms	( )				{ return (getTotalLength()-12) / 4 ; }
	inline	ushort	getTransportStreamId( )				{ return ntols(transport_stream_id) ; }
	inline	ushort	getProgramNum		( int index )	{ return ntols(programs[index].program_number) ; }
	inline	ushort	getProgramPID		( int index )	{ return ntols(programs[index].program_map_PID) ; }



	// creates new PAT on the allocated buffer
	// PATable must be large enough to contain nPrograms ( size must be more than 4*nPrograms+12)
			void create			( ushort nPrograms, ushort transportStreamId, BOOL bApplicable=TRUE ) ;
			void setProgram		( ushort index, ushort prgNum, ushort prgPID ) ;
	inline	void setNetworkPID	( ushort index, ushort ntwPID )		{ setProgram (index,0, ntwPID) ; }
} ;

//------------------------------------------------------------------------------------//
//
//							Program Map Table
//
//------------------------------------------------------------------------------------//

struct PmtStream
{
	enum StreamType {
		video_MPEG1				= 0x01,
		video_MPEG2				= 0x02,
		audio_MPEG1				= 0x03,
		audio_MPEG2				= 0x04,
		private_section			= 0x05,
		PES_with_private_data	= 0x06,
		MHEG					= 0x07,
		DSMCC					= 0x08,
		ITU_T_Rec_H_221_1		= 0x09,
		DSMCC_type_A			= 0x0a,
		DSMCC_type_B			= 0x0b,
		DSMCC_type_C			= 0x0c,
		DSMCC_type_D			= 0x0d,
		auxiliary				= 0x0e,
	};

	uchar		stream_type ;
	// reserved						b3	bslbf
	// elementary_PID				b13	uimsbf
	ushort		elementary_PID ;
	// reserved						b4	bslbf
	// ES_info_length				b12	uimsbf
	ushort		ES_info_length ;	// must be < 0x3FF

public:
	inline ushort	getLength		( )		{ return 5 + ( ntols(ES_info_length)&0x3FF ) ; }
	inline ushort	descSize		( )		{ return ntols(ES_info_length) & 0x3FF ; }
	inline uchar*	descriptors		( )		{ return ((uchar*)this) + 5 ; }
	
	inline uchar	streamType		( )		{ return stream_type ; }
	inline ushort	elementaryPid	( )		{ return ntols(elementary_PID) & 0x1FFF ; }

	inline void		setStreamType	( uchar type )	{ stream_type = type ; }
	inline void		setElementaryPid( ushort pid )	{ elementary_PID = ntols(pid&0x1FFF) ; }
	inline void		setDescSize		( ushort size)	{ ES_info_length = ntols(size&0x3FF) ; }

	static void		streamTypeName	( int streamType, char *buf ) ;
} ;

struct PMTable : public BaseTable
{
	uchar		table_id ;			// must be TABLE_ID_PMT
	// sect_syntax_indicator		b1	bslbf
	// private_indicator			b1	bslbf
	// reserved						b2	bslbf	... "11" [2]
	// section_length				b12	uimsbf
	uchar		section_length[2];	// must be < 1021 (0x3FD)
	ushort		program_number ;
	// reserved						b2	bslbf
	// version_number				b5	bslbf
	// current_next_indicator		b1	bslbf
	uchar		version_number ;
	uchar		section_number;
	uchar		last_section_number;

	// reserved						b3	bslbf
	// PCR_PID						b13	uimsbf
	ushort		PCR_PID ;
	// reserved						b4	bslbf
	// program_info_length			b12	uimsbf
	ushort		program_info_length ;	// must be < 0x3FF

	uchar		descriptors[1] ;

	// program streams
	// crc

	inline ushort	 getProgramNumber	  ( )	{ return ntols(program_number) ; }
	inline ushort	 getPcrPid			  ( )	{ return ntols(PCR_PID) & 0x1FFF ; }

	inline ushort	 getProgramInfoLength ( )	{ return ntols(program_info_length) & 0x3FF ; }
	inline uchar	*pmtStreams			  ( )	{ return descriptors + getProgramInfoLength() ; }
	inline ushort	 getStreamsSize		  ( )	{ return getTotalLength() - 16 - getProgramInfoLength() ; }

	int				 getNumDescriptors	( ) ;
	BaseDescriptor	*getDescriptor		( int index ) ;

	int				 getNumStreams		( ) ;
	PmtStream		*getPmtStream		( int index ) ;

	void			 create				( ushort programNumber, ushort pcrPid, BOOL bApplicable=TRUE ) ;
	void			 addDescriptor		( BaseDescriptor *descriptor ) ;
	void			 addStream			( PmtStream* stream ) ;
	void			 addStreamDescriptor( int streamIndex, BaseDescriptor *descriptor ) ;
} ;

//------------------------------------------------------------------------------------//
//
//							Service Description Table
//
//------------------------------------------------------------------------------------//

struct SdtService
{
	enum RunningStatus
	{
		Undefined=0,
		NotRunning=1,
		StartsInFewSeconds=2,
		Pausing=3,
		Running=4
	} ;

	ushort		service_id ;
	
	//	reserved_future_use			b6	bslbf
	//	EIT_schedule_flag			b1	bslbf
	//	EIT_present_following_flag	b1	bslbf
	uchar		eit_flags ;

	//	running_status				b3	uimsbf
	//	free_CA_mode				b1	bslbf
	//	descriptors_loop_length		b12 uimsbf
	ushort		descriptors_loop_length ;
	uchar		descriptors[1] ;

	inline  ushort	getLength		( )				{ return 5 + getDescLength() ; }
	inline	ushort	getDescLength	( )				{ return 0xFFF&ntols(descriptors_loop_length) ; }
	inline	void	setDescLength	( ushort len )	{ descriptors_loop_length &= ntols(0xF000); descriptors_loop_length |= ntols(len&0xFFF) ; }

	inline	ushort	getServiceId	( )				{ return ntols(service_id) ; }
	inline	void	setServiceId	( ushort id )	{ service_id = ntols(id) ; }
	
	inline	int		getRunningStatus( )				{ return ntols(descriptors_loop_length)>>13 ; }
	inline	void	setRunningStatus( int status )	{ descriptors_loop_length &= ntols(0x1FFF) ; descriptors_loop_length |= ((status&0x07) << 5) ; }

	inline	BOOL	EitPresent		( )				{ return (eit_flags&1) != 0 ; }
	inline	BOOL	EitSchedulePresent( )			{ return (eit_flags&2) != 0 ; }
	inline	BOOL	FreeCAMode		( )				{ return (descriptors_loop_length&0x0010) != 0 ; }

	inline	void	setEitPresent	( )				{ eit_flags |= 1 ; }
	inline	void	setEitSchedulePresent( )		{ eit_flags |= 2 ; }
	inline	void	setFreeCAMode	( )				{ descriptors_loop_length |= 0x0010 ; }
	inline	void	resetEitPresent	( )				{ eit_flags &= 0xFE ; }
	inline	void	resetEitSchedulePresent( )		{ eit_flags &= 0xFD ; }
	inline	void	resetFreeCAMode	( )				{ descriptors_loop_length &= 0xFFEF ; }

	static void		runningStatusAsText	( int status, char *buf ) ;
} ;

struct SDTable : public BaseTable
{
	uchar		table_id ;			// must be TABLE_ID_SDT or TABLE_ID_SDT_OTHER_TS
	// sect_syntax_indicator		b1	bslbf	... "1"
	// reserved						b3	bslbf	... "11" [2]
	// section_length				b12	uimsbf
	uchar		section_length[2];	// must be < 1021 (0x3FD)
	ushort		transport_stream_id ;
	// reserved						b2	bslbf
	// version_number				b5	bslbf
	// current_next_indicator		b1	bslbf
	uchar		version_number ;
	uchar		section_number;
	uchar		last_section_number;

	ushort		original_network_id ;
	// reserved						b8	bslbf
	uchar		reserved ;

	uchar		services[1] ;

	inline	ushort	getOriginalNetworkId	( )				{ return ntols(original_network_id) ; }
	inline	void	setOriginalNetworkId	( ushort id )	{ original_network_id = ntols(id) ; }

	inline	ushort	getTransportStreamId	( )				{ return ntols(transport_stream_id) ; }
	inline	void	setTransportStreamId	( ushort id )	{ transport_stream_id = ntols(id) ; }

	inline  ushort	getServicesSize			( )				{ return getTotalLength() - 15 ; }

	void			 create				( ushort tsId, ushort origNtwId, BOOL bApplicable=TRUE ) ;
	int				 getNumServices		( ) ;
} ;

//------------------------------------------------------------------------------------//
//
//							Network Information Table
//
//------------------------------------------------------------------------------------//

struct NitTransportStream
{
	ushort		transport_stream_id ;
	ushort		original_network_id ;
	// reserved						b4	bslbf
	// transport_descriptors_length	b12 bslbf
	ushort		transport_descriptors_length ;

	uchar		descriptors[1] ;

	inline  ushort	getLength		( )				{ return 6 + getDescLength() ; }
	inline	ushort	getDescLength	( )				{ return 0xFFF&ntols(transport_descriptors_length) ; }
	inline	void	setDescLength	( ushort len )	{ transport_descriptors_length = ntols(len&0xFFF) ; }

	inline	ushort	getTransportStreamId	( )				{ return ntols(transport_stream_id) ; }
	inline	ushort	getOriginalNetworkId	( )				{ return ntols(original_network_id) ; }
	inline	void	setTransportStreamId	( ushort id )	{ transport_stream_id = ntols(id) ; }
	inline	void	setOriginalNetworkId	( ushort id )	{ original_network_id = ntols(id) ; }
} ;

struct NITable : public BaseTable
{
	uchar		table_id ;			// must be TABLE_ID_NIT or TABLE_ID_SDT_OTHER_NET
	// sect_syntax_indicator		b1	bslbf	... "1"
	// reserved						b3	bslbf	... "11" [2]
	// section_length				b12	uimsbf
	uchar		section_length[2];	// must be < 1021 (0x3FD)
	ushort		network_id ;
	// reserved						b2	bslbf
	// version_number				b5	bslbf
	// current_next_indicator		b1	bslbf
	uchar		version_number ;
	uchar		section_number;
	uchar		last_section_number;

	// reserved						b4	bslbf
	// network_descriptors_length	b12 bslbf
	ushort		network_descriptors_length ;

	uchar		descriptors[1] ;
	
	// reserved						b4	bslbf
	// transport_stream_loop_length b12 bslbf

	// transport stream loops

	// crc

	inline	ushort	getNetworkId	( )				{ return ntols(network_id) ; }
	inline	void	setNetworkId	( ushort id )	{ network_id = ntols(id) ; }

	inline	ushort	getDescLength	( )				{ return 0xFFF&ntols(network_descriptors_length) ; }
	inline	uchar*	transpStreams	( )				{ return descriptors+getDescLength()+2 ; }
	inline	ushort	getStreamsSize	( )				{ return 0xFFF&ntols( *((ushort*)(transpStreams()-2)) ) ; }
	inline	void	setStreamsSize	( ushort size )	{ *((ushort*)(transpStreams()-2)) = ntols(0xFFF&size) ; }

			int		getNumStreams	( ) ;

			void	create			( ushort networkId, BOOL bApplicable=TRUE  ) ;
} ;

//------------------------------------------------------------------------------------//
//
//							Conditional Access Table
//
//------------------------------------------------------------------------------------//

struct CATable : public BaseTable
{
	uchar		table_id ;			// must be TABLE_ID_CAT
	// sect_syntax_indicator		b1	bslbf	... "1"
	// '0'							b1
	// reserved						b2	bslbf	... "11" [2]
	// section_length				b12	uimsbf
	uchar		section_length[2];	// must be < 1021 (0x3FD)
	ushort		reserved;
	// reserved						b2	bslbf
	// version_number				b5	bslbf
	// current_next_indicator		b1	bslbf
	uchar		version_number ;
	uchar		section_number;
	uchar		last_section_number;

	uchar		descriptors[1] ;

	inline	ushort	getDescLength	( )				{ return getTotalLength()-12 ; }
			void	create			( BOOL bApplicable=TRUE  ) ;
} ;

void networkIdAsText ( int id, char *txt ) ;

#endif // __INC_TABLES_H__
