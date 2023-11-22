#include "tools2.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define SCRTRACE
#include "ScrTrace.h"

#include	<ctype.h>
#include	<direct.h>


#define OPTOPTIONS	"agt"
//#define OPTOPTIONS	"agty"


static void sStringPtrListDelFun( const char **str )
{
	if( str != NULL  &&  *str != 0 )
		FREE( (void*)*str ) ;
}

sStringPtrArray::sStringPtrArray( ) : sTemplateArray<const char*>( sStringPtrListDelFun)
{
	sTemplateArray<const char*>::add( NULL ) ;
}

sStringPtrArray::sStringPtrArray( const sStringPtrArray &src ) : sTemplateArray<const char*>( sStringPtrListDelFun)
{
	_addStringList( &src ) ;
}

// {//}{/wrd/}{/wrd/}{/wrd/}...
void sStringPtrArray::asText( char *buf, int max_chars ) const
{
	strcpy( buf, "{//}" ) ;
	int n = 4 ;
	for( int j=0 ; j < siz-1 ; ++j )
	{
		const char *w = item(j) ;
		if( max_chars > 0 )
		{
			int cnt = max_chars - (n+4+strlen(w)) ;
			const char *s = w ;
			while( (s = strchr(s,'/')) != NULL )
				cnt--, s++ ;
			if( cnt <= 0 )
				continue ;
		}
		buf[n++] = '{' ;
		buf[n++] = '/' ;
		for( ; *w; w++ )
		{
			if( *w == '/' )
			{
				buf[n++] = '/' ;
				buf[n++] = '/' ;
			}
			else
				buf[n++] = *w ;
		}
		buf[n++] = '/' ;
		buf[n++] = '}' ;
	}
	buf[n] = 0 ;
}

void sStringPtrArray::fromText( const char *buf )
{
	char *tmp = STRDUP(buf) ;
	clearList( ) ;
	if( strncmp( buf, "{//}", 4) != 0 )
	{
		const char *wrd = strtok( tmp, " \t\n" ) ;
		while( wrd )
		{
			add( wrd ) ;
			wrd = strtok( NULL, " \t\n" ) ;
		}
	}

	buf += 4 ;
	while( (buf = strstr( buf, "{/")) != NULL )
	{
		const char *src = buf+2 ;
		char *dst = tmp ;
		while( 1 )
		{
			if( *src == '/' )
			{
				if( src[1] == '/' )
				{
					*dst++ = '/' ;
					src += 2 ;
				}
				else
				if( src[1] == '}' )
				{
					*dst = 0 ;
					add( tmp ) ;
					buf = src+2 ;
					break ;
				}
				else
					*dst++ = *src++ ;
			}
			else
			if( *src == 0 )
				break ;
			else
				*dst++ = *src++ ;
		}
	}

	FREE( tmp ) ;
}

void sStringPtrArray::_addStringList( const sStringPtrArray *src )
{
	if( count() < 0 )
		sTemplateArray<const char*>::add( NULL ) ;
	for( int j=0 ; j < src->count() ; ++j )
		add( src->item(j) ) ;
}

void sStringPtrArray::addStrings( const char **names, int n_names )
{
	if( names == NULL )
		return ;
	if( n_names <= 0 )
	{
		for( n_names=0 ; names[n_names] != NULL ; ++n_names ) ;
	}
	for( int j=0 ; j < n_names ; ++j )
		add( names[j] ) ;
}

int sStringPtrArray::find( const char *str ) const
{
	for( int j=0 ; j < count() ; ++j )
		if( stricmp( item(j), str) == 0 )
			return  j ;
	return  -1 ;
}

int sStringPtrArray::findEqualText( const char *str ) const 	// via textEqual()
{
	for( int j=0 ; j < count() ; ++j )
		if( textEqual( item(j), str) )
			return  j ;
	return  -1 ;
}

int sStringPtrArray::operator== (  const sStringPtrArray &src ) const
{
	if( count() != src.count() )
		return FALSE ;
	for( int j=0 ; j < count() ; ++j )
		if( !textEqual( item(j), src.item(j)) )
			return FALSE ;
	return TRUE ;
}


BOOL sStringPtrArray::addNewOnly( const char *str )
{
	if( findEqualText(str) >= 0 )
		return FALSE ;
	add( str ) ;
	return TRUE ;
}

int sStringPtrArray::addNewOnly( const sStringPtrArray &src )
{
	int cnt=0 ;
	for( int j=0 ; j < src.count() ; ++j )
	{
		const char *str = src.item(j) ;
		if( findEqualText(str) < 0 )
		{
			cnt++ ;
			add( str ) ;
		}
	}
	return cnt ;
}


// binary format
// cnt
// cnt + checked (0/1)
// crc
// string1\0string2\0...

