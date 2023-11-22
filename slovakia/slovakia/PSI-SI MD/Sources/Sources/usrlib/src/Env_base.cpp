#include "tools2.hpp"

#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#include "envX.hpp"


//-------------------------------------------------------------------------------------
//              sENV_SECTION
//-------------------------------------------------------------------------------------


sENV_SECTION::sENV_SECTION( sENV_EXTSECTION *p )
{
	_parent  = p ;
	_changed = 0 ;
	_deleted = 0 ;
}

sENV_SECTION::~sENV_SECTION( )
{
	_varList.removeAllWithData( ) ;
}

void sENV_SECTION::clear( )
{
	_varList.removeAllWithData( ) ;
}

void sENV_SECTION::addVariable( const char *varName, const char *varTxt )
{
	sENV_VAR *var = new sENV_VAR( varName, varTxt) ;
   	_varList.addTail( var ) ;
}


BOOL sENV_SECTION::isItMe( const char *base, const char *usr )
{
	if( base == NULL )
	{
		if( !isStrEmpty(_parent->baseName) )
			return  FALSE ;
	}
	else
	if( stricmp(base,_parent->baseName) != 0 )
		return  FALSE ;

	if( usr == NULL )
		return  this == _parent->baseSect ;
	else
		return  this == _parent->usrSect ;
}


sENV_VAR *sENV_SECTION::getVariable( const char *varName )
{
    if( !_varList.isEmpty() )
    {
		sENV_VAR *var ;
		sPOSITION pos = _varList.rewind() ;
		while( (var = _varList.next(pos)) != NULL )
		{
			if( !STRICMP( var->varname(), varName) )
				return  var ;
		}
	}
	return  NULL ;
}

BOOL sENV_SECTION::delVariable( const char *varName )
{
	sENV_VAR *var = getVariable( varName ) ;
	if( var == NULL )
		return FALSE ;
	_varList.removeWithData( var ) ;
	_changed = 1 ;
	return  TRUE ;
}


int sENV_SECTION::appendVars( char ***varnames, int *n_varnames )
{
    if( isEmpty() )
		return  0 ;

    char  **names =   *varnames ;
    int   n_names = *n_varnames ;

    char **realloc_names = (char **)REALLOC( names, (n_names + count() + 1)*sizeof(char*) ) ;
    if( !realloc_names )
    {
        FREE( names ) ;
        *varnames   = NULL ;
        *n_varnames = 0 ;
        return  ERR_ALLOCERROR ;
    }
    *varnames = names = realloc_names ;

    sENV_VAR *var ;
    sPOSITION pos = _varList.rewind() ;
	while( (var = _varList.next(pos)) != NULL )
    {
        int  j ;
        char *name = var->varname() ;
        for( j=0 ; j < n_names ; ++j )
    	    if( !STRICMP( names[j], name) )
                break ;                      // varname already among varnames
        if( j >= n_names )
            names[ n_names++] = name ;
    }

    names[ n_names] = NULL ;
    *varnames       =   names ;
    *n_varnames     = n_names ;
    return  0 ;
}


int sENV_SECTION::setvarval( const char *name, const char *value )     // -> 0, ERR_ALLOCERROR
{
    sENV_VAR          *var ;
    value = value ? value : "" ;

    if( !isEmpty() )
    {
        sPOSITION pos = _varList.rewind() ;
		while( (var = _varList.next(pos)) != NULL )
        {
            if( !STRICMP( var->varname() , name) )
		    {
	            if( strcmp( var->vartext(), value) != 0 )
                {
					uint  len = strlen(value) ;
					if( strlen(var->vartext()) < len )
						var->_vartext = (char*)REALLOC( var->_vartext, len+1 ) ;
		            strcpy( var->_vartext, value ) ;
                    _changed = 1 ;
                }
		        return  0 ;
		    }
        }
    }

	if( strnicmp(value,"C\"",2) == 0 )		// Cstring
	{
		char buf[1024] ;
		BaseConfigClass::convertCstringToString( value, buf ) ;
	    addVariable( name, buf ) ;
	}
	else
		addVariable( name, value ) ;

    _changed = 1 ;
	_deleted = 0 ;			// if adding new variable to previoiusly deleted section
    return  0 ;
}

