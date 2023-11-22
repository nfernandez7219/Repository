#ifndef INC_SCREEN_H

#define INC_SCREEN_H

#ifndef Z_USERDLL
	#define Z_USERDLL
#endif

#ifndef _INC_IO
#include <io.h>
#endif
#ifndef _INC_STDIO
#include <stdio.h>
#endif

#include "slist.h"

typedef unsigned long	ulong  ;
typedef unsigned int	uint   ;
typedef unsigned char	uchar  ;
typedef long int        lint   ;
typedef unsigned short  ushort ;
typedef unsigned char   BYTE   ;

#ifdef _MSC_VER
#define   _USERENTRY    _cdecl
#endif

long Z_USERDLL screenLibVersion( ) ;

class ConfigClass ;

#ifdef _MSC_VER
#define __size(x) ((sizeof(x)+sizeof(int)-1) & ~(sizeof(int)-1))
#define va_arglist(parmN) ((va_list)((char *)(&parmN)+__size(parmN)))
#else
#define va_arglist(parmN) ((va_list)((char _FAR *)(&parmN)+__size(parmN)))
#endif

class Z_USERDLL ArgList
{
  public :
    va_list args ;
    inline ArgList( va_list par )  { args = par ; } ;
    operator va_list() { return args ; }
} ;



/*** standard dialogs ***/

int  Z_USERDLL stdMessage      ( char *title, int style, char *fmt, ... ) ;
void Z_USERDLL stdOkDialog     ( char *fmt,...) ;
void Z_USERDLL stdInfoDialog   ( char *fmt,...) ;
void Z_USERDLL stdWarningDialog( char *fmt,...) ;
void Z_USERDLL stdErrorDialog  ( char *fmt,...) ;
int  Z_USERDLL askOkDialog     ( char *fmt,...) ;
int  Z_USERDLL askYesDialog    ( char *fmt,...) ;
int  Z_USERDLL askYesNoCancel  ( char *title/*=appName*/, const char *fmt,... ) ;	// -> IDYES, IDNO, IDCANCEL
void Z_USERDLL stdErrorReport( int err_code,...) ;
void Z_USERDLL stdErrorReport( int err_code, char *arg ) ;


/*** string utils ***/

char* Z_USERDLL stristr        ( const char *string, const char *subString ) ;
void  Z_USERDLL cstringToString( const char *Cstring, char *string ) ;
int   Z_USERDLL stringToCstring( const char *string, char *Cstring ) ;		// -> 0-iff strings equal
int   Z_USERDLL isStrEmpty     ( const char *s ) ;
int   Z_USERDLL textEqual      ( const char *s1, const char *s2 ) ;
char  Z_USERDLL *concat        ( char *dst,... ) ;
int   Z_USERDLL isMatch        ( const char *name, const char *mask ) ;      // -> success; */? wildcards in mask allowed

int  Z_USERDLL  isFloat        ( const char *str, float  *x=NULL ) ;
int  Z_USERDLL  isInt          ( const char *str, int    *x=NULL ) ;
int  Z_USERDLL  isDouble       ( const char *str, double *x=NULL ) ;
int  Z_USERDLL  isLong         ( const char *str, long   *x=NULL ) ;

int  Z_USERDLL  stringToStringList( const char *string , char ***stringlist , int *n_strings=NULL , char *separator=NULL ) ;
void Z_USERDLL  freeStringList    ( const char **names, int n_names=-1 ) ;   // -1 for NULL-terminated list
void Z_USERDLL  sortStringList    ( const char **names, int n_names=-1 ) ;
char Z_USERDLL  **dupStringList   ( const char **names, int n_names=-1 ) ;

int  Z_USERDLL  binaryToString ( void *bin, char *buf,int msize ) ;
int  Z_USERDLL  stringToBinary ( void *bin, char *buf ) ;


/*** file utils ***/

// converts full path to the remote path (incl. machine name)
BOOL   Z_USERDLL getRemotePath   ( const char *rootPath, char *remotePath, int maxRemotePathChars ) ;

int    Z_USERDLL delFile         ( const char *filename ) ;		// -> err
inline BOOL Z_USERDLL fileExist  ( const char *path )  { return !access( path, 0) ; }

// if createPath, then dst-path is created if needed; ->err
int    Z_USERDLL copyFile        ( const char *src, const char *dst, BOOL createPath=FALSE) ;

// copy file into *.bak (if bak file already exist the name is decorated by "~1"...)
// bakFile = name of the bak file used
BOOL   Z_USERDLL makeBakFile     ( const char *srcFile, const char *bakExt=0, char *bakFile=0 ) ;

