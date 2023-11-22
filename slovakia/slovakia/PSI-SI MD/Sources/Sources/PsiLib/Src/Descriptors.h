
//------------------------------------------------------------------------------------//
//	PSI/SI tables descriptors header
//  --------------------------------
//
// Version		1.00
// Author		J. Janosik
// Created		24. 1. 2001
// Modified
// Description	Descriptors definition for PSI/SI tables
//
//------------------------------------------------------------------------------------//

#ifndef __INC_DESCRIPTORS_H__
#define __INC_DESCRIPTORS_H__

//////////////////////////
// ISO MPEG2 descriptors

#define TAG_video_stream_descriptor					2	
#define TAG_audio_stream_descriptor					3	
#define TAG_hierarchy_descriptor					4	
#define TAG_registration_descriptor					5	
#define TAG_data_stream_alignment_descriptor		6	
#define TAG_target_background_grid_descriptor		7	
#define TAG_video_window_descriptor					8	
#define TAG_CA_descriptor							9	
#define TAG_ISO_639_language_descriptor				10	
#define TAG_system_clock_descriptor					11	
#define TAG_multiplex_buffer_utilization_descriptor	12	
#define TAG_copyright_descriptor					13	
#define TAG_maximum_bitrate_descriptor				14	
#define TAG_private_data_indicator_descriptor		15	
#define TAG_smoothing_buffer_descriptor				16	
#define TAG_STD_descriptor							17	
#define TAG_IBP_descriptor							18	

//////////////////////////
// ETSI DVB descriptors

#define TAG_network_name_descriptor 				0x40	//
#define TAG_service_list_descriptor 				0x41
#define TAG_stuffing_descriptor 					0x42	//
#define TAG_satellite_delivery_system_descriptor 	0x43	//
#define TAG_cable_delivery_system_descriptor 		0x44
#define TAG_bouquet_name_descriptor					0x47
#define TAG_service_descriptor 						0x48	//
#define TAG_country_availability_descriptor 		0x49
#define TAG_linkage_descriptor 						0x4A
#define TAG_NVOD_reference_descriptor 				0x4B
#define TAG_time_shifted_service_descriptor 		0x4C
#define TAG_short_event_descriptor 					0x4D
#define TAG_extended_event_descriptor 				0x4E
#define TAG_time_shifted_event_descriptor 			0x4F
#define TAG_component_descriptor 					0x50
#define TAG_mosaic_descriptor						0x51
#define TAG_stream_identifier_descriptor 			0x52
#define TAG_CA_identifier_descriptor 				0x53
#define TAG_content_descriptor 						0x54
#define TAG_parental_rating_descriptor 				0x55
#define TAG_teletext_descriptor 					0x56	// no loop
#define TAG_telephone_descriptor 					0x57
#define TAG_local_time_offset_descriptor 			0x58
#define TAG_subtitling_descriptor 					0x59
#define TAG_terrestrial_delivery_system_descriptor 	0x5A
#define TAG_multilingual_network_name_descriptor 	0x5B
#define TAG_multilingual_bouquet_name_descriptor 	0x5C
#define TAG_multilingual_service_name_descriptor 	0x5D
#define TAG_multilingual_component_descriptor		0x5E
#define TAG_private_data_specifier_descriptor 		0x5F
#define TAG_service_move_descriptor 				0x60
#define TAG_short_smoothing_buffer_descriptor		0x61
#define TAG_frequency_list_descriptor 				0x62
#define TAG_partial_transport_stream_descriptor		0x63
#define TAG_data_broadcast_descriptor 				0x64	// no private data
#define TAG_CA_system_descriptor		 			0x65	// no use
#define TAG_data_broadcast_id_descriptor 			0x66	//

size_t	getLangNum() ;
size_t	getCountryNum() ;
#define LANGUAGES_NUMBER	(getLangNum())
#define COUNTRIES_NUMBER	(getCountryNum())

const char *descriptorName( uchar descTag ) ;

