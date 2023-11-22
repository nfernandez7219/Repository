#ifndef			__SERVCGG_HPP__
#define			__SERVCGG_HPP__

#ifndef __SERIALNUM_HPP__
#include "SerialNum.hpp"
#endif
#ifndef __BASECFG_HPP__
#include "BaseCfg.hpp"
#endif
#ifndef	__INC_MFXGLOBALS_HPP__
#include "MfxGlobals.hpp"
#endif

#ifndef	__INC_DRVINTERFACE_HPP__
#include "DrvInterface.hpp"
#endif


class Mux			;
class BigComOut		;
class MuxSetup		;
class MuxOutput		;
class MuxChannel	;
class MuxChannelSetup;
class ServiceChannel;
class ServiceManager;
class CfgUsersSetup ;
class CfgProvidersSetup;
class DvbServerSetup;
class ServiceChannel;
class SchedulerManager;
class CfgOutputScheduler ;
class CfgOutputSchedulerPtrArray ;
class PidStreamAttrib ;
class PidStreamAttribManager ;


//-----------------------------------------------------------------------------------
//	Output rate scheduler
//-----------------------------------------------------------------------------------


// from time ... until time ... use output rate ...
struct CfgOutputRate
{
	uchar 	hourFrom	;		
	uchar	minuteFrom	;
	uchar	hourTo		;
	uchar	minuteTo	;
	float	rate		;		// Mb/s

	int		timeFrom()	{ return hourFrom*60 + minuteFrom; }
	int		timeTo()	{ return hourTo  *60 + minuteTo  ; }
};

typedef sTemplateArray<CfgOutputRate>	CfgOutputRatePtrArray;

// Stores plan for changing the output rate.
class CfgOutputScheduler
{
	friend class ChannelScheduler;
	friend class SchedulerManager;
	friend class DVBPage;
  public:
	enum DayName { Sun, Mon, Tue, Wen, Thu, Fri, Sat, Mon_Sun, Mon_Fri, Weekend };

  private:
	int						_dayName	;
	int 					_viaDayName	;
	BOOL					_bSwitchMode;
	time_t					_date		;
	CfgOutputRatePtrArray	_outputRates;

	inline	BOOL				 viaDayName () const 		{ return _viaDayName;	}
	inline	const CfgOutputRatePtrArray	&outputRates() const{ return _outputRates;  }
	inline	DayName				 dayName	() const		{ return (DayName)_dayName;} 
	inline	time_t				 date		() const		{ return _date;			}
			CfgOutputRate *		 getScheduler( const tm *tm ) ;
			float				 getRate	( const tm *tm, time_t *how_long ) ;
	
  public:
	CfgOutputScheduler &operator= ( const CfgOutputScheduler &usr);
	inline	BOOL		operator!=( const CfgOutputScheduler &usr) const	{ return !( *this==usr );};
	BOOL				operator==( const CfgOutputScheduler &usr) const ;

	// data exchange between the object and config file
	void				storage				( CfgBaseSetup *cfg, LPCTSTR lpSection, LPCTSTR lpValue, BOOL bSave ); //Exc

	// test this this scheduler if overlaps with <sch>
	BOOL				hasEqualDate		( const CfgOutputScheduler &sch ) const;

	// If not then some day period is undefined (transfer will stop).
	BOOL				fullDayDefined		( ) ;

	// conversions of the object contents to/from text
	BOOL				fromStringToRates	( const char *buf );
	BOOL				ratesAsString		( char *buf, int size ) const;

	// date conversions
	BOOL				isDateValid		    ( const char *buf );
	void				dateAsString		( char *buf ) const ;

	// Switch on mode
	BOOL				inSwitchMode() const		{ return _bSwitchMode;	}

	BOOL				setSchedular( LPCTSTR lpDate, LPCTSTR lpRate );

	// strings presented in scheduler combo box
	static const char **stdDates			( int *n_names ) ;
	
	CfgOutputScheduler()							{ _viaDayName=TRUE; time( &_date ); _dayName=Mon_Fri; _bSwitchMode=FALSE; }
	CfgOutputScheduler( CfgOutputScheduler &usr )	{ *this = usr; };
};

class CfgOutputSchedulerPtrArray : public sTemplateArray<CfgOutputScheduler*>
{
	friend class SchedulerPage;

	BOOL	bIgnoreSchedular;

