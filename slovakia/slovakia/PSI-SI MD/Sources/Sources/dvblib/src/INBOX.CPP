/*
 *	Filename:		inbox.cpp
 *
 *	Version:		1.00
 *
 *	Description: Implements Inbox class which is responsible for:
 *		- watching for changes in specified directory
 *		- automatic sending of new files
 *
 *	History:
*/

//
// Implementation of the Inbox class
//
// There are 2 queues:
//	- FA (File Accept) queue containing candidates to send (Files which just appeared
//	  in the Inbox directory, but have to wait for specified period until recognized.)
//	- FS (File Send) queue containing (recognized) candidates for send.
// Both queues are sorted acc. to the time to process (recognize time, resp. send time).
//
// Main controlling thread is Inbox::_dirWatchingThread().
// It watches for directory changes, move send requests (files) between both queues
// and organizes file sending.
//

#include "tools2.hpp"
#include "mux.hpp"

#include <afxconv.h>
#include <process.h>
#include "DvbUser.hpp"
#include "inbox.hpp"
#include "MfxGlobals.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_BUFFER  4096

// Uncomment following line to get detailed messages.
//#define DETAILEDLOG

#ifdef DETAILEDLOG
	#define	detailedLog	msgFun
#else
	#define detailedLog	1 ? 0 : msgFun
#endif


// messages
enum MsgCode {				// par						description
	// debug messages
	NewFile			=101,	// char *filename			new file added to inbox
	ChangedFile		=102,	// char *filename			file in inbox changed
	DeletedFile		=103,	// char *filename			file in inbox deleted
	RequestToSend	=104,	// char *filename			request to send file queued
	NoFreePackets	=105,	// NULL
} ;


//---------------------------------------------------------------------------------------
//	structures for message passing
//---------------------------------------------------------------------------------------

// sended when file sending is started
struct InboxMsgSendStarted
{
	const char  *filename ;
	short		 timesToSendYet ;		// how many times should be sent yet (after this send; 0=infinite)
	int			 maxRebr;				// maximum number of rebroadcasts
	InboxMsgSendStarted( const char *f, short t, int maxr )
	{
		filename		= f;
		timesToSendYet	= t ;
		maxRebr			= maxr;
	}
} ;

// sended when file sending is finished
struct InboxMsgSendCompleted
{
	const char  *filename ;
	short		 timesToSendYet ;		// same as for InboxSendStarted
	int			 maxRebr;
	DWORD		 error ;				// 0 = success
	InboxMsgSendCompleted( const char *f, short t, int maxr, DWORD err )
	{
		filename		= f;
		timesToSendYet	= t ;
		maxRebr			= maxr;
		error			= err ;
	}
} ;

// sended when error occurs during file sending
struct InboxMsgSendFailed
{
	const char  *filename ;
	DWORD		 error ;				// Win32 error
	InboxMsgSendFailed( const char *f, DWORD err )
	{
		filename = f;
		error = err ;
	}
} ;


//---------------------------------------------------------------------------------------
//	InboxQueue	- holds file informations needed during watching and sending
//				- files are sorted by processing time (first item is processed as first)
//---------------------------------------------------------------------------------------

InboxQueue::InboxQueue( const char *nam )
{
	strncpy(_name,nam,3) ; 
	_name[3]=0 ; 
	_fStart=NULL ; 
	_numberInQueue = 0;
	InitializeCriticalSection( &_useInboxQueueLock );
}

InboxQueue::~InboxQueue()						
{
   delAll() ; 	
   DeleteCriticalSection( &_useInboxQueueLock );
}

// find where the file will be placed in the queue
// time specifies the processing time (when will be processed) of the new file
InboxFileArray *InboxQueue::_findWhereToPut( LONGLONG time )
{
	InboxFileArray *fTmp = _fStart, *fPrev = NULL;
	while ( ( fTmp!=NULL ) && ( fTmp->TimeWhenProcess<=time ) )
	{
		fPrev = fTmp;
		fTmp = fTmp->next;
	}
	return fPrev;
}

// find the specified file, return the file specific information structure
InboxFileArray *InboxQueue::_search( char *fn )
{
	InboxFileArray *fTmp = _fStart;
	while ( (fTmp!=NULL) && (strcmp(fn,fTmp->FileName)!=0) )
		fTmp = fTmp->next;
	return fTmp;
}

//add file with delay
// Keeps the queue sorted acc. to the time when the files are to be sent.
void InboxQueue::add( char *fn, LONGLONG delay, int numrb )
{
	//TRACE( "\nFile added %s", fn );
	FILETIME ft;
	InboxFileArray *fNew, *f;
	fNew = new InboxFileArray;
	fNew->FileName = STRDUP( fn );
	ASSERT( fNew->FileName != NULL );
	//fNew->FileName = (char *)malloc( (strlen( fn )+1)*sizeof(char) );
	//strcpy( fNew->FileName, fn );
	CoFileTimeNow( &ft );
	fNew->TimeWhenProcess = *(LONGLONG *)(&ft) + delay;
	fNew->numOfRebr = fNew->maxNumOfRebr = numrb;
	fNew->numOfFails = 0;
	fNew->jobId = 0;
	fNew->firstTime = TRUE ;
	fNew->next = NULL;

	EnterCriticalSection( &_useInboxQueueLock);
	if ( _fStart==NULL )
		_fStart = fNew;
	else
	{
		f = _findWhereToPut( fNew->TimeWhenProcess );
		if ( f==NULL )	// Add to the start
		{
			fNew->next = _fStart;
			_fStart = fNew;
		}
		else
		{
			fNew->next = f->next;
			f->next = fNew;
		}
	}
	_numberInQueue++;
	LeaveCriticalSection( &_useInboxQueueLock);
//	return fNew;
}

