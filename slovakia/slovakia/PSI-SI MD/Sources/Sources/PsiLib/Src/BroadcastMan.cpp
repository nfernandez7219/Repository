
#include "stdafx.h"
#include "TableHolders.h"
#include "ComOut.h"
#include "BroadcastMan.h"
#include "Heap.h"
#include "scrtools.hpp"
#include "Env.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


class PidStreamAttribManager
{
	sTemplateArray<PidStreamAttrib*>	_streamAttribs ;

  public:
	PidStreamAttrib  *getPidStreamAttrib		( ushort pid ) ;
	void			  releasePidStreamAttrib	( PidStreamAttrib *streamAttrib ) ;

	~PidStreamAttribManager() ;
} ;

static UINT SenderThreadWorkerFn( LPVOID pParam )
{
	PsiData *data = (PsiData*)pParam ;
	BroadcastManager *man = (BroadcastManager*)data->_pParam ;

	PidStreamAttrib *pidAttrib = man->_pidStreamManager->getPidStreamAttrib(data->_pid) ;

	HANDLE handles[2] ;

	handles[0] = HANDLE(man->_runningEvent) ;
	handles[1] = HANDLE(man->_terminateEvent) ;

	while ( ::WaitForMultipleObjects( 2, handles, FALSE, INFINITE ) == WAIT_OBJECT_0 )
	{
		// BroadcastManager started => send tables
		UINT freq = data->_freq ;
		if (freq == 0)
			freq = 500 ;	// Wait half second and check whether freq did not changed
		else
		{
			EnterCriticalSection(&man->_critSection) ;
			// test if terminate event was not signalled during entering critical section
			if (::WaitForSingleObject(handles[1], 0) == WAIT_OBJECT_0 )
			{
				LeaveCriticalSection(&man->_critSection) ;
				return 0 ;
			}
			
			man->SendSections( pidAttrib, data->_nSections, data->_privateSections ) ;

			LeaveCriticalSection(&man->_critSection) ;
		}

		// wait 25 msec (or less when terminate event is signalled)
		::WaitForSingleObject( handles[1], freq ) ;
	}

	return 0 ;
}

BroadcastManager::BroadcastManager( CWnd *parent, const char *configFile, PsiDataArray *data, BOOL start )
 :  _runningEvent( FALSE, TRUE ), // manual event without multi-lock
	_terminateEvent( FALSE, TRUE ) // manual event without multi-lock
{
	_parent			= parent ;
	_data			= data ;
	_senderThreads	= NULL ;
	_config			= new ConfigClass(configFile) ;

	_pidStreamManager = new PidStreamAttribManager ;
	
	_config->open(NULL,0,TRUE,TRUE) ;
	
	BOOL bOpened = OpenDevice() ;

	InitializeCriticalSection(&_critSection) ;
	CreateThreads(data) ;

	if (start && bOpened)
		_runningEvent.SetEvent() ;
}

BroadcastManager::~BroadcastManager( )
{
	TerminateThreads() ;

	if (_comOut)
	{
		_comOut->close() ;
		delete _comOut ;
	}

	_config->save(FALSE) ;
	delete _config ;

	DeleteCriticalSection(&_critSection) ;

	delete _pidStreamManager ;
}

BOOL BroadcastManager::OpenDevice( )
{
	const char *connectString = _config->get(NULL, "ConnectString") ;
	if (connectString==NULL)
	{
		connectString="Ethernet" ;
		_config->set(NULL, "ConnectString", connectString) ;
	}

	if (GetProgramLevel()==PrgLevelDemo)
	{
		::MessageBox(NULL, 
			"Application is running in DEMO mode\n"
			"No output device will be created\n\n"
			"Make sure your HW key is plugged into parallel port\n"
			"and check program level in About dialog.",
			"Warning", 
			MB_OK | MB_ICONERROR) ;

		_comOut = NULL ;
		return FALSE ;
	}

	char expl[256] ;
	_comOut = new BigComOut ;
	if ( !_comOut->create(connectString, expl) )
	{
		::MessageBox(NULL, expl, "Fatal error", MB_OK | MB_ICONERROR) ;
		delete _comOut ;
		_comOut = NULL ;
		return FALSE ;
	}

	
	int err ;

	{
		CWaitCursor cur ;
		err = _comOut->open(_config) ;
	}

	if (err)
	{
		char errTxt[512] ;
		strcpy(errTxt, "Error opening output device object.\n\n" ) ;
		_comOut->errorCodeAsText(err, errTxt+strlen(errTxt)) ;
		::MessageBox( NULL, errTxt, "Fatal error", MB_OK | MB_ICONERROR ) ;
		delete _comOut ;
		_comOut = NULL ;
		return FALSE ;
	}

	return TRUE ;
}

