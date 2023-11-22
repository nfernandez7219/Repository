
#include "stdafx.h"
#include "Descriptors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////
// ServiceDescriptor 

void ServiceDescriptor::create( uchar type, const char *providerName, const char *name )
{
	service_type = type ;
	service_provider_name_length = (uchar)strlen(providerName) ;
	memcpy(service_provider_name, providerName, service_provider_name_length) ;

	uchar *servNameLength = (uchar*)service_provider_name + service_provider_name_length ; 
	*servNameLength = strlen(name) ;
	char *servName = (char*)servNameLength + 1 ; 
	memcpy(servName, name, *servNameLength ) ;
	
	descriptor_tag = TAG_service_descriptor ;
	descriptor_length = 3+service_provider_name_length+*servNameLength;
}

void ServiceDescriptor::serviceTypeAsText ( uchar servType, char *buf )
{
	char txt[64] ;

	switch(servType)
	{
		case Digital_television_service		: strcpy(txt,"Digital television service"); break ;
		case Digital_radio_sound_service	: strcpy(txt,"Digital radio sound service"); break ;
		case Teletext_service				: strcpy(txt,"Teletext service"); break ;
		case NVOD_reference_service			: strcpy(txt,"NVOD reference service"); break ;
		case NVOD_time_shifted_service		: strcpy(txt,"NVOD time shifted service"); break ;
		case Mosaic_service					: strcpy(txt,"Mosaic service"); break ;
		case PAL_coded_signal				: strcpy(txt,"PAL coded signal"); break ;
		case SECAM_coded_signal				: strcpy(txt,"SECAM coded signal"); break ;
		case D_D2_MAC						: strcpy(txt,"D/D2 MAC"); break ;
		case FM_Radio						: strcpy(txt,"FM Radio"); break ;
		case NTSC_coded_signal				: strcpy(txt,"NTSC coded signal"); break ;
		case Data_broadcast_service			: strcpy(txt,"Data broadcast service"); break ;
		default								: strcpy(txt,""); break ;
	}

	sprintf(buf, "%s (0x%x)", txt, (int)servType) ;
}

void ServiceDescriptor::getServiceName( char *buf )
{ 
	uchar *size = (uchar*)service_provider_name + service_provider_name_length ;
	char  *name = (char*)size + 1 ;
	memcpy(buf, name, *size) ;
	buf[*size] = '\x0' ;
}

void ServiceDescriptor::getServiceProviderName( char *buf )
{ 
	memcpy(buf, service_provider_name, service_provider_name_length) ;
	buf[service_provider_name_length] = '\x0' ;
}

//////////////////////
// CASystemDescriptor

void CASystemDescriptor::create( ushort id, ushort pid )
{
	descriptor_tag = TAG_CA_descriptor ;
	descriptor_length = 4;
	setCaSystemId(id) ;
	setCaSystemPid(pid);
}

