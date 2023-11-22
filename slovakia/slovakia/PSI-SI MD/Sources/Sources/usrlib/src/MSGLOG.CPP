#include "tools2.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _MSC_VER
#pragma  hdrstop ("winnt\obj\screen.pch")
#else
#pragma  hdrstop
#endif

#include "screen.msg"

#define  SEV            "SEV"
#define  GRP            "GRP"
#define  LVL            "LVL"
#define  DEV            "DEV"
#define  ALL            "ALL"
#define  HLP            "HLP"

#define SEV_DBG         0
#define SEV_INF         1
#define SEV_WARN        2
#define SEV_ERR         3
#define SEV_FATAL       4

#define MAX_MSG_CHARS   1000

static char **cfgGroups, **cfgLevels, **cfgSeverities ;

static LogMem memory( 50000 ) ;

//#endif

#define MSG_NULLFILTER     MsgLevel( 0, 0, 0xFFFF )

//------------------------------------------------------------------------------
//      memory class
//------------------------------------------------------------------------------


LogMem :: LogMem( int maxBytes )    // exception should be thrown if unsuccessfull
{

    memBytes = maxBytes ;
	// don't use CALLOC because memory is static variable
    mem      = ( char *)calloc( memBytes+1, 1) ;
    if( mem == NULL )
        throw -1 ;
    memPos   = 0 ;
}

LogMem::~LogMem( )
{
	if( mem )
	// don't use FREE because memory is static variable
		free( mem ) ;
}


char *LogMem :: _vprintf( char *fmt, ArgList args )
{
    if( (memPos + MAX_MSG_CHARS) >= memBytes )  memPos = 0 ;

    char *dst = mem + memPos ;
    int lenght = (vsprintf( dst, fmt, args.args ) + 1) ;

    memPos += lenght ;
    return  dst ;
}


void *LogMem :: _getMem( int size )
{
    if( (memPos + size) >= memBytes )  memPos = 0 ;
    size = __min( size, memBytes ) ;

    char *dst = mem + memPos ;
    memPos += size ;
    return  dst ;
}


char *LogMem :: _printf( char *fmt, ... )
{
	va_list args ;
	va_start( args, fmt ) ;
    return  _vprintf( fmt, args ) ;
	va_end( args ) ;
}


char *LogMem :: _strdup( char *str )
{
    if( !str )  return  NULL ;
    int length = strlen( str) + 1 ;
    if( (memPos + length) >= memBytes )
    {
        memPos = 0 ;
        length = __min( length, memBytes ) ;
    }

    char *dst = mem + memPos ;
    strncpy( dst, str, length ) ;
    dst[length] = 0 ;

    memPos += length ;
    return  dst ;
}


void LogMem :: _realloc( int size )
{
    if( size > __min( memPos, 2*MAX_MSG_CHARS) )
		// don't use REALLOC because memory is static variable
        if( (mem = (char *)realloc( (char *)mem, size)) != NULL )
        {
            memBytes = size ;
            memPos   = 0 ;
        } ;
}



//------------------------------------------------------------------------------
//      StringTok
//------------------------------------------------------------------------------


class StringTok
{
    char *string ,
         *actual ,
         *_next  ;

  public:
    inline  StringTok(  char *dest )
            {actual = _next = string = STRDUP( dest ) ;} ;
    inline ~StringTok( )              { FREE( string );} ;
    inline  char   *getActual( void ) { return actual; } ;
    char   *next(    const char *sep="\n\t\r " ) ;
    char    nextSep( ) ;
} ;


char StringTok :: nextSep( )
{
    if( !_next )  return  0 ;
    char *p = _next ;
    p += strspn ( p, " \n\t\r" ) ;          // skip white chars

    p  = strpbrk( p, " \n\t\r,;=" ) ;
    if( !p )  return  0 ;

    p += strspn ( p, " \n\t\r" ) ;          // skip white chars
    return  *p ;
}

