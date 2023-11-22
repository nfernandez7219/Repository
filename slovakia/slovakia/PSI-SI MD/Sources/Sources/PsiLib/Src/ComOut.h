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


class  CWnd ;
class  BaseConfigClass ;
class  ConfigClass ;
class  sDllDialog ;
struct DvbDriverSrvProps ;

enum ComIOCapability
{
	ComIO_DriverStatusDlg	= 1,
	ComIO_CanDoHWFiltering	= 2,
	ComIO_DriverDump		= 3,
	ComIO_SetupDialog		= 4,
	ComIO_DynamicPID		= 5,		// PID change via
	ComIO_DynamicSetSpeed	= 6,		// yes: it is allowed to call ComOut::setSpeed() at any time
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

#endif
