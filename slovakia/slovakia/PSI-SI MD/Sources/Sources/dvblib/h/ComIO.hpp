#ifndef __INC_COMIO_HPP__
#define __INC_COMIO_HPP__


//
// TCP connect string
// ------------------
//
// connectStr = [host//]service
//   where
// (optional) host
//			  = "localhost"		... local computer (this is default host),
//				"ftp.microsoft.com",
//				"127.54.67.32",
//				host name as registrated in system...\etc\hosts
// service	  =	"123/tcp"		... port 123, using TCP
//				"456/udp"		... port 456, using udp
//				"xyz"			... port & protocol taken from etc\services
//
// Defaults:
//		open( "123")		// TCP connection on local computer, port=123
//		open( "abc")		// local computer, port and protocol defined in ...\services
//
// LPT/COM
// -------
// connectStr = "LPTx"	where x = 1,2,...
//


struct sUserCA ;
struct MuxPacket ;
class  sDllDialog ;
class  CWnd ;
class  GlobalUserID ;
class  ConfigClass ;
struct DvbDriverSrvProps ;
struct DvbDriverRcvProps ;


enum ComIOCapability
{
	ComIO_DriverStatusDlg	= 1,
	ComIO_CanDoHWFiltering	= 2,
	ComIO_DriverDump		= 3,
	ComIO_SetupDialog		= 4,
	ComIO_DynamicPID		= 5,		// PID change via
	ComIO_DynamicSetSpeed	= 6,		// yes: it is allowed to call ComOut::setSpeed() at any time
} ;


class BaseInpDriver
{
  public :
	// Open Receiver
	virtual int		open ( const char *connectStr)		{ return 0; }

	virtual int		workKernel()						{ return 0; }

	virtual int		close	  ()						{ return 0; }

	virtual BOOL hasCapability( ComIOCapability cap )	{ return FALSE ; }

	virtual void	clearStatistics( )					{ }

	// Get 1 line text containing info about object state. (Used for logging)
	virtual BOOL	getStatisticsLog( char *buf )		{ buf[0]=0 ; return FALSE ; }

	// Created as new ... must be deleted
	virtual sDllDialog *createStatisticsPage()			{ return NULL ; }

	// After extracting command packet, ComInp first sends it to InpDriver::processCommandPacket().
	// On returning TRUE, packet is further ignored.
	virtual BOOL processCommandPacket( MuxPacket *p )	{ return FALSE; }

	virtual BOOL getUserId( GlobalUserID *id ) = 0 ;

	// Following function serves for controlling set of receiving PID's.
	// It requires capability ComIO_DynamicPID.
	enum ChangeRcvPIDCmd
	{
		// Add new PID to the set of receiving PID's
		AddPID	= 1,	// (AddPID, pid, 0)

		// Del PID from the set of receiving PID's
		DelPID	= 2,	// (DelPID, pid, 0)

		// Set the set of receiving PID's
		SetPIDs	= 3,	// (SetPIDs, short *pids, int num_pids)
	} ;
	virtual BOOL changeRcvPID( int cmd, long par1, long par2)	{ return FALSE; }

	// Reset (reframe) the card.
	// Called when too many consecutive bad packets are detected.
	//	hard= TRUE if 3 consecutive calls for resynchronization did not help.
	//		  In such case more effective resynchronization method may be
	//		  applied (if available).
	virtual	void	resynchronize( BOOL hard )					{ return; }
} ;


// ComOut is base class for writing data to the output device.
// It contains only virtual prototypes which have to be implemented for particular device.
//
class ComOut
{
  protected:
	enum Status {
		Closed =0,
		Opened =1,
		Started=2,
		AskStop=3,
	} ;
	Status			_status ;

  public :
	ComOut ()												{ _status=Closed ; }
	virtual ~ComOut ()										{ if( isOpened() )  close() ; }

	virtual BOOL hasCapability( ComIOCapability cap )		{ return FALSE ; }

	virtual int		open ( const char *setupStr)			{ _status = Opened ; return 0 ; }
			BOOL	isOpened()								{ return _status == Opened ; }
	virtual int		close()									{ _status = Closed ; return 0 ; }
	//virtual int		start()								{ return 0 ; }
	//virtual int		stop ()								{ return 0 ; }

	// error code is returned
	// p = a series of TSPacket's
	virtual int		write( const char *p, int n_bytes, int *written )	{ *written = n_bytes; return 0 ; }

	// error code is returned
	virtual	int		setSpeed( float maxSpeed, BaseConfigClass *cfg ) 	{ return 0 ; }	// Mb/s
};


class ServiceReceiver;
class FileReceiver;
class InstallsReceiver;
class InternetReceiver;
class CommandReceiver;


//
// Interface to the driver Dll
//
class BaseComInp
{
	friend class BigComInp ;
  protected:
	BaseInpDriver	*_inpDriver ;