char *StringTok :: next( const char *sep )
{
    actual = NULL ;
    if( _next && *_next )
    {
        if( string == _next )       // first token - skip separators
            _next += strspn( _next, sep ) ;
        actual = _next   ;
        if( (_next=strpbrk( actual, sep)) != NULL )
        {
            char *end = _next ;
            _next += strspn( _next, sep ) ;
            *end = 0 ;
        }
    }
    return actual ;
}


//------------------------------------------------------------------------------
//      general functions
//------------------------------------------------------------------------------


// Analyze  "paramFilt = <type>",  where <type> = number or symb. constant from config:[logDeclare]
// Return: 0-OK, EOF
static int getAttribValue( int &value, char *paramFilt , char *type )
{
    if( !paramFilt || !type )  goto LabelError ;

    if( stricmp( type, ALL) == 0 )
    {
        if( toupper(*paramFilt) == 'G' )
            value = 0xFFFF ;
        else
            value = 0 ;
        return  0 ;
    }

    if( !isInt( type, &value) )
    {
        char  **name=NULL ;
        int   key = toupper( *paramFilt) ;
        switch( key )
        {
            case 'G': name = cfgGroups     ;  break ;
            case 'L': name = cfgLevels     ;  break ;
            case 'S': name = cfgSeverities ;  break ;
        }
        if( !name )
            goto LabelError ;           // section not exist
        int ind; 
        for( ind=0 ; *name != NULL  &&  stricmp( *name, type) ; name++ )
            ind++ ;
        if( !*name )
            goto LabelError ;           // unknown token

        value = (key == 'G') ?  (1 << ind) : ind ;
    }
    return  0 ;

  LabelError :
    //trace_fun( "\n *** error in loggers::getAttribValue( '%s' = '%s' ) *** ", paramFilt, type ) ;
    return  EOF ;
}


//------------------------------------------------------------------------------
//      Msg
//------------------------------------------------------------------------------


void Msg::reset( )
{
    helpTopic    = 0 ;
    helpSubTopic = 0 ;
    _shortLen    = 0 ;
    _longLen     = 0 ;
    longString   = shortString = errString = NULL ;
}


Msg :: Msg( int cod, int lvl, int sev, int grp ):MsgLevel( lvl, sev, grp )
{
	code = cod;
    reset( ) ;
}


Msg :: Msg( int cod, const char *fmt, ... ) : MSG_NULLFILTER
{
    char *fmt2=const_cast<char*>(fmt);
	code = cod ;
	va_list args ;
	va_start( args, fmt ) ;
    vSetString( fmt2, ArgList( args) ) ;
	va_end  ( args ) ;
}

Msg::Msg( const Msg &msg )
{
    level        = msg.level ;
    severity     = msg.severity ;
    group        = msg.group ;
    code         = msg.code ;
	helpTopic    = msg.helpTopic ;
	helpSubTopic = msg.helpSubTopic ;

    _shortLen    = 0 ;
    shortString  = memory._strdup( msg.shortString ) ;
    _longLen     = 0 ;
    longString   = memory._strdup( msg.longString ) ;
    errString    = memory._strdup( msg.errString ) ;
}

Msg &Msg::operator=( const Msg &msg )
{
    level        = msg.level ;
    severity     = msg.severity ;
    group        = msg.group ;
    code         = msg.code ;
	helpTopic    = msg.helpTopic ;
	helpSubTopic = msg.helpSubTopic ;

    _shortLen    = 0 ;
    shortString  = memory._strdup( msg.shortString ) ;
    _longLen     = 0 ;
    longString   = memory._strdup( msg.longString ) ;
    errString    = memory._strdup( msg.errString ) ;
    return  *this ;
}


Msg::~Msg( )
{
}


Msg& Msg::operator << ( double arg )
{
   char buff[40] ; sprintf( buff, "%.5g", arg ) ;  return  *this << buff ;
}

Msg& Msg::operator >> ( double arg )
{
    char buff[40] ; sprintf( buff, "%.5g", arg ) ;  return  *this >> buff ;
}

