#include "tools2.hpp"

#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include "envX.hpp"
#include "errors.msg"
#include "screen.msg"

#define     ENV_SECTOKEN        1
#define     ENV_VARTOKEN        2
#define     ENV_COMMENTCHAR     '*'


static const char *getStr( int id )
{
	const char *s = SCRMSG(id) ;
	if( *s != '?' )
		return s ;
	// english defaults
	switch( id )
	{
		case SCRMSG_ReloadBadSect : s = "Reload of not existing section required" ; break ;
		case SCRMSG_ErrReadCfg :	s = "Error reading configuration file" ; break ;
	}
	return s ;
}


//-------------------------------------------------------------------------------------
//              constructors & destructors
//-------------------------------------------------------------------------------------


ConfigClass::ConfigClass( const char *filename )
{
    _err      = 0 ;
    _sayError = 0 ;
    _errLine  = 0 ;
	// don't use STRDUP because close() does not free this variable
    _filename = filename ? strdup( filename ) : NULL ;
    _message[0] = 0 ;
}

void ConfigClass::clearError()
{
    _err = 0 ;
    _message[0] = 0 ;
}

void ConfigClass::setFileName( const char *filename )
{
    if( _filename )
		free( _filename ) ;
    _filename = filename ? strdup( filename ) : NULL ;
}

ConfigClass::~ConfigClass()
{
    if( _filename )
		// don't use FREE because close() does not free this variable
		free( _filename ) ;
}


//-------------------------------------------------------------------------------------
//              OPEN
//-------------------------------------------------------------------------------------


int ConfigClass::getLineText( char *line, char **txt, int *cont )     // -> 0, ENV_TOKENERR
{
    int  ret = 0 ;
    char *s = line ;
    while( isspace( *s) )  s++ ;

    if( *s == '"' )                                 // "quoted text"
    {
        *txt = s + 1 ;
        while( (s = strchr( s+1, '"')) != NULL  &&  s[1] == '"' )
            memmove( s, s+1, strlen(s+1) ) ;        // double quotation mark

        if( !s )
            return  ENV_TOKENERR ;
        *s = ' ' ;
    }
    else
    if( *s == ENV_COMMENTCHAR )
	{
		*txt = s + strlen(s) ;
	}
    else
    {
        *txt = s ;
        while( *s > ' ' )  s++ ;
    }

    char *endtxt = s ;
    *cont = 0 ;

    while( isspace( *s) )  s++ ;
    if( *s == '\\' )                     // continuation mark
    {                                    // (must be last nonempty char)
        *cont = 1 ;
        while( s++, isspace(*s) ) ;
        ret = *s ? ENV_TOKENERR : 0 ;
    }
    else
    if( *s  &&  *s != ENV_COMMENTCHAR )
        ret = ENV_TOKENERR ;

    *endtxt = 0 ;
    return  ret ;
}


int ConfigClass::readLine( char *line, char **sect, char **var, char **txt, int *cont )
{                                   // -> 0, ENV_SECTOKEN, ENV_VARTOKEN, ENV_TOKENERR
    int  retval=0;
    char *s = line ;
    while( *s  &&  isspace(*s) )  s++ ;

    *cont = 0 ;
    switch( *s )
    {
        case 0 :
        case ENV_COMMENTCHAR :
            break ;
        case '[' :
            *sect = s + 1 ;
            if( (s = strchr( *sect, ']')) != NULL )
                *s = 0 ;
            return  s ? ENV_SECTOKEN : ENV_TOKENERR ;
		case '=' :
            return  ENV_TOKENERR ;

       	default:
       	    retval = ENV_VARTOKEN;
            *var   = s ;
            while( *++s != 0  &&  *s != '=' ) ;

            char *endvar = s ;
            while( isspace( *s) )  s++ ;
            if( *s == '=' )
            {
				while( isspace(endvar[-1]) )
					endvar-- ;
                s++ ;
                while( isspace( *s) )  s++ ;
                if( !strnicmp( s, "C\"",2) )
                {
					convertCstringToString( s, s ) ;
					*txt    = s ;
		            *endvar = 0 ;
					break ;
                }
            }
            if( getLineText( s, txt, cont)  ==  ENV_TOKENERR )
                retval = ENV_TOKENERR ;

            *endvar = 0 ;
       	    break;
    }
    return  retval ;
}


