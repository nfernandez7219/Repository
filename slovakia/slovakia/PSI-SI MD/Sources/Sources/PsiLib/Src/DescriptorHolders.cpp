
#include "stdafx.h"
#include "TableHolders.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL chooseLanguage ( uchar *lang, CWnd *pParent ) ;
BOOL RunMultiLingNameDlg(BOOL bProvider, char *data, int *size ) ;

DescriptorHolder::DescriptorHolder ( uchar descTag )
 : BaseHolder(descTag)
{
}

uchar *DescriptorHolder::loadFromBin ( uchar *buffer )
{
	#pragma message("DescriptorHolder::loadFromBin - ziadna kontrola platnosti descriptora - mozne vybehnutie z buffra")
	BaseDescriptor *desc = (BaseDescriptor*)buffer ;
	int len = desc->getLength() ;
	memcpy(_body, buffer, len) ;

	setHolderType(getType());

	return buffer + len ;
}

uchar *DescriptorHolder::saveToBin ( uchar *buffer )
{
	int len = getLength() ;
	memcpy( buffer, _body, len ) ;
	return buffer + len ;
} ;

static void DescriptorHolderDelFn(DescriptorHolder** p)
{
	delete *p ;
}

DescriptorArray::DescriptorArray() 
// :	BaseHolder(DESC_ARRAY)
{
	setDelFunc( DescriptorHolderDelFn ) ;
}

void DescriptorArray::loadFromBin( uchar *buffer, int size )
{
	uchar *p = buffer ;
	while ( p < buffer+size )
	{
		BaseDescriptor *desc = (BaseDescriptor*)p ;
		DescriptorHolder *holder = appendDesc(desc->getType()) ;
		if (holder)
			p = holder->loadFromBin(p) ;
		else
			p = p + desc->getLength() ;
	}
}

uchar* DescriptorArray::saveToBin ( uchar *buffer )
{
	for ( int i = 0; i < count(); i++ )
		buffer = item(i)->saveToBin(buffer) ;

	return buffer ;
}

DescriptorHolder *DescriptorArray::findHolder( uchar type )
{
	int i = count() ;

	while (i--)
	{
		DescriptorHolder *holder = item(i) ;
		if (holder->getType()==type)
			return holder ;
	}

	return NULL ;
}

DescriptorHolder* DescriptorArray::appendDesc( int type )
{
	DescriptorHolder *holder ;

	switch(type)
	{
		case TAG_service_list_descriptor: 				
		case TAG_stuffing_descriptor: 					
		case TAG_cable_delivery_system_descriptor: 		
		case TAG_bouquet_name_descriptor:				
		case TAG_NVOD_reference_descriptor: 				
		case TAG_time_shifted_service_descriptor: 		
		case TAG_short_event_descriptor: 				
		case TAG_extended_event_descriptor: 				
		case TAG_time_shifted_event_descriptor: 			
		case TAG_component_descriptor: 					
		case TAG_mosaic_descriptor:						
		case TAG_content_descriptor: 					
		case TAG_parental_rating_descriptor: 			
		case TAG_telephone_descriptor: 					
		case TAG_local_time_offset_descriptor: 			
		case TAG_terrestrial_delivery_system_descriptor: 
		case TAG_multilingual_bouquet_name_descriptor: 	
		case TAG_multilingual_component_descriptor:		
		case TAG_service_move_descriptor: 				
		case TAG_short_smoothing_buffer_descriptor:		
		case TAG_frequency_list_descriptor: 				
		case TAG_partial_transport_stream_descriptor:
			return NULL ;

		case TAG_audio_stream_descriptor:
			holder = new AudioStreamDescHolder;
			break ;
		case TAG_video_stream_descriptor:
			holder = new VideoStreamDescHolder;
			break ;
		case TAG_stream_identifier_descriptor:
			holder = new StreamIdentifierDescHolder ;
			break ;
		case TAG_subtitling_descriptor:
			holder = new SubtitlingDescHolder ;
			break ;
		case TAG_teletext_descriptor: 					
			holder = new TeletextDescHolder ; 
			break ;
		case TAG_CA_identifier_descriptor:
			holder = new CAIdentifierDescHolder ;
			break ;
		case TAG_country_availability_descriptor:
		{
			CountryAvailabilityDescHolder *desc1 = new CountryAvailabilityDescHolder(TRUE);
			CountryAvailabilityDescHolder *desc2 = new CountryAvailabilityDescHolder(FALSE);
			desc1->setPairHolder(desc2) ;
			desc2->setPairHolder(desc1) ;
			add(desc1) ;
			add(desc2) ;
			return desc1 ;
		}
		case TAG_linkage_descriptor:
			holder = new LinkageDescHolder ;
			break ;
		case TAG_ISO_639_language_descriptor:
			holder = new LanguageDescHolder ;
			break ;
		case TAG_multilingual_service_name_descriptor:
			holder = new MultiLingServiceNameDescHolder ;
			break ;
		case TAG_multilingual_network_name_descriptor:
			holder = new MultiLingNetworkNameDescHolder ;
			break;
		case TAG_network_name_descriptor:
			holder = new NetworkNameDescHolder ; 
			break ;
		case TAG_private_data_specifier_descriptor:
			holder = new PrivateDataDescHolder ;
			break ;
		case TAG_satellite_delivery_system_descriptor:
			holder = new SatelliteDeliveryDescHolder ;
			break ;
		case TAG_service_descriptor: 					
			holder = new ServiceDescHolder ;
			break ;
		case TAG_data_broadcast_descriptor:
			holder = new DataBroadcastDescHolder ;
			break ;
		case TAG_CA_descriptor:		 			
			holder = new CASystemDescHolder ;
			break ;
		case TAG_data_broadcast_id_descriptor:
			holder = new DataBroadcastIdDescHolder ;
			break ;
		default:
			return NULL ;
	}

	add(holder) ;
	return holder ;
}

						////////////////////////
						// Descriptor holders //
						////////////////////////


////////////////////////
//	CASystemDescHolder

void CASystemDescHolder::getItemName	( int itemIndex, char *buf )
{
	switch(itemIndex)
	{
		case 0 : strcpy(buf,"CA System") ; break ;
		case 1 : strcpy(buf,_isInCAT?"EMM PID":"ECM PID") ; break ;
		case 2 : strcpy(buf,"Private data") ; break ;
	}
}

