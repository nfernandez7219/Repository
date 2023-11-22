#ifndef __INC_INTERNET_HPP__
#define __INC_INTERNET_HPP__

#include "..\..\IoDrivers\IoHNet\IoHNetInterface.h"

class IOHNet
{
	friend class InternetReceiver ;
	friend DWORD WINAPI internetSendThreadFunc( LPVOID arg ) ;

  protected:
	HMODULE		 _dll ;			// IOdriver dll
	void		*_ioCtl ;
	BOOL		 _isOpened ;
	BOOL		 _status ;		// 0:unknown, 1:created, -1:creation failed

	// returns:
	//	 0 : OK
	//	 -1: fatal error
	//	 else: HNet error code
	short callDriverIoctl( char *expl, int cmd, long par1=0, long par2=0, long par3=0 ) ;

  public:
	// Minimum initialization which allows to use errorCodeAsText().
	// On error Msg type exception is thrown.
	IOHNet() ;
   ~IOHNet() ;

	// Functions return TRUE iff the call succeeded.
	BOOL isOnline ( BOOL *yes, char *expl ) ;
	BOOL setOnline( BOOL  yes, char *expl ) ;

	const char *errorCodeAsText( int code, char *buf ) ;

	// Client/server dependent initialization
	BOOL create( char *expl, BOOL runningAsServer ) ;

	BOOL open( char *expl, CardSerialNumber *adr ) ;
	BOOL isOpened()			{ return _isOpened; }
	void close() ;

	void runSetupDialog( HWND parent, BOOL asServer, UCHAR *cfgData=0, BOOL bModal=FALSE ) ;

	BOOL getSetupData( BOOL asServer, UCHAR **cfgData, int *cfgDataLen, char *expl ) ;
	void destroySetupData( UCHAR *cfgData ) ;
} ;


class InternetChannel : public MuxChannel
{
	HANDLE		   _sendingThread ;

  public:
	// Constructor fails (by throwing int exception) if HNet connection could not be established.
	InternetChannel( Mux *mux, const MuxChannelSetup *s, BOOL online=TRUE ) ;
   ~InternetChannel( ) ;
	virtual BOOL  start		  () ;
	virtual BOOL  initiateStop() ;
	virtual BOOL  stop		  () ;

	inline void	  setFatalError( DWORD err ) ;		// immediate stop
	inline BOOL   isKilled() ;
	inline int	  sendData( const char *buf, int cnt, const CardSerialNumber& usrId ) ;
};

extern IOHNet *hnetConnection ;
extern DVBcallback g_fnHNetCallback ;

#endif