void CASystemDescriptor::caSystemAsText( ushort id, char *buf )
{
	sprintf(buf, "0x%x ", id) ;
	strupr(buf+2) ;

	switch (id&0xFF00)
	{
		case 0x0000: strcat(buf, "(Standardized systems)"); break ;
		case 0x0100: strcat(buf, "(Canal Plus)"); break ;
		case 0x0200: strcat(buf, "(CCETT)"); break ;
		case 0x0300: strcat(buf, "(MSG MediaServices GmbH)"); break ;
		case 0x0400: strcat(buf, "(Eurodec)"); break ;
		case 0x0500: strcat(buf, "(France Telecom)"); break ;
		case 0x0600: strcat(buf, "(Irdeto)"); break ;
		case 0x0700: strcat(buf, "(Jerrold/GI)"); break ;
		case 0x0800: strcat(buf, "(Matra Communication)"); break ;
		case 0x0900: strcat(buf, "(News Datacom)"); break ;
		case 0x0A00: strcat(buf, "(Nokia)"); break ;
		case 0x0B00: strcat(buf, "(Norwegian Telekom)"); break ;
		case 0x0C00: strcat(buf, "(NTL)"); break ;
		case 0x0D00: strcat(buf, "(Philips)"); break ;
		case 0x0E00: strcat(buf, "(Scientific Atlanta)"); break ;
		case 0x0F00: strcat(buf, "(Sony)"); break ;
		case 0x1000: strcat(buf, "(Tandberg Television)"); break ;
		case 0x1100: strcat(buf, "(Thomson)"); break ;
		case 0x1200: strcat(buf, "(TV/Com)"); break ;
		case 0x1300: strcat(buf, "(HPT - Croatian Post and Telecommunications)"); break ;
		case 0x1400: strcat(buf, "(HRT - Croatian Radio and Television)"); break ;
		case 0x1500: strcat(buf, "(IBM)"); break ;
		case 0x1600: strcat(buf, "(Nera)"); break ;
		case 0x1700: strcat(buf, "(BetaTechnik)"); break ;
		case 0x1800: strcat(buf, "(Kudelski SA)"); break ;
		case 0x1900: strcat(buf, "(Titan Information Systems)"); break ;
		case 0x2000: strcat(buf, "(Telefónica Servicios Audiovisuales)"); break ;
		case 0x2100: strcat(buf, "(STENTOR (France Telecom, CNES and DGA))"); break ;
		case 0x2200: strcat(buf, "(Tadiran Scopus)"); break ;
		case 0x2300: strcat(buf, "(BARCO AS)"); break ;
		case 0x2400: strcat(buf, "(StarGuide Digital Networks)"); break ;
		case 0x2500: strcat(buf, "(Mentor Data System, Inc.)"); break ;
		case 0x2600: strcat(buf, "(European Broadcasting Union)"); break ;
		case 0x4700: strcat(buf, "(General Instrument)"); break ;
		case 0x4800: strcat(buf, "(Telemann)"); break ;
		default: strcat(buf, "(Unknown CA buf)"); break ;
	}
}

///////////////////////////////////
//	CountryAvailabilityDescriptor

void CountryAvailabilityDescriptor::countryText( uchar *country, char *buf )
{
	int i = (int)COUNTRIES_NUMBER;
	while(i--)
	{
		if (strncmp((const char*)country , s_stCountries[i], 3)==0)
		{
			strcpy(buf, s_stCountries[i]) ;
			return ;
		}
	}
	buf[0]='\x0';
}

BOOL CountryAvailabilityDescriptor::hasCountry( const char *country )
{
	int nCountries = getCountriesNumber() ;
	while (nCountries--)
	{
		char *pCountry = (char*)countries[nCountries].country_id ;
		if ( strncmp(pCountry, (const char*) country, 3) == 0 )
			return TRUE ;
	}
	return FALSE ;
}


///////////////////////////
// DataBroadcastDescriptor

void DataBroadcastDescriptor::broadcastIdAsText( ushort id, char *buf )
{
	char txt[128] ;

	switch (id)
	{
		case Data_pipe:						strcpy(txt,"Data pipe") ; break ;
		case Asynchronous_data_stream:		strcpy(txt,"Asynchronous data stream") ; break ;
		case Synchronous_data_stream:		strcpy(txt,"Synchronous data stream") ; break ;
		case Synchronised_data_stream:		strcpy(txt,"Synchronised data stream") ; break ;
		case Multi_protocol_encapsulation:	strcpy(txt,"Multi protocol encapsulation") ; break ;
		case Data_Carousel:					strcpy(txt,"Data Carousel") ; break ;
		case Object_Carousel:				strcpy(txt,"Object Carousel") ; break ;
		case DVB_ATM_streams:				strcpy(txt,"DVB ATM streams") ; break ;

		case Eutelsat_Data_Piping: strcpy(txt,"Eutelsat Data Piping") ; break ;
		case Eutelsat_Data_Streaming: strcpy(txt,"Eutelsat Data Streaming") ; break ;
		case SAGEM_IP_encapsulation: strcpy(txt,"SAGEM IP encapsulation in MPEG-2 PES packets") ; break ;
		case BARCO_Data_Broadcasting: strcpy(txt,"BARCO Data Broadcasting") ; break ;
		case CyberCity_Multiprotocol_Encapsulation: strcpy(txt,"CyberCity Multiprotocol Encapsulation (New Media Communications Ltd.)") ; break ;
		case CyberSat_Multiprotocol_Encapsulation: strcpy(txt,"CyberSat Multiprotocol Encapsulation (New Media Communications Ltd.)") ; break ;
		case The_Digital_Network: strcpy(txt,"The Digital Network") ; break ;
		case OpenTV_Data_Carousel: strcpy(txt,"OpenTV Data Carousel") ; break ;
		case Panasonic: strcpy(txt,"Panasonic") ; break ;
		case MSG_MediaServices: strcpy(txt,"MSG MediaServices GmbH") ; break ;
		case Televizja_Polsat: strcpy(txt,"Televizja Polsat") ; break ;
		case UK_DTG: strcpy(txt,"UK DTG") ; break ;
		case SkyMedia: strcpy(txt,"SkyMedia") ; break ;
		default: strcpy(txt,"Unknown broadcast service") ; break ;
	} ;

	sprintf(buf, "%s (0x%x)", txt, (int)id) ;
}