void CASystemDescHolder::getValueText	( int itemIndex, char *buf )
{
	CASystemDescriptor *caDesc = (CASystemDescriptor*)getBaseDesc() ;
	if (itemIndex==0)
	{
		ushort id = caDesc->getCaSystemId() ;
		CASystemDescriptor::caSystemAsText(id, buf) ;
		return ;
	}
	if (itemIndex==1)
	{
		int pid = caDesc->getCaSystemPid() ;
		sprintf(buf,"0x%x", pid);
		strupr(buf+2);
		return ;
	}
	if (itemIndex==2)
	{
		int len = caDesc->getPrivateDataLength() ;
		HexStringToString(caDesc->private_data, len, buf) ;
	}
}

void CASystemDescHolder::getTextDescription( char *buf )
{
	CASystemDescriptor *caDesc = getDesc() ;
	ushort id = caDesc->getCaSystemId() ;
	char caSystem[256] ;
	CASystemDescriptor::caSystemAsText(id, caSystem) ;
	int pid = caDesc->getCaSystemPid() ;

	sprintf(buf,"%s (%s PID 0x%x)", caSystem, _isInCAT?"EMM":"ECM", pid);
}

static BOOL editCaSystem( int *id )
{
	DECLARE_EDIT_ENUM(35) ;
	#define EDIT_ENUM_TEXT_ENUMERATOR CASystemDescriptor::caSystemAsText

	ADD_ENUM_VALUE(0x0000)
	ADD_ENUM_VALUE(0x0100)
	ADD_ENUM_VALUE(0x0200)
	ADD_ENUM_VALUE(0x0300)
	ADD_ENUM_VALUE(0x0400)
	ADD_ENUM_VALUE(0x0500)
	ADD_ENUM_VALUE(0x0600)
	ADD_ENUM_VALUE(0x0700)
	ADD_ENUM_VALUE(0x0800)
	ADD_ENUM_VALUE(0x0900)
	ADD_ENUM_VALUE(0x0A00)
	ADD_ENUM_VALUE(0x0B00)
	ADD_ENUM_VALUE(0x0C00)
	ADD_ENUM_VALUE(0x0D00)
	ADD_ENUM_VALUE(0x0E00)
	ADD_ENUM_VALUE(0x0F00)
	ADD_ENUM_VALUE(0x1000)
	ADD_ENUM_VALUE(0x1100)
	ADD_ENUM_VALUE(0x1200)
	ADD_ENUM_VALUE(0x1300)
	ADD_ENUM_VALUE(0x1400)
	ADD_ENUM_VALUE(0x1500)
	ADD_ENUM_VALUE(0x1600)
	ADD_ENUM_VALUE(0x1700)
	ADD_ENUM_VALUE(0x1800)
	ADD_ENUM_VALUE(0x1900)
	ADD_ENUM_VALUE(0x2000)
	ADD_ENUM_VALUE(0x2100)
	ADD_ENUM_VALUE(0x2200)
	ADD_ENUM_VALUE(0x2300)
	ADD_ENUM_VALUE(0x2400)
	ADD_ENUM_VALUE(0x2500)
	ADD_ENUM_VALUE(0x2600)
	ADD_ENUM_VALUE(0x4700)
	ADD_ENUM_VALUE(0x4800)

	BOOL changed = EditEnumValue(
		TRUE,
		"CA system Id",
		"Selects Conditional Access System. In the list you will see the values range allocated for specific vendor. You may edit selected range of values by retyping",
		35,
		ENUM_VALS(),
		ENUM_TEXTS(),
		id
	) ;

	FREE_ENUM_VALUES() ;
	#undef EDIT_ENUM_TEXT_ENUMERATOR

	return changed ;
}

BOOL CAIdentifierDescHolder::editItem( int itemIndex )
{
	int selected = getDesc()->getCaSystemId() ;

	BOOL changed = editCaSystem(&selected) ;

	if (changed)
	{
		_changeFlag = TRUE ;
		getDesc()->setCaSystemId(selected) ;
	}

	return changed ;
}

BOOL CASystemDescHolder::editItem( int itemIndex )
{
	CASystemDescriptor *desc = getDesc() ;

	if(itemIndex==0)
	{
		int selected = desc->getCaSystemId() ;

		BOOL changed = editCaSystem(&selected) ;

		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setCaSystemId(selected) ;
			AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_RefreshTreeItem) ;
		}

		return changed ;
	}

	if(itemIndex==1)
	{
		int pid = desc->getCaSystemPid() ;
	
		char buf[256] ;
		sprintf(buf,
			"Enter the PID of the TS packets which shall contain %s information "
			"for the CA systems" ,
			_isInCAT?"EMM":"ECM"
		) ;

		BOOL changed ;
		while(1)
		{
			changed = EditIntValue(
				TRUE,
				_isInCAT?"EMM PID":"ECM PID",
				buf,
				&pid, 
				0x20, 
				0x1FFF
			) ;

			if (!changed)
				return FALSE ;

			if (TableHolder::CheckPidUse(pid, buf))
			{
				char txt[256] ;
				sprintf(txt, 
					"PID 0x%x is already used in :\n\n%s\n\n"
					"Choose another value.",
					pid, buf
				) ;
				AfxGetMainWnd()->MessageBox( txt, "Error", MB_OK | MB_ICONERROR ) ;
				continue ;
			}

			_changeFlag = TRUE ;
			desc->setCaSystemPid( (ushort)pid ) ;
			AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_RefreshTreeItem) ;
			break ;
		}

		return changed ;
	}
	if (itemIndex==2)
	{
		int len = desc->getPrivateDataLength() ;
		uchar data[256] ;
		memcpy(data, desc->private_data, len) ;
		
		BOOL changed = EditHexStringValue("Private data","Enter hex-string", len, data) ;
		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setPrivateData(len, data);
		}
		return changed ;
	}

	return FALSE ;
}

///////////////////////////////////
//	CountryAvailabilityDescHolder

int	 CountryAvailabilityDescHolder::getItemNumber	( )
{
	int nItems = descItemNumber()+_pair->descItemNumber() ;
	return nItems?nItems:1 ;
}