#define DECL_DESC_ARR()		uchar descArr[40] ; int descArrLength = 0 ;
#define DESC_ARRAY()		descArr
#define DESC_ARR_LENGTH()	descArrLength

#define ADD_DESC_TO_ARR(descTag)	\
	ASSERT(descArrLength<40);		\
	descArr[descArrLength++] = descTag ;



struct BaseDescriptor
{
	uchar	descriptor_tag ;
	uchar	descriptor_length ;

	inline int	getLength	( )				{ return ((int)descriptor_length) + 2; }
	inline int	getType		( )				{ return descriptor_tag ; }
} ;

struct ServiceDescriptor : public BaseDescriptor
{
	enum ServiceType
	{
		Digital_television_service = 0x01,
		Digital_radio_sound_service,
		Teletext_service,
		NVOD_reference_service,
		NVOD_time_shifted_service,
		Mosaic_service,
		PAL_coded_signal,
		SECAM_coded_signal,
		D_D2_MAC,
		FM_Radio,
		NTSC_coded_signal,
		Data_broadcast_service
	} ;

	uchar	service_type ;
	uchar	service_provider_name_length ;
	char	service_provider_name[1] ;

	void create( uchar type, const char *providerName, const char *name ) ;

	static void  serviceTypeAsText ( uchar servType, char *buf ) ;

	inline uchar getServiceType ( )							{ return service_type ; }
	inline void  setServiceType ( uchar type )				{ service_type = type ; }

	void getServiceName( char *buf ) ;
	void getServiceProviderName( char *buf ) ;
} ;

struct AudioStreamDescriptor : public BaseDescriptor
{
	uchar	flags ;
	void	create		  ( )		{ descriptor_tag=TAG_audio_stream_descriptor;	descriptor_length=1 ; flags=0xFF; }

} ;

struct VideoStreamDescriptor : public BaseDescriptor
{
	uchar	flags ;
	void	create		  ( BOOL stillPictures )		{ descriptor_tag=TAG_video_stream_descriptor;	descriptor_length=1 ; flags=stillPictures?0x83:0x82; }

	inline	BOOL	onlyStillPictures ( )				{ return (flags&1)!=0 ; }

} ;

struct CAIdentifierDescriptor : public BaseDescriptor
{
	ushort	CA_system_id ;

	inline ushort	getCaSystemId ( )				{ return ntols(CA_system_id); }
	inline void		setCaSystemId ( ushort id)		{ CA_system_id=ntols(id); }

	inline void		create		  ( ushort id )		{ setCaSystemId(id) ; descriptor_tag=TAG_CA_identifier_descriptor ;	descriptor_length=2 ; }
} ;

struct CASystemDescriptor : public BaseDescriptor
{
	ushort	CA_system_id ;
	// reserved		3  bit
	// CA_PID		13 bit
	ushort	CA_PID ;
	uchar	private_data[1] ;

	inline ushort	getCaSystemId ( )				{ return ntols(CA_system_id); }
	inline void		setCaSystemId ( ushort id)		{ CA_system_id=ntols(id); }

	inline ushort	getCaSystemPid ( )				{ return 0x1FFF&ntols(CA_PID); }
	inline void		setCaSystemPid ( ushort pid)	{ CA_PID=ntols(pid&0x1FFF); }

	inline int		getPrivateDataLength ( )				{ return descriptor_length-4 ; }
	inline void		setPrivateData ( int len, uchar *data )	{ memcpy(private_data,data,len); descriptor_length=len+4 ; }

	void	create( ushort id, ushort pid ) ;

	static void caSystemAsText( ushort id, char *buf ) ;
} ;

struct CountryStruct
{
	uchar country_id[3] ;
} ;

struct CountryAvailabilityDescriptor : public BaseDescriptor
{
	// country_availability_flag	1 bit
	// reserved						7 bits
	uchar			country_availability_flag ;
	CountryStruct	countries[1] ;

	void	create(BOOL bAvailable)			{ country_availability_flag = bAvailable?0x80:0 ; descriptor_tag=TAG_country_availability_descriptor ;	descriptor_length=1 ; }

