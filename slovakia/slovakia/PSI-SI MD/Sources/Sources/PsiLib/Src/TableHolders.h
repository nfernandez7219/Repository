
#ifndef __INC_TABLEHOLDERS_H__
#define __INC_TABLEHOLDERS_H__


#ifndef __INC_TABLES_H__
	#include "Tables.h"
#endif

#ifndef __S_LIST_H
	#include "slist.h"
#endif

class PsiData ;
class TableHolder ;

class BaseHolder ;

class BaseHolder
{
	friend class TableTree ;

protected:
	enum HolderType
	{
		PAT_HOLDER = 0x100 ,
		PMT_HOLDER = 0x101 ,
		SDT_HOLDER = 0x102 ,
		NIT_HOLDER = 0x103 ,
		CAT_HOLDER = 0x104 ,

		DESC_ARRAY = 0x105 ,

		PMT_STREAM_HOLDER	= 0x106,
		NIT_TS_HOLDER		= 0x107,
		SDT_SERVICE_HOLDER	= 0x108,

		PMT_STREAMS_ROOT	= 0x109,

		SUBST_HOLDER  = 0x8000 ,
		UNDEFINED  = 0xFFFF
	} ;

	int			_type ;
	BOOL		_changeFlag ;
	HTREEITEM	_treeItem ;

public:
	BaseHolder ( int type, BOOL subst=FALSE )					{ setHolderType(subst?(type|SUBST_HOLDER):type) ; }

	inline int	getHolderType ( )								{ return _type ; }
	inline void setHolderType ( int type )						{ _type = type ; }
	inline BOOL isSubstHolder ( )								{ return (_type&0xF000)==SUBST_HOLDER ; }
	inline BOOL isDescHolder  ( )								{ return (_type&0x0FFF)<0x100 ; }
	inline BOOL isStreamRootHolder  ( )							{ return (_type&0x0FFF)==PMT_STREAMS_ROOT ; }
	inline BOOL isTableHolder ( )								{ return _type==PAT_HOLDER||_type==PMT_HOLDER||_type==SDT_HOLDER||_type==NIT_HOLDER||_type==CAT_HOLDER||_type==PMT_STREAM_HOLDER||_type==NIT_TS_HOLDER||_type==SDT_SERVICE_HOLDER ; }

	// abstract members
	virtual int	 getItemNumber	( )								{ return 0 ; }
   	virtual void getValueText	( int itemIndex, char *buf )	{ sprintf(buf, ""); }
   	virtual void getItemName	( int itemIndex, char *buf )	{ sprintf(buf, ""); }

	virtual BOOL editItem		( int itemIndex )				{ return FALSE ; }

	virtual void getTextDescription ( char *buf )				{ sprintf(buf, ""); }

	virtual BaseHolder*	createChild	( int type )				{ return NULL ; }
	virtual BOOL		delChild	( BaseHolder *child)		{ return FALSE ; }
} ;

class BaseHolderArray : public sTemplateArray<BaseHolder*>
{
  public:
	BaseHolderArray( ) ;
} ;

class DescriptorHolder : public BaseHolder
{
	uchar	_body[258] ;

  public:
	DescriptorHolder ( uchar descTag ) ;

	inline BaseDescriptor* getBaseDesc	( )		{ return (BaseDescriptor*)_body ; }

	inline uchar getLength	( )		{ return getBaseDesc()->getLength() ; }
	inline uchar getType	( )		{ return getBaseDesc()->getType() ; }

	uchar	*loadFromBin ( uchar *buffer ) ;
	uchar	*saveToBin	 ( uchar *buffer ) ;
} ;

class DescriptorArray : public sTemplateArray<DescriptorHolder*>
{
  public:
	DescriptorArray() ;

	void	 loadFromBin( uchar *buffer, int size ) ;
	uchar	*saveToBin	( uchar *buffer ) ;

	DescriptorHolder*	findHolder	( uchar type ) ;

	DescriptorHolder*	appendDesc	( int type ) ;
} ;

//--------------------------//
//		PAT Holder
//--------------------------//

class PAT_Holder : public BaseHolder
{
	friend class TableHolder ;

