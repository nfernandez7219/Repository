#ifndef	__INC_MFXGLOBALS_HPP__
#define	__INC_MFXGLOBALS_HPP__


//	Hardware Key
enum eProgramLevel
{
	ProgramLevel_Demo		 = 0,
	ProgramLevel_Basic		 = 1,		
	ProgramLevel_Advanced	 = 2
};

// Get HK program level 
eProgramLevel GetProgramLevel(
		char *versionName=0,		// "Demo", "Basic" ...
		float *fMaxSpeed=0,			// maximal transport speed in Mb/s
		BOOL bReadKey=FALSE			// function will re-read HW key rather than return cached values
	);

BOOL runAsDvbIpGateway ( ) ;

#pragma pack( 1)
class DvbGlobalId
{
    static ushort _cnt ;
	static ushort _initialized ;

  protected :
	time_t tim ;
	ushort cnt ;

  public:
	inline		   DvbGlobalId  ( )				{ create() ; }
			void   create		( ) ;

	inline  void   makeInvalid	( )				{ cnt = 0  ; }
	inline  BOOL   isValid		( ) const		{ return cnt != 0 ; }

	// definition of the operators ==, !=, <, >, <=, >=
	friend inline int operator== ( const DvbGlobalId &id1, const DvbGlobalId &id2 )
		{ return id1.tim == id2.tim  &&  id1.cnt == id2.cnt ; }
	friend inline int operator!= ( const DvbGlobalId &id1, const DvbGlobalId &id2 )
		{ return !(id1 == id2) ; } 
} ;
#pragma pack( )


// Dialog Exchange Functions
#define sDDX_FLAG_NOEMPTY			1
#define sDDX_FLAG_FILE				2
#define sDDX_FLAG_DIRECTORY			4
#define sDDX_FLAG_NOSPACE			8
#define sDDX_FLAG_NUMBER			16
#define sDDX_FLAG_FIRSTCHARUPPER	32

void sDDX_Int  ( CDataExchange* pDX, UINT id, int   &num, int minVal,  int maxVal );
void sDDX_Short( CDataExchange* pDX, UINT id, short &num, int minVal,  int maxVal );
void sDDX_Text ( CDataExchange* pDX, UINT id, char *txt,  int maxChar, int flag=sDDX_FLAG_NOEMPTY );
void sDDX_Path ( CDataExchange *pDX, UINT id, char **txt, int maxChar, int flag=sDDX_FLAG_NOEMPTY );
void ExchangeException( CDataExchange* pDX, UINT id, const char *str );

// Other
#define CATCH_AND_DISPLAY_EXCEPTION( x )	try{ x; }catch( Msg &msg ) { AfxMessageBox( msg.shortString, MB_TOPMOST ); }


// Global functions
void  MfxMakeFullAppPath	( CString &path );
char *MfxGetFullPath		( const char *relPath, char *path ) ;
BOOL  MfxOpenFileDialog		( CWnd *wnd,CString &path );
char *getAllocatedFullPath	( const char *path ) ;
int   getAppVersion			( char *versionString=0 ) ;


// Channel context string serves to store info about the volume of data transferred
// via channel particular channel within 1 day.
// (For the case the server run is interrupted.)
const char *MfxGetChannelContextString( const char *key ) ;
void        MfxSetChannelContextString( const char *key, const char *string ) ;

// Message handlers
struct EventQueue
{
	static CRITICAL_SECTION modifyEventQueueLock;

	friend BOOL MfxPostMessage( UINT msg, long wParam, long lParam ) ;
	friend BOOL MfxPostMessage( UINT msg, long wParam, const char *lParam ) ;

	static EventQueue *first;	// the oldest message in queue
	static EventQueue *last;	// the last added message

	long		wParam, lParam;
	UINT		msg;
	BOOL		isString;
	LONGLONG	tim;			// time when event occurs
	EventQueue *next;			// next message acc. to the time ( previous in the queue )

  private:
	EventQueue( UINT m, long wp, long lp ) ;

  public:
	~EventQueue()
	{
		if( isString )
			FREE( (char *)lParam );
	}
	time_t setTime() ;
};


EventQueue *MfxGetMsg() ;

extern BOOL bProcessMfxMessages;

inline void MfxSetMessageMode( BOOL bMsgMode=TRUE )
{
	bProcessMfxMessages = bMsgMode; 
}

inline BOOL MfxIsInMessageMode()
{
	return bProcessMfxMessages; 
}

BOOL MfxPostMessage( UINT msg, long wParam, const char *lParam ) ;
BOOL MfxPostMessage( UINT msg, long wParam = 0, long lParam = 0 ) ;

void MfxSetAssertHandling( BOOL bIgnoreWarnings=TRUE );

#endif