void CountryAvailabilityDescHolder::getItemName( int itemIndex, char *buf )
{
	int nCountries = descItemNumber() ;

	if ( !(itemIndex || nCountries || _pair->descItemNumber()) )
	{
		strcpy(buf, "Click here to add Available/Not available country") ;
		return ;
	}

	if (itemIndex >= nCountries)
	{
		_pair->getItemName(itemIndex - nCountries, buf) ;
		return ;
	}

	CountryStruct *country = &getDesc()->countries[itemIndex] ;
	CountryAvailabilityDescriptor::countryText( country->country_id, buf );
}

void CountryAvailabilityDescHolder::getValueText	( int itemIndex, char *buf )
{
	int nCountries = descItemNumber() ;

	if ( !(itemIndex || nCountries || _pair->descItemNumber()) )
		return ;

	BOOL bAvailable = getDesc()->countriesAvailable() ;
	if (itemIndex >= nCountries)
		bAvailable = !bAvailable ;

	char *avSt = bAvailable?"Available":"Not available" ;
	strcpy(buf, avSt) ;
}

BOOL RunCountryAvailDlg(CountryAvailabilityDescriptor *desc1, CountryAvailabilityDescriptor *desc2) ;

BOOL CountryAvailabilityDescHolder::editItem( int itemIndex )
{
	BOOL bChanged = RunCountryAvailDlg(getDesc(), _pair->getDesc() ) ;
	if (bChanged)
	{
		_changeFlag = TRUE ;
		AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_RefreshValueList, (LPARAM)this ) ;
	}
	return bChanged ;
}

/////////////////////////////
//	DataBroadcastDescHolder

static BOOL editBroadcastId( int *id )
{
	int		vals[21] ;
	LPTSTR	texts[21] ;
	int i=0 ;

	vals[i++] = DataBroadcastDescriptor::Data_pipe ;
	vals[i++] = DataBroadcastDescriptor::Asynchronous_data_stream;
	vals[i++] = DataBroadcastDescriptor::Synchronous_data_stream;
	vals[i++] = DataBroadcastDescriptor::Synchronised_data_stream;
	vals[i++] = DataBroadcastDescriptor::Multi_protocol_encapsulation;
	vals[i++] = DataBroadcastDescriptor::Data_Carousel;
	vals[i++] = DataBroadcastDescriptor::Object_Carousel;
	vals[i++] = DataBroadcastDescriptor::DVB_ATM_streams;

	vals[i++] = DataBroadcastDescriptor::Eutelsat_Data_Piping;
	vals[i++] = DataBroadcastDescriptor::Eutelsat_Data_Streaming;
	vals[i++] = DataBroadcastDescriptor::SAGEM_IP_encapsulation;
	vals[i++] = DataBroadcastDescriptor::BARCO_Data_Broadcasting;
	vals[i++] = DataBroadcastDescriptor::CyberCity_Multiprotocol_Encapsulation;
	vals[i++] = DataBroadcastDescriptor::CyberSat_Multiprotocol_Encapsulation;
	vals[i++] = DataBroadcastDescriptor::The_Digital_Network;
	vals[i++] = DataBroadcastDescriptor::OpenTV_Data_Carousel;
	vals[i++] = DataBroadcastDescriptor::Panasonic;
	vals[i++] = DataBroadcastDescriptor::MSG_MediaServices;
	vals[i++] = DataBroadcastDescriptor::Televizja_Polsat;
	vals[i++] = DataBroadcastDescriptor::UK_DTG;
	vals[i++] = DataBroadcastDescriptor::SkyMedia;
						
	i = 21 ;
	while (i--)
	{
		char b[128] ;
		DataBroadcastDescriptor::broadcastIdAsText(vals[i], b) ;
		texts[i] = strdup(b) ;
	}

	BOOL changed = EditEnumValue(
		FALSE,
		"Data broadcast Id",
		"Select type of broadcast service",
		21,
		vals,
		texts,
		id
	) ;

	i = 21 ;
	while (i--)
		free(texts[i]) ;

	return changed ;
}

void DataBroadcastDescHolder::getItemName	( int itemIndex, char *buf )
{
	switch(itemIndex)
	{
		case 0 : strcpy(buf,"Broadcast ID") ; break ;
		case 1 : strcpy(buf,"Description") ; break ;
	}
}

void DataBroadcastDescHolder::getValueText	( int itemIndex, char *buf )
{
	DataBroadcastDescriptor *desc = (DataBroadcastDescriptor*)getBaseDesc() ;
	if(itemIndex==0)
	{
		ushort id = desc->getBroadcastId() ;
		DataBroadcastDescriptor::broadcastIdAsText(id, buf) ;
		return ;
	}
	if(itemIndex==1)
	{
		desc->getDescription(buf) ;
		return ;
	}
}

void DataBroadcastDescHolder::getTextDescription( char *buf )
{
	ushort id = getDesc()->getBroadcastId() ;
	DataBroadcastDescriptor::broadcastIdAsText(id, buf) ;
}

BOOL DataBroadcastDescHolder::editItem( int itemIndex )
{
	if (itemIndex==0)
	{	
		DataBroadcastDescriptor *desc = getDesc() ;
		int selected = desc->getBroadcastId() ;
		char brDescription[256] ;
		desc->getDescription(brDescription) ;
		
		BOOL changed = editBroadcastId(&selected) ;

		if (changed)
		{
			_changeFlag = TRUE ;
			desc->create(selected, brDescription) ;
		}

		return changed ;
	}
	return FALSE ;
}

///////////////////////////////
//	DataBroadcastIdDescHolder

void DataBroadcastIdDescHolder::getItemName	( int itemIndex, char *buf )
{
	if(itemIndex==0)
		strcpy(buf,"Broadcast ID") ;
}

void DataBroadcastIdDescHolder::getValueText	( int itemIndex, char *buf )
{
	ushort id = getDesc()->getBroadcastId() ;
	DataBroadcastDescriptor::broadcastIdAsText(id, buf) ;
}

void DataBroadcastIdDescHolder::getTextDescription( char *buf )
{
	ushort id = getDesc()->getBroadcastId() ;
	DataBroadcastDescriptor::broadcastIdAsText(id, buf) ;
}

