
#include "stdafx.h"

void networkIdAsText ( int id, char *txt )
{
	char buf[256] ;

	switch (id)
	{
	case 0x0001: strcpy(buf, "Astra 19,2°E (Société Européenne des Satellites)" ) ; break ;
	case 0x0002: strcpy(buf, "Astra 28,2°E (Société Européenne des Satellites)" ) ; break ;
	case 0x0003:
	case 0x0004:
	case 0x0005:
	case 0x0006:
	case 0x0007:
	case 0x0008:
	case 0x0009:
	case 0x000A:
	case 0x000B:
	case 0x000C:
	case 0x000D:
	case 0x000E:
	case 0x000F:
	case 0x0010:
	case 0x0011:
	case 0x0012:
	case 0x0013:
	case 0x0014:
	case 0x0015:
	case 0x0016:
	case 0x0017:
	case 0x0018:
	case 0x0019: sprintf(buf, "Astra %d (Société Européenne des Satellites)", id-0x0002 ) ; break ;
	case 0x0020: strcpy(buf, "ASTRA (Société Européenne des Satellites)" ) ;break ;
	case 0x0021: strcpy(buf, "Hispasat Network 1 (Hispasat S.A.)" ) ; break ;
	case 0x0022: strcpy(buf, "Hispasat Network 2 (Hispasat S.A.)" ) ; break ;
	case 0x0023: strcpy(buf, "Hispasat Network 3 (Hispasat S.A.)" ) ; break ;
	case 0x0024: strcpy(buf, "Hispasat Network 4 (Hispasat S.A.)" ) ; break ;
	case 0x0025: strcpy(buf, "Hispasat Network 5 (Hispasat S.A.)" ) ; break ;
	case 0x0026: strcpy(buf, "Hispasat Network 6 (Hispasat S.A.)" ) ; break ;
	case 0x0027: strcpy(buf, "Hispasat 30°W      (Hispasat FSS )" ) ; break ;
	case 0x0028: strcpy(buf, "Hispasat 30°W      (Hispasat DBS )" ) ; break ;
	case 0x0029: strcpy(buf, "Hispasat 30°W      (Hispasat America )" ) ; break ;
	case 0x0030: strcpy(buf, "Canal+ Satellite Network (Canal+ SA°E)" ) ; break ;
	case 0x0031: strcpy(buf, "Hispasat  VIA DIGITAL (Hispasat S.A.)" ) ; break ;
	case 0x0032: strcpy(buf, "Hispasat Network 7 (Hispasat S.A.)" ) ; break ;
	case 0x0033: strcpy(buf, "Hisp+asat Network 8 (Hispasat S.A.)" ) ; break ;
	case 0x0034: strcpy(buf, "Hispasat Network 9 (Hispasat S.A.)" ) ; break ;
	case 0x0035: strcpy(buf, "Nethold Main Mux System (NetHold IMS)" ) ; break ;
	case 0x0036: strcpy(buf, "TV Cabo (TV Cabo Portugal)" ) ; break ;
	case 0x0037: strcpy(buf, "STENTOR (France Telecom, CNES and DGA)" ) ; break ;
	case 0x0038: strcpy(buf, "OTE(Hellenic Telecommunications Organization S.A.)" ) ; break ;
	case 0x0040: strcpy(buf, "(HPT  Croatian Post and Telecommunications)" ) ; break ;
	case 0x0041: strcpy(buf, "(Mindport)") ; break ;
	case 0x0046: strcpy(buf, "1 degree W (Telenor)" ) ; break ;
	case 0x0047: strcpy(buf, "1 degree W (Telenor)" ) ; break ;
	case 0x0049: strcpy(buf, "Sentech Digital Satellite (Sentech)" ) ; break ;
	case 0x0050: strcpy(buf, "(HRT  Croatian Radio and Television)" ) ; break ;
	case 0x0051: strcpy(buf, "Havas (Havas)" ) ; break ;
	case 0x0052: strcpy(buf, "Osaka Yusen Satellite(StarGuide Digital Networks)" ) ; break ;
	case 0x0055: strcpy(buf, "Sirius Satellite System European Coverage (NSAB (Teracom))" ) ; break ;
	case 0x0058: strcpy(buf, "Thiacom 1 & 2 co-located 78.5°E (UBC Thailand)" ) ; break ;
	case 0x005E: strcpy(buf, "Sirius Satellite System Nordic Coverage (NSAB)" ) ; break ;
	case 0x005F: strcpy(buf, "Sirius Satellite System FSS (NSAB)" ) ; break ;
	case 0x0060: strcpy(buf, "MSG MediaServices GmbH (MSG MediaServices GmbH)" ) ; break ;
	case 0x0069: strcpy(buf, "Optus B3 156°E (Optus Communications)" ) ; break ;
	case 0x0070: strcpy(buf, "BONUM1; 36 Degrees East (NTV+)" ) ; break ;
	case 0x0073: strcpy(buf, "PanAmSat 4 68.5°E (Pan American Satellite System)" ) ; break ;
	case 0x007E: strcpy(buf, "Eutelsat Satellite System at 7°E (EUTELSAT)" ) ; break ;
	case 0x007F: strcpy(buf, "Eutelsat Satellite System at 7°E (EUTELSAT)" ) ; break ;
	case 0x0085: strcpy(buf, "(BetaTechnik)" ) ; break ;
	case 0x0090: strcpy(buf, "National network (TDF)" ) ; break ;
	case 0x00A0: strcpy(buf, "National Cable Network (News Datacom)" ) ; break ;
	case 0x00A1: strcpy(buf, "News Satellite Network (News Datacom)" ) ; break ;
	case 0x00A2: strcpy(buf, "News Satellite Network (News Datacom)" ) ; break ;
	case 0x00A3: strcpy(buf, "News Satellite Network (News Datacom)" ) ; break ;
	case 0x00A4: strcpy(buf, "News Satellite Network (News Datacom)" ) ; break ;
	case 0x00A5: strcpy(buf, "News Satellite Network (News Datacom)" ) ; break ;
	case 0x00A6: strcpy(buf, "ART (ART)" ) ; break ;
	case 0x00A7: strcpy(buf, "Globecast (France Telecom)" ) ; break ;
	case 0x00A8: strcpy(buf, "Foxtel (Foxtel)" ) ; break ;
	case 0x00A9: strcpy(buf, "Sky New Zealand (Sky New Zealand)" ) ; break ;  
	case 0x00B0:
	case 0x00B1:
	case 0x00B2:
	case 0x00B3: strcpy(buf, "TPS (La Télévision Par Satellite)" ) ; break ;
	case 0x00B4: strcpy(buf, "Telesat 107.3°W (Telesat Canada)" ) ; break ;
	case 0x00B5: strcpy(buf, "Telesat 111.1°W (Telesat Canada)" ) ; break ;
	case 0x00BA: strcpy(buf, "Satellite Express 6 (80°E) (Satellite Express)" ) ; break ;
	case 0x00C0:
	case 0x00C1:
	case 0x00C2:
	case 0x00C3:
	case 0x00C4:
	case 0x00C5:
	case 0x00C6:
	case 0x00C7:
	case 0x00C8:
	case 0x00C9:
	case 0x00CA:
	case 0x00CB:
	case 0x00CC:
	case 0x00CD: strcpy(buf, "Canal+ (Canal+)" ) ; break ;
	case 0x00EB: strcpy(buf, "Eurovision Network (European Broadcasting Union)" ) ; break ;
	case 0x0100: strcpy(buf, "ExpressVu (ExpressVu Inc.)" ) ; break ;
	case 0x010E: strcpy(buf, "Eutelsat Satellite System at 10°E European (Eutelsat)" ) ; break ;
	case 0x010F: strcpy(buf, "Eutelsat Satellite System at 10°E (EUTELSAT )" ) ; break ;
	case 0x0110: strcpy(buf, "Mediaset (Mediaset)" ) ; break ;
	case 0x013E: strcpy(buf, "Eutelsat Satellite System 13°E (Eutelsat)" ) ; break ;
	case 0x013F: strcpy(buf, "Eutelsat Satellite System at 13°E (Eutelsat)" ) ; break ;
	case 0x016E: strcpy(buf, "Eutelsat Satellite System at 16°E (Eutelsat)" ) ; break ;
	case 0x016F: strcpy(buf, "Eutelsat Satellite System at 16°E (Eutelsat)" ) ; break ;
	case 0x022E: strcpy(buf, "Eutelsat Satellite System at 21.5°E (Eutelsat)" ) ; break ;
	case 0x022F: strcpy(buf, "Eutelsat Satellite System at 21.5°E (Eutelsat)" ) ; break ;
	case 0x026E: strcpy(buf, "Eutelsat Satellite System at 25.5°E (Eutelsat)" ) ; break ;
	case 0x026F: strcpy(buf, "Eutelsat Satellite System at 25.5°E (Eutelsat)" ) ; break ;
	case 0x029E: strcpy(buf, "Eutelsat Satellite System at 29°E (Eutelsat)" ) ; break ;
	case 0x029F: strcpy(buf, "Eutelsat Satellite System at 28.5°E (Eutelsat)" ) ; break ;
	case 0x02BE: strcpy(buf, "Arabsat (Arabsat Scientific Atlanta, Eutelsat)" ) ; break ;
	case 0x036E: strcpy(buf, "Eutelsat Satellite System at 36°E (Eutelsat)" ) ; break ;
	case 0x036F: strcpy(buf, "Eutelsat Satellite System at 36°E (Eutelsat)" ) ; break ;
	case 0x03E8: strcpy(buf, "Telia (Telia, Sweden)" ) ; break ;
	case 0x047E: strcpy(buf, "Eutelsat Satellite System at 12.5°W (Eutelsat)" ) ; break ;
	case 0x047F: strcpy(buf, "Eutelsat Satellite System at 12.5°W (Eutelsat)" ) ; break ;
	case 0x048E: strcpy(buf, "Eutelsat Satellite System at 48°E (Eutelsat)" ) ; break ;
	case 0x048F: strcpy(buf, "Eutelsat Satellite System at 48°E (Eutelsat)" ) ; break ;
	case 0x052E: strcpy(buf, "Eutelsat Satellite System at 8°W (Eutelsat)" ) ; break ;
	case 0x052F: strcpy(buf, "Eutelsat Satellite System at 8°W (Eutelsat)" ) ; break ;
	case 0x0800: strcpy(buf, "Nilesat 101( Nilesat)" ) ; break ; 
	case 0x0801: strcpy(buf, "Nilesat 101 (Nilesat)" ) ; break ;
	case 0x0880: strcpy(buf, "MEASAT 1, 91.5°E (MEASAT)" ) ; break ;
	case 0x0882: strcpy(buf, "MEASAT 2, 91.5°E (MEASAT )" ) ; break ;
	case 0x0883: strcpy(buf, "MEASAT 2, 148.0°E(Hsin Chi Broadcast Company Ltd.)" ) ; break ;
	case 0x088F: strcpy(buf, "MEASAT 3 (MEASAT)" ) ; break ;
	case 0x1000: strcpy(buf, "Optus B3 156°E (Optus Communications)" ) ; break ;
	case 0x1001: strcpy(buf, "DISH Network (Echostar Communications)" ) ; break ;
	case 0x1002: strcpy(buf, "Dish Network 61.5 W (Echostar Communications)" ) ; break ;
	case 0x1003: strcpy(buf, "Dish Network 83 W (Echostar Communications)" ) ; break ;
	case 0x1004: strcpy(buf, "Dish Network 119 W (Echostar Communications)" ) ; break ;
	case 0x1005: strcpy(buf, "Dish Network 121 W (Echostar Communications)" ) ; break ;
	case 0x1006: strcpy(buf, "Dish Network 148 W (Echostar Communications)" ) ; break ;
	case 0x1007: strcpy(buf, "Dish Network 175 W (Echostar Communications)" ) ; break ;
	case 0x1008: strcpy(buf, "Dish Network W (Echostar Communications)" ) ; break ;
	case 0x1009: strcpy(buf, "Dish Network X (Echostar Communications)" ) ;break ;
	case 0x100A: strcpy(buf, "Dish Network Y (Echostar Communications)" ) ; break ;
	case 0x100B: strcpy(buf, "Dish Network Z (Echostar Communications)" ) ; break ;
	case 0x1011: strcpy(buf, "ABC Network Australia (ABC Network Australia)" ) ; break ;
	case 0x1012: strcpy(buf, "SBS Network Australia (SBS Network Australia)" ) ; break ;
	case 0x1013: strcpy(buf, "Prime Network Australia (Prime Network Australia)" ) ;break ;
	case 0x1014: strcpy(buf, "WIN Network Australia (WIN Network Australia)" ) ; break ;
	case 0x1017: strcpy(buf, "Seven Network Australia (Seven Network Australia)" ) ; break ;
	case 0x1019: strcpy(buf, "Nine Network Australia (Nine Network Australia)" ) ; break ;
	case 0x101A: strcpy(buf, "Ten Network Australia (Ten Network Australia)" ) ; break ;
	case 0x101B: strcpy(buf, "Ten Regional Network Australia (Ten Regional Network Australia)" ) ; break ;
	case 0x2000: strcpy(buf, "Thiacom 1 & 2 co-located 78.5°E (Shinawatra Satellite)" ) ; break ;
	case 0x22D4: strcpy(buf, "Spanish Digital Terrestrial Television (“Spanish Broadcasting Regulator)" ) ; break ;
	case 0x22F1: strcpy(buf, "Swedish Digital Terrestrial Television (“Swedish Broadcasting Regulator”)" ) ;break ;
	case 0x233A: strcpy(buf, "UK Digital Terrestrial Television (Independent Television Commission)" ) ; break ;
	case 0x2024: strcpy(buf, "Australian Digital Terrestrial Television (Australian Broadcasting Authority)" ) ; break ;
	case 0x2114: strcpy(buf, "German Digital Terrestrial Television (German DVB-T broadcasts)" ) ; break ;
	case 0x2174: strcpy(buf, "Irish Digital Terrestrial Television (Irish Telecommunications Regulator)" ) ; break ;
	case 0x2178: strcpy(buf, "Israeli Digital Terrestrial Television (BEZEQ )" ) ; break ;
	case 0x20F6: strcpy(buf, "Finnish Digital Terrestrial Television (TAC, Finland)" ) ; break ;
	case 0x3000: strcpy(buf, "PanAmSat 4 68.5°E (Pan American Satellite System)" ) ; break ;
	case 0x5000: strcpy(buf, "Irdeto Mux System (Irdeto Test Laboratories)" ) ; break ;
	case 0x616D: strcpy(buf, "BellSouth Entertainment (BellSouth Entertainment, Atlanta, GA, USA)" ) ; break ;
	case 0xF000: strcpy(buf, "Small Cable networks (Small cable network network operators)" ) ; break ;
	case 0xF001: strcpy(buf, "Deutsche Telekom (Deutsche Telekom AG)" ) ; break ;
	case 0xF010: strcpy(buf, "Telefónica Cable (Telefónica Cable SA)" ) ; break ;
	case 0xF020: strcpy(buf, "Cable and Wireless Communication (Cable and Wireless Communications) " ) ; break ;
	case 0xF100: strcpy(buf, "Casema (Casema N.V.)" ) ; break ;
	case 0xFBFC: strcpy(buf, "MATAV (MATAV Israel)" ) ; break ;
	case 0xFBFD: strcpy(buf, "Telia Kabel-TV (Telia, Sweden)" ) ; break ;
	case 0xFBFE: strcpy(buf, "TPS (La Télévision Par Satellite)" ) ; break ;
	case 0xFBFF: strcpy(buf, "Stream (Stream Spa.)" ) ; break ;
	case 0xFC10: strcpy(buf, "Rhône Vision Cable (Rhône Vision Cable)" ) ; break ;
	case 0xFC41: strcpy(buf, "France Telecom Cable (France Telecom)" ) ; break ;
	case 0xFD00: strcpy(buf, "National Cable Network (Lyonnaise Communications)" ) ; break ;
	case 0xFE00: strcpy(buf, "TeleDenmark Cable TV (TeleDenmark)" ) ; break ;
	case 0xF750: strcpy(buf, "Telewest Communications Cable Network (Telewest Communications Plc)" ) ; break ;

	//case 0xFEC0-FF00 Network Interface Modules Common Interface
	default: strcpy(buf, ""); break ;
	} ;

	if (id>=0xFF00 && id<0xFFFF)
		strcpy(buf, "Private temporary use" ) ;

	if ( buf[0]!='\x0' )
	{
		sprintf(txt, "0x%x : ", id) ;
		strupr (txt+2) ;
		strcat (txt, buf) ;
	}
	else
		strcpy(txt, "") ;
}

