#ifndef MSGLOG_H
#define MSGLOG_H


class Z_USERDLL LogMem
{
  public :
    char *mem ;
    int   memBytes, memPos ;

  public :
    LogMem( int maxBytes = 5000 ) ;
   ~LogMem( ) ;
    int getMaxSize( void )   { return memBytes ; } ;

    char   *_printf ( char *fmt, ... ) ;
    char   *_vprintf( char *fmt, ArgList args ) ;
    char   *_strdup ( char *str ) ;
    void    _realloc( int size ) ;
    void   *_getMem ( int size ) ;
} ;

class Z_USERDLL MsgLevel
{
  public :
	enum Severity {		// acc. to Win32 error scheme
		Log		= 0,
		Info	= 1,
		Warning	= 2,
		Error	= 3,
	} ;
	enum Level {
		Detailed= 0,
		Standard= 1,
		Brief	= 2,
		NoInfo	= 3,
	} ;
	enum Group {
		GrpFile		= 0x01,
		GrpDbs		= 0x02,
		GrpCom		= 0x04,
		GrpDat		= 0x08,
		GrpWin32	= 0x10,
		GrpUnknown	= 0xFFFFFFFF,
	} ;
    int   level,
		  severity ,
          group ;

    MsgLevel( int lvl=Detailed, int sev=Log,int grp=GrpUnknown ) :
			level(lvl), severity(sev), group(grp) { } ;

	inline BOOL operator>=( const MsgLevel &l )
	{
		if( (group & l.group) == 0 )
			return FALSE ;
		return  level>=l.level  &&  severity>=l.severity ;
	}
} ;


class Z_USERDLL Msg : public MsgLevel
{
  private :
    //LogDevClass *Devices[ 10] ;
    int         _shortLen, _longLen ;
    //int          ignoredLogger ;
    int          setAttribs     ( char *fmt ) ;
    void        _appendLongText ( char *s ) ;
    void        _insertLongText ( char *s ) ;
    void        _appendShortText( char *s ) ;
    void        _insertShortText( char *s ) ;

  public :
    void  reset( ) ;                 // except filter & code
    int   code         ,
	      helpTopic    ,
		  helpSubTopic ;
    char  *shortString  ,
	      *longString  ,
          *errString  ;               // NULL or message about error at msg syntax analysis
    Msg( int cod, int lvl=0, int sev=0, int grp=0xFFFF ) ;
    Msg( int cod, const char *fmt, ... ) ;      
    Msg( const Msg &msg ) ;
    Msg &operator=( const Msg &msg ) ;
   ~Msg( ) ;

    int   setString      ( char *fmt, ... ) ;  // setAttribs + setShort/LongString; -> 0/EOF
    void  setShortString ( char *fmt, ... ) ;
    void  setLongString  ( char *fmt, ... ) ;
    int   vSetString     ( char *fmt, ArgList args ) ;
    void  vSetShortString( char *fmt, ArgList args ) ;
    void  vSetLongString ( char *fmt, ArgList args ) ;
    Msg& operator << ( int    arg ) { return  *this << (long)arg ; }
    Msg& operator >> ( int    arg ) { return  *this >> (long)arg ; }
    Msg& operator <  ( int    arg ) { return  *this <  (long)arg ; }
    Msg& operator >  ( int    arg ) { return  *this >  (long)arg ; }
    Msg& operator << ( long   arg ) { char buf[40] ; return  *this << ltoa(arg,buf,10) ; }
    Msg& operator >> ( long   arg ) { char buf[40] ; return  *this >> ltoa(arg,buf,10) ; }
    Msg& operator <  ( long   arg ) { char buf[40] ; return  *this <  ltoa(arg,buf,10) ; }
    Msg& operator >  ( long   arg ) { char buf[40] ; return  *this >  ltoa(arg,buf,10) ; }
    Msg& operator << ( double arg ) ;
    Msg& operator >> ( double arg ) ;
    Msg& operator <  ( double arg ) ;
    Msg& operator >  ( double arg ) ;
    Msg& operator << ( char*  arg ) { _insertLongText ( arg ) ;  return  *this ; }
    Msg& operator >> ( char*  arg ) { _appendLongText ( arg ) ;  return  *this ; }
    Msg& operator <  ( char*  arg ) { _insertShortText( arg ) ;  return  *this ; }
    Msg& operator >  ( char*  arg ) { _appendShortText( arg ) ;  return  *this ; }
    Msg& operator << ( Msg *  arg ) { return  *this < arg ; }
    Msg& operator >> ( Msg *  arg ) { return  *this > arg ; }
    Msg& operator <  ( Msg *  arg ) ;
    Msg& operator >  ( Msg *  arg ) ;
} ;


class Z_USERDLL ScrMsg : public Msg
{
  public:
    ScrMsg( int err_code, char *arg ) ;
    ScrMsg( int err_code, ... ) ;
} ;


#endif
