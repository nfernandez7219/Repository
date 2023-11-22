#ifndef __INC_ADP_HPP__
#define __INC_ADP_HPP__


#ifdef _IOCOMERGONDLL
    #define IoComergonDll	__declspec( dllexport )
#else
    #define IoComergonDll	__declspec( dllimport )
#endif

class sDllDialog ;
class ResDllDesc ;

#define IOComergon_VERSION		101
#define MIN_CARD_SPEED			125
#define MAX_CARD_SPEED			16000


#pragma warning( disable: 4275 )

#include "ComIo.hpp"
#include "DvbError.hpp"
#include "MuxPacket.hpp"
#include "card.h"


class MyCWinApp : public CWinApp
{
	virtual BOOL InitInstance( ) ;
	virtual int  ExitInstance( ) ;
} ;

BOOL MfxMessageHook( UINT msg, long wParam=0, long lParam=0 ) ;


//---------------- Registry ------------------------------------------


//
// These functions manipulate driver settings under 
//		HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\md_DVBDriver
//
// Contents of the key :
//		DWORD "ErrorControl"	0x00000001
//		DWORD "Type"			0x00000001
//		DWORD "Start"			2(Automatic)
//		DWORD "Sender"			0(Receiver)/1(Sender)
//		DWORD "Base"			I/O port
//		DWORD "Irq"				IRQ
//
// The driver is installed if the settings are set to above values.
// After each change the driver has to be restarted (from the Control Panel or via reboot).
//

typedef struct tagDVBDRIVERINFO {
	DWORD	Base;
	DWORD	Irq;
	DWORD	Sender;
} DVBDRIVERINFO, *PDVBDRIVERINFO;

BOOL DVBDriverRegisteredByTheSystem	( );		// TRUE if dvb drv is installed

//
// Returns error code (0=OK); to convert to the text use
//		const char	*RegistryErrorMessage( int errorCode );
//
int	 RegistryCreateDVBDriverEntry	( PDVBDRIVERINFO pDVB  ) ;
int	 RegistryRetrieveDVBDriverEntry ( PDVBDRIVERINFO pDVB  ) ;
int	 RegistryModifyDVBDriverEntry	( PDVBDRIVERINFO pDVB  ) ;

BOOL RegistryUninstallDVBDriver		( BOOL removeDrvEntries) ;


//---------------- DVB codes -----------------------------------------


#define Event_DVBErrorFlag			(Facility_DVB | Event_ErrorFlag )

#define DVBERR_INVALID_DRV_VERSION	( Event_IOErrorFlag |  2)
#define DVBERR_PROGRAM_NOT_LOADED	( Event_IOErrorFlag |  3)
#define DVBERR_OPEN_ERROR			( Event_IOErrorFlag |  4)
#define DVBERR_DRVHANDSHAKEFAILED	( Event_IOErrorFlag |  5)
#define DVBERR_CARDNOTDETECTED		( Event_IOErrorFlag |  6)
#define DVBERR_CARDPRGNOTEXIST		( Event_IOErrorFlag |  7)
#define DVBERR_INVALIDCARDPRG		( Event_IOErrorFlag |  8)
#define DVBERR_CANTOPENCARDPRG		( Event_IOErrorFlag |  9)
#define DVBERR_CANTREADCARDPRG		( Event_IOErrorFlag | 10)
#define DVBERR_CMDNOTACCEPTED		( Event_IOErrorFlag | 11)
#define DVBERR_CARDHANDSHAKEFAILED	( Event_IOErrorFlag | 12)
#define	DVBERR_NOT_OPENED			( Event_IOErrorFlag | 13)
#define	DVBERR_OPENED				( Event_IOErrorFlag | 14)

const char* ComergonErrorAsText( int errorCode, char *buf ) ;


//--------------- Card setup ------------------------------------------------


