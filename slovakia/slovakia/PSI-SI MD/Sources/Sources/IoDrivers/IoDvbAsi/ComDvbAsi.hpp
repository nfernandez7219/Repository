#ifndef __INC_COMDVBASI_HPP__
#define __INC_COMDVBASI_HPP__


#ifdef  __IODVBASIDLL
    #define IoDvbAsiDll	__declspec( dllexport )
#else
    #define IoDvbAsiDll	__declspec( dllimport )
#endif

class sDllDialog ;
class ResDllDesc ;


#pragma warning( disable: 4275 )

#include "ComIo.hpp"
#include "DvbError.hpp"
#include "MuxPacket.hpp"

BOOL MfxMessageHook( UINT msg, long wParam=0, long lParam=0 ) ;


//---------------- DVB Asi error codes -----------------------------------------


#define ER_CONFIGFILE				( Event_IOErrorFlag |  3)
#define ER_INVALID_HANDLE			( Event_IOErrorFlag |  4)
#define ER_CLOSE_HANDLE				( Event_IOErrorFlag |  5)	
#define ER_WRITE_OPERATION			( Event_IOErrorFlag |  6)
#define ER_READ_OPERATION			( Event_IOErrorFlag |  7)
#define ER_IOCTRL					( Event_IOErrorFlag |  8)
#define ER_INSUFFICIENT_MEMORY		( Event_IOErrorFlag |  9)

const char* DvbAsiErrorAsText( int errorCode, char *buf ) ;


//------------------------------------------------------------------
//	DvbAsiStats struct
//------------------------------------------------------------------

#define MAX_DMA_CHANELS 8 

struct DvbAsiStats
{
	ULONG StartDma		    ; // The channel Dma controller was started
	ULONG MaxDspIntCount    ; // The DSP for this channel has been started by the ISR
	ULONG NumPciAbts		; // Pci abort interrupt signalled from hardware
	ULONG MinNumPend		; // Minimum NumPend occuring since start or last IOCTL_DVB_RD_ST
	ULONG NumPend			; // Number of buffers active for Dma transfer and pending completion
	ULONG NumInts			; // Number of Dma interrupts generated by the channel
	ULONG NumFifoErrs		; // Interrupt count at which FIFO overflow occured (0= no errors)
	ULONG NumQued;			; // Number of buffers queued for Dma activation
	ULONG NumLost			; // The ISR could not start the DSP (Deferred Service Procedure)
	ULONG NumExErr			; // Dma completion error (should never occur !)

	DvbAsiStats( ) { clear( ) ; }  
	BOOL report( char *pBuf ) ;    // report the statistic
	void clear( )  ;			   // clear the statistic
} ;

//////////////////////////////////////////////////////////////////////////////////////////
//	DvbAsi configuration struct
//////////////////////////////////////////////////////////////////////////////////////////


#define DVBASI_CFG_SECTION "DvbAsiConfig"

struct DvbAsiConfig
{
	BOOL	_NoIbStuffing ;         // No Inter-Byte stuffing
	BOOL	_204Bytes ;				// Read Solomon (204 bytes packets)
	BOOL	_pacSynth ;				// Enable Transport Stream packet synchronization for receive
	BOOL	_rfHigh ;				// ReFraming High/Low mode
	float	_speed ;				// transmission speed		[Mbit/s]
	
	BOOL	load	( BaseConfigClass * cfg ) ;  // Load configuration from cfg file
	BOOL	save	( ConfigClass * cfg ) ;		 // Save configuration to cfg file	
} ;



////////////////////////////////////////////////////////////////////////////////////////
//							Low-Level class										////
////////////////////////////////////////////////////////////////////////////////////////

class DvbAsi
{
	HANDLE				_handle ;       // handle of DvbAsi device 
	BaseConfigClass*	_cfg ;			// pointer to configuration class
	BaseComInp*			_baseComInp ;	// pointer to input class which accept the read data 