//
// Format:
//	(i4)	n_bytes		// incl. this byte
//	(i4)	crc32		// from n_items
//	(i4)	n_items
//	n_items pairs "varname\0""vartext\0"
//
size_t sENV_SECTION::binarySize ( )
{
	size_t len = 3*sizeof(long) ;

	sENV_VAR *var ;
	sPOSITION pos = _varList.rewind() ;

	while( (var = _varList.next(pos)) != NULL )
		len += strlen(var->_varname) + strlen(var->_vartext) + 2 ;

	return len ;
}

char *sENV_SECTION::loadFromBin( char *bin )
{
	size_t len = *(long*)bin ;
	bin += sizeof(long) ;
	ulong  crc = *(ulong*)bin ;
	bin += sizeof(ulong) ;
	if( crc != crc32( (uchar*)bin, len-8) )
		return NULL ;

	clear() ;
	int n_items = *(int*)bin ;
	bin += sizeof(int) ;
	for( int j=0 ; j < n_items ; ++j )
	{
		char *txt = bin + strlen(bin) + 1 ;
		addVariable( bin, txt ) ;
		bin = txt + strlen(txt) + 1 ;
	}
	_changed = 1 ;
	return bin ;
}

char *sENV_SECTION::saveToBin( char *bin ) const
{
	long *len = (long*)bin ;
	bin += sizeof(long) ;
	ulong *crc = (ulong*)bin ;
	bin += sizeof(ulong) ;

	char *binData = bin ;
	*(int*)bin = _varList.count() ;
	bin += sizeof(int) ;

	sENV_VAR *var ;
	sPOSITION pos = _varList.rewind() ;
	while( (var = (const_cast<sENV_SECTION*>(this))->_varList.next(pos)) != NULL )
	{
		strcpy( bin,  var->_varname ) ;
		bin += strlen(var->_varname) + 1 ;

		strcpy( bin,  var->_vartext ) ;
		bin += strlen(var->_vartext) + 1 ;
	}
	*len = (bin - (char*)len) ;
	*crc = crc32( (uchar*)binData, (bin-binData) ) ;
	return bin ;
}


//-------------------------------------------------------------------------------------
//              sENV_EXTSECTION
//-------------------------------------------------------------------------------------


sENV_EXTSECTION :: sENV_EXTSECTION( const char *baseSectName )
{
    baseName = STRDUP( baseSectName ? baseSectName : "" ) ;      // may be NULL for NULL-section
    baseSect = usrSect = curSect = NULL ;
}

sENV_EXTSECTION :: ~sENV_EXTSECTION( )
{
    if(   baseSect )
		delete   baseSect ;
    if(    usrSect )
		delete    usrSect ;
	if( baseName   )
		FREE( baseName ) ;
}

void sENV_EXTSECTION::del( )
{
    if(   baseSect )
		baseSect->del( ) ;
    if(    usrSect )
		usrSect->del( ) ;
}

BOOL sENV_EXTSECTION::delVariable( const char *varName )
{
	BOOL ret=FALSE ;
    if(   baseSect )
		if( baseSect->delVariable( varName) )
			ret = TRUE ;
    if(    usrSect )
		if( usrSect->delVariable( varName) )
			ret = TRUE ;
	return  ret ;
}


BOOL sENV_EXTSECTION::changed( )
{
    if(   baseSect )
		if( baseSect->changed() )
			return  TRUE ;
    if(    usrSect )
		if( usrSect->changed() )
			return  TRUE ;
	return  FALSE ;
}

//
// Format:
// (b1)		0/1 (flag1)
// (...)	baseSect if flag1==1
// (b1)		0/1 (flag2)
// (...)	usrSect if flag2==1

