#ifndef	__INC_LOGGER_HPP__
#define	__INC_LOGGER_HPP__

#include <afxcview.h>

struct LogEventStruct
{
	ushort	m_channelID		 ;
	time_t	m_timeNow		 ;
	DWORD	m_errorCode		 ;
	char	m_channelName[32];
	char	m_state[32]		 ;
	CString	m_string		 ;
	CString	m_channelIDStr	 ;
	char	m_timeStr[20]	 ;

	LogEventStruct( ushort channelID, LPCTSTR status, LPCTSTR str, time_t *setTime );
} ;

struct ProgressStruct;
class ListBase : public CListView
{
  protected:
	virtual BOOL PreCreateWindow	( CREATESTRUCT &cs );
	virtual void OnInitialUpdate	( )			{ CListView::OnInitialUpdate( ); initView(); }
	virtual void createColumns		( )			{};

	DECLARE_DYNCREATE( ListBase )

  public:
	void initView			( );

	long getItem			( int  row	 );
	int  getRow				( long value );
	BOOL addItem			( long lParam);
	BOOL deleteItem			( long value );
	void deleteAllItems		();
	BOOL refreshItem		( long value );
	void refreshAllItems	();
};


//
// Interface to the event list box and event log file.
//
class LogEventList : public ListBase
{
	void  _testLogFileCapacityLimit	( ) ;
	void  _saveFirstToLogFile		( ) ;
	void  _makeBakLog				( const char *newFileName=0 ) ;
	void  _saveOneItem				( LogEventStruct *log ) ;

  protected:
	CString		 logFile	  ;
	int			 logFile_capacity;
	int			 numInLogFile;
	int			 numInLogWindow;
	FILE		*file;
	BOOL		 savedContext;

	afx_msg void OnGetDispInfo		( NMHDR* pnmh, LRESULT* pResult);
	afx_msg void OnColumnClick		( NMHDR* pnmh, LRESULT* pResult);
	afx_msg void OnDestroy			( );

			char *getText			( int col, long item );  
	virtual void createColumns		( );

	DECLARE_DYNCREATE	( LogEventList )
	DECLARE_MESSAGE_MAP	()

  public:
	// Add item to the start of the list control + append it to the log file
	BOOL addLogerItem				( LogEventStruct *str	);
	BOOL addLogerItem				( ushort channelID, CString status, const char *stri, time_t *setTime );

	// Add item to the log file only (list box unchanged)
	void saveToLogFile				( LogEventStruct *log );
	void saveToLogFile				( ushort channelID, CString status, const char *stri, time_t *setTime );

	// delete from the list box (logfile unchanged)
	// Unlike add operations items are being deleted from the end.
	BOOL deleteLogerItem			( long lParam );	// delete item with specified lParam
	void deleteNItems				( int num );		// delete last num items
	void deleteAllItems				();

	// Copy current logfile into BAK file (*.old) and open new empty logfile.
	void makeBakLog					()						{ makeBakLog() ; }

	// Logfile specification
	// maxItemsInLogfile=0 means unlimited capacity
	virtual void defineLogFile		( const char *fileName, int maxItemsInLogfile=0 );
	void changeLogFile				( const char *fileName, int maxItemsInLogfile=0 );

	// Logfile info
	const CString &getLogFile		()						{ return logFile; };
	int  numOfItemsInLogFile		()						{ return numInLogFile; }
	int  logFileCapacity			()						{ return logFile_capacity ; }

	// Clear both listbox and logfile
	void clearLog					() ;

	LogEventList();
   ~LogEventList();
};


//
// Interface to the error list box and error log file.
// It is basically LogEventList class with modified setup (different columns and log file).
//
class ErrorList: public LogEventList
{
  protected:
	afx_msg void OnGetDispInfo		( NMHDR* pnmh, LRESULT* pResult);
	afx_msg void OnColumnClick		( NMHDR* pnmh, LRESULT* pResult);

			char *getErrorText		( int col, long item );  
	virtual void createColumns		( );
    
	DECLARE_DYNCREATE	( ErrorList )
	DECLARE_MESSAGE_MAP	()

  public:
	virtual void defineLogFile		( const char *fileName, int numItems );
	~ErrorList();
};

#endif