// delete the specified file from the queue
void InboxQueue::del( char *fn )
{
	EnterCriticalSection( &_useInboxQueueLock);
	InboxFileArray *fTmp = NULL, *fS = _fStart;
	while ( (fS!=NULL) && (strcmp(fn,fS->FileName)!=0) ) 
	{
		fTmp = fS;
		fS = fS->next;
	}
	if ( fS!=NULL )
	{
		if ( fTmp==NULL )
		{
			if( (_fStart)->next == NULL)
				_fStart = NULL;
			else
				_fStart = _fStart->next;
		}
		else
		{
			fTmp->next = fS->next;
		}
		ASSERT( fS->FileName != NULL );
		FREE( fS->FileName );
		delete fS;
		_numberInQueue--;
	}
	LeaveCriticalSection( &_useInboxQueueLock);
}

// removes all files from the queue
void InboxQueue::delAll( )
{
	EnterCriticalSection( &_useInboxQueueLock);
	InboxFileArray *fTmp;
	while ( _fStart!=NULL )
	{
		fTmp = _fStart->next;
		FREE(  _fStart->FileName );
		delete _fStart;
		_fStart = fTmp;
	}
	_numberInQueue = 0;
	LeaveCriticalSection( &_useInboxQueueLock);
}

// Modifies the file (fnold) information.
// Sets the new filename, new processing time, #rebroadcasts etc.
// If create is true and fnold file is not in the queue adds the new file to the queue.
// Keeps the queue sorted acc. to the time when the files are to be sent.
void InboxQueue::modify( char *fnold, char *fnnew, LONGLONG delay, int numrb, uchar jobId, BOOL create )
{
	//TRACE( "\nFile modified %s - %s", fnold, fnnew );
	if ( fnold == NULL ) return;
	EnterCriticalSection( &_useInboxQueueLock);

	// Search for fnold
	InboxFileArray *fTmp = _fStart, *fPrev = NULL;
	while ( ( fTmp!=NULL ) && ( strcmp( fnold, fTmp->FileName )!=0 ) )
	{
		fPrev = fTmp;
		fTmp = fTmp->next;
	}

	if ( fTmp!=NULL )
	{
		// Filename found; update the item
		FILETIME ft;
		CoFileTimeNow( &ft );
		fTmp->TimeWhenProcess = *(LONGLONG *)(&ft) + delay;
		if ( numrb >= 0 )
		{
			fTmp->numOfRebr = fTmp->maxNumOfRebr = numrb;
		}
		if ( fnnew!=NULL )
		{
			ASSERT( fTmp->FileName != NULL );
			FREE( fTmp->FileName );
			fTmp->FileName = STRDUP( fnnew );
			ASSERT( fTmp->FileName != NULL );
		}
		if ( jobId != 0 )
			fTmp->jobId = jobId;
		
		InboxFileArray *f = _findWhereToPut( fTmp->TimeWhenProcess );

		if ( ( f==fPrev ) || ( f==fTmp) )
			goto labelEnd ;

		if ( fPrev != NULL )
			fPrev->next= fTmp->next;
		else
			_fStart = _fStart->next;
		fTmp->next = f->next;
		f->next    = fTmp;
	}
	else
	{
		// Filename not found; add new item
		LeaveCriticalSection( &_useInboxQueueLock);
		if ( create )
			if ( fnnew )
				add( fnnew, delay, numrb );
			else
				add( fnold, delay, numrb );
		return;
	}

  labelEnd:
	LeaveCriticalSection( &_useInboxQueueLock);
}

// increment the number of send file fails for fn and return the old value, if no item -1
long InboxQueue::incSendFails( char *fn )
{
	long l = -1;
	EnterCriticalSection( &_useInboxQueueLock);

	InboxFileArray *f = _search( fn );
	if ( f != NULL )
		l = ++f->numOfFails;

	LeaveCriticalSection( &_useInboxQueueLock);
	return l;
}

BOOL InboxQueue::setSendCompleted( char *fn )	// = TRUE iff it was the last rebroadcast
{
	BOOL l = FALSE;
	EnterCriticalSection( &_useInboxQueueLock);
	InboxFileArray *f = _search( fn );
	if ( f != NULL )
	{
		f->firstTime = FALSE;

		f->numOfRebr--;
		if ( f->numOfRebr<0 )
			f->numOfRebr = 0;
		l = (f->numOfRebr == 1) ;
	}		
	LeaveCriticalSection( &_useInboxQueueLock);
	return l;
}

// Return first waiting item or NULL if the queue is empty.
// Allways the first item is the item for next processing, due to the sorting of the queue.
BOOL InboxQueue::firstItem( char *fn, LONGLONG *delay, int *numrb, int *maxnumrb, uchar *jobId, BOOL *sentFirstTime )
{
	EnterCriticalSection( &_useInboxQueueLock);
	InboxFileArray *l = _fStart;
	if ( l != NULL )
	{
		if ( fn != NULL )
			strcpy( fn, l->FileName );
		if ( delay != NULL )
			*delay = l->TimeWhenProcess;
		if ( numrb != NULL )
			*numrb = l->numOfRebr;
		if ( maxnumrb != NULL )
			*maxnumrb = l->maxNumOfRebr;
		if ( jobId != NULL )
			*jobId = l->jobId;
		if ( sentFirstTime != NULL )
			*sentFirstTime = l->firstTime ;
	}
	LeaveCriticalSection( &_useInboxQueueLock);
	return l != NULL ;
}