	int							_transportStreamId ;
	ushort						_nitPid ;
	sTemplateArray<PatProgram>	_patPrograms ;

  public:
	PAT_Holder ( );

	void	load	( PsiData *psiData ) ;
	void	save	( PsiData *psiData ) ;

	ushort	findPID			( ushort programNumber ) ;
	void	addProgram		( ushort programNumber, ushort pmtPid )		{ _patPrograms.add( PatProgram(programNumber, pmtPid) ) ; }
	void	deleteProgram	( ushort programNumber ) ;
	BOOL	findPidUse		( ushort pid, char *usedBuf ) ;

	virtual int	 getItemNumber	( )	;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;
} ;

//--------------------------//
//		PMT Holder
//--------------------------//

class PmtStreamHolder : public BaseHolder
{
	friend class PMT_Holder ;
	friend class TableTree ;

	uchar		_StreamType ;
	ushort		_ElementaryPID ;

	DescriptorArray	_descriptors ;

  public:
	PmtStreamHolder			( uchar streamType=0, ushort elemPID=0 ) ;
	uchar	*loadFromBin	( uchar *buffer ) ;
	uchar	*saveToBin		( uchar *buffer ) ;


	virtual int	 getItemNumber	( )	;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;
	
	virtual BaseHolder*	createChild (int type)				{ return _descriptors.appendDesc(type) ; }

	virtual BOOL		delChild	( BaseHolder *child) ;

	inline	void addDescriptor	( DescriptorHolder *desc )	{ _descriptors.add(desc) ; }
	inline	int  getStreamType	( )							{ return _StreamType ; }
	inline	int  getElemPid		( )							{ return _ElementaryPID ; }
	inline	void setStreamType	( int type )				{ _StreamType = type ; }
	inline	void setElemPid		( int pid )					{ _ElementaryPID = pid ; }

	inline  DescriptorHolder *getDescHolder ( int i )		{ return (i>=0&&i<_descriptors.count())?(_descriptors[i]):NULL ; }
} ;

class PmtStreamHolderArray : public sTemplateArray<PmtStreamHolder*>
{
  public:
	PmtStreamHolderArray( ) ;
} ;

class PMT_Holder : public BaseHolder
{
	friend class TableHolder ;
	friend class TableTree ;

	ushort		_ProgramNumber ;
	ushort		_PCR_PID ;

	DescriptorArray			_descriptors ;
	PmtStreamHolderArray	_streams ;

  public:
	PMT_Holder( ushort progNum = 0 ) ;
	
	void	load	( PsiData *psiData ) ;
	void	save	( PsiData *psiData ) ;

	BOOL	findPidUse		( ushort pid, char *usedBuf ) ;

	PmtStreamHolder*	addStream ( ) ;
	inline void			addStream ( PmtStreamHolder *stream )	{ _streams.add(stream) ; }

	inline	int	 getProgramNum	( )							{ return _ProgramNumber; }
	inline	void setPcrPid		( int pid )					{ _PCR_PID = pid ; }
	inline	int  getPcrPid		( )							{ return _PCR_PID ; }

	virtual int	 getItemNumber	( )	;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

	virtual BaseHolder*	createChild (int type)				{ return _descriptors.appendDesc(type) ; }

	virtual BOOL		delChild	( BaseHolder *child) ;
} ;

class PMT_HolderArray : public sTemplateArray<PMT_Holder*>
{
  public:
	PMT_HolderArray( ) ;

	void	deleteProgram ( ushort programNumber ) ;
} ;

//--------------------------//
//		SDT Holder
//--------------------------//

class SdtServiceHolder : public BaseHolder
{
	friend class TableHolder ;
	friend class TableTree ;
	friend class SDT_Holder ;

	ushort		_serviceId ;
	BOOL		_EitPresentFlag ;
	BOOL		_EitScheduleFlag ;
	uchar		_RunningStatus ;
	BOOL		_FreeCAMode ;

	DescriptorArray					_descriptors ;

  public:
	// constructor for demo tables
	SdtServiceHolder ( ushort servId=0 ) ;