size_t sStringPtrArray::binarySize( )
{
	size_t len = 2*( sizeof(size_t) + sizeof(ushort)) ;
	for( int j=0 ; j < count() ; ++j )
		len += strlen( item(j)) + 1 ;
	return len ;
}

char * sStringPtrArray::saveToBin ( char *bin, BOOL checked )
{
	if( checked != 0 )
		checked = 1 ;
	*(size_t*)bin = count()           ;  bin += sizeof(size_t) ;
	*(ushort*)bin = count() + checked ;  bin += sizeof(ushort) ;
	size_t   *len = (size_t*)bin      ;  bin += sizeof(size_t) ;
	ushort   *crc = (ushort*)bin      ;  bin += sizeof(ushort) ;

	char *binData = bin ;
	for( int j=0 ; j < count() ; ++j )
	{
		const char *name = item(j) ;
		size_t itemLen = strlen( name)+1 ;
		memcpy( bin, name, itemLen ) ;
		bin += itemLen ;
	}
	*len = bin - binData ;
	if( checked  &&  *len > 0 )
		*crc = crc16( (char*)(crc+1), *len ) ;
	else
		*crc = 0 ;
	return bin ;
}

BOOL sStringPtrArray::loadFromBin ( char *bin, BOOL clearOldContents )
{
	if( clearOldContents )
		clearList( ) ;

	size_t cnt     = *(size_t*)bin       ;  bin += sizeof(size_t) ;
	BOOL   checked = *(ushort*)bin - cnt ;  bin += sizeof(ushort) ;
	size_t len     = *(size_t*)bin       ;  bin += sizeof(size_t) ;
	ushort crc     = *(ushort*)bin       ;  bin += sizeof(ushort) ;

	if( checked != 0  &&  checked != 1 )
		return  FALSE ;
	if( checked  &&  len > 0 )
		if( crc != crc16( bin, len) )
			return  FALSE ;
	for( int j=0 ; j < (int)cnt ; ++j )
	{
		add( bin ) ;
		bin += strlen( bin)+1 ;
	}
	return  TRUE ;
}



sFileNameArray::sFileNameArray( const char *dirName, const char *mask, int mode, int (*f)( const char*, long), long lParam )
{
	ASSERT( dirName != NULL  &&  dirName[0] != 0 ) ;

	strcpy( _dirName, dirName ) ;
	_dirLen  = strlen( _dirName) ;
	if( _dirName[_dirLen-1] != '\\' )
	{
		if( _dirName[_dirLen-1] == '/' )
			_dirName[--_dirLen] = 0 ;
		_dirName[_dirLen++] = '\\' ;
	}

	// dirExist() is very slow!
	//if( !dirExist(dirName) )
	//	return ;

	char **names ;
	int  n_names ;
	int err = createFileList( dirName, mask, mode, f, &names, &n_names, lParam ) ;
	if( err != 0 )
	{
		// Only ERR_NOSUCHDIR possible
		return ;
		/*
		if( err == ERR_NOSUCHDIR )
			return ;
		sTemplateArray<const char*>::clearList( ) ;
		throw ScrMsg( err, dirName ) ;
		*/
	}
	if( n_names > 0 )
	{
		const char *lastStr = last() ;
		last() = names[0] ;
		for( int j=1 ; j < n_names ; ++j )
			sTemplateArray<const char*>::add( (const char*)(names[j]) ) ;
		sTemplateArray<const char*>::add( lastStr ) ;
	}
	FREE( names ) ;
}

const char *sFileNameArray::fullName( int j )
{
	ASSERT( j < count() ) ;
	strcpy( _dirName+_dirLen, item(j) ) ;
	return _dirName ;
}


#ifdef	_MSC_VER
#define	ffblk	_finddata_t
#endif


#pragma optimize( OPTOPTIONS, on )			// optimize in respect of time

#ifdef	_MSC_VER
static int open_dir( const char *path, const char *mask, struct ffblk *desc, long *ffh )
#else
static int open_dir( const char *path, const char *mask, struct ffblk *desc, int mode )
#endif
{
	char   cwd[1024], fmask[1024] ;
	const char *dir = path ?  path  :  getcwd( cwd, 1000 ) ;

	if( !mask  &&  dirExist( dir) )
		mask = "*" ;
	if( mask )
    {
		_makepath( fmask, NULL, dir, mask, NULL ) ;
        mask = fmask ;
    }
    else
        mask = dir ;

    #ifdef	_MSC_VER
    	if((*ffh = _findfirst( mask, desc)) == -1)
    #else
    	if(findfirst( mask, desc, mode) )
    #endif
		return  dirExist( dir) ? 0 : ERR_NOSUCHDIR ;

	return  1 ;
}

#pragma optimize( OPTOPTIONS, on )			// optimize in respect of time


#define ALLOC_INCR	20