BOOL DataBroadcastIdDescHolder::editItem( int itemIndex )
{
	if (itemIndex!=0)
		return FALSE ;
	
	DataBroadcastIdDescriptor *desc = getDesc() ;
	int selected = desc->getBroadcastId() ;
	
	BOOL changed = editBroadcastId(&selected) ;

	if (changed)
	{
		_changeFlag = TRUE ;
		desc->create(selected) ;
	}

	return changed ;
}

////////////////////////
//	LanguageDescHolder

/*
#define LANGUAGES_NUMBER	2
LPCSTR s_stLanguages[LANGUAGES_NUMBER] =
{
	"cze (Czech Republic)",
	"slo (Slovak)",
} ;
*/

class CLanguageDlg : public CDialog
{
	LanguageDescHolder*		_Lang ;
	int						_item ;

protected:
	virtual BOOL OnInitDialog() ;
	virtual void OnOK() ;

public:
	CLanguageDlg (LanguageDescHolder *lang, int item) : CDialog(IDD_LanguageDlg, AfxGetMainWnd()) { _Lang = lang; _item=item ; }
} ;

BOOL CLanguageDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	LanguageStruct *langStr = &_Lang->getDesc()->languages[_item] ;
	const char *language = (const char*)langStr->lang_code ;

	CComboBox *langList = (CComboBox*)GetDlgItem(IDC_cbLanguage) ;
	for ( int i = 0; i < (int)LANGUAGES_NUMBER; i++ )
	{
		int index = langList->AddString(s_stLanguages[i]) ;
		if (strncmp(language , s_stLanguages[i], 3)==0)
			langList->SetCurSel(index) ;
	}

	((CComboBox*)GetDlgItem(IDC_cbAudioType))->SetCurSel(langStr->audio_type-1) ;

	return TRUE ;
}

void CLanguageDlg::OnOK()
{
	CComboBox *langList = (CComboBox*)GetDlgItem(IDC_cbLanguage) ;
	int i = langList->GetCurSel() ;
	char st[128] ;
	langList->GetLBText(i, st) ;

	int type = 1+((CComboBox*)GetDlgItem(IDC_cbAudioType))->GetCurSel() ;

	_Lang->getDesc()->setLang(_item, st, type) ;

	CDialog::OnOK() ;
}

void LanguageDescHolder::getItemName( int itemIndex, char *buf )
{
	if (itemIndex==0)
	{
		strcpy(buf,"Add new language") ;
		return ;
	}

	uchar *lang = getDesc()->languages[itemIndex-1].lang_code ;
	LanguageDescriptor::langAsText(lang, buf) ;
}

void LanguageDescHolder::getValueText( int itemIndex, char *buf )
{
	if (itemIndex==0)
		return ;

	int audioType = getDesc()->languages[itemIndex-1].audio_type ;
	LanguageDescriptor::audioTypeAsText(audioType, buf) ;
}

BOOL LanguageDescHolder::editItem( int itemIndex )
{
	BOOL bNew = (itemIndex==0) ;
	if (bNew)
	{
		itemIndex = getDesc()->getLangNumber()+1 ;
		getDesc()->languages[itemIndex-1].audio_type = 1 ;
	}

	CLanguageDlg dlg(this, itemIndex-1) ;
	BOOL bChanged = dlg.DoModal()==IDOK ;

	if (bChanged && bNew)
		AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_RefreshValueList, (LPARAM)this ) ;

	return bChanged ;
}

//////////////////////
//	LinkageDescHolder

void LinkageDescHolder::getItemName	( int itemIndex, char *buf )
{
	switch(itemIndex)
	{
		case 0 : strcpy(buf,"Transport stream ID") ; break ;
		case 1 : strcpy(buf,"Original network ID") ; break ;
		case 2 : strcpy(buf,"Service ID") ; break ;
		case 3 : strcpy(buf,"Linkage type") ; break ;
		case 4 : strcpy(buf,"Private data") ; break ;
	}
}

void LinkageDescHolder::getValueText	( int itemIndex, char *buf )
{
	LinkageDescriptor *desc = getDesc() ;

	switch (itemIndex)
	{
	case 0: itoa(desc->getTransportStreamId(), buf, 10) ; return ;
	case 1: networkIdAsText(desc->getOriginalNetworkId(), buf) ; return ;
	case 2: itoa(desc->getServiceId(), buf, 10) ; return ;
	case 3:	LinkageDescriptor::linkageTypeAsText(desc->getLinkageType(), buf) ; return ;
	case 4:	HexStringToString(desc->private_data, desc->getPrivateDataLength(), buf) ; return ;
	} ;
}

void LinkageDescHolder::getTextDescription( char *buf )
{
	LinkageDescriptor *desc = getDesc() ;
	LinkageDescriptor::linkageTypeAsText(desc->getLinkageType(), buf) ;
	buf += strlen(buf) ;
	sprintf(buf, "(Serv:%d, TS:%d, Ntw:%d)",
		desc->getServiceId(),
		desc->getTransportStreamId(),
		desc->getOriginalNetworkId()
	) ;
}