void DataBroadcastDescriptor::create ( ushort broadcastId, const char *description ) 
{
	uchar descLen = strlen(description) ;

	descriptor_tag = TAG_data_broadcast_descriptor ;
	descriptor_length = descLen + 8;

	data_broadcast_id = ntols(broadcastId);
	component_tag = 0 ;		// 0x00 if not used
	selector_length = 0 ;	// 0x0000 if selector is not present
	ISO_639_language_code[0]='e' ;
	ISO_639_language_code[1]='n' ;
	ISO_639_language_code[2]='g' ;
	text_length = descLen ;
	memcpy(text_chars, description, descLen) ;
}

/////////////////////////////
// DataBroadcastIdDescriptor

void DataBroadcastIdDescriptor::create ( ushort broadcastId ) 
{
	descriptor_tag = TAG_data_broadcast_id_descriptor ;
	descriptor_length =  2;
	data_broadcast_id = ntols(broadcastId);
}

//////////////////////////////
// SatelliteDeliveryDescriptor

static int getFloatDigit( double num, int decPos )
{
	while (decPos>0)
	{
		decPos-- ;
		num /= 10 ;
	}
	while (decPos<0)
	{
		decPos++ ;
		num *= 10 ;
	}

	return ((int)num) % 10 ;
}

static double getFloatFromBCD( uchar *bcdValues, int nDigits, int decPoint )
{
	double value1 = 0.0 ;
	int i1=0, i2 =nDigits-1 ;

	nDigits -= decPoint ;
	while (decPoint-- > 0)
	{
		value1 *= 10.0 ;
		if (i1&1)
			value1 += bcdValues[i1>>1] & 0x0F ;
		else
			value1 += bcdValues[i1>>1] >> 4;
		i1++ ;
	}

	double value2 = 0.0 ;
	while (nDigits-- > 0)
	{
		if (i2&1)
			value2 += bcdValues[i2>>1] & 0x0F ;
		else
			value2 += bcdValues[i2>>1] >> 4;
		
		value2 /= 10.0 ;
		i2-- ;
	}

	return value1+value2 ;
}

void SatelliteDeliveryDescriptor::create( double freq, double orbitalPos, int westEast, int polariz, double symbolRate, int FEC)
{
	descriptor_tag = TAG_satellite_delivery_system_descriptor ;
	descriptor_length = 11;

	frequency[0] = (getFloatDigit(freq, 2)<<4) | getFloatDigit(freq, 1) ;
	frequency[1] = (getFloatDigit(freq, 0)<<4) | getFloatDigit(freq,-1) ;
	frequency[2] = (getFloatDigit(freq,-2)<<4) | getFloatDigit(freq,-3) ;
	frequency[3] = (getFloatDigit(freq,-4)<<4) | getFloatDigit(freq,-5) ;

	orbital_position[0] = (getFloatDigit(orbitalPos,2)<<4) | getFloatDigit(orbitalPos, 1) ;
	orbital_position[1] = (getFloatDigit(orbitalPos,0)<<4) | getFloatDigit(orbitalPos,-1) ;

	polarization = (polariz&3)<<5 ;
	if (westEast)
		polarization |= 0x80 ;

	// modulation - QPSK
	polarization |= 0x01 ;

	symbol_rate[0] = (getFloatDigit(symbolRate, 2)<<4) | getFloatDigit(symbolRate, 1) ;
	symbol_rate[1] = (getFloatDigit(symbolRate, 0)<<4) | getFloatDigit(symbolRate,-1) ;
	symbol_rate[2] = (getFloatDigit(symbolRate,-2)<<4) | getFloatDigit(symbolRate,-3) ;
	symbol_rate[3] = (getFloatDigit(symbolRate,-4)<<4) | (FEC&0x0F) ;
}