void BroadcastManager::CreateThreads( PsiDataArray *dataArr )
{
	_nThreads = dataArr->count() ;

	ASSERT(_senderThreads==NULL) ;
	_senderThreads = (CWinThread**)malloc( _nThreads*sizeof(CWinThread*) ) ;

	for ( int i=0; i < _nThreads; i++)
	{
		PsiData *data = &dataArr->item(i) ;
		data->_pParam = (void*)this ;
		 CWinThread *pThread = AfxBeginThread( SenderThreadWorkerFn, (LPVOID)data, THREAD_PRIORITY_ABOVE_NORMAL ) ;
		 pThread->m_bAutoDelete = FALSE ;
		 _senderThreads[i] = pThread ;
	}
}

void BroadcastManager::TerminateThreads( )
{
	_terminateEvent.SetEvent( ) ;

	if (_senderThreads)
	{
		for ( int i=0; i < _nThreads; i++)
		{
			CWinThread *pThread = _senderThreads[i] ;
			if (pThread)
			{
				CWaitCursor cur ;
				WaitForSingleObject(pThread->m_hThread, 5000) ; //ASSERT(==WAIT_OBJECT_0) ;
				delete pThread ;
			}
		}

		free(_senderThreads) ;
		_senderThreads = NULL ;
	}

	_terminateEvent.ResetEvent( ) ;
}

void BroadcastManager::SetData( PsiDataArray *data )
{
	TerminateThreads( ) ;

	EnterCriticalSection(&_critSection) ;
	_data = data ;
	CreateThreads(data) ;
	
	LeaveCriticalSection(&_critSection) ;
}

void BroadcastManager::Start( )
{ 
	if ( _comOut && _comOut->isOpened() && _senderThreads!=NULL ) 
		_runningEvent.SetEvent() ; 
	else
		_parent->MessageBox( 
			"Output device is not opened.\n"
			"No tables can be sent.", "Error", 
			MB_OK | MB_ICONERROR
		) ;
}

BOOL BroadcastManager::IsRunning( )
{
	return (::WaitForSingleObject(HANDLE(_runningEvent), 0) == WAIT_OBJECT_0 ) ;
}

int BroadcastManager::GetDevMode( )
{
	if (_comOut==NULL)
		return NoDevice ;

	char conStr[100] ;
	strcpy(conStr, _comOut->connectString());
	strupr(conStr) ;

	if (strstr(conStr, "DVBASI")!=0)
		return DvbAsi ;

	if (strstr(conStr, "DVB")!=0)
		return Dvb ;

	if (strstr(conStr, "TCP")!=0 || strstr(conStr, "UDP")!=0 || strstr(conStr, "ETHERNET")!=0)
		return Ethernet ;

	return NoDevice ;
}

int BroadcastManager::SetDevMode( int mode )
{
	if (IsRunning())
	{
		int res = _parent->MessageBox( 
			"Output device is opened.\n"
			"It must be stop before changing output device.\n\n"
			"Do you want to stop broadcasting ?",
			"Question",
			MB_YESNO | MB_ICONQUESTION
		) ;
		if (res != IDYES)
			return GetDevMode() ;

		_runningEvent.ResetEvent() ; 
	}

	if (_comOut)
	{
		_comOut->close() ;
		delete _comOut ;
		_comOut = NULL ;
	}

	char *connectString ;
	
	switch (mode)
	{
		case DvbAsi:
			connectString="DvbAsi" ; break ;
		case Dvb:
			connectString="Dvb" ; break ;
		case Ethernet:
			connectString="Ethernet" ; break ;
		default:
			return NoDevice ;
	} ;

	_config->set(NULL, "ConnectString", connectString) ;

	if ( OpenDevice() )
	{
		char buf[256] ;
		sprintf( buf, "Output was directed into device : %s\n\nRun \"Device Setup\" to configure selected device.", connectString ) ;
		AfxGetMainWnd()->MessageBox(buf, "Information", MB_OK|MB_ICONINFORMATION ) ;
		return mode ;
	}
	else
		return NoDevice ;
}