//---------------------------------------------------------------------------------------
// Inbox thread for sending single file.
//---------------------------------------------------------------------------------------

// return the file size in kilobytes
static float getSendFileSize( char *fullfn, HANDLE hFile )
{
	union {
		struct {
			DWORD loSize, hiSize;
		};
		DWORDLONG Size;
	} size;
	size.loSize		 = GetFileSize( hFile, &(size.hiSize) );
	//size.Size		+= 2*(strlen(fullfn)+1+sizeof(long)+sizeof(time_t));
	return (LONGLONG)size.Size/1024.f;
}

// compute time of next day at 00:00:00:00 in LARGE_INTEGER format needed for SetWaitableTimer
static void nextDay( LARGE_INTEGER &li )
{
	SYSTEMTIME st;
	FILETIME ftL, ftU;

	CTime ct = CTime::GetCurrentTime();
	ct += CTimeSpan( 1, 0, 0, 0 );
	struct tm* t = ct.GetLocalTm( NULL );

	st.wYear		 = t->tm_year+1900;
	st.wMonth		 = t->tm_mon+1;
	st.wDay			 = t->tm_mday;
	st.wHour		 = 0;
	st.wMinute		 = 0;
	st.wSecond		 = 0;
	st.wMilliseconds = 0;

	SystemTimeToFileTime( &st, &ftL );
	LocalFileTimeToFileTime( &ftL, &ftU );
	li.LowPart = ftU.dwLowDateTime;
	li.HighPart = ftU.dwHighDateTime;
}

// send a file specified by the first item of the fsQueue (file sending queue)
void Inbox::sendFile()
{
	char fn[_MAX_PATH];
	LONGLONG delay;
	int      numrebr, maxnumrebr;
	uchar	 jobId;
	HANDLE   hFile = INVALID_HANDLE_VALUE;
	DWORD    exc=0 ;
	BOOL	 sentFirstTime ;

	InterlockedIncrement( &threadCounter ) ;
	__try
	{
		// get the filename of the first item
		if( !fsQueue.firstItem( fn, &delay, &numrebr, &maxnumrebr, &jobId, &sentFirstTime) )
			goto labelEnd;
		// construct full file name
		//strcpy(fullfn,directory);
		strcat(fullFileName,fn);

		// open file for read; delete on close for last rebroadcast
		#ifdef READ_FILE_OVERLAPPED
			hFile = CreateFile( fullFileName, GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING,  FILE_FLAG_OVERLAPPED, NULL );
		#else
			hFile = CreateFile( fullFileName, GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING,  0, NULL );
		#endif
		
		if( hFile==INVALID_HANDLE_VALUE )
		{
			// Could not open file, set new delay time and increment numOfFails.
			// This failure is not qualified as the Inbox failure.
			DWORD err = GetLastError() ;
			fsQueue.modify( fn, NULL, 10000*(LONGLONG)rebroadcastDelay(), -1, 0, FALSE );
			if( fsQueue.incSendFails( fn) >= maxSendFails )
			{
				InboxMsgSendFailed msg( fn, err );
				msgFun( EMsg_InboxSendFailed, &msg );
				fsQueue.del( fn );		// delete file from queue
			}
			goto labelEnd;
		}
		// the file could not change because it is not opened for shared write
		// but here can be an unprocessed event, which indicates modification of this file
		// after send process the file exist but is modified and was sent without request

		float    sendSize = sentFirstTime ? getSendFileSize( fn, hFile) : 0 ;
		LARGE_INTEGER li;
		nextDay( li );
		if ( nextDayTime.QuadPart < li.QuadPart )
			numVolume	= 0;
		if ( ( maxVolume >= 0 ) && ( numVolume + sendSize > maxVolume ) )
		{
			// Volume exceeded, report critical error.
			// Wait for next day or maxVolume number change or killEvent.
			setStatusBit( VolumeExc );
			msgFun( EMsg_InboxVolumeExceeded, NULL );

			HANDLE hArray[3];
			hArray[0] = _hKillEvent;
			hArray[1] = CreateWaitableTimer( NULL, TRUE, NULL );
			hArray[2] = hVolumeChanged;

			__try
			{	
				// wait for next day or until volume limit changed (or inbox killed)
				for( BOOL ok=FALSE ; !ok ; )
				{
					SetWaitableTimer( hArray[1], &li, 0, NULL, NULL, FALSE );
					DWORD res = WaitForMultipleObjects( 3, hArray, FALSE, INFINITE );
					if ( res == WAIT_OBJECT_0 )			// kill event
					{
						CloseHandle( hArray[1] );
						goto labelEnd;
					}
					if ( res == WAIT_OBJECT_0+2 )		// volume changed
						if ( maxVolume >= 0 )
							ok = (numVolume+sendSize <= maxVolume);
						else
							ok = TRUE;
					else								// next day
					{
						numVolume = 0;
						ok = (numVolume+sendSize <= maxVolume);
						nextDay( li );
					}
				}
				delStatusBit( VolumeExc);
			}
 			CATCH_EXCEPTION_CODE
			{
  				exc = EXCEPTION_CODE ;
			}
			CloseHandle( hArray[1] );
			if( exc != 0 )
				goto labelEnd ;
		}

		InboxMsgSendStarted msgStart( fn, numrebr, maxnumrebr ) ;
		msgFun( EMsg_InboxSendStarted, &msgStart );

		exc = _fileSender->sendFile( fullFileName, fn, hFile, _setup.channel, _setup.streamFormat,
			 _setup.fecLevel, numrebr, maxnumrebr, channelPID(), GlobalUserID(), jobId ) ;
		if ( exc )
		{
			char buf[512];
			const char *msg = DvbEventText( exc, buf );
			MfxPostMessage( EMsg_DvbError, _setup.channel, msg );
		}
		else
		{
			InboxMsgSendCompleted msgEnd( fn, numrebr, maxnumrebr, exc ) ;
			msgFun( EMsg_InboxSendCompleted, &msgEnd );
		}

		CloseHandle( hFile );
		hFile = INVALID_HANDLE_VALUE ;

		//InboxMsgSendCompleted msgEnd( fn, numrebr, maxnumrebr, exc ) ;
		//msgFun( EMsg_InboxSendCompleted, &msgEnd );
		if( exc == 0 )
		{
			if ( sendSize > 0 )
			{
				nextDay( nextDayTime );
				if ( nextDayTime.QuadPart > li.QuadPart )
					numVolume = 0;
				numVolume += sendSize;

				char buf1[30], buf2[124];
				sprintf( buf1, "VolumeForChannel_%d", _setup.channel );
				sprintf( buf2, "%I64d %f", nextDayTime.QuadPart, numVolume );

				MfxSetChannelContextString( buf1, buf2 );
			}

			if( fsQueue.setSendCompleted( fn) ) 	// this is the last rebroadcast
			{	// clear read-only atribute
				DWORD atr = GetFileAttributes( fullFileName ) ;
				if( atr & FILE_ATTRIBUTE_READONLY )
					SetFileAttributes( fullFileName, atr & ~FILE_ATTRIBUTE_READONLY ) ;
				DeleteFile( fullFileName );			//delete file
				fsQueue.del( fn );
			}
			else	// update the time to the next broadcasting
				fsQueue.modify( fn, NULL, 10000*(LONGLONG)rebroadcastDelay(), -1, jobId, FALSE );
		}
		else
		{	// could not send file, set new delay time and increment numOfFails
			fsQueue.modify( fn, NULL, 10000*(LONGLONG)rebroadcastDelay(), -1, jobId, FALSE );
			if( fsQueue.incSendFails( fn) >= maxSendFails )
			{
				fsQueue.del( fn );		// delete file from queue
			}
		}
	}
	CATCH_EXCEPTION_CODE
	{
  		exc = EXCEPTION_CODE ;
	}

  labelEnd:
	if ( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile );
	if( exc != 0 )
		setError( "Error during file send.", exc ) ;
	fullFileName[dirNameLength] = 0;
	InterlockedDecrement( &threadCounter ) ;
}

