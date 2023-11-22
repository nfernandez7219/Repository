Introduction:
=============
IoDvbAsi is a part of Main Data Server/Receiver system. Its responsibility is to provide data 
receiving and transmitting using the Linear Systems DVB Master sending an receiving
cards. 
The Master Send is a PCI computer card with a BNC type connector, which
is capable of transmitting a DVB Asynchronous Serial Interface (ASI) standard
stream. 
The DVB ASI Receive is a PCI computer card with a BNC type connector, which 
will accept DVB Asynchronous Serial Interface (ASI) standard inputs.


Runtime dependencies:
=====================
To use this module with Main Data Receiver system you must have DvbAsi connection
string in the DvbClient.cfg configuration file. It is set by the installation.

   ConnectString = DvbAsi

For sending(receiving) data via DVB Master card you need to install manufacturer driver.
You can do it by running DVBcfg.exe applet.


Sources:
========
IoDvbAsi module is placed in IoDvbAsi.dll library in the Main Data instalation directory.
You can update it building IoDvbASi project placed in IoDrivers\IoDvbASi directory.

Project contains these sources:

	ComDvbAsi.cpp          // ComInpDvbAsi and ComOutDvbasi classes
	drv_ComAsi.cpp	       // driver ioctl function (main dll entry point)
	DvbAsiSetupDlg.cpp     // Configuration, Install and Readme dialogs
	ComAsi.def			   // Exports definition
	IoDvbAsi.rc			   // Dll resources

Project depends on these libraries:

version.lib		- Win32 API
scrm41td.lib	- Screen project - should be recompiled before compiling IoSkyMedia
appm41td.lib	- AppLib project - should be recompiled before compiling IoSkyMedia

See IoDvbAsi.lst file for detailed settings of the project.


Communication model:
====================
Communication with device is provided by standard Win32 Api File  asynchronous 
I/O functions ( CreateFile, ReadFileEx, WiriteFileEx ).

The driver has some basic functions, available using standard API function DeviceIoControl:
	- Read card configuration ( IOCTL_DVB_RD_CFG )
	- Write card configuration ( IOCTL_DVB_SET_CFG )
	- Read card statistics ( IOCTL_DVB_RD_ST )
	- Reset Reframe of card ( IOCTL_DVB_RESET_REFRAME )

A main classes are ComOutDvbAsi for communication with Sender card and ComInpDvbAsi 
for communication with Receiver card.
ComOutDvbAsi class use writeData methos for sending data via DVB Master Sender card,
using standard asynchronous file I/O operation WriteFileEx.
ComInpDvbAsi class use readKernel methos for continual data receiving via DVB Master
Receiver card, using standard asynchronous file I/O operation ReadFileEx.
Both classes use DvbAsi class for all low-level 
functions. 
For sending or receiving is alocated nmumber of buffers specified by manufacturer setup 
program DvbCfg.exe. To keep an overlapped informations for all buffers is allocated 
array of OVERLAPPED structures also.
For I/O completion notification we used CompletionRoutine(see ReadFileEx doc.).


Sending data
============
We implemented a writeData method of DvbAsi class for sending data via DVB ASI card.
All buffers are started to readIf the first buffer is not free (write operation on from this buffer did not finish yet),
the second buffer is checked. If the second is not free too, a SleepEx function is called.
This process repeats, until one fo buffer is free, or timeout elapsed.


Receiving data 
==============
We implemented a readKernel method of DvbAsi class for receiving data via DVB ASI card.
For all buffers reading process is started. If some reading operation is finished, 
received data are passed for further processing by function acceptData. 


The hierarchy of classes:
=========================

[ComOutDvbAsi]
\\----ComOutDvbAsi( BaseConfigClass *cfg)
   |
   |--~ComOutDvbAsi( )
   |
   |--virtual int open( const char *connectStr)			// Openinig sender DVB ASI card
   |
   |--virtual int close( )					// Closing DVB ASI card
   |
   |--virtual int write( const char *p, int n_bytes, int *written ) // Sending data via DVB ASI sender card
   |
   |--setSpeed( float maxSpeed )
   |
   |--virtual BOOL hasCapability( ComIOCapability cap )		// Getting Capabilities of Input class
   |
   |--int resetCard( )						// Reseting Reframing the DVB ASI Sender card
   |
   |--class DvbASi _dvbAsi					// Instance of low-level DvbAsi class


[ComInpDvbAsi]
\\----ComInpDvbAsi( BaseComInp *x, BaseConfigClass *cfg )
   |
   |--~ComInpDvbAsi( )
   |
   |--virtual int open( const char *connectStr, ushort pid )	// Openinig receiver DVB ASI card
   |
   |--virtual int close( )					// Closing DVB ASI card
   |
   |--virtual int workKerne( )					// Loop for receiving data via DVB ASI receiver card
   |
   |--virtual BOOL hasCapability( ComIOCapability cap )		// Getting Capabilities of Input class
   |
   |--virtual void clearStatistics( )				// Getting staistics from card
   |
   |--virtual BOOL getStatisticsLog( char *buf )		// Reading statistics from DVB ASI Receiver card
   |
   |--virtual sDllDialog *createStatisticsPage( )		// Creates and returns receiver Statistic dialog page
   |
   |--int resetCard( )						// Reseting Reframing the DVB ASI Receiver card
   |
   |--class DvbASi _dvbAsi					// Instance of low-level DvbAsi class