	BOOL	hasCountry( const char *country ) ;
	
	inline BOOL countriesAvailable ( )		{ return (country_availability_flag&0x80)!=0 ; }
	inline int  getCountriesNumber ( )		{ return (descriptor_length-1)/3 ; }
	
	static void countryText( uchar *country, char *buf );
} ;

struct DataBroadcastDescriptor : public BaseDescriptor
{
	enum DataBroadcastId
	{
		Data_pipe = 0x0001,
		Asynchronous_data_stream,
		Synchronous_data_stream,
		Synchronised_data_stream,
		Multi_protocol_encapsulation,
		Data_Carousel,
		Object_Carousel,
		DVB_ATM_streams,

		Eutelsat_Data_Piping = 0x0100 ,
		Eutelsat_Data_Streaming,
		SAGEM_IP_encapsulation,
		BARCO_Data_Broadcasting,
		CyberCity_Multiprotocol_Encapsulation,
		CyberSat_Multiprotocol_Encapsulation,
		The_Digital_Network,
		OpenTV_Data_Carousel,
		Panasonic,
		MSG_MediaServices,
		Televizja_Polsat =0x0110,
		UK_DTG,
		SkyMedia,
	} ;

	ushort		data_broadcast_id ;
	uchar		component_tag ;		// 0x00 if not used
	uchar		selector_length ;	// 0x0000 if selector is not present
	uchar		ISO_639_language_code[3] ;
	uchar		text_length ;
	uchar		text_chars[1] ;


	static void broadcastIdAsText( ushort id, char *buf ) ;

	void create ( ushort broadcastId, const char *description ) ;

	inline ushort	getBroadcastId ( )				{ return ntols(data_broadcast_id) ; }
	inline void		getDescription ( char *buf )	{ memcpy(buf, text_chars, text_length); buf[text_length]='\x0';}
} ;

struct DataBroadcastIdDescriptor : public BaseDescriptor
{
	ushort		data_broadcast_id ;

	inline ushort	getBroadcastId ( )				{ return ntols(data_broadcast_id) ; }

	void create ( ushort broadcastId ) ;
} ;

struct LanguageStruct
{
	uchar	lang_code[3] ;
	uchar	audio_type ;
} ;

struct LanguageDescriptor : public BaseDescriptor
{
	LanguageStruct	languages[1] ;

	enum AudioType
	{
		clean_effects = 1,
		hearing_impaired,
		visual_impaired_commentary,
	};

			void	create ( ) ;
	inline	int		getLangNumber()								{ return descriptor_length>>2 ; }
	
	void	setLang		( int i, const char *lang, int audioType ) ;

	static void	langAsText		( uchar *lang, char * buf ) ;
	static void	audioTypeAsText	( int type, char * buf ) ;
} ;

struct LinkageDescriptor : public BaseDescriptor
{
	ushort	transport_stream_id ;
	ushort	original_network_id ;
	ushort	service_id ;
	uchar	linkage_type ;
	
	uchar	private_data[1] ;

	enum LinkageType
	{
		information_service = 1,
		EPG_service,
		CA_replacement_service,
		complete_Network_SI,
		service_replacement_service,
		data_broadcast_service,
	} ;

	void	create	( int type, ushort transportStreamId, ushort originalNetworkId, ushort serviceId ) ;

	ushort	getTransportStreamId	( )				{ return ntols(transport_stream_id) ; }
	ushort	getOriginalNetworkId	( )				{ return ntols(original_network_id) ; }
	ushort	getServiceId			( )				{ return ntols(service_id) ; }
	int		getLinkageType			( )				{ return linkage_type ; }

	void	setTransportStreamId	( ushort id )	{ transport_stream_id=ntols(id) ; }
	void	setOriginalNetworkId	( ushort id )	{ original_network_id=ntols(id) ; }
	void	setServiceId			( ushort id )	{ service_id=ntols(id) ; }
	void	setLinkageType			( int type )	{ linkage_type=type ; }