  public:	
	BOOL	ignoreSchedular()				{ return bIgnoreSchedular; };
	void	setIgnoreFlag  ( BOOL bIgnore)	{ bIgnoreSchedular=bIgnore;};
	  
    CfgOutputSchedulerPtrArray &operator = ( const CfgOutputSchedulerPtrArray &list);
	BOOL addOrModify ( CfgOutputScheduler &adr, int &index );
	BOOL delScheduler( CfgOutputScheduler &adr );
	void storage	 ( CfgBaseSetup *cfg, LPCTSTR lpSection, LPCTSTR lpValue, BOOL bSave ); //Exc

	CfgOutputSchedulerPtrArray();
};

//------------------------------------------------------------------------------------
//	ChannelScheduler
//  Background thread which takes care about multiplexor output rate.
//------------------------------------------------------------------------------------

class ChannelScheduler : public CWinThread
{
	HANDLE	_eventKill  ;
	HANDLE	_eventReset ;
	HANDLE	_handlers[2];

  protected:
	int		getNextTimeout		( CfgOutputSchedulerPtrArray *scheduler, tm &ttm, int &iSwitchFlag, BOOL bStartUp );
	int		refreshChannelsState( BOOL bOnStartUp );

  public:
	BOOL	InitInstance	();
	int		ExitInstance	();

	void	start			()	{ CreateThread(); }
	void	reset			()	{ VERIFY( SetEvent(_eventReset) ); };
	void	kill			();
	ChannelScheduler();
};


//-----------------------------------------------------------------------------------
//	channels
//-----------------------------------------------------------------------------------


//  Class describing data of a single channel.
class CfgChannel
{
	friend class ChannelsPage;
	friend class sChannelPage;
	friend class sChannelOptionsPage;
	friend class sChannelDialog;
	friend class sProviderAddressDialog ;
	friend class ChannelScheduler;
	friend class CfgChannelsSetup;

  public:
	enum { TEXT_SIZE=32,} ;
	enum { TSPIPE_PROTOCOL=0, MPE_HNET_PROTOCOL=1, MPE_UDP_PROTOCOL=2 } ;
	enum { NoFEC=0, LowFEC=1, MediumFEC=2, HighFEC=3 } ;

  private:
	char		_serviceName[TEXT_SIZE];
	ushort		_serviceID		;
	char		_streamFormat	;
	BOOL		_useFec			;
	short		_fecLevel		;
	BOOL		_useFecRebr		;
	short		_maxFecRebrSize ;		// [KB]
	short		_absPriority	;
	short		_relPriority	;
	float		_outputRateMin	;
	float		_outputRateMax	;
	BOOL		_channelSwitchOn;
	int			_numRebroadcasts;
	int			_maxVolumePerDay;
	int			_inboxSendDelay ;
	int			_rebroadcastDelay ;		// [s] 0=no delay, else delay between rebroadcasts
	int			_absPriorityOn	;
	int			_relPriorityOn	;
	int			_maxVolumePerDayOn;
	int			_channelPID		;
	char	   *_inboxPath		;
	CfgOutputSchedulerPtrArray	_schedulers	;

	void		_setDefault();

  public:
	// channel data
	inline const char  *serviceName		() const	{ return _serviceName	 ; }
	inline const char  *inboxPath		() const	{ return _inboxPath		 ; }
	inline BOOL			useFEC			() const	{ return _useFec		 ; }
	inline int			FEClevel		() const	{ return _useFec ? _fecLevel : 0 ; }
	inline BOOL			useFECrebroadcast()const	{ return _useFec && _useFecRebr	 ; }
	inline short		maxFECrebrSize	() const	{ return _maxFecRebrSize ; }
	inline short		absPriority		() const	{ return (short)(_absPriorityOn ? _absPriority:0); }
	inline short		relPriority		() const	{ return (short)(_relPriorityOn ? _relPriority:0); }
	inline ushort		serviceID		() const	{ return _serviceID		 ; }
	inline BOOL			channelSwitchOn	() const    { return _channelSwitchOn; }
	inline float		outputRateMin	() const	{ return _outputRateMin	 ; }
	inline float		outputRateMax	() const	{ return _outputRateMax	 ; }
	inline int			numRebroadcasts	() const	{ return _numRebroadcasts; }
	inline int			rebroadcastDelay() const	{ return _rebroadcastDelay;}
	inline int			maxVolumePerDay	() const	{ return _maxVolumePerDayOn ? _maxVolumePerDay : -1; }
	inline int			inboxSendDelay	() const	{ return _inboxSendDelay ; }
	inline int			channelPID		() const	{ return _channelPID;	   }
	inline char			streamFormat	() const	{ return _streamFormat;	   }