struct ComDVBSetup
{
	int		_flagsCard;
	int		_ackTimeoutCard;
	int		_dvbMemoryDrv;
	int		_flagsDrv;
	int		_cardSpeed;				// kHz (1000b/s)
	unsigned short _PID ;
	GlobalUserID _userId ;
	char		 _drvVersion[20] ;	// Driver version "3.20" or "?.?"

	void load( BaseConfigClass *cfg ) ;
	void save( BaseConfigClass *cfg ) ;
	BOOL driverDumpOn() const
	{
		return _flagsDrv & (DVBDRVFLAG_DumpPackets | DVBDRVFLAG_DumpPacketIDs) ;
	}

	ComDVBSetup()
	{
		_flagsCard		= 0;
		_ackTimeoutCard	= 10;
		_cardSpeed		= 2000;			// 2 MHz
		_dvbMemoryDrv	= 100;
		_flagsDrv		= 0;
		_PID			= 0;
		strcpy( _drvVersion, "?.?" ) ;
	}
} ;

extern ComDVBSetup _setup ;

void DvbSetupDialog( HWND hWndParent, ConfigClass *cfg=0 ) ;
void DvbDestroySetupDialog( ) ;


//--------------- Statistics ------------------------------------------------


struct ComDVBStatistics
{
	ulong	_numPacketsAcceptedDrv;
	ulong	_numInternetPacketsDrv;
	ulong	_numPacketsCorruptedHW;
	ulong	_numPacketsAcceptedHW;
	ulong	_numPacketsFilteredHW;
	ulong	_numPacketsLostHW;
	uchar	_cardFlags ;

	inline void setCardFlags			( uchar flags )		{ _cardFlags = flags; }
	inline BOOL isTimingClockSignalOK	()					{ return _cardFlags & 0x01; }
	inline BOOL isCardSynchronizationOK	()					{ return _cardFlags & 0x02; }

	inline void clear()		{ memset( this, 0, sizeof(ComDVBStatistics) ) ; }
	ComDVBStatistics()		{ clear() ; }
} ;

extern ComDVBStatistics _stat ;


//--------------- I/O -------------------------------------------------------


class IoComergonDll ComOutDVB : public ComOut
{
  public :
	ComOutDVB			() ;
   ~ComOutDVB			() ;

	virtual BOOL hasCapability( ComIOCapability cap ) ;
	virtual int	open	( const char *connectStr) ;
	virtual int	close	();
	virtual int	write	( const char *p, int n_bytes, int *written ) ;
	virtual	int	setSpeed( float maxSpeed, BaseConfigClass *cfg ) ;	// Mb/s
};


class IoComergonDll ComInpDVB : public BaseInpDriver
{
  public :
	BaseComInp		*_baseComInp ;

	ComInpDVB( BaseComInp *x ) ;
   ~ComInpDVB( ) ;

	virtual BOOL hasCapability( ComIOCapability cap ) ;
	virtual int	 open		( const char *connectStr ) ;
	virtual int  close		() ;
	virtual int  workKernel	() ;
	virtual int  normalWorkKernel	();
	virtual BOOL processCommandPacket( MuxPacket *packet ) ;

	// statistics
	virtual void		clearStatistics		( ) ;
	virtual BOOL		getStatisticsLog	( char *buf ) ;
	virtual sDllDialog *createStatisticsPage( ) ;
	virtual BOOL		getUserId( GlobalUserID *id ) ;
} ;


//--------------- Globals ---------------------------------------------------


int openDVB( char *expl ) ;
int closeDVB() ;

BOOL readDrvStatistics( ) ;
BOOL installDialog( HWND parent ) ;
BOOL uninstallDialog( HWND parent ) ;

ResDllDesc *getRcModule() ;
void releaseRcModule() ;

extern ComInpDVB *comInp ;
extern ComOutDVB *comOut ;
extern MyCWinApp theApp ;
extern BOOL		 runningAsServer ;

#endif