double	SatelliteDeliveryDescriptor::getFrequency	( )		{ return getFloatFromBCD(frequency, 8, 3) ; }
double	SatelliteDeliveryDescriptor::getOrbitalPos	( )		{ return getFloatFromBCD(orbital_position, 4, 3) ; }
double	SatelliteDeliveryDescriptor::getSymbolRate	( )		{ return getFloatFromBCD(symbol_rate, 7, 3) ; }

void	SatelliteDeliveryDescriptor::polarizationAsText	( char * buf )
{
	switch (getPolarization())
	{
		case Linear_horizontal	: strcpy(buf,"Linear horizontal") ; break ;
		case Linear_vertical	: strcpy(buf,"Linear vertical") ; break ;
		case Circular_left		: strcpy(buf,"Circular left") ; break ;
		case Circular_right		: strcpy(buf,"Circular right") ; break ;
		default: strcpy(buf,"???") ; break ;
	} ;
}

void	SatelliteDeliveryDescriptor::fecAsText			( char * buf )
{
	switch(getFEC())
	{
		case Conv_code_rate_1_2: strcpy(buf, "1/2") ; break ;
		case Conv_code_rate_2_3: strcpy(buf, "2/3") ; break ;
		case Conv_code_rate_3_4: strcpy(buf, "3/4") ; break ;
		case Conv_code_rate_5_6: strcpy(buf, "5/6") ; break ;
		case Conv_code_rate_7_8: strcpy(buf, "7/8") ; break ;
		case No_conv_coding:	 strcpy(buf, "No conv. coding") ; break ;
		default:  strcpy(buf, "Not defined") ; break ;
	} ;
}

////////////////////////
//	LanguageDescriptor

void	LanguageDescriptor::create ( )
{ 
	descriptor_tag=TAG_ISO_639_language_descriptor; 
	descriptor_length=0; 
}
	
void	LanguageDescriptor::setLang	(int i, const char *lang, int audioType ) 
{ 
	memcpy(languages[i].lang_code, lang, 3) ; 
	languages[i].audio_type=audioType ; 
	int newSize = 4*(i+1) ;
	if ( newSize > descriptor_length )
		descriptor_length = newSize ;
}

void	LanguageDescriptor::langAsText	( uchar *lang, char * buf )
{
	int i = (int)LANGUAGES_NUMBER;
	while(i--)
	{
		if (strncmp((const char*)lang , s_stLanguages[i], 3)==0)
		{
			strcpy(buf, s_stLanguages[i]) ;
			return ;
		}
	}
	buf[0]='\x0';
}

void	LanguageDescriptor::audioTypeAsText	( int type, char * buf )
{
	switch(type)
	{
		case clean_effects: strcpy(buf, "Clean effects") ; break ;
		case hearing_impaired: strcpy(buf, "Hearing impaired") ; break ;
		case visual_impaired_commentary: strcpy(buf, "Visual impaired commentary") ; break ;
		default:  strcpy(buf, "Not defined") ; break ;
	} ;
}

///////////////////////
//	LinkageDescriptor

void LinkageDescriptor::create( int type, ushort transportStreamId, ushort originalNetworkId, ushort serviceId )
{
	descriptor_tag = TAG_linkage_descriptor ;
	descriptor_length = 7 ;

	setTransportStreamId	( transportStreamId );
	setOriginalNetworkId	( originalNetworkId );
	setServiceId			( serviceId );
	setLinkageType			( type );
}

void LinkageDescriptor::linkageTypeAsText( int type, char *buf )
{
	switch (type)
	{
		case information_service:			strcpy(buf, "Information service") ; break ;
		case EPG_service:					strcpy(buf, "Electronic Programme Guide (EPG) service") ; break ;
		case CA_replacement_service:		strcpy(buf, "CA replacement service") ; break ;
		case complete_Network_SI:			strcpy(buf, "TS containing complete Network/Bouquet SI") ; break ;
		case service_replacement_service:	strcpy(buf, "Service replacement service") ; break ;
		case data_broadcast_service:		strcpy(buf, "Data broadcast service") ; break ;
		default:							strcpy(buf, "Unknown linkage") ; break ;
	} ;
}