	inline int		getPrivateDataLength ( )				{ return descriptor_length-7 ; }
	inline void		setPrivateData ( int len, uchar *data )	{ memcpy(private_data,data,len); descriptor_length=len+7 ; }

	static void linkageTypeAsText( int type, char *buf ) ;
} ;

struct MultiLingServiceNameDescriptor : public BaseDescriptor
{
	uchar	names[1] ;

	void	create		( ) ;
	void	getName		( int i, char *buf ) ;
	void	getProvider	( int i, char *buf ) ;
	void	getLanguage	( int i, char *buf ) ;
	
	int		getNamesNumber ( ) ;

	inline	int		size	( )							{ return descriptor_length ; }
	inline	void	setSize	( int size )				{ descriptor_length=(uchar)size ; }
} ;

struct MultiLingNetworkNameDescriptor : public BaseDescriptor
{
	uchar	names[1] ;

	void	create		( ) ;
	void	getName		( int i, char *buf ) ;
	void	getLanguage	( int i, char *buf ) ;
	
	int		getNamesNumber ( ) ;

	inline	int		size	( )							{ return descriptor_length ; }
	inline	void	setSize	( int size )				{ descriptor_length=(uchar)size ; }
} ;

struct NetworkNameDescriptor : public BaseDescriptor
{
	uchar	name[1] ;

	void	create	( const char *ntwName ) ;
	void	getName	( char *buf ) ;
} ;

struct PrivateDataDescriptor : public BaseDescriptor
{
	uchar	private_data[1] ;

	inline int		getPrivateDataLength ( )				{ return descriptor_length ; }
	inline void		setPrivateData ( int len, uchar *data )	{ memcpy(private_data,data,len); descriptor_length=len ; }

	void	create( ) ;
} ;

struct SatelliteDeliveryDescriptor : public BaseDescriptor
{
	// Frequency: The frequency is a 32-bit field giving 
	// the 4-bit BCD values specifying 8 characters of the frequency value. 
	// For the satellite_delivery_system_descriptor the frequency is coded in GHz
	// Decimal point occurs after the third character (e.g. 011.75725 GHz).
	uchar frequency[4] ;
	
	//Orbital_position: The orbital_position is a 16-bit field giving
	// the 4-bit BCD values specifying 4 characters of the orbital position in degrees 
	// Decimal point occurs after the third character (e.g.019.2 degrees).
	uchar orbital_position[2] ;

	// west_east_flag 1 bslbf
	// Polarisation 2 bslbf
	// Modulation 5 bslbf
	uchar	polarization ;
	
	// symbol_rate: The symbol_rate is a 28-bit field giving 
	// the 4-bit BCD values specifying 7 characters of the symbol_rate in Msymbol/s 
	// Decimal point occurs after the third character (e.g. 027.4500).	
	// symbol_rate	28 bit
	// FEC			4  bit
	uchar	symbol_rate[4] ;

	enum Polarization
	{
		Linear_horizontal = 0,
		Linear_vertical,
		Circular_left,
		Circular_right
	} ;

	enum FEC
	{
		Not_defined = 0,
		Conv_code_rate_1_2,
		Conv_code_rate_2_3,
		Conv_code_rate_3_4,
		Conv_code_rate_5_6,
		Conv_code_rate_7_8,
		No_conv_coding = 0x0F
	} ;

	void create( double freq, double orbitalPos, int westEast, int polariz, double symbolRate, int FEC) ;

	double	getFrequency	( ) ;
	double	getOrbitalPos	( ) ;
	int		getWestEast		( )				{ return polarization>>7 ; }
	int		getPolarization	( )				{ return (polarization>>5) & 3 ; }
	int		getModulation	( )				{ return polarization & 0x1F ; }
	double	getSymbolRate	( )	;
	int		getFEC			( )				{ return symbol_rate[3] & 0x0F ; }

	void	westEastAsText		( char * buf )	{ strcpy(buf, getWestEast()?"East":"West") ; }
	void	polarizationAsText	( char * buf )	;
	void	modulationAsText	( char * buf )	{ strcpy(buf, "QPSK") ; }
	void	fecAsText			( char * buf )	;
} ;