size_t sENV_EXTSECTION::binarySize( )
{
	return 2 +
		(baseSect ? baseSect->binarySize() : 0) +
		(usrSect  ?  usrSect->binarySize() : 0) ;
}

char *sENV_EXTSECTION::loadFromBin( char *bin )
{
	if( baseSect != NULL )
		baseSect->clear() ;
	if( usrSect != NULL )
		usrSect->clear() ;
	if( *bin++ )
	{
		if( baseSect == NULL )
			baseSect = new sENV_SECTION( this ) ;
		bin = baseSect->loadFromBin( bin ) ;
	}

	if( bin != NULL  &&  *bin++ )
	{
		if( usrSect == NULL )
			usrSect = new sENV_SECTION( this ) ;
		bin = usrSect->loadFromBin( bin ) ;
	}

	return bin ;
}

char *sENV_EXTSECTION::saveToBin( char *bin ) const
{
	if( baseSect )
	{
		*bin++ = 1 ;
		bin = baseSect->saveToBin( bin ) ;
	}
	else
		*bin++ = 0 ;
	if( usrSect )
	{
		*bin++ = 1 ;
		bin = usrSect->saveToBin( bin ) ;
	}
	else
		*bin++ = 0 ;
	return bin ;
}


//-------------------------------------------------------------------------------------
//              constructors & destructors
//-------------------------------------------------------------------------------------


BaseConfigClass::BaseConfigClass( const char *filename )
{
    _changed  = 0 ;
    _username = NULL ;
    sectList  = new sListPtr<sENV_EXTSECTION>( ) ;
    actSect   = NULL ;
}

BaseConfigClass::~BaseConfigClass()
{
    close( ) ;
	delete  sectList ;
}

void  BaseConfigClass::close( )
{
	clear() ;
    if( _username )
	{
		FREE( _username ) ;
		_username = NULL ;
	}
}

void  BaseConfigClass::clear( )
{
	sectList->removeAllWithData( ) ;
    actSect = NULL ;
}



        // empty base/usr  are returned as NULL pointers
void BaseConfigClass::decodeSectName( char *sectName, char **base, char **usr )
{
    *usr=NULL ;

    if( !sectName )
    {
        *base = NULL ;
        return ;
    }
    while( isspace( *sectName) )  sectName++ ;          // cut off leading and closing spaces
    int  len = strlen( sectName ) ;
    while( --len >= 0  &&  isspace( sectName[len]) )  sectName[len] = 0 ;

    *base = sectName ;
	char *app ;
    if( (app = strchr( *base, '.')) != NULL )
    {
		*app = 0 ;
        if( (*usr = strchr( app+1, '.')) != NULL )
        {
            *(*usr)++ = 0 ;
            if( !**usr )
				*usr = NULL ;
        }
    }
    if( !**base )
		*base = NULL ;
}


void BaseConfigClass::convertCstringToString( const char *cstring, char *string )
{
	char  val1[1024] ;
	strncpy( val1, cstring, sizeof(val1) ) ;
	val1[sizeof(val1)-1] = 0 ;
	char *ptr = strrchr( val1+2, '\"' ) ;
	if( ptr != NULL )
		*ptr = 0 ;
	cstringToString( val1+2, string ) ;
}


void BaseConfigClass::computeChangeFlag( )
{
	_changed = FALSE ;
    sENV_EXTSECTION  *sect ;
    sPOSITION pos = sectList->rewind() ;
	while( (sect = sectList->next(pos)) != NULL )
    {
		_changed = sect->changed( ) ;
		if( _changed )
			break ;
	}
}


//-------------------------------------------------------------------------------------
//              linearization
//-------------------------------------------------------------------------------------

//
// Format:
//	(c4)	"CFG\0"
//	(i4)	version
//	(i4)	size
//	(i4)	crc32			// from the next member
//	(i4)	num sections
//	(c?)	"userName\0"	// empty user name allowed
//	... "sectionName\0", section data ...
//
#define	PASSW	"CFG"
#define VERSION	1