BOOL LinkageDescHolder::editItem( int itemIndex )
{
	LinkageDescriptor *desc = getDesc() ;

	if (itemIndex==0)
	{
		int id = desc->getTransportStreamId() ;
		
		BOOL changed = EditIntValue(
			FALSE,
			"Transport Stream ID",
			"Enter ID of the transport stream which should be linked with specified service.",
			&id, 
			0,
			0xFFFF
		) ;

		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setTransportStreamId(id) ;
		}
		return changed ;
	}

	if (itemIndex==1)
	{
		int id = desc->getOriginalNetworkId();

		if (editNetworkId(&id, TRUE))
		{
			desc->setOriginalNetworkId(id) ;
			_changeFlag = TRUE ;
			return TRUE ;
		}
	}

	if (itemIndex==2)
	{
		int id = desc->getServiceId() ;
		
		BOOL changed = EditIntValue(
			FALSE,
			"Service ID",
			"Enter ID of the service which should be linked with specified transport stream.",
			&id, 
			0,
			0xFFFF
		) ;

		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setServiceId(id) ;
		}
		return changed ;
	}

	if (itemIndex==3)
	{
		DECLARE_EDIT_ENUM(10) ;
		#define EDIT_ENUM_TEXT_ENUMERATOR LinkageDescriptor::linkageTypeAsText

		ADD_ENUM_VALUE(LinkageDescriptor::information_service)
		ADD_ENUM_VALUE(LinkageDescriptor::EPG_service)
		ADD_ENUM_VALUE(LinkageDescriptor::CA_replacement_service)
		ADD_ENUM_VALUE(LinkageDescriptor::complete_Network_SI)
		ADD_ENUM_VALUE(LinkageDescriptor::service_replacement_service)
		ADD_ENUM_VALUE(LinkageDescriptor::data_broadcast_service)

		int selected = desc->getLinkageType() ;

		BOOL changed = EditEnumValue(
			FALSE,
			"Linkage type",
			"Select type of linkage",
			6,
			ENUM_VALS(),
			ENUM_TEXTS(),
			&selected
		) ;

		FREE_ENUM_VALUES() ;
		#undef EDIT_ENUM_TEXT_ENUMERATOR

		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setLinkageType(selected) ;
		}

		return changed ;
	}

	if (itemIndex==4)
	{
		int len = desc->getPrivateDataLength() ;
		uchar data[256] ;
		memcpy(data, desc->private_data, len) ;
		
		BOOL changed = EditHexStringValue("Private data","Enter hex-string", len, data) ;
		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setPrivateData(len, data);
		}
		return changed ;
	}

	return FALSE ;
}

////////////////////////////////////
//	MultiLingServiceNameDescHolder

int	 MultiLingServiceNameDescHolder::getItemNumber	( )
{ 
	_nItems = getDesc()->getNamesNumber(); 
	return _nItems?_nItems:1 ;
}

void MultiLingServiceNameDescHolder::getItemName	( int itemIndex, char *buf )
{
	if ( _nItems==0 )
	{
		strcpy(buf, "Click here to add new language name") ;
		return ;
	}

	getDesc()->getLanguage(itemIndex,buf) ;
}

void MultiLingServiceNameDescHolder::getValueText	( int itemIndex, char *buf )
{
	if ( _nItems==0 )
		return ;

	char prov[256], name[256] ;
	getDesc()->getProvider(itemIndex,prov) ;
	getDesc()->getName(itemIndex,name) ;
	sprintf(buf, "Prov: %s, Name: %s ", prov, name) ;
}


BOOL MultiLingServiceNameDescHolder::editItem		( int itemIndex )
{
	int len = getDesc()->descriptor_length ;
	BOOL changed = RunMultiLingNameDlg(TRUE, (char*)getDesc()->names, &len) ;
	if (changed)
	{
		_changeFlag = TRUE ;
		getDesc()->setSize(len) ;
		_nItems = getDesc()->getNamesNumber(); 
		AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_RefreshValueList, (LPARAM)this ) ;
	}
	return changed ;
}

////////////////////////////////////
//	MultiLingNetworkNameDescHolder

int	 MultiLingNetworkNameDescHolder::getItemNumber	( )
{ 
	_nItems = getDesc()->getNamesNumber(); 
	return _nItems?_nItems:1 ;
}

void MultiLingNetworkNameDescHolder::getItemName	( int itemIndex, char *buf )
{
	if ( _nItems==0 )
	{
		strcpy(buf, "Click here to add new language name") ;
		return ;
	}

	getDesc()->getLanguage(itemIndex,buf) ;
}

void MultiLingNetworkNameDescHolder::getValueText	( int itemIndex, char *buf )
{
	if ( _nItems==0 )
		return ;
	
	getDesc()->getName(itemIndex,buf) ;
}


BOOL MultiLingNetworkNameDescHolder::editItem		( int itemIndex )
{
	int len = getDesc()->descriptor_length ;
	BOOL changed = RunMultiLingNameDlg(FALSE, (char*)getDesc()->names, &len) ;
	if (changed)
	{
		_changeFlag = TRUE ;
		getDesc()->setSize(len) ;
		_nItems = getDesc()->getNamesNumber(); 
		AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_RefreshValueList, (LPARAM)this ) ;
	}
	return changed ;
}

///////////////////////////
//	NetworkNameDescHolder

void NetworkNameDescHolder::getItemName	( int itemIndex, char *buf )
{
	strcpy(buf, "Network name") ;
}

void NetworkNameDescHolder::getValueText	( int itemIndex, char *buf )
{
	if (itemIndex==0)
	{
		NetworkNameDescriptor *desc = (NetworkNameDescriptor*)getBaseDesc() ;
		desc->getName(buf) ;
	}
}

BOOL NetworkNameDescHolder::editItem( int itemIndex )
{
	char name[255] ;

	getDesc()->getName(name) ;

	BOOL changed = EditStringValue("Network name","",name) ;

	if (changed)
	{
		_changeFlag = TRUE ;
		getDesc()->create(name) ;
	}

	return changed ;
}

////////////////////////
//	ServiceDescHolder

void ServiceDescHolder::getItemName	( int itemIndex, char *buf )
{
	switch(itemIndex)
	{
		case 0 : strcpy(buf,"Service type") ; break ;
		case 1 : strcpy(buf,"Provider name") ; break ;
		case 2 : strcpy(buf,"Service name") ; break ;
	}
}

void ServiceDescHolder::getValueText	( int itemIndex, char *buf )
{
	ServiceDescriptor *servDesc = (ServiceDescriptor*)getBaseDesc() ;
	if (itemIndex==0)
	{
		uchar type = servDesc->getServiceType() ;
		ServiceDescriptor::serviceTypeAsText(type, buf) ;
		return ;
	}
	if (itemIndex==1)
	{
		servDesc->getServiceProviderName(buf) ;
		return ;
	}
	if (itemIndex==2)
		servDesc->getServiceName(buf) ;
}

void ServiceDescHolder::getTextDescription( char *buf )
{
	ServiceDescriptor *servDesc = getDesc() ;

	char name[255], provider[255], servType[64] ;
	servDesc->getServiceName(name) ;
	servDesc->getServiceProviderName(provider) ;
	uchar type = servDesc->getServiceType() ;
	ServiceDescriptor::serviceTypeAsText(type, servType) ;

	sprintf(buf,"%s (%s) - %s", name, provider, servType) ;
}