void BroadcastManager::RunDeviceSetup( )
{
	if (_comOut==NULL)
		return ;

	if (IsRunning())
	{
		int res = _parent->MessageBox( 
			"Output device is opened.\n"
			"It must be stop before changing device setup.\n\n"
			"Do you want to stop broadcasting ?",
			"Question",
			MB_YESNO | MB_ICONQUESTION
		) ;
		if (res != IDYES)
			return ;

		_runningEvent.ResetEvent() ; 
	}

	_comOut->close() ;
	_comOut->runSetupDialog(_parent, _config) ;
	CWaitCursor cur ;
	_comOut->open(_config) ;
}


//////////////////////////////////////////////
//	Transport stream packet					//
//////////////////////////////////////////////

class PidStreamAttrib
{
	friend class PidStreamAttribManager ;

	ushort	_pid ;					// PID of the stream
	ushort	_convertedPid ;			// PID with swapped bytes (as it is in the TS packet header)
	uchar	_continuityCounter ;	// number of the next TS packet in the PID stream
	long	_nReferences ;			// number of channels which referenced this class

  public:
	PidStreamAttrib( ushort pid )			{ _pid=pid; _convertedPid=ntols(pid); _continuityCounter=0; }

	inline uchar	getContinuityCounter ()	{ return (_continuityCounter++)&0x0F; }
	inline ushort	getConvertedPid		 () { return _convertedPid ; }
	inline ushort	pid					 () { return _pid ; }
} ;

PidStreamAttrib *PidStreamAttribManager::getPidStreamAttrib( ushort pid )
{
	PidStreamAttrib *streamAttrib ;
	int i = _streamAttribs.count() ;
	while(i--)
	{
		streamAttrib = _streamAttribs[i];
		if (streamAttrib->_pid==pid)
		{
			InterlockedIncrement(&streamAttrib->_nReferences);
			return streamAttrib ;
		}
	}

	streamAttrib = new PidStreamAttrib(pid) ;
	_streamAttribs.add(streamAttrib) ;
	return streamAttrib ;
}

void PidStreamAttribManager::releasePidStreamAttrib ( PidStreamAttrib *streamAttrib )
{
	long nRef = InterlockedDecrement(&streamAttrib->_nReferences) ;
/*	if (nRef==0)
	{
		_streamAttribs.delObj(streamAttrib) ;
		delete streamAttrib ;
	}
*/
}

static void PidStreamAttribDelFn( PidStreamAttrib **p )
{
	delete *p ;
	*p = NULL ;
}

PidStreamAttribManager::~PidStreamAttribManager()
{
	_streamAttribs.setDelFunc(PidStreamAttribDelFn) ;
}

#define TSPAYLOAD_SIZE		184
#define TsPacketSyncByte	0x47	// 'G'

struct TsPacket
{
	uchar	sync ;		// 0x47

	// transport_error_indicator:1=0
	// payload_unit_start_indicator:1=0
	// transport_priority:1=0
	// pid:13 = pid
	ushort	pid ;

	// transport_scrambling_control:2=0
	// AF:1 = 0
	// data:1 = 1
	// counter:4=0,1,2... incremented for subsequent packets with the same PID
	uchar	flags ;

	uchar	data[TSPAYLOAD_SIZE] ;

	void create( const uchar *mp, PidStreamAttrib *pidStrAttr, size_t size=TSPAYLOAD_SIZE )
	{
		sync = TsPacketSyncByte ;
		pid = pidStrAttr->getConvertedPid();
		flags = 0x10 | pidStrAttr->getContinuityCounter() ;
		memcpy( data, mp, size );

		if (size < TSPAYLOAD_SIZE)
			memset(data+size, 0 , TSPAYLOAD_SIZE-size) ;
	}