size_t BaseConfigClass::binarySize( )
{
	size_t len = 
		4 +										// password
		4*sizeof(long) +						// version, size, crc, nSections
		1 + (_username ? strlen(_username):0) ;	// user name

    sENV_EXTSECTION  *sect ;
    sPOSITION pos = sectList->rewind() ;
	while( (sect = sectList->next(pos)) != NULL )
		len += sect->binarySize() + strlen(sect->baseName) + 1 ;
	return len ;
}

char *BaseConfigClass::loadFromBin( char *bin )
{
	close() ;
	_changed = 1 ;

	if( strcmp( bin, PASSW) != 0 )
		return NULL ;
	bin += 4 ;

	if( *bin != VERSION )
		return NULL ;
	bin += sizeof(long) ;

	ulong size		= *(ulong*)bin ;  ; bin += sizeof(ulong) ;
	ulong crc		= *(ulong*)bin ;  ; bin += sizeof(ulong) ;

	if( crc != crc32( (uchar*)bin, size) )
		return NULL ;

	ulong nSections = *(ulong*)bin ;  ; bin += sizeof(ulong) ;

	if( *bin )
	{
		_username = strdup( bin ) ;
		bin += strlen(bin) ;
	}
	bin++ ;

	for( int j=0 ; j < nSections ; ++j )
	{
		addSection( bin, _username ) ;
		bin += strlen(bin) + 1 ;
		bin  = actSect->loadFromBin( bin ) ;
		if( bin == NULL )
			return NULL ;
	}
	return bin ;
}

char *BaseConfigClass::saveToBin( char *bin ) const
{
	strcpy( bin, PASSW )			  ; bin += 4 ;
	*bin		  = VERSION			  ; bin += sizeof( long) ;
	ulong *size   = (ulong*)bin		  ; bin += sizeof(ulong) ;
	ulong *crc    = (ulong*)bin		  ; bin += sizeof(ulong) ;

	char  *binData= bin ;

	*(ulong*)bin  = sectList->count() ; bin += sizeof(ulong) ;

	if( _username )
	{
		strcpy( bin, _username ) ;
		bin += strlen(_username) ;
	}
	*bin++ = 0 ;

    sENV_EXTSECTION  *sect ;
    sPOSITION pos = sectList->rewind() ;
	while( (sect = sectList->next(pos)) != NULL )
	{
		strcpy( bin, sect->baseName) ;
		bin += strlen( sect->baseName ) + 1 ;
		bin = sect->saveToBin( bin ) ;
	}

	*size= bin - binData ;
	*crc = crc32( (uchar*)binData, *size ) ;
	return bin ;
}


//-------------------------------------------------------------------------------------
//              GET
//-------------------------------------------------------------------------------------


int BaseConfigClass :: getGetSection( const char *sectname, sENV_SECTION **sect )      // -> 0, ENV_SECTNOTEXIST
{
    char  *base, *usr ;
    char  fullSectName[ 120] ;
    int   isAbsAdr=0 ;

    *sect = NULL ;
    if( !sectname )
        base = usr = NULL ;
    else
    {
        if( *sectname == '#' )
        {
            isAbsAdr = 1 ;
            sectname++ ;
        }
        strcpy( fullSectName, sectname ) ;
        decodeSectName( fullSectName, &base, &usr ) ;
    }

    if( setActSection( base) != 0 )
        return  ENV_SECTNOTEXIST ;

    if( !isAbsAdr )
		return  0 ;

    *sect = usr ? actSect->usrSect : actSect->baseSect ;
    return  *sect ? 0 : ENV_SECTNOTEXIST ;
}