BOOL ServiceDescHolder::editItem( int itemIndex )
{
	ServiceDescriptor *desc = getDesc() ;

	if (itemIndex==0)
	{
		int		vals[12] ;
		LPTSTR	texts[12] ;
		int i=0 ;

		vals[i++] = ServiceDescriptor::Digital_television_service;
		vals[i++] = ServiceDescriptor::Digital_radio_sound_service;
		vals[i++] = ServiceDescriptor::Teletext_service;
		vals[i++] = ServiceDescriptor::NVOD_reference_service;
		vals[i++] = ServiceDescriptor::NVOD_time_shifted_service;
		vals[i++] = ServiceDescriptor::Mosaic_service;
		vals[i++] = ServiceDescriptor::PAL_coded_signal;
		vals[i++] = ServiceDescriptor::SECAM_coded_signal;
		vals[i++] = ServiceDescriptor::D_D2_MAC;
		vals[i++] = ServiceDescriptor::FM_Radio;
		vals[i++] = ServiceDescriptor::NTSC_coded_signal;
		vals[i++] = ServiceDescriptor::Data_broadcast_service ;

		i = 12 ;
		while (i--)
		{
			char b[128] ;
			ServiceDescriptor::serviceTypeAsText(vals[i], b) ;
			texts[i] = strdup(b) ;
		}

		int selected = desc->getServiceType() ;

		BOOL changed = EditEnumValue(
			FALSE,
			"Service type",
			"Select type of service",
			12,
			vals,
			texts,
			&selected
		) ;

		i = 12 ;
		while (i--)
			free(texts[i]) ;

		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setServiceType(selected) ;
		}

		return changed ;
	}

	if (itemIndex==1)
	{
		char provName[256] ;
		desc->getServiceProviderName(provName) ;
	
		BOOL changed = EditStringValue("Service provider","",provName) ;

		if (changed)
		{
			char servName[256] ;
			uchar type = desc->getServiceType() ;
			desc->getServiceName(servName) ;
			desc->create(type, provName, servName) ;
			_changeFlag = TRUE ;
		}

		return changed ;
	}

	if (itemIndex==2)
	{
		char servName[256] ;
		desc->getServiceName(servName) ;
	
		BOOL changed = EditStringValue("Service name","",servName) ;

		if (changed)
		{
			char provName[256] ;
			uchar type = desc->getServiceType() ;
			desc->getServiceProviderName(provName) ;
			desc->create(type, provName, servName) ;
			_changeFlag = TRUE ;
		}

		return changed ;
	}

	return FALSE ;
}

/////////////////////////////////
//	SatelliteDeliveryDescHolder

void SatelliteDeliveryDescHolder::getItemName	( int itemIndex, char *buf )
{
	switch(itemIndex)
	{
		case 0 : strcpy(buf,"Frequency") ; break ;
		case 1 : strcpy(buf,"Orbital position") ; break ;
		case 2 : strcpy(buf,"West/East") ; break ;
		case 3 : strcpy(buf,"Polarization") ; break ;
		case 4 : strcpy(buf,"Modulation") ; break ;
		case 5 : strcpy(buf,"Symbol rate") ; break ;
		case 6 : strcpy(buf,"Forward Error Correction (FEC) rate") ; break ;
	}
}

void SatelliteDeliveryDescHolder::getValueText	( int itemIndex, char *buf )
{
	SatelliteDeliveryDescriptor *desc = getDesc() ;
	switch(itemIndex)
	{
		case 0 : sprintf(buf,"%.5f", desc->getFrequency()  ) ; break ;
		case 1 : sprintf(buf,"%.1f", desc->getOrbitalPos() ) ; break ;
		case 2 : desc->westEastAsText(buf) ; break ;
		case 3 : desc->polarizationAsText(buf) ; break ;
		case 4 : desc->modulationAsText(buf) ; break ;
		case 5 : sprintf(buf,"%.4f", desc->getSymbolRate() ) ; break ;
		case 6 : desc->fecAsText(buf) ; break ;
	}
}

void SatelliteDeliveryDescHolder::getTextDescription( char *buf )
{
	SatelliteDeliveryDescriptor *desc = getDesc() ;
	char pol[30], fec[20] ;
	desc->polarizationAsText(pol) ;
	desc->fecAsText(fec) ;
	sprintf(buf,"Freq:%f, SR:%f, Pol:%s, FEC:%s", desc->getFrequency(), desc->getSymbolRate(), pol, fec ) ;
}

BOOL RunSatDelivEdit(SatelliteDeliveryDescriptor *satDesc, int itemIndex) ;

BOOL SatelliteDeliveryDescHolder::editItem( int itemIndex )
{
	BOOL changed = RunSatDelivEdit( getDesc(), itemIndex ) ;
/*	double	freq = desc->getFrequency	( ) ;
	double	orbitPos = desc->getOrbitalPos	( ) ;
	int		westEast = desc->getWestEast		( ) ;
	int		polar = desc->getPolarization	( ) ;
	int		modul = desc->getModulation	( )	;
	double	symbRate = desc->getSymbolRate	( )	;
	int		fec = desc->getFEC			( )	;
	BOOL changed = FALSE ;

	switch (itemIndex)
	{
	case 0 :
		 changed = EditFloatValue(
			"Frequency",
			"Select frequency of the transpoder which carry the transport stream.",
			&freq, 
			0.0,
			200.0
		) ; break ;
	case 1 :
		 changed = EditFloatValue(
			"Orbital position",
			"Select orbital position of the transpoder which carry the transport stream.",
			&freq, 
			0.0,
			200.0
		) ; break ;
	} ;

	if (changed)
	{
		_changeFlag = TRUE ;
		desc->create( freq, orbitPos, westEast, polar, symbRate, fec ) ;
	}
*/	return changed ;
}

///////////////////////////
//	PrivateDataDescHolder

void PrivateDataDescHolder::getItemName	( int itemIndex, char *buf )
{ 
	strcpy(buf, "Private data") ; 
}

void PrivateDataDescHolder::getValueText( int itemIndex, char *buf )	
{
	int len = getDesc()->getPrivateDataLength() ;
	HexStringToString(getDesc()->private_data, len, buf) ;
}

void PrivateDataDescHolder::getTextDescription ( char *buf )
{
	getValueText(0, buf) ;
}