long   Z_USERDLL fileSize        ( const char *filename ) ;
time_t Z_USERDLL fileTime        ( const char *filename ) ;
time_t Z_USERDLL setFileTime     ( const char *filename, time_t t ) ;	// -> time really set (NT: must be even)

BOOL   Z_USERDLL dirExist        ( const char *path ) ;
BOOL   Z_USERDLL isDirEmpty      ( const char *dir ) ;
int    Z_USERDLL delDir          ( const char *dir ) ;		// empty only; ->err (0=OK)
int    Z_USERDLL makeDir         ( const char *dir ) ;		// recursive; -> 0-OK, ERR_MAKEDIR
BOOL   Z_USERDLL rmWholeDir      ( const char *pathname) ;	// any directory; recursive
const char		*getFileDir		 ( const char *fileName, char *buf ) ;

BOOL   Z_USERDLL copyDir         ( const char *srcDir, const char *dstDir, BOOL delSrcDir = FALSE );
BOOL   Z_USERDLL renameDir       ( const char *srcDir, const char *dstDir, BOOL moveIfDstExist = FALSE );

BOOL clearReadonlyFileAttribute( const char *fileName ) ;

// case insensitive!
class Z_USERDLL sStringPtrArray : public sTemplateArray<const char*>
{
	void _addStringList( const sStringPtrArray *src ) ;
  public:
	sStringPtrArray ( ) ;
	sStringPtrArray ( const sStringPtrArray &src ) ;
	void  addStrings( const char **names, int n_names=0 ) ;
	inline  sStringPtrArray & operator= ( const sStringPtrArray &src )
	{
		clearList( ) ;
		_addStringList( (sStringPtrArray *)&src ) ;
		return *this ;
	}
	inline  sStringPtrArray & operator<<( const sStringPtrArray &src )
	{
		_addStringList( (sStringPtrArray *)&src ) ;
		return *this ;
	}
	inline  sStringPtrArray & operator<<( const char **names )
	{
		addStrings( names ) ;
		return *this ;
	}
	inline  sStringPtrArray & operator<<( const char *name )
	{
		add( name) ;
		return  *this ;
	}
	int operator== (  const sStringPtrArray &src ) const;

	// returns bin + binarySize()
	char * saveToBin ( char *bin, BOOL checked=FALSE ) ;
	size_t binarySize( ) ;
	BOOL loadFromBin ( char *bin, BOOL clearOldContents=TRUE ) ;
    void clearList   ( )
	{
		sTemplateArray<const char*>::clearList( ) ;
		sTemplateArray<const char*>::add( NULL ) ;
	}

	inline void add( const char *name )
	{
		last() = (const char*)STRDUP(name) ;
		sTemplateArray<const char*>::add( NULL ) ;
	}
    inline int  count( ) const
	{
		return  sTemplateArray<const char*>::count() - 1 ;
	}
	inline void sort( )
	{
		if( count() > 0 )
			sortStringList( (const char**)*this, count() ) ;
	}
	int find( const char *str ) const ;				// via stricmp()
	int findEqualText( const char *str ) const ;	// via textEqual()
	operator const char **( ) const
	{
		return _first ;
	}
	BOOL addNewOnly( const char *str ) ;			// via textEqual()
	int  addNewOnly( const sStringPtrArray &src );	// -> num added strings

	void asText  ( char *buf, int max_chars=0 ) const ;
	void fromText( const char *buf ) ;
} ;


#define A_FILES			1		// CreateFileList() or sFileNameArray mode
#define A_DIRS			2		//
#define A_PROPERDIRS	6		// A_DIRS except "." and ".."

// No exception!
class Z_USERDLL sFileNameArray : public sStringPtrArray
{
	int  _dirLen ;
	char _dirName[1024] ;
  public:
	sFileNameArray( const char *dirName, const char *mask=NULL, int mode=A_FILES, int (*filter)( const char* name, long lParam)=NULL, long lParam=0 ) ;
	const char *fullName( int j ) ;
} ;


long   Z_USERDLL getDiskFreeSpace( char diskName=0 ) ;

// -> 0, ERR_NOSUCHDIR
int Z_USERDLL createFileList( const char *path, const char *mask, int mode,
			  int (*filter)( const char *, long lParam), char ***files=NULL, int *n_files=NULL, long lParam=0 ) ;		// -> err

#define sDRIVE_REMOVABLE  1
#define sDRIVE_FIXED      2
#define sDRIVE_REMOTE     4
#define sDRIVE_CDROM      8
#define sDRIVE_RAMDISK    16
int  Z_USERDLL getDriveList( char ***drives, int *n_drives=0, uint type=sDRIVE_FIXED | sDRIVE_REMOTE ) ;
void Z_USERDLL destroyDriveList( char **drives ) ;