char *BaseConfigClass :: get( const char *sectname, const char *varName )
{
    sENV_SECTION *sect ;
    if( !varName  ||  getGetSection( sectname, &sect) != 0 )
        return  NULL ;

	if( sect )
	{
		// Absolute section
		if( !sect->deleted() )
		{
			sENV_VAR *var = sect->getVariable( varName ) ;
			if( var != NULL )
				return  var->vartext() ;
		}
		return NULL ;
	}

	sect = actSect->usrSect ;
	if( sect  &&  !sect->deleted() )
	{
		sENV_VAR *var = sect->getVariable( varName ) ;
		if( var != NULL )
			return  var->vartext() ;
		return NULL ;
	}

	sect = actSect->baseSect ;
	if( sect  &&  !sect->deleted() )
	{
		sENV_VAR *var = sect->getVariable( varName ) ;
		if( var != NULL )
			return  var->vartext() ;
		return NULL ;
	}

    return  NULL ;
}


int BaseConfigClass::varList( const char *sectname, char ***varnames, int *n_names )
{
    int  n_vars=0 ;
    *varnames = NULL ;
    if( n_names )  *n_names = 0 ;

    sENV_SECTION *sect ;
    if( getGetSection( sectname, &sect) != 0 )
        return  0 ;

    if( sect )
    {
		if( sect->deleted() )
			return 0 ;
        if( sect->appendVars( varnames, &n_vars) )
            return  ERR_ALLOCERROR ;
    }
    else
    {
        for( int i=0 ; i < 2 ; i++ )
        {
			if( i == 0 )
				sect = actSect->usrSect ;
			else
				sect = actSect->baseSect ;

    	    if( sect  &&  !sect->deleted() )
                if( sect->appendVars( varnames, &n_vars) )
                    return  ERR_ALLOCERROR ;
        }
    }

    if( n_names )  *n_names = n_vars ;
    return  0 ;
}


void BaseConfigClass::destroyVarList( char **varnames )
{
    if( varnames )  FREE( varnames ) ;
}

static BOOL isSectionDeleted( sENV_EXTSECTION *sect )
{
	if( sect->baseSect != NULL  &&  !sect->baseSect->deleted() )
		return FALSE ;
	if( sect->usrSect != NULL  &&  !sect->usrSect->deleted() )
		return FALSE ;
	return TRUE ;
}

sStringPtrArray *BaseConfigClass::sectionList( const char *mask )
{
	sStringPtrArray *list = new sStringPtrArray( ) ;

    sENV_EXTSECTION  *sect ;
    sPOSITION pos = sectList->rewind() ;
	while( (sect = sectList->next(pos)) != NULL )
    {
		if( isSectionDeleted(sect) )
			continue ;
		const char *s = sect->baseName ;
		if( *s != 0 )
			if( mask == NULL  ||  isMatch(s,mask) )
				*list  << s ;
    }
	return  list ;
}

//-------------------------------------------------------------------------------------
//              delete
//-------------------------------------------------------------------------------------


BOOL BaseConfigClass :: delSection( const char *sectname )
{
	sENV_SECTION *sect ;
	if( getGetSection( sectname, &sect) != 0 )
		return  FALSE ;
	if( sect )
	{
		if( sect->deleted() )
			return TRUE ;
		sect->del( ) ;
	}
	else
	{
		if( isSectionDeleted( actSect) )
			return TRUE ;
		actSect->del( ) ;
	}
	_changed = 1 ;
	return  TRUE ;
}


BOOL BaseConfigClass :: delVariable( const char *sectname, const char *varname )
{
	sENV_SECTION *sect ;
	if( getGetSection( sectname, &sect) != 0 )
		return  FALSE ;

	BOOL  ret ;
	if( sect )
		ret = sect->delVariable( varname ) ;
	else
		ret = actSect->delVariable( varname ) ;
	if( ret )
		_changed = 1 ;
	return  ret ;
}


//-------------------------------------------------------------------------------------
//              Private functions
//-------------------------------------------------------------------------------------


