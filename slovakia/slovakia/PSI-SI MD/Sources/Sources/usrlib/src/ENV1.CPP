#include "tools2.hpp"

#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif


#include <ctype.h>
#include "envX.hpp"
#include "screen.msg"

#define     ENV_SECTOKEN        1
#define     ENV_VARTOKEN        2
#define     ENV_TOKENERR        3
#define     ENV_SECTNOTEXIST    4


static const char *getStr( int id )
{
	const char *s = SCRMSG(id) ;
	if( *s != '?' )
		return s ;
	// english defaults
	switch( id )
	{
		case SCRMSG_CantCreateCfgFile:	s = "Can't create new configuration file" ; break ;
		case SCRMSG_ErrSaveCfgFile :	s = "Error saving configuration file" ; break ;
	}
	return s ;
}


//-------------------------------------------------------------------------------------
//              SAVE
//-------------------------------------------------------------------------------------

int ConfigClass::writeSectToFile( FILE *fp, sENV_SECTION *sect, const char *base, const char *usr )
{                                           // -> 0, ERR_WRITEFILE
    if( !sect )  return  0 ;

    if( base || usr )		// except NULL section
    {
        char  title[80] ;
        sectTitle( title, base, usr ) ;
        fprintf( fp, "%s", title ) ;
    }

    if( !sect->isEmpty() )
    {
	    sENV_VAR *var ;
        sPOSITION pos = sect->_varList.rewind() ;
		while( (var = sect->_varList.next(pos)) != NULL )
        {
		    char *value   = var->vartext() ;
		    char *varname = var->varname() ;

		    // convert to Cstring if necessary
		    char  cstring[1024] ;
			if( stringToCstring( value, cstring) )
                fprintf( fp, "\n   %s = C\"%s\"", varname, cstring ) ;
			else
			{
				int   moreWords=0 ;
				for( char *s=value ; *s ; s++ )
					if( *s <= ' ' )
					{
						moreWords = 1 ;
						break ;
					}

				if( moreWords )
					fprintf( fp, "\n   %s = \"%s\"", varname, value ) ;
				else
					fprintf( fp, "\n   %s = %s", varname, value ) ;
			}
        }
    }

    fprintf( fp, "\n" ) ;
    return  ferror(fp) ? ERR_WRITEFILE : 0 ;
}