Msg& Msg::operator <  ( double arg )
{
    char buff[40] ; sprintf( buff, "%.5g", arg ) ;  return  *this <  buff ;
}

Msg& Msg::operator >  ( double arg )
{
   char buff[40] ;  sprintf( buff, "%.5g", arg ) ;  return  *this >  buff ;
}

Msg& Msg::operator >  ( Msg *msg )
{
   return   (*this > msg->shortString) >> msg->longString ;
}

Msg& Msg::operator <  ( Msg *msg )
{
   return   (*this < msg->shortString) << msg->longString ;
}



#define BLKSIZE 1000

void Msg::_insertShortText( char *s )
{
    if( !s  ||  !*s )  return ;
    int  len = strlen( s ) ;
    len = __min( len, BLKSIZE-_shortLen ) ;
    if( len <= 0 )  return ;

    if( !shortString  ||  !*shortString )
        shortString = memory._strdup( s ) ;
    else
    {
        if( _shortLen <= 0 )    // if short string was not expanded yet
        {
            char *block = (char*)memory._getMem( BLKSIZE ) ;
            _shortLen   = strlen( shortString) + 1 ;
            memmove( block+len, shortString, _shortLen ) ;
            shortString = block ;
        }
        else
            memmove( shortString+len, shortString, _shortLen ) ;
        memmove( shortString, s, len ) ;
        _shortLen += len ;
    }
}


void Msg::_appendShortText( char *s )
{
    if( !s  ||  !*s )  return ;
    int  len = strlen( s ) ;
    len = __min( len, BLKSIZE-_shortLen ) ;
    if( len <= 0 )  return ;

    if( !shortString  ||  !*shortString )
        shortString = memory._strdup( s ) ;
    else
    {
        if( _shortLen <= 0 )
        {
            char  *block = (char*)memory._getMem( BLKSIZE ) ;
            _shortLen    = strlen( shortString) + 1 ;
            memmove( block, shortString, _shortLen ) ;
            shortString  = block ;
        }
        memmove( shortString+_shortLen-1, s, len ) ;
        _shortLen += len ;
        shortString[_shortLen-1] = 0 ;
    }
}


void Msg::_insertLongText( char *s )
{
    if( !s  ||  !*s )
		return ;
    int  len = strlen( s ) ;
    len = __min( len, BLKSIZE-_longLen ) ;
    if( len <= 0 )
		return ;

    if( !longString  ||  !*longString )
        longString = memory._strdup( s ) ;
    else
    {
        if( _longLen <= 0 )
        {
            char *block = (char*)memory._getMem( BLKSIZE ) ;
            _longLen    = strlen( longString) + 1 ;
            memmove( block+len, longString, _longLen ) ;
            longString  = block ;
        }
        else
            memmove( longString+len, longString, _longLen ) ;
        memmove( longString, s, len ) ;
        _longLen += len ;
    }
}


void Msg::_appendLongText( char *s )
{
    if( !s  ||  !*s )
		return ;
    int  len = strlen( s ) ;
    len = __min( len, BLKSIZE-_longLen ) ;
    if( len <= 0 )
		return ;

    if( !longString  ||  !*longString )
        longString = memory._strdup( s ) ;
    else
    {
        if( _longLen <= 0 )
        {
            char  *block = (char*)memory._getMem( BLKSIZE ) ;
            _longLen     = strlen( longString) + 1 ;
            memmove( block, longString, _longLen ) ;
            longString   = (char *)block ;
        }
        memmove( longString+_longLen-1, s, len ) ;
        _longLen += len ;
        longString[_longLen-1] = 0 ;
    }
}


void Msg :: vSetLongString( char *fmt, ArgList args )
{
    _longLen   = 0 ;
    longString = memory._vprintf( fmt, args ) ;
}