  public:
	virtual void	_acceptMuxData	( char *buf, int n_bytes )	{ }
	virtual void	_acceptTsData	( char *buf, int n_bytes )	{ }
	virtual BOOL	stopRequested	()							{ return FALSE ; }

	BaseComInp()	{ _inpDriver = NULL; }

	inline BOOL hasCapability( ComIOCapability cap )
	{
		if( _inpDriver == NULL )
			return FALSE ;
		return _inpDriver->hasCapability( cap ) ;
	}

	inline void	clearStatistics( )
	{
		if( _inpDriver != NULL )
			_inpDriver->clearStatistics() ;
	}

	inline BOOL	getStatisticsLog( char *buf )
	{
		if( _inpDriver == NULL )
			return FALSE ;
		return _inpDriver->getStatisticsLog( buf ) ;
	}

	inline sDllDialog *createStatisticsPage()
	{
		if( _inpDriver == NULL )
			return NULL ;
		return _inpDriver->createStatisticsPage() ;
	}

	BOOL getUserId( GlobalUserID *id ) ;

	void resynchronize( BOOL hard )
	{
		if( _inpDriver != NULL )
			_inpDriver->resynchronize( hard ) ;
	}
} ;


// ComInp is base class for reading MuxPacket's from the input device.
// It contains only virtual prototypes which have to be implemented for particular device.
//
// ComInp consists of:
//	- Stand alone input thread which receives and stores TSPackets (up to <_maxPacketsStored>)
//	  into internal buffer. This thread performs also the packet filtering. (Packets not
//	  belonging to the current user are discarded.)
//	  Input processing can be switched on/off via start()/stop() functions.
//	- read() function delivering MuxPacket's for further processing.
// 

class ComInp : public BaseComInp
{
	friend DWORD WINAPI inputThreadFnc( void *param ) ;
	friend DWORD WINAPI updateFileRecieversThreadFnc( void *param );

  protected:
	enum Status {
		Closed =0,
		Opened =1,
		Started=2,
		AskStop=3,
		Stoped =4,
	} ;


	CRITICAL_SECTION	_acceptDataLock;
	CRITICAL_SECTION	_freceiverLock;
	CRITICAL_SECTION	_userCALock;

	HANDLE			_hInputThread ;
	HANDLE			_hUpdateFileRecieversThread;

	Status			_status ;
	BOOL			_acceptMuxPacket( MuxPacket *mp ) ;		// FALSE iff mp is not MuxPacket
	void			_acceptMuxData	( char *buf, int n_bytes ) ;
	void			_acceptTsData	( char *buf, int n_bytes ) ;

	inline void		_processCorruptedPacket( ) ;

	void		    (*_openHook)() ;

	// packet receivers ---------------------------------------------
	// processor for internet packets
	InternetReceiver	*_internetReceiver;

	// processor for services (eg. messages, CA-table...)
	ServiceReceiver		*_serviceReceiver;

	// processor for files transferred via service channel
	FileReceiver		*_serviceChannelFileReceiver;

	// processor for packets composing program upgrade
	InstallsReceiver	*_upgradeReceiver;							// recevier for sw upgrade

	// 1 file receiver per existing channel
	FileReceiver		*_fileReceivers[USHRT_MAX];					// one file receiver for a channel

	// processor for incoming commands
	CommandReceiver		*_commandReceiver;
	// end of receivers ---------------------------------------------

	sTemplateArray<ushort>	*_updChannels;
	sTemplateArray<ushort>	_updatedChannels[2];
	int						_updChannelsIndex;
	sUserCA			*_userCA;
	sUserCA			*_oldUserCA;
	BOOL			_newUserCA;
	BOOL			_isStarted;
	BOOL			_doFiltering;

	// workKernel() returns only on error or after stopRequested() was signalled.
	// (Make it a thread.)
	// During the processing each packet found is passed to processPacket().
	// Each specialized class derived from ComInp must implement workKernel() so that
	// accepted data are sent (raw = unprocessed) to _acceptMuxData() or _acceptTsData().
	//virtual int		workKernel()					{ return 0 ; }
			BOOL	stopRequested()					{ return _status == AskStop ; }

	int		updateFileRecievers	();

  public :
	ComInp() ;
	virtual ~ComInp()
	{
		if( isOpened() )
			close();
		DeleteCriticalSection( &_userCALock );
		DeleteCriticalSection( &_freceiverLock );
		DeleteCriticalSection( &_acceptDataLock );
	};

	// Use:
	//		1. open()
	//		2. start()
	//		3. stop()	// optional
	//		4. close()
	virtual int		open		( const char *setupStr);
	virtual int		close		();
	void			setOpenHook ( void (*hook)() )	{ _openHook = hook ; }

			BOOL	isOpened	()					{ return _status != Closed ; }