void BaseConfigClass:: sectTitle( char *buf, const char *base, const char *usr )
{
    if( !base  &&  !usr )
    {
        buf[0] = 0 ;
        return  ;
    }
    buf[0] = '[' ;
    char *s = buf + 1 ;

    if( base )
        s += sprintf( s, "%s", base ) ;

    if( usr )
        s += sprintf( s, "..%s", usr ) ;
    strcpy( s, "]" ) ;
}


int BaseConfigClass::setActSection( const char *baseSectName )       // -> 0, ENV_SECTNOTEXIST
{
    if( !baseSectName )  baseSectName = "" ;

    if( actSect )
	    if( !STRICMP( baseSectName, actSect->baseName) )
	        return  0 ;

    if( !sectList->isEmpty() )
    {
        sENV_EXTSECTION  *sect ;
        sPOSITION pos = sectList->rewind() ;
		while( (sect = sectList->next(pos)) != NULL )
        {
	        if( !STRICMP( baseSectName, sect->baseName) )
		    {
		        actSect = sect ;
                actSect->curSect = NULL ;
		        return  0 ;
		    }
        }
    }

    return  ENV_SECTNOTEXIST ;
}

int BaseConfigClass::setActSection( const char *base, const char *usr )       // -> 0, ENV_SECTNOTEXIST
{
    if( setActSection( base) != 0 )
        return  ENV_SECTNOTEXIST ;

    if( usr  &&  isStrEmpty( usr) )
		usr = NULL ;

    actSect->curSect = usr ? actSect->usrSect : actSect->baseSect ;
    return  actSect->curSect ? 0 : ENV_SECTNOTEXIST ;
}


int BaseConfigClass::addSection( const char *base, const char *usr )
{                                          // -> 0, ERR_ENV_DUPLICATESECT, ERR_ALLOCERROR
    int  ret=0 ;
    if( setActSection( base) != 0 )
    {
        actSect = new sENV_EXTSECTION( base ) ;
        if( !actSect )  return  ERR_ALLOCERROR ;
        sectList->addTail( actSect ) ;
    }

    if( usr  &&  isStrEmpty( usr) )
		usr = NULL ;
    if( usr )
    {
        if( actSect->usrSect )
        {
            if( !actSect->usrSect->isEmpty() )
                ret = ERR_ENV_DUPLICATESECT ;
        }
        else
            actSect->usrSect = new sENV_SECTION( actSect ) ;
        actSect->curSect = actSect->usrSect ;
    }
    else
    {
        if( actSect->baseSect )
        {
            if( !actSect->baseSect->isEmpty() )
                ret = ERR_ENV_DUPLICATESECT ;
        }
        else
            actSect->baseSect = new sENV_SECTION( actSect ) ;
        actSect->curSect = actSect->baseSect ;
    }
    return  actSect->curSect ? ret : ERR_ALLOCERROR ;
}


//-------------------------------------------------------------------------------------
//              UTILS
//-------------------------------------------------------------------------------------


static int getTokens( char *var, char ***tokens, int *n_tokens, char *terms )
{
    int   err = 0 ;
	*n_tokens = 0 ;
    *tokens   = NULL ;
    if( !var )       return  0 ;
	char *var_copy = STRDUP( var ) ;
	if( !var_copy )
		return  ERR_ALLOCERROR ;

	int max_names=0,  n_names=1 ;
	char **names=NULL ;

	for( char *s=strtok(var_copy,terms) ; s ; s=strtok(NULL,terms) )
	{
		if( n_names >= max_names-1 )
		{
			char **realloc_names = (char **)REALLOC( names, (max_names+10)*sizeof(char*)) ;
			if( !realloc_names )
            {
                err = ERR_ALLOCERROR ;
                break ;
            }
			max_names += 10 ;
			names = realloc_names ;
		}
		names[ n_names++] = s ;
	}

    if( !err )
    {
	    *n_tokens = n_names-1 ;
	    if( names )
	    {
		    names[0] = var_copy ;
    		*tokens  = names+1 ;
            (*tokens)[ *n_tokens] = NULL ;
            return  0 ;
	    }
    }
	FREE( var_copy ) ;
	return  err ;
}