int ConfigClass::isMySection( const char *usr )
{
    int  ret = (!usr  ||  (_username  &&  !STRICMP( usr, _username)))  ;
    return  ret ;
}

int ConfigClass::readFromFile( sENV_SECTION *thisSectionOnly, BOOL allowNewFile, BOOL delInvFile )       // sets _errLine
{                                // -> 0, ERR_ENV_SYNTAXERROR, ERR_READFILE, ERR_ENV_DUPLICATESECT, ERR_ALLOCERROR
    int  ret=0 ;
	FILE *fp = NULL ;
	if( !fileExist( _filename) )
		return allowNewFile ? 0 : ERR_NOSUCHFILE ;
	else
	if( (fp = fopenRetry( _filename, "r", NULL)) == NULL )
		return ERR_OPENFILE ;

	char          nameString[80] ;
    char         *sectname, *varName, *vartxt, *base, *usr ;
    char          line[1024] ;
    int           cont ;                  // if line ends with continuation char
    char         *varString = (char*)MALLOC( 20000 ) ;
    sENV_SECTION *curSect   = actSect ? actSect->curSect : NULL ;

    for( _errLine=0 ; ret==0  &&  fgets( line, sizeof(line)-2, fp) ; _errLine++ )
    {
        switch( readLine( line, &sectname, &varName, &vartxt, &cont) )
		{
    		case ENV_SECTOKEN:
                decodeSectName( sectname, &base, &usr ) ;
                if( !isMySection( usr) )
                    curSect = NULL ;
                else
				if( thisSectionOnly != NULL )
				{
					if( thisSectionOnly->isItMe( base, usr) )
					{
						curSect = thisSectionOnly ;
						curSect->clear( ) ;
					}
					else
						curSect = NULL ;
				}
				else
                {
                    if( (ret = addSection( base, usr)) != 0 )           // name already exist
            	        goto errorLabel ;
                    curSect = actSect->curSect ;
                }
			    break ;
		    case ENV_VARTOKEN:        // variable name token
                if( cont )
                {
					int  lenString=strlen( vartxt) ;
					strcpy( varString, vartxt ) ;
                    strncpy( nameString, varName, sizeof(nameString)-1 ) ;
					nameString[sizeof(nameString)-1] = 0 ;
                    while( cont )
                    {
                        if( !fgets( line, sizeof(line)-2, fp)  ||
                            getLineText( line, &vartxt, &cont) == ENV_TOKENERR )
						{
        	            	ret = ERR_ENV_SYNTAXERROR ;
	            	        goto errorLabel ;
						}
                        _errLine++ ;
						strcpy( varString+lenString, vartxt ) ;
						lenString += strlen( vartxt) ;
                    }
                    vartxt  = varString ;
                    varName = nameString ;
                }

                if( curSect )
   		            curSect->addVariable( varName, vartxt ) ;
	        	break;
    	    case ENV_TOKENERR:
	        	ret = ERR_ENV_SYNTAXERROR ;
       	        goto errorLabel ;
    	}
    }

    if( ferror( fp) )
		ret = ERR_READFILE ;
  errorLabel:
	FREE( varString ) ;
	fclose( fp ) ;
	if( delInvFile  &&  ret != 0 )
	{
		delFile( _filename ) ; 
		ret = 0 ;
		_changed = 1 ;
	}
	return  ret ;
}


void ConfigClass::reloadSection( const char *sectname )
{
    sENV_SECTION *sect ;
	ASSERT( sectname != NULL  &&  sectname[0] == '#' ) ;
    if( getGetSection( sectname, &sect) != 0 )
		throw Msg( -1, "%s: %s", getStr(SCRMSG_ReloadBadSect), sectname ) ;
	
    actSect  = NULL ;
	_sayError= 0 ;

	sect->clear( ) ;
	int err  = readFromFile( sect ) ;
	if( err != 0 )
		throw Msg( err, "%s %s:\n%s", getStr(SCRMSG_ErrReadCfg), _filename, errorMessage() ) ;
	sect->_changed = 0 ;
	sect->_deleted = 0 ;
	computeChangeFlag( ) ;
}


void ConfigClass::reload( )
{
	// Instead of setting allowNewFile to TRUE better solution would be to remember that
	// config file did not exist and use that. (Or pass allowNewFile as parameter.)
	int err = open( NULL, _sayError, TRUE ) ;
	if( err != 0 )
		throw Msg( err, "%s %s:\n%s", getStr(SCRMSG_ErrReadCfg), _filename, errorMessage() ) ;
}