// the file sending thread function
// 1. - set the state to sending
// 2. - send the file
// 3. - clear the sending state
static DWORD WINAPI threadFncSendFile( void *param )
{
	((Inbox *)param)->setSending() ;
	((Inbox *)param)->sendFile();
	((Inbox *)param)->clearSending() ;
	return 0;
}

//---------------------------------------------------------------------------------------
//	thread watching directory (FA)
//---------------------------------------------------------------------------------------

// test when the file in faQueue (files which were modified by the user in a short time)
// is ready to be send - was not modified during a short time and is not opened for editation
void Inbox::sendRequest()
{ 
	char fullfn[_MAX_PATH], fn[_MAX_PATH];
	LONGLONG delay;
	int numrebr;
	// get the filename of the first item
	if( !faQueue.firstItem( fn, &delay, &numrebr) )
		return;
	
	strcpy(fullfn,directory);
	strcat(fullfn,fn);
	HANDLE hFile = INVALID_HANDLE_VALUE;
	__try
	{
		hFile = CreateFile( fullfn, GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 0, NULL );
		if ( hFile==INVALID_HANDLE_VALUE )
			faQueue.modify( fn, NULL, 10000*(LONGLONG)requestDelay(), -1, FALSE );
		else 
		{
			detailedLog(RequestToSend,fullfn);
			int file_size = GetFileSize( hFile, NULL ) ;
			file_size /= 1024 ;			// [K]
			CloseHandle( hFile );
			hFile = INVALID_HANDLE_VALUE ;
			//to send the file for processing
			//strcpy( fileName, fn );

			int n_rebr = numrb ;
			if( _setup.fecLevel  &&  _setup.fecRebrSize > file_size )
				n_rebr *= 2 ;
			fsQueue.add( fn, 0, n_rebr );		// add this file to the queue with files waiting for processing
			setStatusBit( PendingRequests ) ;	// set the status indicating that we have data to send

			faQueue.del( fn );					// remove file from modification queue
		}
	}
	CATCH_EXCEPTION_CODE
	{
		DWORD exc = EXCEPTION_CODE ;
		if ( hFile != INVALID_HANDLE_VALUE )
			CloseHandle( hFile );
		throw exc;
	}
}


inline DWORD Inbox::_wait( HANDLE *HandleArray, HANDLE h_timer, LARGE_INTEGER &howLong )
{
	HandleArray[2] = h_timer;
	SetWaitableTimer( h_timer, &howLong, 0, NULL, NULL, TRUE );
	return WaitForMultipleObjects( 3, HandleArray, FALSE, INFINITE );	// wait with fa timer
}