int BaseConfigClass::tokenList( const char *sect,const  char *var, char ***tokens, int *n_tokens, char *terms )   // -> 0, ERR_ALLOCERROR
{
    char *val = get( sect, var ) ;
    *tokens = NULL ;

    int n ;
    int ret = getTokens( val, tokens, &n, terms ? terms : " \t\n" ) ;
    if( n_tokens )  *n_tokens = n ;
    return  ret ;
}


void BaseConfigClass::destroyTokenList( char **tokens )
{
	if( tokens )
	{
		FREE( tokens[-1] ) ;
		FREE( tokens - 1 ) ;
	}
}


BOOL BaseConfigClass::getInt( const char *sectname, const char *varName, int *num )     // -> success
{
    char *val = get( sectname, varName ) ;
    return  val ? isInt( val, num) : 0 ;
}

int BaseConfigClass::setInt( const char *sectname, const char *varName, int num )
{
	char  buf[80] ;
	itoa( num, buf, 10 ) ; 
	return  set( sectname, varName, buf ) ;
}

BOOL BaseConfigClass::getFloat( const char *sectname, const char *varName, float *num )     // -> success
{
    char *val = get( sectname, varName ) ;
    return  val ? isFloat( val, num) : 0 ;
}

BOOL BaseConfigClass::getDouble( const char *sectname, const char *varName, double *num )     // -> success
{
    char *val = get( sectname, varName ) ;
    return  val ? isDouble( val, num) : 0 ;
}

int BaseConfigClass::setDouble( const char *sectname, const char *varName, double num )
{
	char  buf[80] ;
	sprintf( buf, "%f", num ) ;
	return  set( sectname, varName, buf ) ;
}
/*
BOOL BaseConfigClass::getColor( const char *sectname, const char *varName, long *rgb )     // -> success
{
    char *val = get( sectname, varName ) ;
    return  val ? colorNameToRgb( val, rgb) : 0 ;
}

int BaseConfigClass::setColor( const char *sectname, const char *varName, long rgb )     // -> success
{
	char  buf[80] ;
	rgbToColorName( buf, rgb ) ;
	set( sectname, varName, buf ) ;
}
*/
BOOL BaseConfigClass::getFont( const char *sectname, const char *varName, LOGFONT *f )     // -> success
{
    char *val = get( sectname, varName ) ;
	int   ret;

    if( val == NULL )
		return  FALSE ;
/* @Ma  Nebezpecne -> ak val zacina medzerou tak nekorektne nacitanie
	if( isdigit(val[0]) )
		return  sscanf( val, "%ld %ld %ld %ld %ld %d %d %d %d %d %d %d %d %s",
			&f->lfHeight, &f->lfWidth, &f->lfEscapement, &f->lfOrientation, &f->lfWeight,
			&f->lfItalic, &f->lfUnderline, &f->lfStrikeOut, &f->lfCharSet, &f->lfOutPrecision,
			&f->lfClipPrecision, &f->lfQuality, &f->lfPitchAndFamily, f->lfFaceName ) != EOF ;

	if( sscanf( val, "%s %ld %ld", f->lfFaceName, &f->lfHeight, &f->lfWidth) == EOF )   
		return  FALSE ;
*/
	char  buf[1024] ;
	f->lfFaceName[0] = 0 ;
    

    if( (ret = sscanf( val, "%[^;]; %ld %ld %ld %ld %ld %d %d %d %d %d %d %d %d",
		buf, &f->lfHeight, &f->lfWidth, &f->lfEscapement, &f->lfOrientation,
        &f->lfWeight, &f->lfItalic, &f->lfUnderline, &f->lfStrikeOut, &f->lfCharSet,
         &f->lfOutPrecision, &f->lfClipPrecision, &f->lfQuality, &f->lfPitchAndFamily )) < 3 )
        return FALSE;
    if( ret != 14 )
    {
        f->lfEscapement     = 0  ;
        f->lfOrientation    = 0  ;
        f->lfWeight         = FW_MEDIUM ; 
        f->lfItalic         = FALSE ; 
        f->lfUnderline      = FALSE ; 
        f->lfStrikeOut      = FALSE ; 
        f->lfCharSet        = DEFAULT_CHARSET ; 
        f->lfOutPrecision   = OUT_DEFAULT_PRECIS ; 
        f->lfClipPrecision  = CLIP_DEFAULT_PRECIS ; 
        f->lfQuality        = DEFAULT_QUALITY ; 
        f->lfPitchAndFamily = DEFAULT_PITCH | FF_ROMAN ; 
    }
	strncpy( f->lfFaceName, buf, LF_FACESIZE ) ;
	f->lfFaceName[LF_FACESIZE-1] = 0 ;
	return  TRUE ;
}

