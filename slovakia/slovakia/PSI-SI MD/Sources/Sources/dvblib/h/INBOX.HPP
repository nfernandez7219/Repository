
#ifndef INBOX_HPP
#define INBOX_HPP

#ifndef __INC_FILEIO_HPP__
	#include "fileio.hpp"
#endif

class Mux ;

struct InboxFileArray
{
	char			*FileName;
	LONGLONG		 TimeWhenProcess;
	int				 numOfRebr;
	int				 maxNumOfRebr;
	int				 numOfFails;		// number of failed rebroadcasts
	uchar			 jobId;
	BOOL			 firstTime;			// sending first time
	InboxFileArray  *next;
};


// Array of requests sorted acc. to the member TimeWhenProcess
class InboxQueue
{
	CRITICAL_SECTION _useInboxQueueLock;					// lock queue to use
	char			 _name[4] ;
	InboxFileArray	*_fStart ;
	uint			 _numberInQueue;

	InboxFileArray	*_findWhereToPut( LONGLONG time );
	InboxFileArray	*_search( char *fn );

  public:
	InboxQueue( const char *nam ) ;
   ~InboxQueue() ;

	inline const char *name		( )		{ return _name ; }
	inline BOOL		 isEmpty	( )		{ return _fStart == NULL ; }
	inline uint		 numOfItems ( )		{ return _numberInQueue  ; }

	void			 add		(
						char *fn,					// filename to send
						LONGLONG delay,				// delay (since now) when processing should start
						int numrb					// # of rebroadcasts; 0=infinite broadcast
					 );
	void			 modify		(
						char *fnold,				// old request name
						char *fnnew,				// new request name or NULL
						LONGLONG delay,				// time period by which processing time is shifted
						int numrb,					// # of rebroadcasts; 0=infinite broadcast
						uchar jobId,				// id passed to sendFile()
						BOOL createIfNotExist=TRUE	// TRUE: Create new item if fnold is not in the queue
					 );
	void			 del		( char *fn );
	void			 delAll		( );

	long			 incSendFails( char *fn );		// = new value of # of send fails
	BOOL			 setSendCompleted( char *fn ) ;	// = TRUE iff it was the last rebroadcast

	// Information about first item (the one which should be processed)
	BOOL			 firstItem	( char *fn=0, LONGLONG *timeWhenProcess=0, int *numrb=0, int *maxnumrb=0, uchar *jobId=0, BOOL *sentFirstTime=0 );
} ;

#pragma pack(4)
class Inbox : public MuxChannel
{
	friend DWORD WINAPI threadFncSendFile( void *param ) ;
	friend DWORD WINAPI dirWatchingThread( void *param ) ;

  private :
	// setup
	long			  reqDelay;					// [ms] interval to accept new file after it appears in the directory
	long			  rebrDelay;				// [ms] delay between rebroadcasts
	long			  numrb;					// num rebroadcasts for each file
	float			  numVolume;				// [Kb] sent today
	long			  maxVolume;				// max [Kb]/day
	LARGE_INTEGER	  nextDayTime;
	long			  maxSendFails;				// max number of fails to send a file
	char			  directory[_MAX_PATH] ;	// inbox directory
	char			  fullFileName[_MAX_PATH] ;	// inbox file's full file name prefix
	int				  dirNameLength ;			// length of directory name
	//BOOL			  sendExisting;				// send existing files when opening inbox
	char			  name[32];					// channel name

	// signals
	HANDLE			  hDir;						//
	HANDLE			  hDirectoryEvent;			//
	HANDLE			  hTimer;					//
	HANDLE			  hThread;					//
	HANDLE			  hTimerRb;					//
	HANDLE			  hVolumeChanged;			// set by setMaxVolume()

	// thread control
	long			  threadCounter ;			// #of threads launched from inbox (0-3)
	void			 _dirWatchingThread(void*);	// thread to watch the directory

	// queues
	InboxQueue		  faQueue;					// queue of requests
	InboxQueue		  fsQueue;					// queue of the files to send

	// other vars
	CRITICAL_SECTION _useInboxLock;				// only single call to start and stop allowed
	char			 *fileName;					//

	// utils
	void			  sendRequest	();			// send file from watch list to send list
	void			  sendFile		();			// send file from send list to the net
	inline void		  zeroHandles	();
	DWORD			  makeHandles	();
	void			  closeHandles	();
	BOOL			  isDirectory	( char *fn, BOOL completize = FALSE );
	BOOL			  fileExisting	( char *fn, BOOL completize = FALSE );
	void			  searchDirTree ( char *dir );
	void			  close			() ;

	inline long		  requestDelay		()				{ return reqDelay ;}
	inline long		  rebroadcastDelay	()				{ return rebrDelay ;}
	inline long		  numRebroadcasts	()				{ return numrb ;}
	inline DWORD	  _wait				( HANDLE *HandleArray, HANDLE h_timer, LARGE_INTEGER &howLong ) ;

  public:
	//------------------------------------------------------------------------------
	//
	// Inbox watches for changes in the inbox directory and sends each new file
	// to the multiplexor in the form of a packet stream.
	// Inbox has limited capacity for sending - expressed as # of packets allowed to be sent.
	// This capacity has to be periodically incremented (incNumFreePackets()).
	//
	// Inbox is initially created as inactive, i.e. it must be started via (start()).
	// To receive messages about Inbox activity, override msgFun().
	//
	// stop() fails if some of the inbox threads could not be terminated in time (e.g. due to
	// deadlock); in such a case another stop() later should help (done automatically by start())
	//
	Inbox( Mux *mux, const MuxChannelSetup *s, BOOL online=TRUE ) ;		// throw DWORD
   ~Inbox()											{ initiateStop() ; stop(); close() ; }
	//virtual void msgFun				( long code, void *param );
	const char  *msgAsText			( long code, void *param, char *buf ) ;

	virtual BOOL start				();		// start threads
	virtual BOOL create				();		// create threads
	virtual BOOL initiateStop		();		// stop threads
	virtual BOOL stop				();		// delete threads
	virtual void reset				( const MuxChannelSetup *s ) ;
	//------------------------------------------------------------------------------
	virtual	void	setInboxDirectory( const char *newDir );
};
#pragma pack()

#endif