	// Creates the TS packet containing DSMCC section
	// Sets "Payload unit start" flag and adds section start pointer
	// If the section is smaller than payload size,
	// rest of the packet is filled with 0xFF stuffing bytes
	int createSection	( const uchar *dataPtr, int size, PidStreamAttrib *pidStrAttr ) ;

	// Adds the DSMCC section into packet
	// Use only if packet has enough size to contain the section header
	// You must know offset where the data ends in the TS packet
	// If not already set addSection sets "Payload unit start" flag
	// and adds section start pointer
	int addSection		( const uchar *dataPtr, uchar &offset, int size ) ;

	TsPacket( )		
	{ 
		sync = TsPacketSyncByte ;
		pid = 0;
		flags = 0x10 ;
	}

	TsPacket( const uchar *mp, PidStreamAttrib *pidStrAttr )
	{
		create( mp, pidStrAttr ) ;
	}

	inline isPayloadUnitStart	( )								{ return pid&0x40 ; }
	inline setPayloadUnitStart	( )								{ pid = 0x40|pid ; }
	inline isCompatible			( PidStreamAttrib *pidStrAttr )	{ return (pid&0xFF1F)==pidStrAttr->getConvertedPid() ; }
} ;


//-------------------------------------------------------//
//	Transport stream (TS packet)
//-------------------------------------------------------//

int TsPacket::createSection( const uchar *dataPtr, int size, PidStreamAttrib *pidStrAttr )
{
	// first packet of the section
	//  - payload_unit_start_indicator is set
	//  - next section pointer (zero byte) inserted before section
	sync = TsPacketSyncByte ;
	pid = pidStrAttr->getConvertedPid();
	setPayloadUnitStart() ;
	flags = 0x10 | pidStrAttr->getContinuityCounter() ;

	char *tpData = (char*)data ;
	*tpData = '\x0' ; // next section pointer
	if (size >= TSPAYLOAD_SIZE-1)
	{
		memcpy(tpData+1, dataPtr, TSPAYLOAD_SIZE-1) ;
		return TSPAYLOAD_SIZE-1 ;
	}
	else
	{
		memcpy(tpData+1, dataPtr, size) ;
		memset(tpData+1+size, 0xFF, TSPAYLOAD_SIZE-(1+size)) ;
		return size ;
	}
}

int TsPacket::addSection( const uchar *dataPtr, uchar &offset, int size )
{
	uchar *dataStart = data ;

	if (!isPayloadUnitStart())
	{
		setPayloadUnitStart() ;

		//  append next section pointer before the data
		memmove(dataStart+1, dataStart, offset) ;
		*dataStart = offset++ ;
	}

	size = __min(size, TSPAYLOAD_SIZE-offset) ;
	memcpy(dataStart+offset, dataPtr, size) ;

	offset += size ;
	return size ;
}


void BroadcastManager::SendSections( PidStreamAttrib *pidAttr, int nSections,  PrivateSection *sections )
{
	int nBytes, written ;
	// create TS packets

	TsPacket *ts = (TsPacket*)_buffer ;
	int nTS = 0 ;

	for (int i = 0; i < nSections; i++)
	{
		int sectLength = sections->getTotalLength() ;
		int sectPos = ts[nTS++].createSection((uchar*)sections, sectLength, pidAttr) ;
		uchar *sectStart = (uchar*)sections ;

		while (sectPos < sectLength)
		{
			ts[nTS++].create(sectStart+sectPos, pidAttr, __min(sectLength-sectPos,TSPAYLOAD_SIZE) ) ;
			sectPos += TSPAYLOAD_SIZE ;
		}

		sections = (PrivateSection*)(sectStart+sectLength) ;
	}

	nBytes = nTS * sizeof(TsPacket) ;
	// send TS packets
	if (_comOut)
	{
		_comOut->com()->write( _buffer, nBytes, &written ) ;
//		ASSERT(nBytes==written) ;
	}
}