		   void			setInboxPath	( const char *dir ) ;
		   void			setDefaults		() ;
	inline BOOL			isEmpty			() const	{ return !_serviceName[0];	};

	// data exchange between the object and config file
		   void			storage			( CfgBaseSetup *cfg, BOOL saveFlag, const char *sectName=NULL );	//Exc
	// Config section used to store the data.
		   const char  *getConfigSection( char *sect, int siz) const;

	inline BOOL			isHNetChannel	() const	{ return _serviceID == 0xFFFF ; }
	inline BOOL			isServiceChannel() const	{ return _serviceID == 0 ; }
	inline BOOL			isSpecialChannel() const	{ return _serviceID == 0  ||  _serviceID == 0xFFFF; }

	// operators and constructors
			CfgChannel( ushort chID, const char *nm);
	inline	CfgChannel( CfgChannel &usr )			{ _inboxPath=NULL; *this = usr;	};
	inline  CfgChannel()							{ _inboxPath=NULL; _serviceName[0]=0; _serviceID=1 ; setDefaults();};
	       ~CfgChannel();
	
	inline CfgChannel  &operator=		( const CfgChannel &usr)		{ copy( usr, TRUE) ; return *this ; }
	inline BOOL		    operator!=		( const CfgChannel &usr) const	{ return !( *this==usr );};
	inline BOOL		    operator==		( const CfgChannel &usr) const	{ return equals( usr, TRUE ) ; }

	BOOL equals( const CfgChannel &adr, BOOL withPID ) const ;
	void copy  ( const CfgChannel &adr, BOOL withPID ) ;
};

typedef sTemplateArray<CfgChannel*> CfgChannelsPtrArray;		 

//  class representing data for all channels
class CfgChannelsSetup : public CfgBaseSetup
{
	friend class ChannelsPage;
	friend class DvbServerSetup;
	friend class sProviderAddressDialog ;
	friend void  destroyChannel( CfgChannel *ch ) ;

	CfgChannelsPtrArray	channels;
	CRITICAL_SECTION   _channelServerSetupLock;
	
	BOOL			addOrModifyChannel	( CfgChannel &usr, BOOL modify=FALSE );
	BOOL			delChannel			( CfgChannel &usr, CfgUsersSetup *userSetup, CfgProvidersSetup *providerSetup );

	// data exchange between the object and config file
	void			storage				( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR lpErrBuffer=NULL );	//Exc

  public:
	// channels
	inline int				 numChannels     ()	const					{ return channels.count(); };
	inline const CfgChannel *channel         ( int index ) const		{ return (index > -1 && index < numChannels()) ? channels.item(index) : NULL; }
	inline const char		*getChannelName  ( int index ) const		{ return (index > -1 && index < numChannels()) ? (channels.item(index))->serviceName() : NULL; }
		   const CfgChannel *getChannelByName(const char *name);
		   const CfgChannel *getChannelById  ( ushort id ) ;
		   void				 shortChannelList( sStringPtrArray &list ) ;	
		   void				 channelIdsList  ( sTemplateArray<int> &list ) ;	

	// Allocate memory for *table and fill it with channle names (used in sending CA table)
	ulong createChannelNamesTable( char **table );

	// Returns stream format used by all channels or -1 if the channels use different formats
	char globalStreamFormat() ;

	// Set stream format for all channels
	// Returns TRUE if there was some change.
	BOOL setGlobalStreamFormat( char f, char *warning ) ;

	CfgChannelsSetup( const char *file = "Config\\Channels.cfg" );
   ~CfgChannelsSetup( )								{ DeleteCriticalSection( &_channelServerSetupLock); }
};


//-----------------------------------------------------------------------------------
//	Address object
//-----------------------------------------------------------------------------------


class CfgAddress
{
	friend class sProviderAddressDialog;
	friend class sUserAddressDialog;
	friend class ProviderContactDialog;
	friend class ProvidersPage ;
  public:
	enum { TEXT_SIZE=32, CODE_SIZE=24} ;