	uchar	*loadFromBin	( uchar *buffer ) ;
	uchar	*saveToBin		( uchar *buffer ) ;

	inline	int	 getServiceId	( )							{ return _serviceId ; }
	inline	void addDescriptor	( DescriptorHolder *desc )	{ _descriptors.add(desc) ; }

	virtual int	 getItemNumber	( )	;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

	virtual BaseHolder*	createChild (int type)			{ return _descriptors.appendDesc(type) ; }
	virtual BOOL		delChild	( BaseHolder *child) ;
} ;

class SdtServiceHolderArray : public sTemplateArray<SdtServiceHolder*>
{
  public:
	SdtServiceHolderArray ( ) ;
} ;

class SDT_Holder : public BaseHolder
{
	friend class TableHolder ;
	friend class TableTree ;

	ushort		transport_stream_id ;
	ushort		original_network_id ;

	SdtServiceHolderArray	_services ;

  public:
	SDT_Holder		( ushort tsId=0, ushort origNtwId=0 ) ;
	
	void	load	( PsiData *psiData ) ;
	void	save	( PsiData *psiData ) ;

	void	addService	  ( SdtServiceHolder *service )		{ _services.add(service) ; }
	void	deleteService ( ushort id ) ;

	virtual int	 getItemNumber	( )	;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

	virtual BOOL delChild		( BaseHolder *child) ;
} ;

//--------------------------//
//		NIT Holder
//--------------------------//

class NitTransportStreamHolder : public BaseHolder
{
	friend class TableTree ;
	friend class NIT_Holder ;

	ushort		_transportStreamId ;
	ushort		_originalNetworkId ;

	DescriptorArray					_descriptors ;

  public:
	NitTransportStreamHolder ( ushort tsId=0, ushort origNtwId=0 ) ;
	
	uchar	*loadFromBin	( uchar *buffer ) ;
	uchar	*saveToBin		( uchar *buffer ) ;

	virtual int	 getItemNumber	( )	;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

	virtual BaseHolder*	createChild (int type)			{ return _descriptors.appendDesc(type) ; }
	virtual BOOL		delChild	( BaseHolder *child) ;

	inline DescriptorArray* descriptors ( )				{ return &_descriptors ; }
} ;

class NitTSHolderArray : public sTemplateArray<NitTransportStreamHolder*>
{
  public:
	NitTSHolderArray( ) ;
} ;

class NIT_Holder : public BaseHolder
{
	friend class TableHolder ;
	friend class TableTree ;

	ushort				_networkId ;

	DescriptorArray		_descriptors ;
	NitTSHolderArray	_transpStreams ;

  public:
	NIT_Holder ( ) ;

	void	load	( PsiData *psiData ) ;
	void	save	( PsiData *psiData ) ;

	virtual int	 getItemNumber	( )	;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

	virtual BaseHolder*	createChild (int type)			{ return _descriptors.appendDesc(type) ; }
	virtual BOOL		delChild	( BaseHolder *child) ;
} ;

//--------------------------//
//		CAT Holder
//--------------------------//
class CASystemDescHolder ;

class CAT_Holder : public BaseHolder
{
	friend class TableHolder ;
	friend class TableTree ;

	DescriptorArray		_descriptors ;

  public:
	CAT_Holder ( ) ;

	void	load	( PsiData *psiData ) ;
	void	save	( PsiData *psiData ) ;
	
	CASystemDescHolder*		getItem	( int index ) ;
	DescriptorHolder*		addItem	( ) ;

	BOOL	findPidUse( ushort pid, char *usedBuf ) ;

	virtual int	 getItemNumber	( )	;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

	virtual BaseHolder*	createChild (int type)			{ return _descriptors.appendDesc(type) ; }
} ;

//--------------------------//
//		PSI Data Array
//--------------------------//

//-----------------------------
//	Saved file structure
//	--------------------
//
//	signature				- 6 bytes containg string "PSI/SI"
//	number_of_records		- 4 bytes
//	Records:
//		PID					- 2 bytes
//		number_of_sections	- 4 bytes
//		frequency			- 4 bytes
//		Sections:


class PsiData
{
public:
	ushort				_pid ;
	int					_nSections ;
	UINT				_freq ;
	PrivateSection*		_privateSections ;
	void*				_pParam ;

	PsiData()	{ _privateSections=NULL ; } 
	~PsiData()	{ clear() ; } 

	inline void clear()									{ if (_privateSections) free(_privateSections) ; } 
	
	BOOL		verifySections	( int *sectionSize ) ;
	uchar		getDataType	( )							{ return _privateSections->table_id ; }
} ;

class PsiDataArray : public sTemplateArray<PsiData>
{
public:
	PsiDataArray() ;

	BOOL loadFromFile	( const char *fileName ) ;
	BOOL saveToFile		( const char *fileName ) ;

	void getTableSizes	( UINT *patSize, UINT *pmtSize, UINT *catSize, UINT *sdtSize, UINT *nitSize ) ;
	UINT getTotalSpeed	( ) ;
} ;

//--------------------------//
//		Table Holder
//--------------------------//

class TableHolder
{
	friend class TableTree ;

	PAT_Holder					_patHolder ;
	PMT_HolderArray				_pmtHolders ;
	SDT_Holder					_sdtHolder ;
	NIT_Holder					_nitHolder ;
	CAT_Holder					_catHolder ;

	BaseHolderArray				_substHolders ;

public:
	TableHolder	(PsiDataArray *arr)			{ LoadFromPsiDataArray(arr) ; }

	PsiDataArray*	GeneratePsiDataArray	( ) ;
	void			LoadFromPsiDataArray	( PsiDataArray *arr ) ;

	BaseHolder*		ReplaceSubstHolder		( BaseHolder *substHolder, BaseHolder *parent ) ;
	BaseHolder*		CreateSubstHolder		( int type ) ;

	void			AddProgram				( ushort prgNum, ushort pmtPid, PMT_Holder *pmtHolder, SdtServiceHolder *service ) ;
	BOOL			DeleteProgram			( int prgNum ) ;
	void			UpdateSpeed				( PsiDataArray *arr ) ;
	void			UpdateSpeed				( TableHolder  *holder ) ;
	void			GetRunningTablesNames	( char *buf ) ;
	void			FindPidUse				( ushort pid, char *usedBuf ) ;

	BOOL			programExists			( int prgNum )			{ return _patHolder.findPID(prgNum)!=0 ; }

	inline	DescriptorArray*	NIT_descriptors		( )		{ return &_nitHolder._descriptors ; }
	inline	NitTSHolderArray*	NIT_transpStreams	( )		{ return &_nitHolder._transpStreams ; }
	inline	void				setNITnetId ( ushort id )	{ _nitHolder._networkId = id ; }
	inline	void				setSDT_TS   ( ushort tsId, ushort origNtwId )	{ _sdtHolder.transport_stream_id = tsId ; _sdtHolder.original_network_id = origNtwId ; }

public:
	UINT	_patFreq ;
	UINT	_pmtFreq ;
	UINT	_catFreq ;
	UINT	_sdtFreq ;
	UINT	_nitFreq ;

	static BOOL CheckPidUse( UINT pid, char *usedBuf ) ;
} ;

//--------------------------//
//		Descriptor Holders
//--------------------------//

class ServiceDescHolder : public DescriptorHolder
{
	virtual int	 getItemNumber	( )								{ return 3 ; }
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;
	virtual void getTextDescription ( char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

public:
	ServiceDescHolder		( ) 
		: DescriptorHolder(TAG_service_descriptor)
	{ 
		getDesc()->create(ServiceDescriptor::Data_broadcast_service,"Unknown provider","Untitled service") ; 
	}

	inline ServiceDescriptor *getDesc()							{ return (ServiceDescriptor*)getBaseDesc() ; }
} ;

class AudioStreamDescHolder : public DescriptorHolder
{
public:
	AudioStreamDescHolder		( ) 
		: DescriptorHolder(TAG_audio_stream_descriptor)
	{ 
		getDesc()->create() ; 
	}