// main Inbox thread
// - watches the specified directory with subdirectories
// - if file is added to the dir or was modified or was deleted, modifies the queues
// - takes care to send files in specified time intervall
void Inbox::_dirWatchingThread( void * )
{
	if( hasStatusBit(StopDueToError) )
		return;
	InterlockedIncrement( &threadCounter ) ;

	OVERLAPPED Overlapped;
	DWORD  bytesRet;
	HANDLE HandleArray[4];
	HANDLE hPacketThread = NULL;

	__try
	{
		Overlapped.hEvent = hDirectoryEvent;

		HandleArray[0] = _hKillEvent;
		HandleArray[1] = hDirectoryEvent;

		DWORD	result;
		char	lpBuffer[MAX_BUFFER];
		char	ch[_MAX_PATH];
		FILE_NOTIFY_INFORMATION *info;
		fileName = ch;

		ReadDirectoryChangesW( hDir, lpBuffer, MAX_BUFFER, TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
			&bytesRet, &Overlapped, NULL );

		BOOL faTimer, onlyThread;
		LARGE_INTEGER dueFA, dueFS;
		do
		{
			//----------------------------------------
			// 1. Wait until important event happens
			//
			if ( hPacketThread==NULL )
			{
				// Wait until one of these events happens:
				//	- kill
				//	- directory change
				//	- first item in either of the queues
				onlyThread = FALSE;
				if( faQueue.firstItem( 0, &dueFA.QuadPart) )
				{
					if( fsQueue.firstItem( 0, &dueFS.QuadPart) )
					{
						if ( dueFA.QuadPart < dueFS.QuadPart && fsQueue.numOfItems() < 255 )
						{	// if the FA queue's timeout is smaller than FS's and have space in FS to add next item
							faTimer = TRUE;
							result  = _wait( HandleArray, hTimer, dueFA ) ; 
						}
						else
						{
							faTimer = FALSE;
							result  = _wait( HandleArray, hTimerRb, dueFS ) ; 
						}
					}
					else
					{
						faTimer = TRUE;
						result  = _wait( HandleArray, hTimer, dueFA ) ; 
					}
				}
				else
				{
					if( fsQueue.firstItem( 0, &dueFS.QuadPart) )
					{
						faTimer = FALSE;
						result  = _wait( HandleArray, hTimerRb, dueFS ) ; 
					}
					else
					{
						result = WaitForMultipleObjects( 2, HandleArray, FALSE, INFINITE );//wait without timer
					}
				}
			}
			else
			{
				// Send in process
				// Wait until it completes or until next item in faQueue is ready.
				if( fsQueue.numOfItems() < 255  &&  faQueue.firstItem( 0, &dueFA.QuadPart) )
				{
					faTimer = TRUE;
					onlyThread = FALSE;
					HandleArray[2] = hTimer;
					SetWaitableTimer( hTimer, &dueFA, 0, NULL, NULL, TRUE );
					HandleArray[3] = hPacketThread;
					result = WaitForMultipleObjects( 4, HandleArray, FALSE, INFINITE );	// wait with fa timer and thread
				}
				else
				{
					// fsQueue is full; we wait only until send completes
					faTimer = FALSE;
					onlyThread = TRUE;
					HandleArray[2] = hPacketThread;
					result = WaitForMultipleObjects( 3, HandleArray, FALSE, INFINITE );	// wait with thread only
				}
			}

			//----------------------------------------
			// 2. Handle the event
			//
			switch ( result ) 
			{
				case WAIT_OBJECT_0 : //end process
					break;
				case WAIT_OBJECT_0+1 : //changes in directory
					info = (FILE_NOTIFY_INFORMATION *)lpBuffer;
					while( 1 )
					{
						WCHAR fn[MAX_BUFFER];
						memcpy( fn, info->FileName, info->FileNameLength );
						fn[ info->FileNameLength/sizeof(WCHAR) ] = 0;
						USES_CONVERSION;
						char *fn2 = W2A( fn );
						strcpy( fileName, fn2 );
						switch ( info->Action )
						{
							case FILE_ACTION_ADDED :
								if ( !isDirectory( fileName, TRUE ) && fileExisting( fileName, TRUE ) )
								{
									detailedLog(NewFile,fileName);
									faQueue.add( fileName, 10000*(LONGLONG)requestDelay(), (int)numrb );
								}
 								break;
							case FILE_ACTION_REMOVED :
								detailedLog(DeletedFile,fileName);

								fsQueue.del( fileName );
								if( fsQueue.isEmpty() )				// to send message to progress log
									delStatusBit( PendingRequests );
								faQueue.del( fileName );
								break;
							case FILE_ACTION_MODIFIED :
								if ( !isDirectory( fileName, TRUE ) && fileExisting( fileName, TRUE ) )
								{
									detailedLog(ChangedFile,fileName);
									fsQueue.del( fileName );
									if( fsQueue.isEmpty() )				// to send message to progress log
										delStatusBit( PendingRequests );
									faQueue.modify( fileName, NULL, 10000*(LONGLONG)requestDelay(), (int)numrb, 0 );
								}
								break;
							case FILE_ACTION_RENAMED_OLD_NAME :
								detailedLog(ChangedFile,fileName);
								fsQueue.del( fileName );
								if( fsQueue.isEmpty() )				// to send message to progress log
									delStatusBit( PendingRequests );
								{
									char fn[_MAX_PATH];
									info = (FILE_NOTIFY_INFORMATION *)( (int)info+info->NextEntryOffset );

									WCHAR ffnn[MAX_BUFFER];
									memcpy( ffnn, info->FileName, info->FileNameLength );
									ffnn[ info->FileNameLength/sizeof(WCHAR) ] = 0;
									USES_CONVERSION;
									char *ffnn2 = W2A( ffnn );
									strcpy( fn, ffnn2 );
									if ( !isDirectory( fn, TRUE ) && fileExisting( fn, TRUE ) )
										faQueue.modify( fileName, fn, 10000*(LONGLONG)requestDelay(), (int)numrb, 0, TRUE );
								}
								break;
						}
						if( info->NextEntryOffset ==0 )
							break ;
						info = (FILE_NOTIFY_INFORMATION *)((int)info+info->NextEntryOffset);
					}
					ReadDirectoryChangesW( hDir, lpBuffer, MAX_BUFFER, TRUE,
						FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
						&bytesRet, &Overlapped, NULL );
					break;
				case WAIT_OBJECT_0+2 : //timeout
					if ( onlyThread )
					{
						CloseHandle( hPacketThread );
						hPacketThread  = NULL;
						if( fsQueue.isEmpty() )			// to send message to progress log
						{
							_fileSender->zeroSpeed() ;
							delStatusBit( PendingRequests );
						}
					}
					else
					{
						if ( faTimer )					// fa timeout
							sendRequest();
						else							// fs timeout
						{
							DWORD dwthrID;
							hPacketThread  = CreateThread( NULL, 0, threadFncSendFile, this, 0, &dwthrID );
						}
					}
					break;
				case WAIT_OBJECT_0+3 : //thread
					CloseHandle( hPacketThread );
					hPacketThread  = NULL;
					if( fsQueue.isEmpty() )			// to send message to progress log
					{
						_fileSender->zeroSpeed() ;
						delStatusBit( PendingRequests );
					}
					break;
			}
		}
		while ( result!=WAIT_OBJECT_0 );		// Quit on Kill event
	}
	CATCH_EXCEPTION_CODE
	{
  		DWORD exc = EXCEPTION_CODE ;
		setError( "Unknown error in Inbox.", exc ) ; 
	}
	if ( hPacketThread != NULL )
	{
		WaitForSingleObject( hPacketThread, INFINITE );
		CloseHandle( hPacketThread );
	}
	InterlockedDecrement( &threadCounter ) ;
}