  protected:
	char	_name			[TEXT_SIZE] ;
	char	_surname		[TEXT_SIZE] ;
	char	_street			[TEXT_SIZE] ; 
	char	_zipCode		[CODE_SIZE] ;
	char	_city			[TEXT_SIZE] ;
	char	_country		[TEXT_SIZE] ; 
	char	_telephone		[CODE_SIZE] ;
	char	_fax			[CODE_SIZE] ;
	char	_email			[TEXT_SIZE*2] ;
	char	_contactPerson	[TEXT_SIZE*2];
	DvbGlobalId			   _id ;
	sTemplateArray<ushort> _channels ;
	DWORD				   _TCPAddress;
	UINT				   _TCPport;

	void _copy( const CfgAddress &src ) ;
	void _clear() ;		// except name & surname

  public:
	inline DvbGlobalId tmpId	() const			{ return _id ; }

	// data stored in address
	inline const char *name	    () const			{ return _name;		};
	inline const char *surname	() const			{ return _surname;	};
	inline const char *street	() const			{ return _street;	};
	inline const char *city	    () const			{ return _city;		};
	inline const char *zipCode  () const			{ return _zipCode;	};
	inline const char *country  () const			{ return _country;	};

	inline const char *e_mail	() const			{ return _email;	};
	inline const char *telephone() const			{ return _telephone;};
	inline const char *fax		() const			{ return _fax;		};

	inline DWORD	getTCPAddress()const			{ return _TCPAddress;}
	inline UINT		getTCPPort	 ()const			{ return _TCPport;	}
		   LPCTSTR	ipAddressToString( LPTSTR lpBuff );

	inline const char *contactPerson() const		{ return _contactPerson;}

	// channels
	inline int			numChannels	() const				{ return _channels.count(); };
	inline const ushort *arrayOfChannels() const			{ return (ushort*)_channels;   };
	inline ushort		channel		( int index ) const		{ return (ushort)( (index<_channels.count()) ? _channels.item(index) : 0) ; };  //0 on error
	inline BOOL			hasChannel	( ushort channel) const	{ return _channels.find(channel) >= 0 ; }
		   void			setChannels ( short *channels, int count ) ;
	inline BOOL			delChannel	( ushort id )			{ return _channels.delObj(id) ; }

	CfgAddress &operator= ( const CfgAddress &adr );
	BOOL		operator==( const CfgAddress &adr ) const ;
	inline BOOL	operator!=( const CfgAddress &adr ) const	{ return !( *this==adr );	};
	
	// This is not ==, but test whether address denotes same users.
		   BOOL		isIdentical ( const CfgAddress &adr ) const;

	inline BOOL		isEmpty	    () const			{ return _surname[0] ? FALSE : TRUE;		};

	// data exchange between the object and config file
		   void		storage	    ( CfgBaseSetup *cfg, BOOL saveFlag );		//Exc
	
	// Config section used to store the data.
	const char *getConfigSection( char *sect, int bufSize ) const;

	// str = text (allowed channels) stored in config file (or edited in the dialog)
	// setChannelsFromText(): defines allowed channels for this user
	// channelsAsText(): converts allowed channels to text
	void		setChannelsFromText( const char *str ) ;
	const char *channelsAsText( char *str ) ;

	CfgAddress( const char *srname, const char *name );
	CfgAddress( const CfgAddress &src )	;
	CfgAddress()							{ _clear(); _name[0]=0; _surname[0]=0; _id.create() ; }
   ~CfgAddress()							{ }
};

typedef sTemplateArray<CfgAddress*> CfgAddressPtrArray;


//-----------------------------------------------------------------------------------
//	providers
//-----------------------------------------------------------------------------------


//	Structure representing Providers definition dialog.
class CfgProvidersSetup : public CfgBaseSetup 
{
	friend class ProvidersPage;
	friend class sProviderAddressDialog ;

  protected:	
	CfgAddressPtrArray	_providers;

	// data exchange between the object and config file
	void				storage				( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR lpErrBuffer=NULL );	// Exc
	void				addOrModifyAddress	( CfgAddress &adr, BOOL modify=FALSE );	// Exc
	BOOL				delAddress			( CfgAddress &adr );

  public:
	// providers
	inline int				numProviders() const					{ return _providers.count(); };
	inline const CfgAddress *provider( int index ) const			{ return (index > -1 && index < numProviders()) ? _providers.item(index) : NULL; }

	// list of strings in the form "surname name; city"
	void				shortAddressList( sStringPtrArray &list ) const ;	
			