BOOL PrivateDataDescHolder::editItem( int itemIndex )
{
	PrivateDataDescriptor *desc = getDesc() ;

	if (itemIndex==0)
	{
		int len = desc->getPrivateDataLength() ;
		uchar data[256] ;
		memcpy(data, desc->private_data, len) ;
		
		BOOL changed = EditHexStringValue("Private data","Enter hex-string", len, data) ;
		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setPrivateData(len, data);
		}
		return changed ;
	}
	return FALSE ;
}

////////////////////////////////
//	StreamIdentifierDescHolder

void StreamIdentifierDescHolder::getItemName	( int itemIndex, char *buf )
{ 
	strcpy(buf, "Component tag") ; 
}

void StreamIdentifierDescHolder::getValueText( int itemIndex, char *buf )	
{
	itoa( getDesc()->getComponentTag(), buf, 10 ) ;
}

void StreamIdentifierDescHolder::getTextDescription ( char *buf )
{
	getValueText(0, buf) ;
}

BOOL StreamIdentifierDescHolder::editItem( int itemIndex )
{
	StreamIdentifierDescriptor *desc = getDesc() ;

	if (itemIndex==0)
	{
		int tag = desc->getComponentTag() ;
		
		BOOL changed = EditIntValue(
			FALSE,
			"Component tag",
			"Enter program stream (service component) identifier. "
			"Within a program map section each stream identifier "
			"descriptor shall have a different value for this field.",
			&tag, 
			0,
			255
		) ;

		if (changed)
		{
			_changeFlag = TRUE ;
			desc->create(tag) ;
		}
		return changed ;
	}
	return FALSE ;
}

/////////////////////////
//	SubtitlingDescHolder

void SubtitlingDescHolder::getItemName	( int itemIndex, char *buf )
{
	switch(itemIndex)
	{
		case 0 : strcpy(buf,"Language") ; break ;
		case 1 : strcpy(buf,"Subtitles type") ; break ;
		case 2 : strcpy(buf,"Composition page id") ; break ;
		case 3 : strcpy(buf,"Ancillary page id") ; break ;
	}
}

void SubtitlingDescHolder::getValueText	( int itemIndex, char *buf )
{
	SubtitlingDescriptor *desc = getDesc() ;
	switch(itemIndex)
	{
		case 0 : LanguageDescriptor::langAsText((uchar*)desc->ISO_639_language_code, buf) ; break ;
		case 1 : SubtitlingDescriptor::subtitlesTypeAsText(desc->getType(),buf) ; break ;
		case 2 : itoa(desc->getComposPageId(),buf,10) ; break ;
		case 3 : itoa(desc->getAncillaryPageId(),buf,10) ; break ;
	}
}

void SubtitlingDescHolder::getTextDescription ( char *buf )
{
	SubtitlingDescriptor *desc = getDesc() ;
	SubtitlingDescriptor::subtitlesTypeAsText(desc->getType(),buf) ;
	buf += strlen(buf) ;
	char lang[5] ;
	desc->getLang(lang) ;
	sprintf(buf, "(Lang : %s)", lang) ;
}

BOOL SubtitlingDescHolder::editItem( int itemIndex )
{
	SubtitlingDescriptor *desc = getDesc() ;
	
	if(itemIndex==0)
	{
		char lang[255] ;
		getDesc()->getLang(lang) ;
		
		BOOL changed = chooseLanguage((uchar*)lang, AfxGetMainWnd() ) ;
			//EditStringValue("Subtitles language","Enter 3 character value of the language",lang) ;

		if (changed)
		{
			_changeFlag = TRUE ;
			getDesc()->setLang(lang) ;
		}

		return changed ;
	}

	if(itemIndex==1)
	{
		DECLARE_EDIT_ENUM(15) ;
		#define EDIT_ENUM_TEXT_ENUMERATOR SubtitlingDescriptor::subtitlesTypeAsText

		ADD_ENUM_VALUE(SubtitlingDescriptor::EBU_Teletext_subtitles							);
		ADD_ENUM_VALUE(SubtitlingDescriptor::Associated_EBU_Teletext						);
		ADD_ENUM_VALUE(SubtitlingDescriptor::DVB_subtitles_no_aspect_ratio					);
		ADD_ENUM_VALUE(SubtitlingDescriptor::DVB_subtitles_aspect_ratio_4_3					);
		ADD_ENUM_VALUE(SubtitlingDescriptor::DVB_subtitles_aspect_ratio_16_9				);
		ADD_ENUM_VALUE(SubtitlingDescriptor::DVB_subtitles_aspect_ratio_2_21_1				);
		ADD_ENUM_VALUE(SubtitlingDescriptor::DVB_subtitles_HardHearing_no_aspect_ratio		);
		ADD_ENUM_VALUE(SubtitlingDescriptor::DVB_subtitles_HardHearing_aspect_ratio_4_3		);
		ADD_ENUM_VALUE(SubtitlingDescriptor::DVB_subtitles_HardHearing_aspect_ratio_16_9	);
		ADD_ENUM_VALUE(SubtitlingDescriptor::DVB_subtitles_HardHearing_aspect_ratio_2_21_1	);

		int selected = desc->getType() ;

		BOOL changed = EditEnumValue(
			FALSE,
			"Subtitles type",
			"Select type of subtitles for the selected service",
			10,
			ENUM_VALS(),
			ENUM_TEXTS(),
			&selected
		) ;

		FREE_ENUM_VALUES() ;
		#undef EDIT_ENUM_TEXT_ENUMERATOR

		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setType(selected) ;
		}

		return changed ;
	}
	
	if(itemIndex==2)
	{
		int num = desc->getComposPageId() ;
	
		BOOL changed = EditIntValue(
			FALSE,
			"Composition Page Id",
			"This field identifies the composition page. DVB_subtitling_segments signalling this page_id shall be decoded if the previous data in the subtitling descriptor matches the user's selection criteria.",
			&num, 
			0, 
			0xFFFF
		) ;

		if (changed)
			desc->setComposPageId(num) ;

		return changed ;
	}

	if(itemIndex==3)
	{
		int num = desc->getAncillaryPageId() ;
	
		BOOL changed = EditIntValue(
			FALSE,
			"Ancillary Page Id",
			"This identifies the (optional) ancillary page. DVB_subtitling_segments signalling this page_id shall also be decoded if the previous data in the subtitling descriptor matches the user's selection criteria. The values in theancillary_page_id and the composition_page_id fields shall be the same if no ancillary page is provided.",
			&num, 
			0, 
			0xFFFF
		) ;

		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setAncillaryPageId(num) ;
		}

		return changed ;
	}

	return FALSE ;
}