static DWORD WINAPI dirWatchingThread( void *param )
{
	((Inbox *)param)->_dirWatchingThread( NULL );
	return 0;
}


//---------------------------------------------------------------------------------------
//	Inbox handles
//---------------------------------------------------------------------------------------

// zeroes all handles
inline void Inbox::zeroHandles()
{
	hDirectoryEvent			=NULL ;
	hVolumeChanged			=NULL ;
	hTimer					=NULL ;
	hTimerRb				=NULL ;
	hDir					=INVALID_HANDLE_VALUE ;
}

// create all handles, on error return win32 error code else return 0
DWORD Inbox::makeHandles()
{
	hDirectoryEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( hDirectoryEvent==NULL )
		goto labelError ;

	hVolumeChanged = CreateEvent( NULL, FALSE, FALSE, NULL );
	if ( hVolumeChanged==NULL )
		goto labelError ;

	hTimer = CreateWaitableTimer( NULL, TRUE, NULL );
	if ( hTimer==NULL )
		goto labelError ;

	hTimerRb = CreateWaitableTimer( NULL, TRUE, NULL );
	if ( hTimerRb==NULL )
		goto labelError ;

	hDir = CreateFile( 
		directory, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL 
	);
	if ( hDir==INVALID_HANDLE_VALUE )
		goto labelError ;
	return 0 ;

  labelError:
	return GetLastError() ;
}

// close all created handles
void Inbox::closeHandles()
{
	if ( hDirectoryEvent		!=NULL )
		CloseHandle( hDirectoryEvent );
	if ( hVolumeChanged			!=NULL )
		CloseHandle( hVolumeChanged );
	if ( hTimer					!=NULL )
		CloseHandle( hTimer );
	if ( hTimerRb				!=NULL )
		CloseHandle( hTimerRb );
	if ( hDir					!=INVALID_HANDLE_VALUE )
		CloseHandle( hDir );

	zeroHandles() ;
}


//---------------------------------------------------------------------------------------
//	constructor/destructor
//---------------------------------------------------------------------------------------

// initialize Inbox variables
// - creates handles
// - sets and creates inbox directory
// - read configuration settings (volume from prev. run and prev. run's time)
Inbox::Inbox( Mux *mux, const MuxChannelSetup *s, BOOL online ) :
	MuxChannel(MuxChannel::FileType,mux,s,online),  faQueue( "REQ" ),  fsQueue( "SND" )
{
	InitializeCriticalSection( &_useInboxLock );
	zeroHandles() ;

	strncpy( name, s->name, sizeof(name) ) ;
	name[sizeof(name)-1] = 0 ;

	threadCounter= 0;
	maxSendFails = 5;	// read value from config file
	numVolume	 = 0;	// read value from config file

	hThread		 = NULL ;

	strcpy( directory, s->inboxDir);
    int   len  = strlen( directory ) ;
    if( directory[ --len] != '\\'  &&  directory[len] != '/' )
		strcat( directory, "\\" );
	if( makeDir(directory) )
		setFatalError( "Inbox: Can't create inbox directory.", GetLastError() ) ; 
	dirNameLength = strlen( directory );
	strcpy( fullFileName, directory );

	reset( s ) ;

	DWORD err = makeHandles() ;
	if( err != 0 )
	{
		close() ;
		setFatalError( "Inbox: Can't create process handles.", err ) ; 
		throw err ;
	}

	char buf1[30];
	sprintf( buf1, "VolumeForChannel_%d", _setup.channel );

	LARGE_INTEGER li;
	nextDay( li );

	const char *buf2 = MfxGetChannelContextString( buf1 );
	
	if( buf2 == NULL  ||  sscanf( buf2, "%I64d %f", &(nextDayTime.QuadPart), &numVolume ) != 2 )
	{
		numVolume			 = 0;
		nextDayTime.QuadPart = 0;
	}
	else
	{
		if ( nextDayTime.QuadPart < li.QuadPart )
		{
			numVolume			 = 0;
			nextDayTime.QuadPart = 0;
		}
	}
}