// removes readonly attributes
// - mode specifies the type of access requested for the file;
//   for available settings see help on fopen
// - share specifies the share attributes for the file;
//   can be any of sFILESHARE_READ, sFILESHARE_WRITE, sFILESHARE_READWRITE or sFILESHARE_NOSHARE
// - err_code (if not NULL) = 0, ERR_OPENFILE, ERR_NOSUCHFILE
#define sFILESHARE_NOSHARE		0
#define sFILESHARE_READ			1
#define sFILESHARE_WRITE		2
#define sFILESHARE_READWRITE	3
FILE Z_USERDLL *fopenRetry( const char *filename, const char *mode, uint share=sFILESHARE_NOSHARE, int *err_code=NULL ) ;

#define	FULL_VERSION		1		// version format "a.b.c.d"
#define UPPER_VERSION		2		// version format "a.b"
// get file version for file filename
// version must be enough to hold version number (max.24 chars)
// version info is in format "a.b.c.d" retrived from version info block
// PRODUCTVERSION variable
BOOL Z_USERDLL getFileVersion( char *filename, char *version, int type = FULL_VERSION );

/*** other utils ***/

ulong       Z_USERDLL crc32		( const uchar *buf, uint len, ulong seed=0 ) ;
ushort      Z_USERDLL crc16     ( char* buf, int n, ushort seed=0 ) ;
void        Z_USERDLL crcEncrypt( char *buf, int n_chars ) ;
inline void Z_USERDLL crcDecrypt( char *buf, int n_chars )  { crcEncrypt( buf, n_chars ) ; }

#define  PLATFORM_UNKNOWN    0
#define  PLATFORM_WINNT      1
#define  PLATFORM_WIN95      2
#define  PLATFORM_OS2        3
void    Z_USERDLL getSystemVersion( int *major , int *minor , int *platform ) ;

char *  Z_USERDLL getFullExePath  ( const char *exeFile, char *path ) ;	// -> path if exe found; otherwise NULL
void    Z_USERDLL removeTabs      ( char *lin, int tabSize ) ;

int     Z_USERDLL existDll( char *filename ) ;
HMODULE Z_USERDLL openDLL ( const char *DLLname, char *errBuf ) ;


/*** string tables ***/

#define declStringTable(       name)  StringTable Z_USERDLL  name##StringTable =     StringTable( #name, (float)(name##MSG_VERSION))
#define declStringTablePtr(    name)  StringTable Z_USERDLL *name##StringTable = new StringTable( #name, (float)(name##MSG_VERSION))
#define destroyStringTablePtr( name)  delete name##StringTable

class Z_USERDLL StringTable
{
	float ver ;                 // current version
	float minVersion ;          // min. allowed version
	char  lang[ 12] ;
	char  _baseFilename[8] ;
	char  *_filename ;
    char  type ;                // '#'-local, '$'-system
	char  _changed ;
	char  **msgs ;
	int   n_msgs ;
    int   _offset ;
    char  **_languages ;
    int   n_languages ;
    int  _load( char typeChar, char *lang, char *new_filename=NULL ) ;
    StringTable& _makeCopy( const StringTable &t ) ;

	static char **longLanguages   ;
	static int    n_longLanguages ;
	static char **shortLanguages  ;
	static int    n_shortLanguages;

  public :
	 BOOL   isSystemTable() const	{ return type == '$' ; }
     void   release( ) ;
     StringTable( char *baseFileName=NULL, float minVersion=0 ) ;
     inline StringTable( const StringTable &t )            { _makeCopy( t ) ; }
     inline StringTable& operator=( const StringTable &t ) { release() ; return _makeCopy( t ) ; }
    ~StringTable( ) ;

	 // If sayLoadingDefault==TRUE then in case default English string table is loaded
	 // instead of required one, warning dialog opens.
	 // On failure Msg type exception is thrown.
     void  load            ( char *lang=NULL, char *new_filename=NULL, BOOL sayLoadingDefault=TRUE ) ;

     int   loadSystemTable ( char *lang=NULL, char *new_filename=NULL ) ;

     inline float  version    ( )  { return  ver  ; }
     inline char  *language   ( )  { return  lang ; }
     inline char  *text( int ind)  { ind -= _offset ; return  ind < n_msgs ? msgs[ind] : "?" ; }
     inline char **messages   ( )  { return    msgs  ; }
     inline int    numMessages( )  { return  n_msgs  ; }
     inline int    isLoaded   ( )  { return  n_msgs  ; }
     inline int    offset     ( )  { return _offset  ; }
     inline char  *filename   ( )  { return _filename; }