////////////////////////
//	TeletextDescHolder

void TeletextDescHolder::getItemName	( int itemIndex, char *buf )
{
	switch(itemIndex)
	{
		case 0 : strcpy(buf,"Language") ; break ;
		case 1 : strcpy(buf,"Teletext type") ; break ;
		case 2 : strcpy(buf,"Magazine number") ; break ;
		case 3 : strcpy(buf,"Page number") ; break ;
	}
}

void TeletextDescHolder::getValueText	( int itemIndex, char *buf )
{
	TeletextDescriptor *desc = getDesc() ;
	switch(itemIndex)
	{
		case 0 : LanguageDescriptor::langAsText((uchar*)desc->ISO_639_language_code, buf) ; break ;
		case 1 : TeletextDescriptor::teletextTypeAsText(desc->getType(),buf) ; break ;
		case 2 : itoa(desc->getMagazineNum(),buf,10) ; break ;
		case 3 : itoa(desc->getPageNum(),buf,10) ; break ;
	}
}

void TeletextDescHolder::getTextDescription ( char *buf )
{
	TeletextDescriptor *desc = getDesc() ;
	TeletextDescriptor::teletextTypeAsText(desc->getType(),buf) ;
	buf += strlen(buf) ;
	char lang[5] ;
	desc->getLang(lang) ;
	sprintf(buf, "(Lang : %s, Magazine : %d, Page : %d)", lang, desc->getMagazineNum(), desc->getPageNum() ) ;
}

BOOL TeletextDescHolder::editItem( int itemIndex )
{
	TeletextDescriptor *desc = getDesc() ;
	
	if(itemIndex==0)
	{
		char lang[255] ;
		getDesc()->getLang(lang) ;
		
		BOOL changed = chooseLanguage((uchar*)lang, AfxGetMainWnd()) ;
			//EditStringValue("Teletext language","Enter 3 character value of the language",lang) ;

		if (changed)
		{
			_changeFlag = TRUE ;
			getDesc()->setLang(lang) ;
		}

		return changed ;
	}

	if(itemIndex==1)
	{
		DECLARE_EDIT_ENUM(10) ;
		#define EDIT_ENUM_TEXT_ENUMERATOR TeletextDescriptor::teletextTypeAsText

		ADD_ENUM_VALUE(TeletextDescriptor::InitialTeletextPage ) ;
		ADD_ENUM_VALUE(TeletextDescriptor::TeletextSubtitlePage ) ;
		ADD_ENUM_VALUE(TeletextDescriptor::AdditionalInformationPage ) ;
		ADD_ENUM_VALUE(TeletextDescriptor::ProgrammeSchedulePage ) ;
		ADD_ENUM_VALUE(TeletextDescriptor::TeletextSubtitlePageForHearingImpairedPeople ) ;

		int selected = desc->getType() ;

		BOOL changed = EditEnumValue(
			FALSE,
			"Teletext type",
			"Select type of the teletext service",
			5,
			ENUM_VALS(),
			ENUM_TEXTS(),
			&selected
		) ;

		FREE_ENUM_VALUES() ;
		#undef EDIT_ENUM_TEXT_ENUMERATOR

		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setType(selected) ;
		}

		return changed ;
	}
	
	if(itemIndex==2)
	{
		int num = desc->getMagazineNum() ;
	
		BOOL changed = EditIntValue(
			FALSE,
			"Magazine Number",
			"",
			&num, 
			0, 
			7
		) ;

		if (changed)
			desc->setMagazineNum(num) ;

		return changed ;
	}

	if(itemIndex==3)
	{
		int num = desc->getPageNum() ;
	
		BOOL changed = EditIntValue(
			FALSE,
			"Page Number",
			"",
			&num, 
			0, 
			255
		) ;

		if (changed)
		{
			_changeFlag = TRUE ;
			desc->setPageNum(num) ;
		}

		return changed ;
	}

	return FALSE ;
}

BOOL VideoStreamDescHolder::editItem( int )
{
	DECLARE_EDIT_ENUM(3) ;
	#define EDIT_ENUM_TEXT_ENUMERATOR(val, buf) strcpy(buf, val?"Yes":"No");

	ADD_ENUM_VALUE(1)
	ADD_ENUM_VALUE(0)

	int selected = getDesc()->onlyStillPictures() ;

	BOOL changed = EditEnumValue(
		FALSE,
		"Still pictures only",
		"",
		2,
		ENUM_VALS(),
		ENUM_TEXTS(),
		&selected
	) ;

	FREE_ENUM_VALUES() ;
	#undef EDIT_ENUM_TEXT_ENUMERATOR

	if (changed)
	{
		_changeFlag = TRUE ;
		getDesc()->create(selected) ;
	}

	return changed ;
}

/////////////////////////////////////////////////////////////////////////////////

BOOL chooseLanguage ( uchar *lang, CWnd *pParent )
{
	int nLanguages = (int)LANGUAGES_NUMBER ;

	DECLARE_EDIT_ENUM(4096) ;
	#define EDIT_ENUM_TEXT_ENUMERATOR(i,buf)	strcpy(buf,s_stLanguages[i]) ;

	int selected = -1 ;
	for ( int i = 0; i < nLanguages; i++ )
	{
		ADD_ENUM_VALUE(i) ;
		if (strncmp((const char*)lang , s_stLanguages[i], 3)==0)
			selected = i ;
	}

	BOOL changed = EditEnumValue(
		FALSE,
		"Language",
		"Select language",
		nLanguages,
		ENUM_VALS(),
		ENUM_TEXTS(),
		&selected,
		pParent
	) ;

	FREE_ENUM_VALUES() ;
	#undef EDIT_ENUM_TEXT_ENUMERATOR

	if ( changed )
		memcpy(lang, s_stLanguages[selected], 3) ;

	return changed ;
}