// 0, ERR_NOSUCHDIR
int Z_USERDLL createFileList( const char *path, const char *mask, int mode,
		int (*filter)( const char *, long lParam), char ***files, int *n_files, long lParam )
{
	char	**names= NULL ;
	int 	alloc_names=0, err, n_names=0 ;
	int     files_only = ( (mode & A_FILES)  &&  !(mode & A_DIRS) ) ;
	int     dirs_only  = ( (mode & A_DIRS )  &&  !(mode & A_FILES) ) ;
	int		proper_dirs= ( (mode & A_PROPERDIRS) == A_PROPERDIRS ) ;
	struct ffblk desc ;

	if(   files )   *files = NULL ;
	if( n_files ) *n_files = 0 ;
	if( !mode   )    mode  = A_FILES ;

    #define PROPER_DIR( name)    ( (name)[0] != '.'  ||  ((name)[1] != 0  &&  ((name)[1] !='.' || (name)[2]!=0)) )
#ifdef	_MSC_VER
    long  ffh ;
	err = open_dir( path, mask, &desc, &ffh) ;
	if( err <= 0 )
        return  err ;	// 0, ERR_NOSUCHDIR

	do
	{
        char *name = desc.name ;
		if( desc.attrib & _A_SUBDIR )
#else
    if( mode & A_DIRS )
        mode = FA_DIREC ;
    else
        mode = 0 ;
    mode |= FA_RDONLY ;
	err = open_dir( path, mask, &desc, mode ) ;
	if( err <= 0 )
        return  err ;	// 0, ERR_NOSUCHDIR

	do
	{
        char *name = desc.ff_name ;
		if( desc.ff_attrib & FA_DIREC )
#endif
		{
			if( files_only )  continue ;
			if( proper_dirs )               		// exclude "."  &  ".."
                if( !PROPER_DIR( name) )
                    continue ;
		}
		else
		if( dirs_only )  continue ;

		if( !filter  ||  (*filter)(name,lParam) )
		{
			if( files != NULL )
			{
				if( n_names >= alloc_names-1 )
				{
					char **realloc_names = (char **)REALLOC
						(names, (alloc_names += ALLOC_INCR)*sizeof(char*) ) ;
					if( !realloc_names )
						goto LabelAllocError ;
					names = realloc_names ;
				}
				if( (names[ n_names] = STRDUP( name)) == NULL )
					goto LabelAllocError ;
			}
			++n_names ;
		}
	}
#ifdef	_MSC_VER
	while( _findnext( ffh, &desc) == 0 ) ;
	_findclose( ffh ) ;
#else
	while( findnext( &desc) == 0 ) ;
#endif

  LabelAllocError :		// allocation error ignored
	if( files )
	{
		sortStringList( (const char **)names, n_names ) ;
		*files = names ;
		if( n_names )
			names[ n_names] = NULL ;
	}
	if( n_files )
		*n_files = n_names ;
	return  0 ;
  /*
  LabelAllocError :
	if( names )
		freeStringList( (const char **)names, n_names ) ;
	return  ERR_ALLOCERROR ;
  */
}


//------------------------------------------------------------------------
//		drive list
//------------------------------------------------------------------------


int Z_USERDLL getDriveList( char ***drives, int *n_drives, uint type )
{
    char **names = (char**)CALLOC( 30, sizeof(char*)+4 ) ;
    int  n_names=0 ;
    #define  DRIVE_NAME(j) ((char*)(names+30+j))
    #define  ADD_DRIVE( j) sprintf( names[n_names] = DRIVE_NAME(n_names), "%c:", (char)((j) + 64) ), n_names++

//    #ifdef _MSC_VER
      DWORD drivesMap = 0 ;
      DWORD drivesMask = 1 ;
      drivesMap = GetLogicalDrives() ;

    for( int j=1 ; j < 27 ; j++ )
    {
        if( drivesMap & drivesMask  )
        {
			char  buf[10] ;
			strcpy( buf, "a:\\" ) ;
			buf[0] = (int)(64 + j) ;

			int  ok = 0 ;
			switch( GetDriveType( buf) )
			{
				case DRIVE_REMOVABLE :  ok = type & sDRIVE_REMOVABLE ;  break ;
				case DRIVE_FIXED     :  ok = type & sDRIVE_FIXED     ;  break ;
				case DRIVE_REMOTE    :  ok = type & sDRIVE_REMOTE    ;  break ;
				case DRIVE_CDROM     :  ok = type & sDRIVE_CDROM     ;  break ;
				case DRIVE_RAMDISK   :  ok = type & sDRIVE_RAMDISK   ;  break ;
			}
			if( ok )  ADD_DRIVE( j ) ;
        }
        drivesMask <<= 1 ;
	}
    if(   drives )    *drives =   names ;
    if( n_drives )  *n_drives = n_names ;
    return  0 ;
}


void Z_USERDLL destroyDriveList( char **drives )
{
    FREE( drives ) ;
}