	inline AudioStreamDescriptor *getDesc()							{ return (AudioStreamDescriptor*)getBaseDesc() ; }
} ;

class VideoStreamDescHolder : public DescriptorHolder
{
	virtual int	 getItemNumber	( )								{ return 1 ; }
   	virtual void getItemName	( int itemIndex, char *buf )	{ strcpy(buf, "Still pictures only") ; }
   	virtual void getValueText	( int itemIndex, char *buf )	{ strcpy(buf, getDesc()->onlyStillPictures()?"Yes":"No") ; }

	virtual BOOL editItem		( int itemIndex ) ;
public:
	VideoStreamDescHolder		( ) 
		: DescriptorHolder(TAG_video_stream_descriptor)
	{ 
		getDesc()->create(FALSE) ; 
	}

	inline VideoStreamDescriptor *getDesc()							{ return (VideoStreamDescriptor*)getBaseDesc() ; }
} ;

class CAIdentifierDescHolder : public DescriptorHolder
{
	inline CAIdentifierDescriptor *getDesc()					{ return (CAIdentifierDescriptor*)getBaseDesc() ; }

	virtual int	 getItemNumber	( )								{ return 1 ; }
   	virtual void getValueText	( int itemIndex, char *buf )	{ CASystemDescriptor::caSystemAsText(getDesc()->getCaSystemId(), buf) ; }
   	virtual void getItemName	( int itemIndex, char *buf )	{ strcpy(buf, "CA System"); }
	virtual void getTextDescription ( char *buf )				{ getValueText(0,buf) ; }

	virtual BOOL editItem		( int itemIndex ) ;

public:
	CAIdentifierDescHolder	( )
		: DescriptorHolder(TAG_CA_identifier_descriptor)
	{ 
		getDesc()->create(0) ; 
	}
} ;

class CASystemDescHolder : public DescriptorHolder
{
	BOOL	_isInCAT ;

	inline CASystemDescriptor *getDesc()						{ return (CASystemDescriptor*)getBaseDesc() ; }

	virtual int	 getItemNumber	( )								{ return 3 ; }
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

public:
	CASystemDescHolder		( BOOL bIsInCAT = FALSE )
		: DescriptorHolder(TAG_CA_descriptor)
	{ 
		_isInCAT = bIsInCAT ;
		getDesc()->create(0,0) ; 
	}

	virtual void getTextDescription ( char *buf ) ;

	inline int getCaSystemPid	( )				{ return getDesc()->getCaSystemPid() ; }
	inline int getCaSystemId	( )				{ return getDesc()->getCaSystemId() ; }

	inline void setCatFlag		( )				{ _isInCAT=TRUE ;}
} ;

class CountryAvailabilityDescHolder  : public DescriptorHolder
{
	CountryAvailabilityDescHolder *_pair ;

	virtual int	 getItemNumber	( ) ;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

protected:
	inline	int	descItemNumber	( )									{ return getDesc()->getCountriesNumber() ; }

public:
	CountryAvailabilityDescHolder		( BOOL bAvailable )
		: DescriptorHolder(TAG_country_availability_descriptor)
	{ 
		getDesc()->create(bAvailable) ; 
	}