//////////////////////////
//	MultiLingServiceName

static uchar* getNameStart( uchar *names, int i, BOOL bHasProvName )
{
	while (i--)
	{
		names += 3 ;	// skip language code
		if (bHasProvName)
		{
			uchar provLen = *names ;
			names += provLen+1 ;
		}
		uchar nameLen = *names ;
		names += nameLen+1 ;
	}
	return names ;
}

int MultiLingServiceNameDescriptor::getNamesNumber ( )
{
	int count = 0 ;
	int p = 0 ;

	while (p < descriptor_length)
	{
		count++ ;
		p += 3 ;	// skip language code
//		if (bHasProvName)
//		{
			uchar provLen = names[p] ;
			p += provLen+1 ;
//		}
		uchar nameLen = names[p] ;
		p += nameLen+1 ;
	}
	ASSERT(p==descriptor_length);
	return count ;
}

void MultiLingServiceNameDescriptor::create( )
{ 
	descriptor_tag=TAG_multilingual_service_name_descriptor ; 
	descriptor_length=0 ; 
}

void MultiLingServiceNameDescriptor::getName		( int i, char *buf )
{
	uchar *start = getNameStart(names, i, TRUE) ;
	// skip language code
	start += 3 ;
	// skip provider name
	uchar provLen = *start ;
	start += provLen+1 ;

	uchar nameLen = *start ;
	memcpy(buf, start+1, nameLen) ;
	buf[nameLen] = '\x0';
}

void MultiLingServiceNameDescriptor::getProvider	( int i, char *buf )
{
	uchar *start = getNameStart(names, i, TRUE) ;
	// skip language code
	start += 3 ;

	uchar provLen = *start ;
	memcpy(buf, start+1, provLen) ;
	buf[provLen] = '\x0';
}

void MultiLingServiceNameDescriptor::getLanguage	( int i, char *buf )
{
	uchar *start = getNameStart(names, i, TRUE) ;
	LanguageDescriptor::langAsText(start, buf) ;
}

////////////////////////////////////
//	MultiLingNetworkNameDescriptor


int MultiLingNetworkNameDescriptor::getNamesNumber ( )
{
	int count = 0 ;
	int p = 0 ;

	while (p < descriptor_length)
	{
		count++ ;
		p += 3 ;	// skip language code
		uchar nameLen = names[p] ;
		p += nameLen+1 ;
	}
	ASSERT(p==descriptor_length);
	return count ;
}

void MultiLingNetworkNameDescriptor::create( )
{ 
	descriptor_tag=TAG_multilingual_network_name_descriptor ; 
	descriptor_length=0 ; 
}

void MultiLingNetworkNameDescriptor::getName		( int i, char *buf )
{
	uchar *start = getNameStart(names, i, FALSE) ;
	// skip language code
	start += 3 ;

	uchar nameLen = *start ;
	memcpy(buf, start+1, nameLen) ;
	buf[nameLen] = '\x0';
}

void MultiLingNetworkNameDescriptor::getLanguage	( int i, char *buf )
{
	uchar *start = getNameStart(names, i, FALSE) ;
	LanguageDescriptor::langAsText(start, buf) ;
}


//////////////////////////
// NetworkNameDescriptor

void NetworkNameDescriptor::create	( const char *ntwName )
{
	descriptor_tag = TAG_network_name_descriptor ;
	descriptor_length = strlen(ntwName) ;
	memcpy(name, ntwName, descriptor_length) ;
}

void NetworkNameDescriptor::getName	( char *buf )
{
	int len = descriptor_length ;
	memcpy(buf, name, len) ;
	buf[len] = '\x0' ;
}

///////////////////////////
//	PrivateDataDescriptor

void PrivateDataDescriptor::create( )
{
	descriptor_tag = TAG_private_data_specifier_descriptor ;
	descriptor_length = 0 ;
}