	CfgProvidersSetup( const char *file = "Config\\Providers.cfg" );
};


//-----------------------------------------------------------------------------------
//	users
//-----------------------------------------------------------------------------------


//	Structure representing user or provider definition dialog.
class CfgUser
{
	friend class sUserAddressDialog;
	friend class UsersPage;
	friend class CfgUsersSetup ;

  public:
	enum { TEXT_SIZE=32 } ;

  private:
	GlobalUserID	_userID ;
	CfgAddress		_address;
	BOOL			_canHNet;
	BOOL			_doHWFiltering ;

	ulong	createUserCATable( char **table );

	// data exchange between the object and config file
		   void		storage( CfgBaseSetup *cfg, BOOL saveFlag ); //Exc
	// Config section used to store the data.
	inline const char *getConfigSection( char *sect, int siz) const { return _address.getConfigSection( sect, siz);};

  public:
	// user data
	// tmpId is temporary id valid within 1 run which enables safe address comparison (better than isIdentical())
	inline DvbGlobalId		 tmpId		 () const			{ return _address.tmpId() ; }
	inline const CfgAddress &address	 () const			{ return _address;	};
	inline const GlobalUserID	userID	 () const			{ return _userID;	};
	inline BOOL				 canHybridNet() const			{ return _canHNet;	};
	inline BOOL				 doHWFiltering() const			{ return _doHWFiltering ; }

	// channels
	inline int		numChannels		() const				{ return _address.numChannels(); };
	inline ushort	channel			( int index ) const		{ return _address.channel( index ); };  //-1 on error
	inline BOOL		hasChannel		( ushort channel ) const{ return _address.hasChannel( channel ); };
	inline BOOL		delChannel		( ushort id )			{ return _address.delChannel(id) ; }

	// utilities
	inline BOOL		isRegistered() const					{ return _userID.isValid() ; }
	inline BOOL		isEmpty()	   const					{ return _address.isEmpty(); };
	inline BOOL		isIdentical( const CfgUser &usr ) const { return _address.isIdentical( usr.address() ); };

	// constructors and ooperators
	inline CfgUser( const char *srname, const char *name)	{ _canHNet = 0; _doHWFiltering = 0; _address = CfgAddress( srname, name); }
	inline CfgUser( CfgUser &usr ) : _address(usr._address)	{ _canHNet = usr.canHybridNet(); _doHWFiltering = usr.doHWFiltering(); _userID = usr.userID();}
	inline CfgUser() : _address()							{ _canHNet = 0; _doHWFiltering = 0; }

	inline CfgUser &operator= ( const CfgUser &usr)			{ _userID.makeInvalid(); _address = usr.address(); _canHNet = usr.canHybridNet(); _doHWFiltering = usr.doHWFiltering(); _userID =  usr.userID(); return *this; }
	inline BOOL		operator!=( const CfgUser &usr) const	{ return !( *this==usr );};
		   BOOL		operator==( const CfgUser &usr) const ;
};

typedef sTemplateArray<CfgUser*> CfgUserPtrArray;		 

// class describing set of all users
class CfgUsersSetup : public CfgBaseSetup
{
	friend class UsersPage;

	CfgUserPtrArray	  users ;
	int				  as_fromUser ;		// counter for alive signal
	CRITICAL_SECTION _userServerSetupLock;
	
	BOOL			addOrModifyUser	( CfgUser &usr, BOOL modify=FALSE );
	BOOL			delUser			( CfgUser &usr );

	// data exchange between the object and config file
	void			storage			( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR lpErrBuffer=NULL );	//Exc

	ulong	createUserCATable( const GlobalUserID& usrID, char **table )
	{
		EnterCriticalSection( &_userServerSetupLock);
		CfgUser *user = (CfgUser *)getUserById( usrID );
		ulong ret;
		if( user == NULL )
		{
			*table = NULL;
			ret = 0;
		}
		else
			ret = user->createUserCATable( table );
		LeaveCriticalSection( &_userServerSetupLock);
		return ret;
	}

  public:
	// users
	inline int			   numUsers() const						{ return users.count(); };
	inline const CfgUser  *user( int index ) const				{ return (index > -1 && index < numUsers()) ? users.item(index) : NULL; }
		   const CfgUser  *getUserById( const GlobalUserID& id, const CfgUser *exceptThisUser=NULL ) ;