int BaseConfigClass::setFont( const char *sectname, const char *varName, LOGFONT *f )
{
	char buf[256] ;
	 sprintf( buf, "%s; %ld %ld %ld %ld %ld %d %d %d %d %d %d %d %d",    //@Ma
		f->lfFaceName, f->lfHeight, f->lfWidth, f->lfEscapement, f->lfOrientation, f->lfWeight,
		f->lfItalic, f->lfUnderline, f->lfStrikeOut, f->lfCharSet, f->lfOutPrecision,
		f->lfClipPrecision, f->lfQuality, f->lfPitchAndFamily ) ;
	return  set( sectname, varName, buf ) ;
}

BOOL BaseConfigClass::getStrings( const char *sect, const char *var, sStringPtrArray &arr )
{
	const char *str = get( sect, var ) ;
	if( str == NULL )
	{
		arr.clearList() ;
		return FALSE ;
	}
	arr.fromText( str ) ;
	return TRUE ;
}

int BaseConfigClass::setStrings( const char *sect, const char *var, const sStringPtrArray &arr )
{
	char buf[1024] ;
	arr.asText( buf, sizeof(buf) ) ;
	return set( sect, var, buf ) ;
}


//-------------------------------------------------------------------------------------
//              SET
//-------------------------------------------------------------------------------------


int BaseConfigClass::printf( const char *sectname, const char *varName, const char *fmt... )
{
    char  buf[1024] ;
    vsprintf( buf, fmt, va_arglist(fmt) ) ;
    return  set( sectname, varName, buf ) ;
}



int BaseConfigClass::getSetSection( const char *sectname, sENV_SECTION **sect )
{                                   // -> 0, ERR_ALLOCERROR
    char  *base, *usr ;
    char  fullSectName[ 256] ;
    int   isAbsAdr=0 ;

    if( !sectname )
        base = usr = NULL ;
    else
    {
        if( *sectname == '#' )
        {
            isAbsAdr = 1 ;
            sectname++ ;
        }
        strcpy( fullSectName, sectname ) ;
        decodeSectName( fullSectName, &base, &usr ) ;
    }

    int  found = !setActSection( base ) ;

    *sect=NULL ;
    if( isAbsAdr )
    {
        addSection( base, usr ) ;
        *sect = actSect->curSect ;
    }
    else
    {                                                     // 2. [sect..usr]
        if( addSection( base, _username) != ERR_ALLOCERROR )
            *sect = _username ? actSect->usrSect : actSect->baseSect ;
    }

	return *sect == NULL ? ERR_ALLOCERROR : 0 ;
}

int BaseConfigClass::set( const char *sectname, const char *varName, const char *value )
{                                   // -> 0, ERR_ALLOCERROR
    if( !varName )
		return  0 ;

    sENV_SECTION *sect ;
	int ret = getSetSection( sectname, &sect ) ;
	if( ret != 0 )
		return  ret ;

    ret = sect->setvarval( varName, value) ;
    if( sect->changed() )
        _changed = sect->_changed ;
    return  ret ;
}