int ConfigClass :: open( const char *usr, BOOL say_error, BOOL allowNewFile, BOOL delInvFile  )
{                             // -> 0, ERR_ENV_SYNTAXERROR, ERR_NOSUCHFILE, ERR_OPENFILE, ERR_READFILE, ERR_ENV_DUPLICATESECT
	ASSERT( _filename ) ;

    sListPtr<sENV_EXTSECTION> *oldSectList = sectList ;
	sectList  = NULL ;
	sectList  = new sListPtr<sENV_EXTSECTION>( ) ;
    actSect   = NULL ;
	_sayError = say_error ;
    
	if( usr )
    {
		if( _username != NULL )
		{
			FREE( _username ) ;
			_username = NULL ;
		}
        while( *usr  &&  isspace(*usr) )  ++usr ;
        if( !isStrEmpty(usr)  &&  (_username = STRDUP( usr)) != NULL )
        {
            int  len = strlen( _username ) ;
            while( --len >= 0  &&  isspace( _username[len]) )  _username[len] = 0 ;
        }
    }

    addSection( NULL, NULL ) ;                        // create NULL section (-> actSection)
    int err = readFromFile( NULL, allowNewFile, delInvFile ) ;

	if( err == 0 )
	{
		oldSectList->removeAllWithData( ) ;
		delete oldSectList ;
	}
	else
	{
	    actSect  = NULL ;
		errorReport( err, _filename ) ;
		sectList->removeAllWithData( ) ;
		delete  sectList ;
		sectList = oldSectList ;
	}
	return  err ;
}


//-------------------------------------------------------------------------------------
//              ERROR
//-------------------------------------------------------------------------------------


void ConfigClass::errorReport( int errnum, char *filename )
{
    int    msg ;
	switch( errnum )
	{
        case ERR_NOTEMPFILE       : msg = errorsMSG_ENV_NOTEMPFILE   ;  break ;
        case ERR_NOSUCHFILE       : msg = errorsMSG_ENV_NOSUCHFILE   ;  break ;
        case ERR_OPENFILE         : msg = errorsMSG_ENV_OPENFILE     ;  break ;
        case ERR_READFILE         : msg = errorsMSG_ENV_READFILE     ;  break ;
        case ERR_WRITEFILE        : msg = errorsMSG_ENV_WRITEFILE    ;  break ;
        case ERR_ENV_SYNTAXERROR  : msg = errorsMSG_ENV_SYNTAXERROR  ;  break ;
        case ERR_ENV_DUPLICATESECT: msg = errorsMSG_ENV_DUPLICATESECT;  break ;
        default                   : msg = errorsMSG_ENV_CONFIGERROR  ;  break ;
	}

    char    *fmt = errorsMSG( msg ) ;

	if( *fmt == '?' )			/* message not defined yet */
	switch( errnum )
	{
        case ERR_NOTEMPFILE :
            fmt = "Can't open temporary file to save the config file" ;    // save()
            break ;
		case ERR_NOSUCHFILE :
			fmt = "Can't find config file:\n %s" ;
			break ;
		case ERR_OPENFILE :
			fmt = "Can't open config file:\n %s" ;
			break ;
		case ERR_WRITEFILE :
			fmt = "Can't write config file:\n %s" ;                        // save()
			break ;
		case ERR_READFILE :
			fmt = "Can't read config file:\n %s" ;
			break ;
		case ERR_ENV_SYNTAXERROR :
			fmt = "Syntax error in config file:\n\t%s  (line %d)" ;
			break ;
        case ERR_ENV_DUPLICATESECT :
            fmt = "Same section used twice in config file\n\t%s  (line %d)" ;
            break ;
		default :
			fmt = "Error reading config file\n\t%s  (line %d)" ;
			break ;
	}

	char fileBuf[125] ;
	int len = strlen( filename ) ;
	if( len < 120 )
		strcpy( fileBuf, filename ) ;
	else
		sprintf( fileBuf, "...%s", filename+len-117 ) ;

	sprintf( _message, fmt, fileBuf, _errLine ) ;
    _err = errnum ;
	if( _sayError )                                     /*&&*/
		stdOkDialog( _message ) ;
}