// set the inbox directory to the new directory
void Inbox::setInboxDirectory( const char *newDir )
{
	strcpy( directory, newDir );
	int   len  = strlen( directory ) ;
	if( directory[ --len] != '\\'  &&  directory[len] != '/' )
		strcat( directory, "\\" );
	if( makeDir(directory) )
		setFatalError( "Inbox: Can't create inbox directory.", GetLastError() ) ; 
	dirNameLength = strlen( directory );
	strcpy( fullFileName, directory );
	dirNameLength = strlen( directory );
	MuxChannel::setInboxDirectory( newDir );
}

void Inbox::close()
{
	closeHandles();
	DeleteCriticalSection( &_useInboxLock );
}

// reset the inbox parameters as #rebr., rebr. delay, etc.
void Inbox::reset( const MuxChannelSetup *s )
{
	InterlockedExchange( &reqDelay   , s->fileSendDelay * 1000 ) ;
	InterlockedExchange( &rebrDelay  , s->rebroadcastDelay>0 ? s->rebroadcastDelay*1000 : -1 ) ;
	InterlockedExchange( &numrb      , s->numRebroadcasts ) ;
	InterlockedExchange( &maxVolume  , s->volumeLimitPerDay ) ;
	PulseEvent( hVolumeChanged );
	MuxChannel::reset( s ) ;
}


//---------------------------------------------------------------------------------------
//	start
//---------------------------------------------------------------------------------------

// TRUE iff specified file name specifies a directory
// if completize is TRUE fn is expanded to the full file name (with inbox directory)
BOOL Inbox::isDirectory	( char *fn, BOOL completize )	// exc
{
	DWORD attr;
	if ( completize )
	{
		char fullfn[_MAX_PATH];
		strcpy(fullfn,directory);
		fullfn[dirNameLength] = 0;
		strcat( fullfn, fn );
		attr = GetFileAttributes( fullfn );
		fullfn[dirNameLength] = 0;
	}
	else
		attr = GetFileAttributes( fn );
	if ( attr == 0xFFFFFFFF )	// error
		return TRUE;
	return ( attr & FILE_ATTRIBUTE_DIRECTORY);
}

// TRUE iff specified file exist
BOOL Inbox::fileExisting( char *fn, BOOL completize )	// exc
{
	BOOL res;
	if ( completize )
	{
		char fullfn[_MAX_PATH];
		strcpy(fullfn,directory);
		fullfn[dirNameLength] = 0;
		strcat( fullfn, fn );
		res = fileExist( fullfn );
		fullfn[dirNameLength] = 0;
	}
	else
		res = fileExist( fn );
	return res;
}

// search the specified directory and all subdirectories for files
// and add files to the faQueue
void Inbox::searchDirTree ( char *dir )
{	// recurently search for files in directories
	char fullfn[_MAX_PATH];
	strcpy( fullfn, directory);
	fullfn[dirNameLength] = 0;
	strcat( fullfn, dir );
	int fnoffset = strlen( fullfn );
	strcat( fullfn, "*" );

	WIN32_FIND_DATA finddata;
	HANDLE			hFindFile = FindFirstFile( fullfn, &finddata );
	BOOL			next = ( hFindFile!=INVALID_HANDLE_VALUE );
	__try
	{
		while ( next )
		{
			fullfn[ fnoffset ] = 0;	// delete characters after directory name
			if ( finddata.cFileName[0] != '.' )
			{
				strcat( fullfn, finddata.cFileName );
				strcpy( finddata.cFileName, fullfn+dirNameLength ); // copy the local path
				if ( !isDirectory( fullfn ) )
				{
					faQueue.add( finddata.cFileName, 10000*(LONGLONG)requestDelay(), (int)numrb );
					//setStatusBit( PendingRequests ) ;
				}
				else
				{
					strcat( finddata.cFileName, "\\" );
					searchDirTree( finddata.cFileName );
				}
			}
			next = FindNextFile( hFindFile, &finddata );
		}
	}
	CATCH_EXCEPTION_CODE
	{
		DWORD exc = EXCEPTION_CODE ;
		if ( hFindFile!=INVALID_HANDLE_VALUE )
			FindClose( hFindFile );
		throw exc;
	}
	if ( hFindFile!=INVALID_HANDLE_VALUE )
		FindClose( hFindFile );
}

// start the inbox directory watching and file sending
BOOL Inbox::start( )
{
	if( isStarted() )
		return TRUE ;
	if( !isOnline() )
		return FALSE ;

	MuxChannel::start() ;
	const char *errMsg = "Inbox start failed." ;
	__try
	{
		create() ;
		if ( hThread == NULL )
			return FALSE;
		//setStatusBit( Started ) ;
		ResumeThread( hThread );
		msgFun( EMsg_InboxStart, NULL );
		setStatusBit( Started ) ;
		//_isStarted = TRUE ;
	}
	CATCH_EXCEPTION_CODE
	{	// fatal error
		DWORD exc = EXCEPTION_CODE ;
		stop();
		setFatalError( errMsg, exc ) ; 
	}
	//return _isStarted;
	return isStarted();
}