[DvbAsi]
\\----DvbAsi( BaseConfigClass *cfg, BaseComInp *baseComInp = NULL )
   |
   |--~DvbAsi( )
   |
   |--int openDvbAsiReceiver( )					// Openinig receiver DVB ASI card
   |
   |--int openDvbAsiTransmitter( )				// Openinig sender DVB ASI card
   |
   |--int closeDvbAsi( )					// Closing DVB ASI card
   |
   |--int workKernel( )						// Loop for receiving data via DVB ASI receiver card
   |
   |--writeData( const char *buf, int n_bytes, int *n_written ) // Sending data via DVB ASI sender card
   |
   |--BOOL getStats( int sendOrReceive )			// Reading statistics from DVB asi sender(receiver) card
   |
   |--int resetCard( )						// Reseting Reframing the DVB ASI card
   |
   |--int allocateBuffers( )					// allocation of buffers used by async I/O 
   |
   |--HANDLE _handle       					// handle of DvbAsi device 
   |
   |--BaseConfigClass* _cfg 					// pointer to configuration class
   |
   |--BaseComInp* _baseComInp 					// pointer to input class which accept the read data 
   |
   |--OVERLAPPED* _ov 						// pointer to array of overlapped structures used by async I/O
   |
   |--char* _buf						// pointer to arrat of buffers used by async I/O 
   |
   |--int allocateBuffers( )   					// allocation of read(write) buffers
   |	
   |--int _numBufs       					// # of buffers used by async I/O
   |
   |--int _bufSize      					// size of buffers used by async I/O


Dialog classes:
---------------
[DvbAsiSetupDlg]							// Base dialog for ServerSetupDlg and ClientSetupDlg
\\----DvbAsiSetupDlg( int id, CWnd *parent, ConfigClass *cfg )
   |
   |--~DvbAsiSetupDlg( )
   |
   |--virtual BOOL OnInitDialog( )
   |
   |--virtual void OnOK( )
   |
   |--void setChanged( )						// Notify that controls changed
   |
   |--int workKernel( )
   |
   |--ConfigClass* _cfg
   |
   |--BOOL _controlsChanged
   |
   |--BOOL _readonly 
   |
   |--UINT _id


[ServerSetupDlg] : DvbAsiSetupDlg					// Setup dialog for DVB ASI sender card
\\----ServerSetupDlg( CWnd *parent, ConfigClass *cfg, BOOL readonly )
   |
   |--virtual BOOL OnInitDialog( )
   |
   |--virtual void DoDataExchange( CDataExchange* pDX )
   |
   |--afx_msg void OnControlChanged( )					// Called when controls changed
   |
   |--afx_msg void OnHelp( )						// Help button serve method


[ClientSetupDlg] : DvbAsiSetupDlg					// Setup dialog for DVB ASI receive card
\\----ClientSetupDlg( CWnd *parent, ConfigClass *cfg, BOOL readonly )
   |
   |--virtual BOOL OnInitDialog( )
   |
   |--virtual void DoDataExchange( CDataExchange* pDX )
   |
   |--afx_msg void OnControlChanged( )					// Called when controls changed
   |
   |--afx_msg void OnHelp( )						// Help button serve method


[DvbAsiClientStatDlg]							// Display the card driver statistics (child dialog)				
\\----DvbAsiClientStatDlg(  )
   |
   |--~DvbAsiClientStatDlg(  )
   |
   |--virtual BOOL OnInitDialog( )
   |
   |--virtual void DoDataExchange( CDataExchange* pDX )


The hierarchy of structs:
=========================
[DvbAsiStats]	]
\\----DvbAsiStats( )			// consturctor 
   |
   |--BOOL report( char *pBuf )         // reports the statistic
   |
   |--void clear( )			// clear the statistic
   |
   |--ULONG StartDma		     	// The channel Dma controller was started
   |
   |--ULONG MaxDspIntCount 	     	// The DSP for this channel has been started by the ISRvoid setChanged( )
   |
   |--ULONG NumPciAbts	 	    	// Pci abort interrupt signalled from hardware
   |
   |--ULONG MinNumPend			// Minimum NumPend occuring since start or last IOCTL_DVB_RD_ST
   |
   |--ULONG NumPend			// Number of buffers active for Dma transfer and pending completion
   |
   |--ULONG NumInts			// Number of Dma interrupts generated by the channel
   |
   |--ULONG NumFifoErrs			// Interrupt count at which FIFO overflow occured (0= no errors)
   |
   |--ULONG NumQued;			// Number of buffers queued for Dma activation
   |
   |--ULONG NumLost			// The ISR could not start the DSP (Deferred Service Procedure)
   |
   |--ULONG NumExErr			// Dma completion error (should never occur !)


[DvbAsiConfig]
\\----BOOL load	( BaseConfigClass * cfg )   	// Load configuration from cfg file
   |
   |--BOOL save	( ConfigClass * cfg ) 		// Save configuration to cfg file	
   |
   |--BOOL	_NoIbStuffing          		// No Inter-Byte stuffing
   |
   |--BOOL	_204Bytes 			// Read Solomon (204 bytes packets)
   |
   |--BOOL	_pacSynth 			// Enable Transport Stream packet synchronization for receive
   |
   |--BOOL	_rfHigh 			// ReFraming High/Low mode
   |
   |--float	_speed 				// transmission speed		[Mbit/s]



Notes:
======
DvbAsi API uses 8 Byte struct alignment without setting it explicitly (Error in the header.),
so if you don't use 8 Byte alignment you must change it when you include "dvbioctl.h".

Asynchronous file operation. Use more buffers for read(write) from(to) card.
When one read(write) operation finished, the other read(write) operation already waits
in a DMA queue, so there is no (time consuming) software intervention needed. 
That's also reason, why at least two buffer are needed to receive all data in the stream
without loss.

In order to accept the changes you write by IOCTL_DVB_SET_CFG command you have to
close and re-open device.
