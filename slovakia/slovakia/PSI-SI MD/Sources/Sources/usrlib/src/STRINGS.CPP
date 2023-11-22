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

//#define SCRTRACE
#include "ScrTrace.h"

#include <ctype.h>


char* Z_USERDLL stristr( const char *string, const char *subString )
{
	char  str[1024], sub[1024] ;
	strncpy( str, string, sizeof(str)-1 ) ;
	str[ sizeof(str)-1] = 0 ;
	strupr( str ) ;
	strncpy( sub, subString, sizeof(sub)-1 ) ;
	sub[ sizeof(sub)-1] = 0 ;
	strupr( sub ) ;
	char *s = strstr( str, sub ) ;
	if( s != NULL )
		return  (char *)(string + (s-str)) ;
	else
		return  NULL ;
}



void Z_USERDLL cstringToString( const char *Cstring, char *string )
{
    while( *Cstring )
    {
        if( *Cstring == '\\' )
        {
            Cstring ++ ;
            if( *Cstring == 'n' )
            {
                *string++ = '\r' ;
                *string++ = '\n' ;
                //*string++ = '\n' ;
                Cstring ++ ;
                continue ;
            }
            else
            if( *Cstring == 't' )
            {
                *string++ = '\t' ;
                Cstring ++ ;
                continue ;
            }
            else
            if( *Cstring == 'x'  ||  *Cstring == 'X' )
            {
                sscanf( Cstring+1, "%2x", string++ ) ;
                Cstring += 3 ;
                continue ;
            }
        }
        *string++ = *Cstring++ ;
    }
    *string = '\0' ;
}

/*
void Z_USERDLL stringToCstring( const char *str , char *cstring )
{
   UCHAR* string = (UCHAR*)str;
    for(; *string; string++ )
    {
        if( *string == '\\' )
        {
            *cstring++ = '\\';
            *cstring++ = '\\';
        }
        else if( isprint( *string ) )
            *cstring++=*string;
        else
            switch( *string )
            {
                case '\n' :
                   cstring += sprintf( cstring , "\\n" ) ;
                   break ;
                case '\t' :
                   cstring += sprintf( cstring , "\\t" ) ;
                   break ;
                default :
                   cstring += sprintf( cstring,"\\x%02X", (UINT)*string) ;
                   break ;
	        }
    }
    *cstring = '\0' ;
}
*/

int Z_USERDLL stringToCstring( const char *str , char *cstring )
{
	int  ret=0 ;
	UCHAR* string = (UCHAR*)str;
	for( ; *string ; string++ )
    {
        switch( *string )
        {
            case '\\' :
				*cstring++ = '\\';
				*cstring++ = '\\';
				ret = 1 ;
				break ;
            case '"' :
				*cstring++ = '\\';
				*cstring++ = '"';
				ret = 1 ;
				break ;
            case '\r' :
				*cstring++ = '\\';
				if( string[1] == '\n' )
				{
					*cstring++ = 'n';
					string++ ;
				}
				else
					*cstring++ = 'r';
				ret = 1 ;
				break ;
            case '\n' :
				*cstring++ = '\\';
				*cstring++ = 'n';
				ret = 1 ;
				break ;
            case '\t' :
				*cstring++ = '\\';
				*cstring++ = 't';
				ret = 1 ;
				break ;
            default :
				if( isprint( *string ) )
					*cstring++ = *string;
				else
				{
					ret = 1 ;
					cstring += sprintf( cstring,"\\x%02X", (UINT)*string) ;
				}
				break ;
	    }
    }
    *cstring = '\0' ;
	return  ret ;
}


int Z_USERDLL stringToStringList( const char *string , char ***stringlist , int *n_strings , char *separator ) 
{
	char *token ; 
	int  maxalloc  = 0 ;
    int  allocstep = 32 ;
	if( n_strings )
		*n_strings = 0 ;
	*stringlist = NULL ;
	
	char *stringcopy = STRDUP( string ) ;
	if( stringcopy == NULL )
		return ERR_ALLOCERROR ;
	char **strs = NULL ;
	int  n_strs = 0    ;

	if( separator == NULL )
		separator = " \t" ;

	token = strtok( stringcopy , separator ) ;
	while( token != NULL )
	{
		if(n_strs >= maxalloc-1 )
		{
			maxalloc += allocstep ;
			char **reallocstrs = (char**)REALLOC( strs, maxalloc*sizeof(char*) ) ;
			if( reallocstrs == NULL )
			{
				freeStringList( (const char**)strs, n_strs ) ;
				FREE( stringcopy ) ;
				return ERR_ALLOCERROR ;
			}
		    strs = reallocstrs ;
		}

		strs[n_strs++] = STRDUP( token )  ;

		token = strtok( NULL , separator ) ;
	}
    
	if( strs )
		strs[n_strs] = NULL ;
	FREE( stringcopy ) ;
	if( n_strings )
		*n_strings = n_strs ; 
    *stringlist = strs ;     

	return 0 ;
}