void Msg :: setLongString( char *fmt, ... )
{
    _longLen   = 0 ;
	va_list args ;
	va_start( args, fmt ) ;
    longString = memory._vprintf( fmt, args ) ;
	va_end( args ) ;
    //longString = memory._vprintf( fmt, va_arglist( fmt ) ) ;
}

void Msg :: vSetShortString( char *fmt, ArgList args )
{
    _shortLen   = 0 ;
    shortString = memory._vprintf( fmt, args ) ;
}

void Msg :: setShortString( char *fmt, ... )
{
    _shortLen   = 0 ;
	va_list args ;
	va_start( args, fmt ) ;
    shortString = memory._vprintf( fmt, args ) ;
	va_end( args ) ;
    //shortString = memory._vprintf( fmt, va_arglist( fmt ) ) ;
}

int Msg :: setString( char *fmt, ... )
{
	va_list args ;
	va_start( args, fmt ) ;
    return  vSetString( fmt, ArgList(args) ) ;
    //ArgList args = ArgList( va_arglist( fmt) ) ;
    //return  vSetString( fmt, args ) ;
}


int Msg :: vSetString( char *fmt, ArgList args )
{
    char *pointer = (char*)fmt;
    int _error    = 0  ;

    reset( ) ;
    while( *fmt == ' ' ) fmt++ ;

    if( *fmt == '<' && (pointer = strchr( fmt, '>' )) != NULL )
    {
        (MsgLevel)(*this) = MSG_NULLFILTER ;
        _error = setAttribs( fmt+1 ) ;
    }

    if( !pointer || pointer == fmt)
        pointer = fmt ;
    else
        pointer++ ;

    vSetShortString( pointer, args ) ;
    longString = shortString ;
    while( (longString = strchr( longString, '|')) != NULL )
        if( *(longString+1) != '|' )
        {
            *longString = '\0';
             longString++ ;
             break ;
        }
        else
             longString+= 2 ;

    if( shortString )
        while( (pointer=strstr( shortString, "||" )) != NULL )
            *pointer = ' ' ;

    if( longString  )
        while( (pointer=strstr( longString, "||" )) != NULL )
            *pointer = ' ' ;
    else
        setLongString( "" ) ;

    return  _error ;
}


// reads format attribs into msg; return : 0/EOF
int Msg :: setAttribs( char *fmt )
{
    char      *command, separator ;
    StringTok  tok( fmt ) ;
    command  = strchr( tok.getActual(), '>' ) ;
    *command = '\0' ;

	int x ;
    while( (separator= tok.nextSep()) == '='  &&  (command = tok.next( "=\n\t\r ")) != NULL )
    {
        switch( toupper(*command) )
        {
            case 'L' :
                if( getAttribValue( x, command, tok.next()) == 0 )
				{
					level = x ;
					break ;
				}
                errString = memory._printf( "bad level" ) ;
                return  EOF ;

            case 'S' :
                if( getAttribValue( x, command, tok.next()) == 0 )
				{
					severity = x ;
					break ;
				}
                errString = memory._printf( "bad severity" ) ;
                return  EOF ;

            case 'G' :
                if( getAttribValue( x, command, tok.next()) == 0 )
				{
					group = x ;
					break ;
				}
                errString = memory._printf( "bad group" ) ;
                return  EOF ;

            case 'H' :
                separator = tok.nextSep( ) ;
                if( !tok.next(",\n\t\r ")  ||
                    !isInt( tok.getActual(), &helpTopic)  ||
                    ( separator == ','  &&  !isInt( tok.next(), &helpSubTopic) ) )
                {
                    errString = memory._printf( "bad help id" ) ;
                    return  EOF ;
                }
                break ;

            default  :
                errString = memory._printf( "unrecognized option: %s", command ) ;
                return EOF ;
        }
    }

    char *s ;
    if( (s = tok.next()) != NULL )
    {
        errString = memory._printf( "unrecognized option: %s", s ) ;
    }
    else
    if( separator )
    {
        errString = memory._printf( "bad separator" ) ;
    }
    else
        return 0 ;
    return  EOF ;
}