	// sending data
		   // Alive signal ports UserLog info, therefore placed here.
		   int             sendAliveSignal( ) ;
		   // if userId == 0 send UserCA for all user, else send UserCA for user with userId
		   BOOL			   sendUserTable( const GlobalUserID& userId );
		   BOOL			   sendUserTableForAllUser();


	// "surname name; city; state", ...(next user)
	void			shortAddressList( sStringPtrArray &list ) ;

	CfgUsersSetup( const char *file = "Config\\Users.cfg" );
   ~CfgUsersSetup( )											{ DeleteCriticalSection( &_userServerSetupLock); }
};


//-----------------------------------------------------------------------------------
//	CfgDVBSetup
//	Represents all data stored in the config file.
//-----------------------------------------------------------------------------------


class CfgDVBSetup : public CfgBaseSetup
{
	friend class DVBPage;
	friend class SchedulerManager;
	friend class SavedDVBOptionsPage ;
	friend class DvbServerSetup;
	friend class DVBOptionsPage;

	BOOL						_autoStart		  ;
	int							_numLogEntries	  ;
	BOOL						_aliveSgn		  ;
	int							_aliveInterval	  ;
	CfgOutputSchedulerPtrArray	_schedulers		  ;
	char					   *_logFile		  ;
	char					   *_connectString	  ;
	int							_absPriorityPart  ;
	int							_singlePID		  ;
	BOOL						_useSinglePID	  ;
	BOOL						_useMultiSection  ;
	CfgChannel				   *_channelDefaults  ;
	DvbDriverSrvProps		    _driverSrvProps;
	sTemplateArray<UINT>	    _multiplePIDs;
	int							_monitorringPort;

	void					setDefaults			();

  public:
	inline	BOOL			autoStart			() const		{ return _autoStart  ;		}
	inline	const char*		connectString		() const		{ return _connectString	 ;	};
	inline	int				absPriorityPart		() const		{ return _absPriorityPart;	}
	inline	int				aliveInterval		() const		{ return _aliveSgn ? _aliveInterval : 0 ;}
	inline	const char*		logFile				() const		{ return _logFile;			}	
	inline	int				numEntriesForNewLog () const		{ return _numLogEntries;	}
	inline  int				getSinglePID		() const		{ return _singlePID;		}
	inline  BOOL			useSinglePID		() const		{ return _useSinglePID;		}
	inline  BOOL			useMultiSection		() const		{ return _useMultiSection;	}
	inline	const CfgChannel *channelDefaults	() const		{ return _channelDefaults ; }
	inline	const char*		sendCATableString	() const		{ return config->get("","sendCA_Table"); }
	inline	CfgOutputSchedulerPtrArray *schedulers()			{ return &_schedulers; }
	inline  float			getOutRate			() const		{ return _driverSrvProps.outputRate; }
		
	// data exchange between the object and config file
			void			storage				( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR lpErrBuffer=NULL );	//Exc
			BOOL			initDriverProp		();

	CfgDVBSetup( const char *fileName = "Config\\Dvb.cfg" );
   ~CfgDVBSetup() ;
};


//-----------------------------------------------------------------------------------
//	DvbServerSetup
//	Setup of the whole application. Contains all important data objects.
//-----------------------------------------------------------------------------------


typedef sTemplateArray<MuxChannel*>	MuxChannelPtrArray;

class DvbServerSetup
{
	HANDLE _thread;

	friend DWORD WINAPI threadStartChannels		 ( void *param ) ;
	friend DWORD WINAPI threadStopChannels		 ( void *param ) ;

	friend class CfgChannelsSetup ;
	friend class CfgProvidersSetup;
	