int Z_USERDLL isStrEmpty( const char *s )
{
	for( ; *s ; s++ )
		if( *s > ' ' )
			return FALSE ;
	return TRUE ;
    //return  s[ strspn( s, " \t")] == '\0' ;
}


int Z_USERDLL textEqual( const char *s1, const char *s2 )
{
    const char *ss1, *ss2 ;

	if( s1 == s2 )
		return  1 ;
	if( s1 == NULL  ||  s2 == NULL )
		return  0 ;
    while( *s1  &&  *s1 <= ' ' )  ++s1 ;        /* trim off left spaces */
    while( *s2  &&  *s2 <= ' ' )  ++s2 ;

    if( !*s1 )    return  !*s2 ;                    /* both empty ? */
    if( !*s2 )  return  0 ;
                                                /* trim off right spaces */
    for( ss1= s1 + strlen( s1) ; *(ss1-1) <= ' '; --ss1 ) ;
    for( ss2= s2 + strlen( s2) ; *(ss2-1) <= ' '; --ss2 ) ;

    int len = ss1 - s1 ;
    if( len != (ss2 - s2) )  return  0 ;
    return  !strnicmp( s1, s2, len ) ;
}


/*
 * Functions test if the string represents a number.
 * If so, TRUE is returned and 2nd argument is set to that number.
 * If not (f.e. there are non white chars following the number),
 * FALSE(0) is returned and 2nd argument is unchanged.
 */

#define isNum( funName, type, fmt )                   \
    int Z_USERDLL funName( const char *str, type *x )                 \
    {                                                 \
        int   n_chars ;                               \
        type  xx ;                                    \
        if( sscanf( str, fmt, &xx, &n_chars ) <= 0 )  \
            return  0 ;                               \
        for( str += n_chars ; *str; ++str )           \
            if( *str > ' ' )  return  0 ;             \
        if( x )  *x = xx ;                            \
        return  1 ;                                   \
    }

isNum( isFloat , float , "%f%n"  ) ;
isNum( isDouble, double, "%lf%n" ) ;
isNum( isInt   , int   , "%d%n"  ) ;
isNum( isLong  , long  , "%ld%n" ) ;



/*
 *        Concatenate strings
 *
 *    usage:    concat( dst_string, src_string1, src_string2, ..., NULL )
 *
 *  returns: dst_string
 */

char* Z_USERDLL concat( char *dst,... )
{
  va_list  arg ;
  char     *src ;

  *dst = '\0' ;
  va_start( arg, dst ) ;
                                    /* loop while next arg != NULL */
  while( src = va_arg(arg, char *) )
    strcat( dst, src ) ;
  return dst ;
}


/*** string list ***/

void Z_USERDLL freeStringList( const char **files, int n_files )
{
  if( files )
  {
    if( n_files < 0 )               // NULL-terminated list
        for( n_files=0 ; files[n_files] ; n_files++ ) ;

    while( n_files )  FREE( (char*)files[ --n_files] ) ;
    FREE( files ) ;
  }
}


static int _USERENTRY __cmp( const void *elem1, const void *elem2 )
{
  return  stricmp( (char *)*(char **)elem1, (char *)*(char **)elem2 ) ;
}


void Z_USERDLL sortStringList( const char **files, int n_files )
{
    if( files )
    {
        if( n_files < 0 )               // NULL-terminated list
            for( n_files=0 ; files[n_files] ; n_files++ ) ;
        if( n_files )
            qsort( (void *)files, (size_t)n_files, sizeof( void * ), __cmp ) ;
    }
}


char Z_USERDLL **dupStringList( const char **files, int n_files )
{
    if( files == NULL )
        return  NULL ;

    if( n_files < 0 )               // NULL-terminated list
        for( n_files=0 ; files[n_files] ; n_files++ ) ;

	char **strs = NULL ;
    strs = (char **)CALLOC( n_files+1, sizeof(char*) ) ;

    if( strs != NULL )
        for( int j=0 ; j < n_files ; ++j )
            if( files[j] )
                strs[j] = STRDUP( files[j] ) ;
    return  strs ;
}



