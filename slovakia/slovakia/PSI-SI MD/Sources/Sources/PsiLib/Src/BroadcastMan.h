
#ifndef __INC_BROADCASTMAN_H__
#define __INC_BROADCASTMAN_H__

#include <afxmt.h>

class BigComOut ;
class ConfigClass ;
class PidStreamAttrib ;
class PidStreamAttribManager ;

class BroadcastManager
{
	friend UINT SenderThreadWorkerFn( LPVOID pParam ) ;

	CWnd			*_parent ;
	PsiDataArray	*_data ;
	BigComOut		*_comOut ;
	ConfigClass		*_config ;
	
	CEvent				_runningEvent ;
	CEvent				_terminateEvent ;
	int					_nThreads ;
	CWinThread**		_senderThreads ;
	CRITICAL_SECTION	_critSection ;
	
	PidStreamAttribManager*	_pidStreamManager ;

	char				 _buffer[65536] ;

	void	CreateThreads		( PsiDataArray *dataArr ) ;
	void	TerminateThreads	( ) ;
	BOOL	OpenDevice			( ) ;

public:
	 BroadcastManager ( CWnd *parent, const char *configFile, PsiDataArray *data, BOOL start = FALSE ) ;
	~BroadcastManager ( ) ;

			void	SetData		( PsiDataArray *data ) ;
			inline	PsiDataArray *GetData		( )			{ return _data ; }

			void	Start		( )	;
	inline	void	Stop		( )							{ _runningEvent.ResetEvent() ; }

			void	RunDeviceSetup	( ) ;

			BOOL	IsRunning	( )	;
			int		GetDevMode	( ) ;
			int		SetDevMode	( int mode ) ;

	enum DevMode
	{
		NoDevice, DvbAsi, Dvb, Ethernet

	} ;

protected:

	void	SendSections	( PidStreamAttrib *pidAttr, int nSections,  PrivateSection *sections ) ;
} ;

#endif //__INC_BROADCASTMAN_H__