int ConfigClass::writeToFile( FILE *src, FILE *dst, sENV_SECTION *thisSectionOnly, const char *thisSectName  )           // sets _errLine
{                                           // -> 0, ERR_ENV_SYNTAXERROR, ERR_WRITEFILE, ERR_READFILE
    char *sectname, *varName, *vartxt, *base ;
    char line[1024];
    int  cont ;         // if line ends with continuation char

    char  lineCopy[sizeof(line)] ;
    lineCopy[sizeof(line)-1] = lineCopy[sizeof(line)-2] = 0 ;

    int  skipSect=0 ;
    if( setActSection( NULL, NULL) != 0 )		// NULL section empty or deleted
        skipSect = 1 ;							
	else											// first NULL section must be written
    if( actSect->curSect->changed() )				// modified, therefore saved from memory
	{
		if( thisSectionOnly == NULL  ||  thisSectionOnly == actSect->curSect )
        {
            if( writeSectToFile( dst, actSect->curSect, NULL, NULL) )
                return  ERR_WRITEFILE ;
            skipSect = 1 ;
            actSect->curSect->_changed = -1 ;		// changed, but saved
        }
    }

	int  thisSectionSaved = 0 ;
    int  lastCharWasEol = 1 ;
    if( src )
    for( _errLine=0 ; fgets( line, sizeof(line), src) ; _errLine++ )
    {
        strncpy( lineCopy, line, sizeof(line)-2 ) ;
        switch( readLine( line, &sectname, &varName, &vartxt, &cont) )
		{
    		case ENV_SECTOKEN:
			{
				char *usr ;
                skipSect = 0 ;
                decodeSectName( sectname, &base, &usr ) ;

                if( isMySection( usr) )
				{
					if(  setActSection( base, usr) == 0 )
					{
						if( actSect->curSect->deleted() )
							skipSect = 1 ;
						else
						if( actSect->curSect->changed() )
						{
							if( thisSectionOnly == NULL  ||  thisSectionOnly == actSect->curSect )
							{
								if( !lastCharWasEol )
									fprintf( dst, "\n" ) ;
								if( writeSectToFile( dst, actSect->curSect, base, usr) )
									return  ERR_WRITEFILE ;
								skipSect = 1 ;
								actSect->curSect->_changed = -1 ;    // changed, but saved
								thisSectionSaved = 1 ;
							}
						}
					}
					else
						skipSect = 1 ;						// Section does not exist
				}
			}
		    default :
                while( 1 )
                {
                    if( !skipSect )
                    {
                        lastCharWasEol = strchr( lineCopy, '\n') ? 1 : 0 ;
                        fputs( lineCopy, dst ) ;
                    }
                    if( !cont )  break ;

                    if( !fgets( line, sizeof(line)-2, src) )
      	            	return  ERR_ENV_SYNTAXERROR ;
                    strncpy( lineCopy, line, sizeof(line)-2 ) ;
                    if( getLineText( line, &vartxt, &cont) == ENV_TOKENERR )
       	            	return  ERR_ENV_SYNTAXERROR ;
                    _errLine++ ;
                }
	        	break ;
    	    case ENV_TOKENERR :
	        	return  ERR_ENV_SYNTAXERROR ;
    	}
    }
    if( !lastCharWasEol )
        fprintf( dst, "\n" ) ;

	if( thisSectionOnly != NULL )
	{
		if( !thisSectionSaved )
		{
			sENV_EXTSECTION *p = thisSectionOnly->parent() ;
			char *user = NULL ;
			if( p != NULL )
			{
				base = p->baseName ;
				if( p->baseSect != thisSectionOnly )
					user = _username ;
			}
			else
			{
	            decodeSectName( (char*)thisSectName, &base, &user ) ;		// ???
			}
			if( writeSectToFile( dst, thisSectionOnly, base, user) )
				return  ERR_WRITEFILE ;
		}
	}
	else
	{
		sENV_EXTSECTION  *extsect ;            // we save remaining sections
		#define SAVE_SECT( sect, username)      \
			if( (sect)  &&  !(sect)->deleted()  &&  (sect)->_changed > 0 )         \
				writeSectToFile( dst, (sect), (char*)extsect->baseName, (username) ) ;

		if( !sectList->isEmpty() )
		{
			sPOSITION pos = sectList->rewind() ;
			while( (extsect = sectList->next(pos)) != NULL )
			{
				SAVE_SECT( extsect->baseSect  , NULL ) ;
				SAVE_SECT( extsect->usrSect   , _username ) ;
			}
		}
	}
    if( src  &&  ferror( src) )
        return  ERR_READFILE ;
    else
    if( ferror( dst) )
        return  ERR_WRITEFILE ;
    return  0 ;
}


void ConfigClass::saveSection( const char *sectname )
{
	ASSERT( sectname != NULL /* &&  sectname[0] == '#'*/ ) ;
    sENV_SECTION *sect ;
	int ret = getSetSection( sectname, &sect ) ;
	if( ret != 0 )
		return  ;

    if( !sect->changed() )
        return ;
	
	_sayError= 0 ;
	int  err = _save( sect, sectname ) ;
	if( err != 0 )
		throw Msg( err, "%s %s:\n%s", getStr(SCRMSG_ErrSaveCfgFile), _filename, errorMessage() ) ;
	sect->_changed = 0 ;
	computeChangeFlag( ) ;
}


int ConfigClass :: _save( sENV_SECTION *thisSectionOnly, const char *thisSectName )
{                             // -> 0, ERR_NOTEMPFILE, ERR_OPENFILE, ERR_WRITEFILE, ERR_READFILE
    int  err  = 0 ;
    _errLine  = 0 ;
    char *tmpname = TEMPNAM( "", "" ) ;
    FILE *src = NULL ,
         *dst = tmpname ? fopenRetry( tmpname, "wt") : NULL ;

    if( !dst )
        err = ERR_NOTEMPFILE ;
    else
    if( fileExist( _filename) )
    {
        if( (src = fopenRetry( _filename, "r", NULL)) != NULL )
            err = writeToFile( src, dst, thisSectionOnly, thisSectName ) ;
        else
            err = ERR_OPENFILE ;
    }
    else
        err = writeToFile( NULL, dst, thisSectionOnly, thisSectName ) ;

    if( src )  fclose( src ) ;
    if( dst )  fclose( dst ) ;

	if( err )
    {
		errorReport( err, _filename ) ;
        if( tmpname )
            delFile( tmpname ) ;
    }
    else
    {
        for( int j=0 ; j < 3 ; ++j )
        {
            if( (err = copyFile( tmpname, _filename)) == 0 )        // ERR_READFILE, ERR_WRITEFILE
                break ;
        }
        if( err == ERR_READFILE )
            errorReport( err, err == ERR_READFILE ? tmpname : _filename ) ;
        else
            delFile( tmpname ) ;
    }

    if( tmpname )
        FREE( tmpname ) ;
	return  err ;
}