int  Z_USERDLL isMatch( const char *name, const char *mask )
{
    if( !mask )  return  1 ;
	if( !name )  return  0 ;

    for( ; *name  &&  *mask ; ++mask, ++name )
    {
        if( *mask == '*' )
        {
			if( !*++mask )
				return  1 ;
			for( mask ; *name ; name++ )
				if( isMatch( name, mask) )
					return  1 ;
			return  0 ;
		}
		if( *mask != '?' )
        	if( *name != *mask )
				return  0 ;
    }
	while( *mask  &&  *mask == '*' )  mask++ ;
    return  *mask == *name ;
}


// ----------------------------------------------------------------------------
// Binary convert functions
//---------------------------------------------------------------------------

// Converts a binary string to the form 0xNNNNNNNNNNNNNNNN ...
// buf must be big enough to accept the whole output string (2+2*msize+1)

int Z_USERDLL binaryToString( void *bin, char *buf,int msize )
{
    const char *Hexa ="0123456789ABCDEF";
    char *ctmp = (char *)bin ;
    *buf++ = '0' ;
    *buf++ = 'x' ;
    for( int i=0 ; i < msize ; i++ )
    {
        char b=*ctmp++;
        *buf++=Hexa[(b>>4)&0xF];
        *buf++=Hexa[b&0xF];
    }
    *buf=0;
    return  2 + 2*msize + 1 ;
}


// Converts a string of the form 0xNNNNNNNNNNNNNNNN ... to binary
// returns the size of the binary array created and a pointer to this array
int Z_USERDLL stringToBinary( void *bin, char *buf )
{
  char  b, c ;
  int   i, mode, msize=0 ;
  int   len = strlen(buf)-2 ;
  char *ctmp = (char*)bin ;
  if( *buf++ != '0'  ||  *buf++ != 'x'  ||  (len&1) )  return 0 ;
                        // the count of characters must be even
  mode = 0 ;
  for ( i=0 ; i < len ; i++ )
  {
     b=(unsigned char)(*buf++);
     switch( b )
     {
       case '0': case '1': case '2': case '3': case '4':
       case '5': case '6': case '8': case '9':
           b -= (char)'0';
           break;
       case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
           b -= (char)('A'-10) ;
           break;
       case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
           b -= (char)('a'-10) ;
           break;
       default:
           // a wrong character , whats now ?
           b=0xF;
     }
     mode=!mode;
     if( mode )
        c = (char)( b << 4) ;
     else
     {
        ctmp[i>>1] = c | b ;
        msize++ ;
      }
  }
  return  msize ;
}

// ----------------------------------------------------------------------------


#define TRACE_FILE    "\\trace.out"

/*
 * printf-style function, appending text to the TRACE_FILE
 */

int Z_USERDLL trace_fun( char *s, ... )
{
    static long  first_time=1 ;
    static char  filename[256] ;
	static CRITICAL_SECTION lock;		// synchronization

	const char *mode ;
	if( InterlockedExchange(&first_time,0) )
	{
		InitializeCriticalSection( &lock );
		EnterCriticalSection( &lock );
		char *s = filename ;
		if( GetModuleFileName( NULL, filename, sizeof(filename)-10) != 0 )
		{
			s = strrchr( filename, '\\' ) ;
			if( s == NULL )
			{
				s = strrchr( filename, '/' ) ;
				if( s == NULL )
					s = filename ;
			}
		}
		strcpy( s, TRACE_FILE ) ;
		mode = "w" ;
	}
	else
	{
		EnterCriticalSection( &lock );
		mode = "a" ;
	}

	FILE *fp = NULL ;

	try
	{
		fp = fopen( filename, mode ) ;
		if( fp != NULL )
		{
			va_list  ap ;
			va_start( ap, s ) ;
			vfprintf( fp, s, ap ) ;
		    fclose( fp ) ;
		}
	}
	catch( ... )
	{
		try
		{
			if( fp != NULL )
			{
				fprintf( fp, "\nWRITE EXCEPTION (fmt = %p)", s ) ;
				fprintf( fp, " (%s)", s ) ;
				fclose( fp ) ;
			}
		}
		catch( ... ) {}
	}

	LeaveCriticalSection( &lock );
    return  0 ;
}


void Z_USERDLL removeTabs( char *lin, int tabSize )
{
	char buf[1024] ;
	char *src=lin, *dst=buf ;

	for( ; *src ; ++src )
	{
		if( *src != '\t' )
			*dst++ = *src ;
		else
		{
			int  pos = src - lin ;
			int  cnt = (pos + tabSize) / tabSize *tabSize - pos ;
			for( int j=0 ; j < cnt ; ++j )
				*dst++ = ' ' ;
		}
	}
	*dst = 0 ;
	strcpy( lin, buf ) ;
}