	 // Languages available for this string table
     char		 **languages   () ;
     int		   numLanguages() ;

	 // Languages defined in the configuration file ([languages] section)
	 static int    loadLanguages      ( ConfigClass *cfg  ) ;	// -> # languages
     static char  *shortToLongLanguage( const char *shortL) ;	// "e" -> "English"
     static char  *longToShortLanguage( const char *longL ) ;	// "English" -> "e"
	 static void   destroyLanguages	  ( ) ;

	 void  set ( int ind, const char *new_text ) ;
	 void  save( ) ;
	 BOOL  isChanged() const			{ return _changed ; }
} ;


typedef long int JULIANDAY ;


struct DATETIME2
{
  private:
	void addDay() ;
	void subDay() ;
  public:
	short          year ;
	unsigned short month;
	unsigned short day  ;
	unsigned short hour ;
	unsigned short min  ;
	unsigned short sec  ;
	unsigned long  fraction;
	inline DATETIME2( short y=0, ushort mon=1, ushort d=1, ushort h=0, ushort m=0, ushort s=0 )
		{ year=y ; month=mon ; day=d ; hour=h ; min=m ; sec=s ; }

	inline int operator== ( const DATETIME2 &t ) const  { return year==t.year && month==t.month && day==t.day && hour==t.hour && sec==t.sec ; }
	inline int operator!= ( const DATETIME2 &t ) const  { return !(*this == t) ; } 
		   int operator<  ( const DATETIME2 &t ) const ; 
		   int operator<= ( const DATETIME2 &t ) const ; 
	inline int operator>  ( const DATETIME2 &t ) const  { return !(*this <= t) ; }
	inline int operator>= ( const DATETIME2 &t ) const  { return !(*this <  t) ; }

	inline DATETIME2& operator++( )     { addDay() ; return *this ; } // Prefix increment
	inline DATETIME2& operator++( int ) { addDay() ; return *this ; } // Postfix increment
	inline DATETIME2& operator--( )     { subDay() ; return *this ; } // Prefix decrement
	inline DATETIME2& operator--( int ) { subDay() ; return *this ; } // Postfix decrement

		   BOOL isValid			 ( ) ;
		   int  dayOfWeek		 ( ) ;
		   int  weekOfYear		 ( ) ;
		   int  isSunday		 ( ) ;
		   int  isSutterday		 ( ) ;
	inline int  isLeapYear		 ( )		{ return isLeapYear(year); }
		   int  isHoliday		 ( ) ;
	inline void clearTime		 ( ) { hour = 0 ; min = 0 ; sec = 0 ; }
		   int  numMonthDays	 ( int month , int year = 0 ) ;     // 0 - bad month
		   int  dayOfYear		 ( ) ;

	static JULIANDAY  ymdToLong  ( int  year, int  month, int  day ) ;
	static void       longToYmd  ( int *year, int *month, int *day, JULIANDAY ldate ) ;
	static int        isLeapYear ( int year ) ;
	static inline int isValidDate( int year, int month, int day )  { return ymdToLong(year,month,day) >= 0 ; }
	static int        dayOfYear  ( int year, int month, int day ) ;
	static int		  dayOfYear  ( JULIANDAY day ) ;
} ;


void Z_USERDLL delay(ULONG milisecs ) ;

BOOL MESSAGEBEEP( UINT uType ) ;
#define BEEP_WARNING   MESSAGEBEEP(MB_ICONHAND)
#define BEEP_NOTE      MESSAGEBEEP(MB_ICONASTERISK)
#define BEEP_ERROR     MESSAGEBEEP(MB_ICONEXCLAMATION)

#define	STD_ERRORDIALOGSTYLE      MB_ICONERROR
#define	STD_WARNDIALOGSTYLE       MB_ICONWARNING
#define	STD_INFODIALOGSTYLE       MB_ICONINFORMATION
#define	STD_OKDIALOGSTYLE         MB_OK
#define	STD_YESNODIALOGSTYLE     (MB_YESNO | MB_ICONQUESTION)
#define	STD_OKCANCELDIALOGSTYLE  (MB_OKCANCEL | MB_ICONINFORMATION)

#ifndef __min
#define __min(a,b)  ( ( (a) <= (b) ) ? (a) : (b) )
#endif
#ifndef __max
#define __max(a,b)  ( ( (a) >= (b) ) ? (a) : (b) )
#endif


#endif
