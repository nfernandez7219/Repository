#ifndef INC_GENERAL_H
#define INC_GENERAL_H

#define RESTYPE_DEF     0
#define RESTYPE_CFG     1
#define RESTYPE_MSG     2
#define RESTYPE_BMP     3
#define RESTYPE_AVI     5
#define RESTYPE_HLP     6
#define RESTYPE_DOC     7


// system directory: GetWindowsDir()

class Z_USERDLL sLoginClass
{
  private:
    char _buf[128] ;
    char _bmpDir[80] , _sysBmpDir[40] ,
         _msgDir[80] , _sysMsgDir[40] ,
					   _sysTskDir[40] ,
         _cfgDir[80] , _sysCfgDir[40] ,
         _aviDir[80] , _sysAviDir[40] ,
         _hlpDir[80] , _sysHlpDir[40] ,
         _docDir[80] , _sysDocDir[40] ,
         _tmpDir[20] ;
	char _csDir[80] ;
    char _sysPath[80] ;
    char _defLanguage[4] ;
    int  _isTextApp ;		// N/A

    void readCommandArgs( ) ;
	void destroyCommandArgs( ) ;
    void expandResourceName( char *res, char *appDir, char *sysDir, char *lang ) ;

  public:
    int   argc ;					// program arguments
    char  **argv  ;
	char *basePath ;				// current working directory

	// program info (as passed to STD_PROLOG())
    char *prgName ;
    long  prgId ,
          prgVersion ;				// "102" -> version 1.02

    // login info
    char  usrName [ 12] ;
    char  language[4] ;

    void  setDefaultLanguage( char *l )  { strcpy( _defLanguage, l ) ; }
    char  *defaultLanguage  ( )          { return  _defLanguage ; }
    int   isDefaultLanguage ( )          { return  !stricmp( language, _defLanguage) ; }

	inline const char *getCsDir		( ) const  { return  _csDir  ; }		// c:\cs
	inline const char *getSysDir	( ) const  { return  _sysPath ; }		// c:\cs\mfc

    inline const char *getTmpDir    ( ) const  { return  _tmpDir ; }
    char			  *getTmpFile   ( char *buf=NULL ) ;

	// CS_DIR\apps\programs
    inline const char *getSysTskDir ( ) const  { return  _sysTskDir ; }
    char			  *getSysTskFile( const char *file, char *buf=NULL ) ;

	// local resources (appDir\...)
	// NULL buffer means shared class buffer is used
    inline const char *getBmpDir    ( ) const  { return  _bmpDir ; }
    inline const char *getMsgDir    ( ) const  { return  _msgDir ; }
    inline const char *getCfgDir    ( ) const  { return  _cfgDir ; }
    inline const char *getAviDir    ( ) const  { return  _aviDir ; }
    inline const char *getHlpDir    ( ) const  { return  _hlpDir ; }
    inline const char *getDocDir    ( ) const  { return  _docDir ; }
    char *getBmpFile   ( const char *file, char *buf=NULL ) ;
    char *getMsgFile   ( const char *file, char *buf=NULL ) ;
    char *getCfgFile   ( const char *file, char *buf=NULL ) ;
    char *getAviFile   ( const char *file, char *buf=NULL ) ;
    char *getHlpFile   ( const char *file, char *buf=NULL ) ;
    char *getDocFile   ( const char *file, char *buf=NULL ) ;

	// system resources (CS_DIR\mfc\...)
	// NULL buffer means shared class buffer is used
    inline const char *getSysBmpDir ( ) const  { return  _sysBmpDir ; }
    inline const char *getSysMsgDir ( ) const  { return  _sysMsgDir ; }
    inline const char *getSysCfgDir ( ) const  { return  _sysCfgDir ; }
    inline const char *getSysAviDir ( ) const  { return  _sysAviDir ; }
    inline const char *getSysHlpDir ( ) const  { return  _sysHlpDir ; }
    inline const char *getSysDocDir ( ) const  { return  _sysDocDir ; }
    char *getSysBmpFile( const char *file, char *buf=NULL ) ;
    char *getSysMsgFile( const char *file, char *buf=NULL ) ;
    char *getSysCfgFile( const char *file, char *buf=NULL ) ;
    char *getSysAviFile( const char *file, char *buf=NULL ) ;
    char *getSysHlpFile( const char *file, char *buf=NULL ) ;
    char *getSysDocFile( const char *file, char *buf=NULL ) ;

	// following metacharacters are replaced:
	// # ... by local resource directory
	// $ ... by system resource directory
	// @ ... by usrName
    const char *getResDir ( int resType=RESTYPE_DEF, int autoCreatePath=0 ) ;
    const char *getResFile( char *file, char *buf=NULL, int resType=RESTYPE_DEF, char *lang=NULL, int autoCreatePath=0, int appendLang=1 ) ;

    void init( const char *appName, long appId, long appVersion ) ;
	void close( ) ;
    sLoginClass( ) ;
   ~sLoginClass( )  { close() ; }
} ;


extern sLoginClass Z_USERDLL  login ;

sLoginClass Z_USERDLL *getGlbLogin( void ) ;

/*** stdProlog styles ***/
#define PRG_Cfg           0x000010L      // load alexis.cfg
#define PRG_CfgWarn       0x000020L
#define PRG_CfgIgnore     0x000030L
#define PRG_CfgMask       0x000030L
#define PRG_Scr           0x004000L      // load string tables scr.e, errors.e
#define PRG_ScrWarn       0x008000L
#define PRG_ScrIgnore     0x00C000L
#define PRG_ScrMask       0x00C000L
#define PRG_StdPath       0x040000L		// check that app is running from correct path
#define PRG_StdPathWarn   0x080000L
#define PRG_StdPathIgnore 0x0C0000L
#define PRG_StdPathMask   0x0C0000L

#define PRG_MINSUPPORT   (0)
#define PRG_STDSUPPORT   (PRG_Cfg | PRG_Scr)
#define PRG_FULLSUPPORT  (PRG_STDSUPPORT | PRG_StdPathWarn)

/*** stdEpilog styles ***/
#define PRG_CfgSave        0x0001
#define PRG_CfgAskSave     0x0003


// Depending on 'style' prolog:
//		- loads config
//		- initializes <login> global variable
//		- loads screen string tables
//		- checks if application is run from CS_DIR
// For PRG_MINSUPPORT screen.lib will work with english defaults.
int  Z_USERDLL STD_PROLOG( long style, const char *appName, long appId=0, long appVersion=0 ) ;
void Z_USERDLL STD_EPILOG( int style, int say_error ) ;
     
int  Z_USERDLL STD_TXTPROLOGFUN( long style, const char *appName, long appId=0, long appVersion=0 ) ;
void Z_USERDLL STD_TXTEPILOG( int  style, int say_error ) ;

#define STD_TXTPROLOG( style, appName, appId, appVersion )  \
 login.argv = argv ,                                        \
 login.argc = argc ,                                        \
 STD_TXTPROLOGFUN( style, appName, appId, appVersion )      

BOOL Z_USERDLL getRegistryString     ( HKEY hKeyFrom, const char *envKey, const char *envVar, char *retBuf, int retSize );
BOOL Z_USERDLL getLocalMachineString ( const char *envKey, const char *envVar, char *retBuf, int retSize );
BOOL Z_USERDLL getCurrentUserString  ( const char *envKey, const char *envVar, char *retBuf, int retSize );
const char * Z_USERDLL getCSDirectory( char *csDir );

#endif