struct StreamIdentifierDescriptor : public BaseDescriptor
{
	uchar	component_tag ;

	void	create	( uchar componentTag ) ;

	inline int getComponentTag ( )							{ return component_tag ; }
} ;

struct SubtitlingDescriptor : public BaseDescriptor
{
	//	ISO_639_language_code		24 bslbf
	char	ISO_639_language_code[3] ;
	// subtitling_type 8 bslbf
	uchar	subtitling_type;
	// composition_page_id 16 bslbf
	ushort	composition_page_id ;
	// ancillary_page_id 16 bslbf	
	ushort	ancillary_page_id ;

	enum SubtitlesType
	{
		EBU_Teletext_subtitles							= 1,
		Associated_EBU_Teletext							= 2,
		DVB_subtitles_no_aspect_ratio					= 0x10,
		DVB_subtitles_aspect_ratio_4_3					= 0x11,
		DVB_subtitles_aspect_ratio_16_9					= 0x12,
		DVB_subtitles_aspect_ratio_2_21_1				= 0x13,
		DVB_subtitles_HardHearing_no_aspect_ratio		= 0x20,
		DVB_subtitles_HardHearing_aspect_ratio_4_3		= 0x21,
		DVB_subtitles_HardHearing_aspect_ratio_16_9		= 0x22,
		DVB_subtitles_HardHearing_aspect_ratio_2_21_1	= 0x23,
	} ;

	inline	void getLang			( char *buf )	{ memcpy(buf,ISO_639_language_code,3) ; buf[3] = '\x0' ; }
	inline	int	 getType			( )				{ return subtitling_type ; }
	inline	int	 getComposPageId	( )				{ return ntols(composition_page_id) ; }
	inline	int	 getAncillaryPageId	( )				{ return ntols(ancillary_page_id) ; }

	inline	void setLang			( char *buf )	{ memcpy(ISO_639_language_code,buf,3) ; }
	inline	void setType			( int type )	{ subtitling_type = type ; }
	inline	void setComposPageId	( int id )		{ composition_page_id = ntols(id) ; }
	inline	void setAncillaryPageId	( int id )		{ ancillary_page_id = ntols(id) ; }

	void create	( char *lang, int type, int compositionPageId, int ancillaryPageId ) ;

	static void subtitlesTypeAsText( int type, char *buf ) ;
} ;

struct TeletextDescriptor : public BaseDescriptor
{
	//	ISO_639_language_code		24 bslbf
	char	ISO_639_language_code[3] ;
	//	teletext_type				 5 uimsbf
	//	teletext_magazine_number	 3 uimsbf
	uchar	teletext_type;
	//	teletext_page_number		 8 uimsbf
	uchar	teletext_page_number ;

	enum TeletextType
	{
		InitialTeletextPage = 1 ,
		TeletextSubtitlePage ,
		AdditionalInformationPage ,
		ProgrammeSchedulePage ,
		TeletextSubtitlePageForHearingImpairedPeople
	} ;

	inline	void getLang		( char *buf )	{ memcpy(buf,ISO_639_language_code,3) ; buf[3] = '\x0' ; }
	inline	int	 getType		( )				{ return teletext_type>>3 ; }
	inline	int	 getMagazineNum ( )				{ return teletext_type & 0x7 ; }
	inline	int	 getPageNum		( )				{ return teletext_page_number ; }

	inline	void setLang		( char *buf )	{ memcpy(ISO_639_language_code,buf,3) ; }
	inline	void setType		( int type )	{ teletext_type&=0x07; teletext_type|=(type&0x1F)<<3 ; }
	inline	void setMagazineNum ( int num )		{ teletext_type&=0xF8; teletext_type|=num&0x07 ; }
	inline	void setPageNum		( int num )		{ teletext_page_number = (uchar)num ; }

	void create	( char *lang, int type, int magazineNumber, int pageNumber ) ;

	static void teletextTypeAsText( int type, char *buf ) ;
} ;

#endif // __INC_DESCRIPTORS_H__