	CRITICAL_SECTION	 _dvbServerSetupLock;
	MuxChannelPtrArray   _muxChannels	;		// file channels + IP channel
	ServiceChannel		*_serviceChannel ;		// sevice channel
	PidStreamAttrib		*_serviceChannelPid ;	// PID of the service channel (not updated after setup changes)
	ServiceManager		*_serviceManager;		// service manager
	CfgContext			*_context		;		// storage for context strings (eg. amount of data transferred via channel in 1 day)
	CfgDVBSetup			*_dvbSetup		;		// app configuration
	CfgChannelsSetup	*_channelsSetup	;		// access to channel configuration
	CfgProvidersSetup	*_providersSetup;		// access to provider configuration
	CfgUsersSetup		*_usersSetup	;		// access to user configuration
	BigComOut			*_bigComOut		;		// physical output
	MuxSetup			*_muxSetup		;		// multiplexor setup
	MuxOutput			*_muxOutput		;		// multiplexor output (uses _bigComOut)
	SchedulerManager	*_schedManager  ;		// scheduler
	ChannelScheduler	*_channelSchedular;		// channel schedular
	PidStreamAttribManager*_pidStreamManager;	// PID stream manager
	Mux					*_mux			;		// multiplexor
	BOOL				 _channelsStarted ;		// TRUE is channels started
	char				 _usrName[10]   ;		// curent user
	BOOL	openServiceChannel ( ServiceChannel *ch ) ;
	BOOL	closeServiceChannel();
	void	getMuxChannelSetup ( const CfgChannel *ch, MuxChannelSetup *s ) ;

	// sent CA Table
	struct sTimeInterval { long from; long to; };
	sTemplateArray<sTimeInterval>	_sendCATable;
	int								_actualTimeInterval;

	int		cmpTimeIntervals( sTimeInterval i1, sTimeInterval i2 );
	BOOL	addTimeInterval	( sTimeInterval interval );

	void	startAllChannelsThr  ();
	void	stopAllChannelsThr	 ();

  public:
	DvbServerSetup();
   ~DvbServerSetup();
	void	load  ();							// Exc
	void	save  ();							// Exc
	void	open  ();							// Exc
	void	close ();
	void	resetScheduler() ;

	// data
	inline  CfgContext			*context			()	{ return _context;		 }
	inline	CfgDVBSetup			*dvbSetup			()	{ return _dvbSetup;		 }
	inline	CfgChannelsSetup	*channelsSetup		()	{ return _channelsSetup; }	
	inline	CfgProvidersSetup	*providersSetup		()	{ return _providersSetup;}
	inline	CfgUsersSetup		*usersSetup			()	{ return _usersSetup;	 }
	inline	ServiceManager		*serviceManager		()	{ return _serviceManager;}
	inline	PidStreamAttribManager*pidStreamManager ()	{ return _pidStreamManager;}
	inline  Mux					*mux				() 	{ return _mux ;			 }
	inline  MuxOutput			*muxOutput			()  { return _muxOutput ;	 }
	inline  BOOL				 channelsStarted	()  { return _channelsStarted;}
	inline  const char          *userName			() const { return _usrName;  }

	// MuxChannels
	MuxChannel *getMuxChannel		 ( ushort chanID  );
	MuxChannel *createMuxChannel	 ( CfgChannel *ch, char *expl=NULL );
	void		deleteMuxChannel	 ( CfgChannel *ch );
	BOOL		resetMuxChannel		 ( CfgChannel *ch );	// warning only: FALSE if sending in process
	inline ServiceChannel *serviceChannel	 ( ) const		{ return _serviceChannel ; }
	inline PidStreamAttrib*serviceChannelPid ( ) const		{ return _serviceChannelPid ; }

	void		startMuxChannel		 ( CfgChannel *ch );
	void		stopMuxChannel		 ( CfgChannel *ch );
	BOOL		startAllChannels	 ();
	BOOL		startServiceChannel  ();
	void		stopAllChannels		 ();
	void		stopServiceChannel	 ();

	// send CA Table
	long	getSendCATableWaitTime();	// interval between consecutive sends
	BOOL	initSendCATable	();			// preparation of variables; call once before 1st send
} ;

//-----------------------------------------------------------------------------------
//	globals
//-----------------------------------------------------------------------------------

extern DvbServerSetup *serverSetup;

// Components
inline CfgDVBSetup			*MfxDvbSetup		()	{ return serverSetup ? serverSetup->dvbSetup() :	  NULL; }
inline CfgChannelsSetup		*MfxChannelsSetup	()	{ return serverSetup ? serverSetup->channelsSetup() : NULL; }	
inline CfgProvidersSetup	*MfxProvidersSetup	()	{ return serverSetup ? serverSetup->providersSetup(): NULL; }
inline CfgUsersSetup		*MfxUsersSetup		()	{ return serverSetup ? serverSetup->usersSetup() :	  NULL;	}
inline ServiceChannel		*MfxServiceChannel  ()  { return serverSetup ? serverSetup->serviceChannel(): NULL; }

// Main object
inline DvbServerSetup		*MfxServerSetup		()	{ return serverSetup; };


#endif