	OVERLAPPED*			_ov ;			// pointer to array of overlapped structures used by async I/O
	char*				_buf ;			// pointer to arrat of buffers used by async I/O 

	int		allocateBuffers( ) ;		// allocation of buffers used by async I/O  

  public:
	int					_numBufs ;      // # of buffers used by async I/O
	int					_bufSize ;      // size of buffers used by async I/O

	DvbAsi( BaseConfigClass *cfg, BaseComInp *baseComInp = NULL ) ;
	~DvbAsi() ;
	int		 openDvbAsiReceiver( )			;	// Openinig receiver DVB ASI card
	int		 openDvbAsiTransmitter( )		;	// Openinig sender DVB ASI card
	int 	 closeDvbAsi( )					;	// Closing DVB ASI card
	int		 writeData( const char *buf, int n_bytes, int *n_written ) ; // Sending data via DVB ASI sender card
	int      workKernel( )					;	// Loop for receiving data via DVB ASI receiver card
	int	     resetCard( )					;	// Reseting Reframing the DVB ASI card
	BOOL	 getStats( int sendOrReceive )  ;	// Reading statistics from DVB asi sender(receiver) card

} ;



//--------------- I/O ---------------------------------------------------------
///     Main Classes for I/O communication									///
//-=---------------------------------------------------------------------------

class IoDvbAsiDll ComOutDvbAsi : public ComOut
{
  public :
	DvbAsi			*_dvbAsi ;								; // Instance of low-level DvbAsi class

	ComOutDvbAsi( BaseConfigClass *cfg) ;
   ~ComOutDvbAsi( )						;

	virtual BOOL	hasCapability( ComIOCapability cap )	; // Getting Capabilities of Output class
	virtual int		open( const char *connectStr)			;			
	virtual int		close( )								;
	virtual int		write( const char *p, int n_bytes, int *written ) ;
	virtual	int		setSpeed( float maxSpeed, BaseConfigClass *cfg ) ; // Mb/s
	int				resetCard( )							;	
};


class IoDvbAsiDll ComInpDvbAsi : public BaseInpDriver
{
  public :
	DvbAsi*			_dvbAsi ;								; // Instance of low-level DvbAsi class

	ComInpDvbAsi( BaseComInp *x, BaseConfigClass *cfg )		;
   ~ComInpDvbAsi( )											;

	virtual BOOL hasCapability( ComIOCapability cap )		; // Getting Capabilities of Input class
	virtual int	 open( const char *connectStr ) ;
	virtual int  close( )									;
	virtual int  workKernel( )								;
	int			 resetCard( )								;
	BOOL		 getUserId( GlobalUserID *id )				;

	virtual void		 clearStatistics( )					; // Clear of receiver statistics (global stats)
	virtual BOOL		 getStatisticsLog( char *buf )		; // Getting staistics from card
	virtual sDllDialog   *createStatisticsPage( )			; // Creates and returns receiver Statistic dialog page
} ;


//--------------- Globals ---------------------------------------------------


extern ResDllDesc *resMod	;	// Resource module
extern ComInpDvbAsi *comInp ;	// Instance of main Input class
extern ComOutDvbAsi *comOut ;   // Instance of main Output class
extern CWinApp theApp		;	// Application instance
extern BOOL runningAsServer ;	// Information variable Dll is used by server or by client
extern DvbAsiStats  stats   ;	// Stores the DVB ASI receiver statistics
extern DvbAsiConfig	_setup  ;   // Stores the DVB ASI card configuration.


//----------------Setup and install Dialogs--------------------------------------------------

void DvbAsiSetupDialog( CWnd* wnd, ConfigClass *cfg, BOOL asServer ) ; // Runs setup dialogs
void installDialog( HWND parent, ConfigClass *cfg, BOOL bServer )    ; // Runs instalation dialogs
void uninstallDialog( HWND parent, ConfigClass *cfg, BOOL bServer )  ; // Uninstalation of DVB ASI card 


#endif