	inline void setPairHolder(CountryAvailabilityDescHolder *pair)	{ _pair = pair ; }
	inline CountryAvailabilityDescriptor *getDesc()					{ return (CountryAvailabilityDescriptor*)getBaseDesc() ; }
} ;

class DataBroadcastDescHolder : public DescriptorHolder
{
	virtual int	 getItemNumber	( )								{ return 1 ; }
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;
	virtual void getTextDescription ( char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

public:
	DataBroadcastDescHolder		( )								
		: DescriptorHolder(TAG_data_broadcast_descriptor)
	{ 
		getDesc()->create(DataBroadcastDescriptor::Data_pipe,"") ; 
	}

	inline DataBroadcastDescriptor *getDesc()					{ return (DataBroadcastDescriptor*)getBaseDesc() ; }
} ;

class DataBroadcastIdDescHolder : public DescriptorHolder
{
	virtual int	 getItemNumber	( )								{ return 1 ; }
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;
	virtual void getTextDescription ( char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

public:
	DataBroadcastIdDescHolder		( )							
		: DescriptorHolder(TAG_data_broadcast_id_descriptor)
	{ 
		getDesc()->create(DataBroadcastDescriptor::Data_pipe) ; 
	}

	inline DataBroadcastIdDescriptor *getDesc()					{ return (DataBroadcastIdDescriptor*)getBaseDesc() ; }
} ;

class SatelliteDeliveryDescHolder : public DescriptorHolder
{
	virtual int	 getItemNumber	( )								{ return 7 ; }
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;
	virtual void getTextDescription ( char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

public:
	SatelliteDeliveryDescHolder		( )								
		: DescriptorHolder(TAG_satellite_delivery_system_descriptor)
	{ 
		getDesc()->create(
					11.73217,										// freq
					19.2,											// orbital pos
					0,												// westEast
					SatelliteDeliveryDescriptor::Linear_vertical,	// polarization
					22.0,											// symbol rate
					SatelliteDeliveryDescriptor::Conv_code_rate_3_4	// FEC rate
		) ; 
	}

	inline SatelliteDeliveryDescriptor *getDesc()				{ return (SatelliteDeliveryDescriptor*)getBaseDesc() ; }
} ;

class LinkageDescHolder : public DescriptorHolder
{
	inline LinkageDescriptor *getDesc()							{ return (LinkageDescriptor*)getBaseDesc() ; }

	virtual int	 getItemNumber	( )								{ return 5; }
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual void getTextDescription ( char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

public:
	LinkageDescHolder		( )								
		: DescriptorHolder(TAG_linkage_descriptor)
	{ 
		getDesc()->create(0,0,0,0) ; 
	}
} ;

class LanguageDescHolder : public DescriptorHolder
{
	virtual int	 getItemNumber	( )								{ return getDesc()->getLangNumber()+1; }
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;
public:
	
	LanguageDescHolder		( )								
		: DescriptorHolder(TAG_ISO_639_language_descriptor)
	{ 
		getDesc()->create() ; 
	}

	inline LanguageDescriptor *getDesc()						{ return (LanguageDescriptor*)getBaseDesc() ; }
} ;

class MultiLingServiceNameDescHolder : public DescriptorHolder
{
	int		_nItems ;

	virtual int	 getItemNumber	( ) ;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;
public:
	
	MultiLingServiceNameDescHolder( )								
		: DescriptorHolder(TAG_multilingual_service_name_descriptor)
	{ 
		getDesc()->create() ; 
		_nItems = 0 ;
	}
	
	inline MultiLingServiceNameDescriptor *getDesc()			{ return (MultiLingServiceNameDescriptor*)getBaseDesc() ; }
} ;

class MultiLingNetworkNameDescHolder : public DescriptorHolder
{
	int		_nItems ;

	virtual int	 getItemNumber	( ) ;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;
public:
	
	MultiLingNetworkNameDescHolder( )								
		: DescriptorHolder(TAG_multilingual_network_name_descriptor)
	{ 
		getDesc()->create() ; 
		_nItems = 0 ;
	}
	
	inline MultiLingNetworkNameDescriptor *getDesc()			{ return (MultiLingNetworkNameDescriptor*)getBaseDesc() ; }
} ;

class NetworkNameDescHolder : public DescriptorHolder
{
	virtual int	 getItemNumber	( )								{ return 1; }
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual void getTextDescription ( char *buf )				{ getDesc()->getName(buf) ; }

	virtual BOOL editItem		( int itemIndex ) ;

public:
	NetworkNameDescHolder		( )								
		: DescriptorHolder(TAG_network_name_descriptor)
	{ 
		getDesc()->create("Noname") ; 
	}

	inline NetworkNameDescriptor *getDesc()						{ return (NetworkNameDescriptor*)getBaseDesc() ; }
} ;

class PrivateDataDescHolder : public DescriptorHolder
{
	inline PrivateDataDescriptor *getDesc()						{ return (PrivateDataDescriptor*)getBaseDesc() ; }