void StreamIdentifierDescriptor::create( uchar componentTag )
{ 
	component_tag = componentTag ; 
	descriptor_tag = TAG_stream_identifier_descriptor ;
	descriptor_length = 1 ;
}

/////////////////////////
//	SubtitlingDescriptor

void SubtitlingDescriptor::create( char *lang, int type, int compositionPageId, int ancillaryPageId )
{
	descriptor_tag = TAG_subtitling_descriptor ;
	descriptor_length = 8 ;
	if (lang==NULL)
		lang = "eng" ;
	setLang(lang) ;
	setType				(type);
	setComposPageId		(compositionPageId);
	setAncillaryPageId	(ancillaryPageId);
}

void SubtitlingDescriptor::subtitlesTypeAsText( int type, char *buf )
{
	switch (type)
	{
	case EBU_Teletext_subtitles							: strcpy(buf,"EBU Teletext subtitles"); break ;
	case Associated_EBU_Teletext						: strcpy(buf,"Associated EBU Teletext"); break ;
	case DVB_subtitles_no_aspect_ratio					: strcpy(buf,"DVB subtitles (normal) with no monitor aspect ratio criticality"); break ;
	case DVB_subtitles_aspect_ratio_4_3				 	: strcpy(buf,"DVB subtitles (normal) for display on 4:3 aspect ratio monitor"); break ;
	case DVB_subtitles_aspect_ratio_16_9				: strcpy(buf,"DVB subtitles (normal) for display on 16:9 aspect ratio monitor"); break ;
	case DVB_subtitles_aspect_ratio_2_21_1				: strcpy(buf,"DVB subtitles (normal) for display on 2.21:1 aspect ratio monitor"); break ;
	case DVB_subtitles_HardHearing_no_aspect_ratio		: strcpy(buf,"DVB subtitles (hard hearing) with no monitor aspect ratio criticality"); break ;
	case DVB_subtitles_HardHearing_aspect_ratio_4_3		: strcpy(buf,"DVB subtitles (hard hearing) for display on 4:3 aspect ratio monitor"); break ;
	case DVB_subtitles_HardHearing_aspect_ratio_16_9	: strcpy(buf,"DVB subtitles (hard hearing) for display on 16:9 aspect ratio monitor"); break ;
	case DVB_subtitles_HardHearing_aspect_ratio_2_21_1	: strcpy(buf,"DVB subtitles (hard hearing) for display on 2.21:1 aspect ratio monitor"); break ;
	default:	strcpy(buf,"Unknown subtitles type"); break ;
	} ;
}
	
////////////////////////
//	TeletextDescriptor

void TeletextDescriptor::create( char *lang, int type, int magazineNumber, int pageNumber )
{
	descriptor_tag = TAG_teletext_descriptor ;
	descriptor_length = 5 ;
	if (lang==NULL)
		lang = "eng" ;
	setLang(lang) ;
	teletext_type = (type<<3) | (magazineNumber&0x07) ;
	setPageNum(pageNumber) ;
}

void TeletextDescriptor::teletextTypeAsText( int type, char *buf )
{
	switch (type)
	{
	case InitialTeletextPage:		strcpy(buf,"Initial teletext page"); break ;
	case TeletextSubtitlePage:		strcpy(buf,"Teletext subtitle page"); break ;
	case AdditionalInformationPage:	strcpy(buf,"Additional information page"); break ;
	case ProgrammeSchedulePage:		strcpy(buf,"Programme schedule page"); break ;
	case TeletextSubtitlePageForHearingImpairedPeople:	strcpy(buf,"Teletext subtitle page for hearing impaired people"); break ;
	default:						strcpy(buf,"Unknown teletext type"); break ;
	} ;
}

///////////////////////////////
// Common descriptor functions

#define DECL_DESC_NAME(desc, name)	static const char* STR_##desc = name ;
#define CASE_STAT(desc)	case TAG_##desc: return STR_##desc ;