// only create threads in suspended mode
BOOL Inbox::create( )
{
	BOOL sendExistingFiles=TRUE ;

	if( isStarted() )
		return TRUE ;
	if( !isOnline() )
		return FALSE ;
	EnterCriticalSection( &_useInboxLock);

	const char *errMsg = "Inbox creation failed." ;
	__try
	{
		stop();
		if( threadCounter != 0 )
		{
			setFatalError( errMsg, DvbErr_UnknownError ) ; 
			LeaveCriticalSection( &_useInboxLock);
			return FALSE ;
		}

		DWORD dwthrID;
		hThread = CreateThread( NULL, 0, dirWatchingThread, this, CREATE_SUSPENDED, &dwthrID );
		if ( hThread==NULL )
		{
			setFatalError( errMsg, GetLastError() ) ; 
		}
		else
		{
			//if ( sendExisting )
				searchDirTree( "" );
		}
	}
	CATCH_EXCEPTION_CODE
	{	// fatal error
		DWORD exc = EXCEPTION_CODE ;
		stop();
		setFatalError( errMsg, exc ) ; 
		LeaveCriticalSection( &_useInboxLock);
		return FALSE ;
	}
	LeaveCriticalSection( &_useInboxLock);
	return TRUE ;
}


//---------------------------------------------------------------------------------------
//	stop
//---------------------------------------------------------------------------------------


// stop() may fail because of the dead lock; therefore handles are closed only if 
// thread counter shows zero (consequently another stop() may be needed)

// only stop threads, use for multistop
BOOL Inbox::initiateStop()
{
	EnterCriticalSection( &_useInboxLock);
	BOOL event_set ;
	if ( threadCounter > 0 )
	{
		event_set = TRUE ;
		SetEvent( _hKillEvent );
	}
	else
		event_set = FALSE ;
	//delStatusBit( Started | PendingRequests ) ;
	LeaveCriticalSection( &_useInboxLock);
	return event_set;
}

// stop and delete threads
BOOL Inbox::stop()
{
	EnterCriticalSection( &_useInboxLock);
	try
	{
		BOOL event_set ;
		if ( threadCounter > 0 )
		{
			event_set = TRUE ;
			SetEvent( _hKillEvent );
		}
		else
			event_set = FALSE ;
		while ( threadCounter > 0 )
		{
			// can fail in the deadlock (e.g. in the Windows message processing)
			if ( WaitForSingleObject( hThread, 500) == WAIT_FAILED )
				break;
		}

		if ( threadCounter == 0 )
		{
			if( hThread != NULL )
				CloseHandle( hThread );
			hThread = NULL;
		}
		if( isStarted() )
		{
			//_isStarted = FALSE ;
			msgFun( EMsg_InboxEnd, NULL );
		}
		ResetEvent( _hKillEvent );

		CloseHandle( hDir );
		hDir = CreateFile( 
			directory, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL 
		);
		//CloseHandle( hDirectoryEvent );
		//hDirectoryEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

		faQueue.delAll( );
		fsQueue.delAll( );
	}
	catch( ... )
	{
		TRACE( "\nUnhandled exception in Inbox::stop()" ) ;
	}
	delStatusBit( Started | PendingRequests | StopDueToError ) ;
	LeaveCriticalSection( &_useInboxLock);
	return threadCounter == 0 ;
}


//---------------------------------------------------------------------------------------
//	utils
//---------------------------------------------------------------------------------------

// convert messages sent by Inbox to the string
const char *Inbox::msgAsText( long code, void *param, char *buf )
{
	switch ( code )
	{	// standard messages
		case EMsg_InboxStart :
			sprintf( buf, "Start watching Inbox\n" );
			break;
		case EMsg_InboxEnd : 
			sprintf( buf, "End watching Inbox\n" );
			break;
		case EMsg_InboxSendStarted :
		{
			InboxMsgSendStarted *msg = (InboxMsgSendStarted*)param ;
			if ( msg->maxRebr == 0 )
				sprintf( buf, "(infinite)%s Sending...", msg->filename ) ;
			else
				sprintf( buf, "(%d/%d)%s Sending...", msg->maxRebr - msg->timesToSendYet + 1, msg->maxRebr, msg->filename ) ;
			break;
		}
		case EMsg_InboxSendCompleted :
		{
			InboxMsgSendCompleted *msg = (InboxMsgSendCompleted*)param ;
			char txt[256] ;
			if ( msg->maxRebr == 0 )
				sprintf( buf, "(infinite)%s Send completed... %s",
					msg->filename, DvbEventText(msg->error,txt)  ) ;
			else
				sprintf( buf, "(%d/%d)%s Send completed... %s",
					msg->maxRebr - msg->timesToSendYet + 1, msg->maxRebr, msg->filename, DvbEventText(msg->error,txt)  ) ;
			break;
		}
		case EMsg_InboxSendFailed :
		{
			char txt[256] ;
			InboxMsgSendFailed *msg = (InboxMsgSendFailed*)param ;
			sprintf( buf, "%s; send failed - %s",
				msg->filename, DvbEventText(msg->error,txt) );
			break;
		}
		case EMsg_InboxVolumeExceeded :
			sprintf( buf, "Volume exceeded, waiting for tomorrow." );
			break;
		//case EMsg_InboxSendWaiting :
		//	sprintf( buf, "Waiting for free packets." );
		//	break;
		default : 
			buf[0] = 0 ;
	}

	return buf ;
}