	virtual int	 getItemNumber	( )								{ return 1 ; }
   	virtual void getItemName	( int itemIndex, char *buf ) ;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
	virtual void getTextDescription ( char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

public:
	PrivateDataDescHolder ( )
		: DescriptorHolder(TAG_private_data_specifier_descriptor)
	{
		getDesc()->create() ; 
	}
} ;

class StreamIdentifierDescHolder : public DescriptorHolder
{
	inline StreamIdentifierDescriptor *getDesc()				{ return (StreamIdentifierDescriptor*)getBaseDesc() ; }

	virtual int	 getItemNumber	( )								{ return 1 ; }
   	virtual void getItemName	( int itemIndex, char *buf ) ;
   	virtual void getValueText	( int itemIndex, char *buf ) ;
	virtual void getTextDescription ( char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

public:
	StreamIdentifierDescHolder ( )
		: DescriptorHolder(TAG_stream_identifier_descriptor)
	{
		getDesc()->create(0) ; 
	}
} ;

class SubtitlingDescHolder : public DescriptorHolder
{
	inline SubtitlingDescriptor *getDesc()						{ return (SubtitlingDescriptor*)getBaseDesc() ; }

	virtual int	 getItemNumber	( )								{ return 4; }
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual void getTextDescription ( char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

public:
	SubtitlingDescHolder		( )								
		: DescriptorHolder(TAG_subtitling_descriptor)
	{ 
		getDesc()->create(NULL, SubtitlingDescriptor::EBU_Teletext_subtitles, 0, 0) ; 
	}
} ;

class TeletextDescHolder : public DescriptorHolder
{
	inline TeletextDescriptor *getDesc()						{ return (TeletextDescriptor*)getBaseDesc() ; }

	virtual int	 getItemNumber	( )								{ return 4; }
   	virtual void getValueText	( int itemIndex, char *buf ) ;
   	virtual void getItemName	( int itemIndex, char *buf ) ;

	virtual void getTextDescription ( char *buf ) ;

	virtual BOOL editItem		( int itemIndex ) ;

public:
	TeletextDescHolder		( )								
		: DescriptorHolder(TAG_teletext_descriptor)
	{ 
		getDesc()->create(NULL, TeletextDescriptor::InitialTeletextPage, 0, 0) ; 
	}
} ;

/////////////////////////////////
//	data presentation functions

void HexStringToString( const uchar *data, int nBytes, char *buf ) ;
BOOL StringToHexString( const char *buf, uchar *data, int &nBytes ) ;

BOOL EditIntValue	( BOOL bHexVal, const char *item, const char *description, int*	 value, int	   min=-0x7FFFFFFF, int	   max=0x7FFFFFFF, CWnd *parent=NULL ) ;
BOOL EditFloatValue	( const char *item, const char *description, double* value, double min=-1.7E307 ,   double max=1.7E307,	   CWnd *parent=NULL ) ;
BOOL EditStringValue( const char *item, const char *description, char *value,	CWnd *parent=NULL ) ;
BOOL EditHexStringValue( const char *item, const char *description, int &nBytes, uchar *data, CWnd *parent=NULL ) ;
BOOL EditEnumValue	( BOOL bEditValue, const char *item, const char *description, int nValues,	int *values, char **meanings, int *selected, CWnd *parent=NULL ) ;

BOOL editNetworkId ( int *id, BOOL bOrigId ) ;

//////////////////////
// enumerator macros

#define DECLARE_EDIT_ENUM(num)	int EDEN_i=0, EDEN_vals[num] ; LPTSTR EDEN_texts[num] ;

#define ADD_ENUM_VALUE(val)						\
	EDEN_vals[EDEN_i] = val ;					\
	{											\
		char EDEN_st[128] ;						\
		EDIT_ENUM_TEXT_ENUMERATOR(val, EDEN_st);\
		EDEN_texts[EDEN_i] = strdup(EDEN_st) ;	\
	}											\
	EDEN_i++ ;

#define	FREE_ENUM_VALUES()	while (EDEN_i--) free(EDEN_texts[EDEN_i]) ;

#define ENUM_VALS()		(EDEN_vals)

#define ENUM_TEXTS()	(EDEN_texts)

#endif