const char *descriptorName( uchar descTag )
{
	DECL_DESC_NAME(video_stream_descriptor					,	"Video stream descriptor")
	DECL_DESC_NAME(audio_stream_descriptor					,	"Audio stream descriptor")
	DECL_DESC_NAME(hierarchy_descriptor						,	"Hierarchy descriptor")
	DECL_DESC_NAME(registration_descriptor					,	"Registration descriptor")
	DECL_DESC_NAME(data_stream_alignment_descriptor			,	"Data stream alignment descriptor")
	DECL_DESC_NAME(target_background_grid_descriptor		,	"Target background grid descriptor")
	DECL_DESC_NAME(video_window_descriptor					,	"Video window descriptor")
	DECL_DESC_NAME(CA_descriptor							,	"CA descriptor")
	DECL_DESC_NAME(ISO_639_language_descriptor				,	"ISO 639 language descriptor")
	DECL_DESC_NAME(system_clock_descriptor					,	"System clock descriptor")
	DECL_DESC_NAME(multiplex_buffer_utilization_descriptor	,	"Multiplex buffer utilization descriptor")
	DECL_DESC_NAME(copyright_descriptor						,	"Copyright descriptor")
	DECL_DESC_NAME(maximum_bitrate_descriptor				,	"Maximum bitrate descriptor")
	DECL_DESC_NAME(private_data_indicator_descriptor		,	"Private data indicator descriptor")
	DECL_DESC_NAME(smoothing_buffer_descriptor				,	"Smoothing buffer descriptor")
	DECL_DESC_NAME(STD_descriptor							,	"STD descriptor")
	DECL_DESC_NAME(IBP_descriptor							,	"IBP descriptor")

	DECL_DESC_NAME(undefined_descriptor						, "Undefined descriptor")
	DECL_DESC_NAME(network_name_descriptor 					, "Network name descriptor")
	DECL_DESC_NAME(service_list_descriptor 					, "Service list descriptor")
	DECL_DESC_NAME(stuffing_descriptor 						, "Stuffing Descriptor")
	DECL_DESC_NAME(satellite_delivery_system_descriptor		, "Satellite delivery system descriptor")
	DECL_DESC_NAME(cable_delivery_system_descriptor 		, "Cable delivery system descriptor")
	DECL_DESC_NAME(bouquet_name_descriptor					, "Bouquet name descriptor")
	DECL_DESC_NAME(service_descriptor 						, "Service descriptor")
	DECL_DESC_NAME(country_availability_descriptor 			, "Country availability descriptor")
	DECL_DESC_NAME(linkage_descriptor 						, "Linkage descriptor")
	DECL_DESC_NAME(NVOD_reference_descriptor 				, "NVOD reference descriptor")
	DECL_DESC_NAME(time_shifted_service_descriptor 			, "Time shifted service descriptor")
	DECL_DESC_NAME(short_event_descriptor 					, "Short event descriptor")
	DECL_DESC_NAME(extended_event_descriptor 				, "Extended event descriptor")
	DECL_DESC_NAME(time_shifted_event_descriptor 			, "Time shifted event descriptor")
	DECL_DESC_NAME(component_descriptor 					, "Component_descriptor")
	DECL_DESC_NAME(mosaic_descriptor						, "Mosaic descriptor")
	DECL_DESC_NAME(stream_identifier_descriptor 			, "Stream identifier descriptor")
	DECL_DESC_NAME(CA_identifier_descriptor 				, "CA identifier descriptor")
	DECL_DESC_NAME(content_descriptor 						, "Content descriptor")
	DECL_DESC_NAME(parental_rating_descriptor 				, "Parental rating descriptor")
	DECL_DESC_NAME(teletext_descriptor 						, "Teletext descriptor")
	DECL_DESC_NAME(telephone_descriptor 					, "Telephone descriptor")
	DECL_DESC_NAME(local_time_offset_descriptor 			, "Local time offset descriptor")
	DECL_DESC_NAME(subtitling_descriptor 					, "Subtitling descriptor")
	DECL_DESC_NAME(terrestrial_delivery_system_descriptor 	, "Terrestrial delivery system descriptor")
	DECL_DESC_NAME(multilingual_network_name_descriptor 	, "Multilingual network name descriptor")
	DECL_DESC_NAME(multilingual_bouquet_name_descriptor 	, "Multilingual bouquet name descriptor")
	DECL_DESC_NAME(multilingual_service_name_descriptor 	, "Multilingual service name descriptor")
	DECL_DESC_NAME(multilingual_component_descriptor		, "Multilingual component descriptor")
	DECL_DESC_NAME(private_data_specifier_descriptor 		, "Private data specifier descriptor")
	DECL_DESC_NAME(service_move_descriptor 					, "Service move descriptor")
	DECL_DESC_NAME(short_smoothing_buffer_descriptor		, "Short smoothing buffer descriptor")
	DECL_DESC_NAME(frequency_list_descriptor 				, "Frequency list descriptor")
	DECL_DESC_NAME(partial_transport_stream_descriptor		, "Partial transport stream descriptor")
	DECL_DESC_NAME(data_broadcast_descriptor 				, "Data broadcast descriptor")
	DECL_DESC_NAME(CA_system_descriptor		 				, "CA system descriptor")
	DECL_DESC_NAME(data_broadcast_id_descriptor				, "Data broadcast id descriptor")

	switch (descTag)
	{

		CASE_STAT(video_stream_descriptor				)
		CASE_STAT(audio_stream_descriptor				)
		CASE_STAT(hierarchy_descriptor					)
		CASE_STAT(registration_descriptor				)
		CASE_STAT(data_stream_alignment_descriptor		)
		CASE_STAT(target_background_grid_descriptor		)
		CASE_STAT(video_window_descriptor				)
		CASE_STAT(CA_descriptor							)
		CASE_STAT(ISO_639_language_descriptor			)
		CASE_STAT(system_clock_descriptor				)
		CASE_STAT(multiplex_buffer_utilization_descriptor)
		CASE_STAT(copyright_descriptor					)
		CASE_STAT(maximum_bitrate_descriptor			)
		CASE_STAT(private_data_indicator_descriptor		)
		CASE_STAT(smoothing_buffer_descriptor			)
		CASE_STAT(STD_descriptor						)
		CASE_STAT(IBP_descriptor						)

		CASE_STAT(network_name_descriptor				)
		CASE_STAT(service_list_descriptor 				)
		CASE_STAT(stuffing_descriptor 					)
		CASE_STAT(satellite_delivery_system_descriptor	)
		CASE_STAT(cable_delivery_system_descriptor 		)
		CASE_STAT(bouquet_name_descriptor				)	
		CASE_STAT(service_descriptor 					)	
		CASE_STAT(country_availability_descriptor 		)	
		CASE_STAT(linkage_descriptor 					)	
		CASE_STAT(NVOD_reference_descriptor 			)	
		CASE_STAT(time_shifted_service_descriptor 		)	
		CASE_STAT(short_event_descriptor 				)	
		CASE_STAT(extended_event_descriptor 			)	
		CASE_STAT(time_shifted_event_descriptor 		)	
		CASE_STAT(component_descriptor 					)
		CASE_STAT(mosaic_descriptor						)
		CASE_STAT(stream_identifier_descriptor 			)
		CASE_STAT(CA_identifier_descriptor 				)
		CASE_STAT(content_descriptor 					)	
		CASE_STAT(parental_rating_descriptor 			)	
		CASE_STAT(teletext_descriptor 					)	
		CASE_STAT(telephone_descriptor 					)
		CASE_STAT(local_time_offset_descriptor 			)
		CASE_STAT(subtitling_descriptor 				)	
		CASE_STAT(terrestrial_delivery_system_descriptor)	
		CASE_STAT(multilingual_network_name_descriptor 	)
		CASE_STAT(multilingual_bouquet_name_descriptor 	)
		CASE_STAT(multilingual_service_name_descriptor 	)
		CASE_STAT(multilingual_component_descriptor		)
		CASE_STAT(private_data_specifier_descriptor 	)	
		CASE_STAT(service_move_descriptor 				)	
		CASE_STAT(short_smoothing_buffer_descriptor		)
		CASE_STAT(frequency_list_descriptor 			)	
		CASE_STAT(partial_transport_stream_descriptor	)	
		CASE_STAT(data_broadcast_descriptor 			)	
		CASE_STAT(CA_system_descriptor		 			)	
		CASE_STAT(data_broadcast_id_descriptor			)	
	}

	return STR_undefined_descriptor ;
}

ushort ntols ( ushort num ) 
{
	return (num<<8) | (num>>8) ;
}