			BOOL	start		() ;
	inline	BOOL	isStarted	()					{ return _isStarted ; }
			void	stop		() ;

	// -> TRUE iff packet belongs to this user
	BOOL	filterPacket		( MuxPacket *mp ) ;

	// send packet for further processing
	void	processPacket		( MuxPacket *p );

	void	updateUserCA		( sUserCA *userCA );
	void	synchUserCAWithCard	( void *bitmap );

	// Is some of the data receivers receiving data in this moment?
	BOOL	isReceiving			();

	// send request for statistics to hardware
	//virtual void	statistics	()					{};

	inline	void	setFiltering( BOOL doFiltering )			{ _doFiltering = doFiltering; }

	InternetReceiver *internetReceiver()						{ return _internetReceiver; }
} ;


//-----------------------------------------------------------------------------
//	generalized Com classes
//  Wrapper classes for different Com classes to unify the treatment.
//-----------------------------------------------------------------------------


// connectStr:
//	NULL				... dummy (NULL) connection
//	12345/tcp           ... running tcp connection on the local machine
//	RolandH//12345/udp  ... running udp connection to 'RolandH' machine
//	lpt1                ... sending data through parallel port
//  DVB					... DVB driver
//	ADP					... Adaptec Satellite Receiver Card
//
// No exceptions thrown.
//

class BigComIO
{
  protected:
	HMODULE		 _dll ;			// IOdriver dll
	void		*_ioCtl ;
	char		*_connectString ;

	BigComIO() ;
   ~BigComIO() ;

	BOOL create( const char *connectString, char *expl, BOOL runningAsServer ) ;
	BOOL callDriverIoctl( char *expl, int cmd, long par1=0, long par2=0, long par3=0 ) ;

	static const char *getDllName( const char *connectStr, BOOL *isTestConnection=NULL ) ;

  public:
	const char *errorCodeAsText( int code, char *buf ) ;

	virtual BOOL hasCapability( ComIOCapability cap )		{ return FALSE ; }

	// Get 1 line text containing info about object state. (Used for logging)
	virtual BOOL getStatisticsLog( char *txt )				{ return FALSE ; }

	// Must be created as new
	virtual sDllDialog *createStatisticsPage()				{ return NULL ; }

	virtual void		clearStatistics		()				{ }

	virtual BOOL showDriverStatusDialog( CWnd *w ) ;
	virtual void driverDump( ) ;

	// Run modal setup dialog
	// = TRUE iff settings changed
	BOOL runSetupDialog( CWnd *parent, ConfigClass *cfg ) ;

	// TRUE -
	//	expl[0] =0 ... standard connect string (dvb, dvbasi, adp...)
	//	expl[0]!=0 ... tcp/udp/com/lpt connect string
	// FALSE- invalid connect string
	static BOOL isConnectStringOk( const char *connectStr, char *expl ) ;

	inline const char *connectString()						{ return _connectString ; }
} ;

class BigComOut : public BigComIO
{
	ComOut *_com ;
	BOOL	_openFailed ;

  public:
	inline  BigComOut()						{ _com = NULL ; _openFailed=FALSE; }
	inline ~BigComOut()						{ close() ; }
	inline BOOL create( const char *connectString, char *expl )	{ return BigComIO::create(connectString,expl,TRUE) ; }

	int			open( BaseConfigClass *cfg);
	inline BOOL	isOpened()					{ return _com != NULL  &&  !_openFailed ; }
	void		close() ;

	inline ComOut *com ()					{ return _com ; }

	virtual BOOL hasCapability( ComIOCapability cap ) ;

	BOOL getDrvProperties( DvbDriverSrvProps *props ) ;
} ;

class BigComInp : public BigComIO
{
	ComInp *_com ;
	BOOL	_openFailed ;

  public:
	inline  BigComInp()						{ _com = NULL ; _openFailed=FALSE; }
	inline ~BigComInp()						{ close() ; }
	inline BOOL create( const char *connectString, char *expl )	{ return BigComIO::create(connectString,expl,FALSE) ; }

	int			open( BaseConfigClass *cfg, void (*hook)()=0 );
	inline BOOL	isOpened()					{ return _com != NULL; }
	void		close() ;

	inline ComInp *com ()					{ return _com ; }

	virtual BOOL hasCapability( ComIOCapability cap ) ;

	virtual BOOL		getStatisticsLog	( char *txt ) ;
	virtual sDllDialog *createStatisticsPage() ;
	virtual void		clearStatistics		() ;

	inline	BOOL	start		()			{ return (_com && !_openFailed) ? _com->start() : FALSE; }
	inline	BOOL	isStarted	()			{ return _com ? _com->isStarted() : FALSE; }
	inline	void	stop		()			{ if( _com ) _com->stop() ; }

	BOOL getDrvProperties( DvbDriverRcvProps *props ) ;
} ;

extern BigComIO *bigComIO ;

#endif