int ConfigClass :: save( BOOL say_error )
{                             // -> 0, ERR_NOTEMPFILE, ERR_OPENFILE, ERR_WRITEFILE, ERR_READFILE
    _sayError = say_error ;
    if( !isChanged() )
        return  0 ;

	int  err = _save( NULL ) ;

    if( !sectList->isEmpty() )
    {
		int             change_flag = err ? 1 : 0 ;
		sENV_EXTSECTION *extsect ;
        sPOSITION       pos ;

		/*
		// if saved OK, remove deleted sections
		if( err == 0 )
		{
			pos = sectList->rewind() ;
			while( (extsect = sectList->next(pos)) != NULL )
			{
			}
		}
		*/

		// we delete or reset change flags depending on whether save() finished OK or not
        pos = sectList->rewind() ;
		while( (extsect = sectList->next(pos)) != NULL )
        {
            if( extsect->baseSect )
                if( extsect->baseSect->changed() )
                    extsect->baseSect->_changed = (char)change_flag ;
            if( extsect->usrSect )
                if( extsect->usrSect->changed() )
                    extsect->usrSect->_changed = (char)change_flag ;
        }
    }

	computeChangeFlag( ) ;
	return  err ;
}

//-------------------------------------------------------------------------------------
//              INSTALL
//-------------------------------------------------------------------------------------


void ConfigClass::install( const char **defaults, int n_defaults, int installNewSectionsOnly )
{
	int  j ;

	BOOL ready = FALSE ;
	if( !fileExist( _filename) )
	{
		FILE *fp = fopenRetry( _filename, "wt" ) ;
		if( fp == NULL )
			throw Msg( -1, "%s\n  %s", getStr(SCRMSG_CantCreateCfgFile), _filename ) ;
		
		for( j=0 ; j < n_defaults ; ++j )
		{
			const char *var=defaults[3*j] ;
			if( var[0] == '[' )
				fprintf( fp, "\n\n%s", var ) ;
			else
			{
				const char *val=defaults[3*j+1] ;
				const char *rem=defaults[3*j+2] ;
				if( val == NULL )
					fprintf( fp, "\n%-25s=", var ) ;
				else
				if( (strchr(val,' ')  ||  strchr(val,'\t'))  &&  (strnicmp(val,"C\"",2) != 0) )
					fprintf( fp, "\n%-25s= \"%s\"", var, val ) ;
				else
					fprintf( fp, "\n%-25s= %-25s", var, val ) ;
				if( rem != NULL )
					fprintf( fp, "   * %s", rem ) ;
			}
		}
		fclose( fp ) ;
		ready = TRUE ;
	}

	if( sectList->isEmpty() )
		if( open( NULL, 1) != 0 )
			throw Msg( error(), errorMessage() ) ;
	if( ready )
		return ;

	//ASSERT( defaults[0][0] == '[' ) ;
	char  section[80] ;
	int   ignore=0 ;
	strcpy( section, "#" ) ;
	for( j=0 ; j < n_defaults ; ++j )
	{
		const char *var=defaults[3*j] ;
		if( var[0] == '[' )
		{
			sprintf( section, "#%s", var+1 ) ;
			char *ptr = strchr( section, ']' ) ;
			ASSERT( ptr ) ;
			*ptr   = 0 ;
			ignore = 0 ;
			if( installNewSectionsOnly )
			{
				sENV_SECTION *dummySect ;
				if( getGetSection( section, &dummySect) == 0 )
					ignore = 1 ;
			}
			continue ;
		}

		if( !ignore )
		{
			char *val = get( section, var ) ;
			if( val == NULL )
				set( section, var, defaults[3*j+1] ) ;
		}
	}
	if( isChanged() )
		save( 1 ) ;
}
