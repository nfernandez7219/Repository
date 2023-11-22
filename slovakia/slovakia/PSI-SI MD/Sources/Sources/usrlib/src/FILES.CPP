#include "tools2.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define SCRTRACE
#include "ScrTrace.h"


#include <direct.h>
#ifdef __BORLANDC__
#include <dir.h>
#include <utime.h>
#endif
#include <errno.h>
#include <sys/stat.h>
#ifdef    _MSC_VER
#include <sys\utime.h>
#include <io.h>
#include <share.h>
#endif


BOOL Z_USERDLL isDirEmpty( const char *path )        // -> 0-nonempty, 1-empty, ERR_NOSUCHDIR
{
    char   cwd[ 256], fmask[ 256] ;
    const char *dir = path ? path : getcwd( cwd, 200 ) ;

    if( !dirExist( dir) )
        return  ERR_NOSUCHDIR ;

    #define PROPER_DIR( name)    ( (name)[0] != '.'  ||  ((name)[1] != 0  &&  ((name)[1] !='.' || (name)[2]!=0)) )
    _makepath( fmask, NULL, dir, "*", NULL ) ;

    #ifdef _MSC_VER

        struct _finddata_t desc ;
        long   handle = _findfirst( fmask, &desc ) ;
        if( handle == -1 )
            return  ERR_NOSUCHDIR ;

			do
            {
                if( PROPER_DIR( desc.name) )
				{
			        _findclose( handle ) ;
                    return  0 ;
				}
            }
            while( _findnext( handle, &desc) == 0 ) ;

        _findclose( handle ) ;
        return  1 ;

    #else

        struct ffblk desc ;
        if( findfirst( fmask, &desc, 0) == 0 )  // if file found
        {
            do
            {
                if( PROPER_DIR( desc.ff_name) )
                    return  0 ;
            }
            while( findnext( &desc) == 0 ) ;
        }
        return  1 ;
    #endif
}

static int rmWholeDirRecur( const char *pathname )
{
    char mypath[256] ;
    char **files=NULL ;
    int  n_files= 0   ; 

    createFileList( pathname , NULL , A_PROPERDIRS , NULL , &files , &n_files ) ;
     int i;
    for( i = 0 ; i < n_files ; i++ )
	{
	    concat( mypath , pathname , "\\" , files[i] , NULL ) ;
        int retval = rmWholeDirRecur( mypath ) ;
		if( retval )
			return retval ;
	}
    freeStringList( (const char **)files ) ;

    files=NULL ;
    n_files= 0   ; 

    createFileList( pathname , NULL , A_FILES , NULL , &files , &n_files ) ;
    for( i = 0 ; i < n_files ; i++ )
	{
	    concat( mypath , pathname , "\\" , files[i] , NULL ) ;
        int retval = unlink( mypath ) ;
		if( retval )
			return retval ;
	}
    freeStringList( (const char **)files ) ;
	delDir( pathname ) ;

	return 0 ;
}

BOOL Z_USERDLL rmWholeDir( const char *pathname )
{
	int ret = rmWholeDirRecur( pathname ) ;
	return ret == 0 ;
}



time_t Z_USERDLL setFileTime( const char *filename, time_t t )		// -> time really set (NT: must be even)
{

	#ifdef      _MSC_VER

	struct _utimbuf  utb ;
	utb.actime = utb.modtime = t ;
	if( _utime( filename, &utb) != 0 )
		_utime( filename, &utb ) ;

	#else

	struct utimbuf  utb ;
	utb.actime = utb.modtime = t ;
	if( utime( filename, &utb) != 0 )
		utime( filename, &utb ) ;

	#endif

	return  fileTime( filename ) ;
}


int Z_USERDLL copyFile( const char *srcFile, const char *dstFile, BOOL createDirs )          // -> 0, ERR_READFILE, ERR_WRITEFILE
{
    if( !srcFile  ||  !dstFile  ||  !*srcFile  ||  !*dstFile )
        return  0 ;
    char absDstPath[256] ;
    _fullpath( absDstPath, dstFile, sizeof(absDstPath)-1 ) ;
    char absSrcPath[256] ;
    _fullpath( absSrcPath, srcFile, sizeof(absSrcPath)-1 ) ;
    if( !stricmp( absSrcPath, absDstPath) )
        return  0 ;

    time_t fTime   = fileTime( srcFile );    
    struct utimbuf uTime={ fTime, fTime } ;
    FILE *src = fopenRetry( srcFile, "rb" );

    if( !src )
        return  ERR_OPENFILE ;
    if( createDirs )
    {
        
    }
    FILE *dst = fopenRetry( dstFile, "wb" ) ;
    if( !dst )
    {
        if( createDirs )
        {
            char drive[_MAX_DRIVE] ;
            char dir[_MAX_DIR] ;
            char path[_MAX_PATH] ;
            _splitpath( dstFile , drive , dir , NULL , NULL ) ;  
            _makepath( path , drive , dir , NULL ,NULL ) ;
            if( !dirExist(path) )
                makeDir( path ) ;
            dst = fopenRetry( dstFile, "wb" ) ;
        }
        if( !dst )
        {
            fclose( src ) ;
            return  ERR_WRITEFILE ;
        }
    }

    char buf[512] ;
    int  cnt, err=0 ;
    while( (cnt = fread( buf, 1, 512, src)) != 0 )
        fwrite( buf, 1, cnt, dst ) ;

    if( ferror( src) )
        err = ERR_READFILE ;
    else
    if( ferror( dst) )
        err = ERR_WRITEFILE ;

    fclose( src ) ;
    fclose( dst ) ;
    
    setFileTime( dstFile, fTime ) ;
    return  err ;
}

int Z_USERDLL delFile( const char *filename )       //  -> 0-OK, ERR_NOACCESS
{
	if( !filename )
		return  0 ;
	if( remove( filename) == 0 )
		return 0 ;
	if( errno == ENOENT )		// not exist or directory
		return 0 ;

	// No access; try to remove readonly attribute and retry delete
	DWORD atr = GetFileAttributes( filename ) ;
	if( (atr != -1)  &&  (atr & FILE_ATTRIBUTE_READONLY) )
	{
		if( SetFileAttributes( filename, atr & ~FILE_ATTRIBUTE_READONLY) )
			if( remove( filename) == 0 )
				return 0 ;
	}
	return ERR_NOACCESS ;
}

/*
int Z_USERDLL delFile( const char *filename )       //  -> 0-OK, ERR_NOACCESS
{
	if( !filename )
		return  0 ;

	clearReadonlyFileAttribute( filename ) ;
	if( remove( filename) )
		return  errno == ENOENT ? 0 : ERR_NOACCESS ;
	else
		return  0 ;
}
*/

static void make_correct_dirname( char *dst, const char *src )
{
    int  len ;
    strcpy( dst, src ) ;
    len = strlen( dst )   ;

     if( len > 0 )
        if( dst[ --len] == '/'  ||  dst[ len] == '\\' )     /* path may end with '/' */
            dst[ len] = '\0' ;
}


int Z_USERDLL delDir( const char *dir )         // -> 0-OK, ERR_NOACCESS, ERR_DIRNOTEMPTY
{
    char  dirname[ 256] ;

    if( !dir )  return  0 ;
    make_correct_dirname( dirname, dir ) ;

    if( rmdir( dirname) == 0 )
          return  0 ;                 // OK
     else
     if( !dirExist( dirname) )
          return  0 ;                 // OK
     else
     if( isDirEmpty( dirname) )
          return  ERR_NOACCESS ;
     else
          return  ERR_DIRNOTEMPTY ;
}


int Z_USERDLL makeDir( const char *dir )        // -> 0-OK, ERR_MAKEDIR
{
    if( !dir  ||  !*dir )  return  0 ;

    char  dirname[ 256] ;
    make_correct_dirname( dirname, dir ) ;
    if( dirExist( dirname) )  return  0 ;

    char kod, *end, *start = dirname ;
    char *column = strchr( dirname, ':' ) ;
    if( column )
        start = column + 1 ;
    while( *start )
    {
        if( (end = strpbrk( start+1, "/\\" )) != NULL )
        {
            kod = *end ;
            *end = 0 ;
        }
        if( !dirExist( dirname) )
            if( mkdir( dirname) != 0 )
                return  ERR_MAKEDIR ;

        if( !end )  break ;
        *end  = kod ;
        start = end ;
    }
    return  0 ;
}


BOOL Z_USERDLL dirExist( const char *dir_name )
{
    if( !dir_name  ||  !*dir_name )
        return  0 ;
	DWORD attr = GetFileAttributes( dir_name ) ;
	if( attr == -1 )
		return FALSE ;
	return attr & FILE_ATTRIBUTE_DIRECTORY ;
}

/*
BOOL Z_USERDLL dirExist( const char *dir_name )
{
    if( !dir_name  ||  !*dir_name )
        return  0 ;

    char buf[ 256] ;
    const char *path = dir_name ;
    int   len  = strlen( path ) ;
    if( path[ --len] == '\\'  ||  path[len] == '/' )
    {
        strcpy( buf, path ) ;
        buf[ len  ] = '.' ;
        buf[ len+1] = 0 ;
        path = buf ;
    }

    if( !fileExist( path) )
        return  0 ;

    struct stat st ;
    stat( path, &st ) ;
    return  st.st_mode & S_IFDIR ;
}
*/

time_t Z_USERDLL fileTime( const char *filename )
{
    struct stat st ;
    return  stat( filename, &st) ? 0L : st.st_mtime ;
}


long Z_USERDLL fileSize( const char *filename )
{
    struct stat st ;
    return  stat( filename, &st) ? 0L : st.st_size ;
}

// diskName is name of driver( 0-current, 'A', 'B', ...)
// function returns -1 on error, or free disk space in KB
long Z_USERDLL getDiskFreeSpace( char diskName )
{
#ifdef      _MSC_VER
      DWORD       sec_per_clast,
                  bytes_per_sec,
                  free_clast,
                  total_clast ;
	  char        *disk, buf[10] ;

	  if( diskName != 0 )
	  {
		  strcpy( buf, "a:\\" ) ;
		  buf[0] = diskName ;
		  disk = buf ;
	  }
	  else
		  disk = NULL ;

      if( GetDiskFreeSpace( disk, &sec_per_clast, &bytes_per_sec,
													&free_clast, &total_clast) )
			return (long)((free_clast*bytes_per_sec*sec_per_clast) /1024) ;

#else
      struct  dfree free;

      getdfree( (unsigned char)( (!diskName)?0:(_toupper(diskName)-'A'+1)), &free) ;

	  if(free.df_sclus != 0xFFFF)
            return  (long)((free.df_avail * free.df_bsec * free.df_sclus)/1024) ;
#endif
      return -1L;
}


BOOL Z_USERDLL makeBakFile( const char *srcFile, const char *bakExt, char *BakFile )
{
	char bakFile[512], drive[10], dir[512], file[30] ;
	_splitpath( srcFile, drive, dir, file, NULL ) ;

	_makepath ( bakFile, drive, dir, file, bakExt ? bakExt : "bak" ) ;
	if( fileExist(bakFile) )
	{
		for( int i=1 ; ; i++ )
		{
			char  buf[1024] ;
			sprintf( buf, "%s~%d", bakFile, i ) ;
			if( !fileExist(buf) )
			{
				strcpy( bakFile, buf ) ;
				break ;
			}
		}
	}

	if( BakFile != NULL )
		strcpy( BakFile, bakFile ) ;
	return copyFile  ( srcFile, bakFile) == 0 ;
}


inline int isAbsPath(const char *path) { return strchr(path, ':') != NULL || path[0] == '\\' || path[0] == '/'; }

char * Z_USERDLL getFullExePath( const char *exeFile, char *path )	// -> path if exe found; otherwise NULL
{
	if( isAbsPath(exeFile) )
	{
		if( !fileExist(exeFile) ) 
			return  NULL ;
		
		strcpy( path, exeFile ) ;
		return  path ;
	}
		
	if( fileExist(exeFile) ) 
		return  _fullpath( path, exeFile, _MAX_PATH ) ;

	char *envPath = getenv( "path" ) ;
	if( envPath != NULL )
	{
		char buf[2048] ;
		strncpy( buf, envPath, sizeof(buf)-1 ) ;
		buf[sizeof(buf)-1] = 0 ;
		for( char *dir=strtok(buf," ;") ; dir != NULL ; dir=strtok(NULL," ;") )
		{
			char fullPath[_MAX_PATH] ;
			_makepath( fullPath, NULL, dir, exeFile, NULL ) ;
			if( fileExist(fullPath) )
			{
				strcpy( path, fullPath ) ;
				return  path ;
			}
		}
	}
	return  NULL ;
}



// err_code (if not NULL) = 0, ERR_OPENFILE, ERR_NOSUCHFILE

const char *getFileDir( const char *fileName, char *buf )
{
	char dir[1024], drive[80] ;
	_splitpath( fileName, drive, dir, NULL, NULL ) ;
	_makepath( buf, drive, dir, NULL, NULL ) ;
	return buf ;
}


BOOL clearReadonlyFileAttribute( const char *fileName )
{
	DWORD atr = GetFileAttributes( fileName ) ;
	if( atr == -1 )
		return FALSE ;
	if( atr & FILE_ATTRIBUTE_READONLY )
		return  SetFileAttributes( fileName, atr & ~FILE_ATTRIBUTE_READONLY ) ;
	return TRUE ;
}


FILE* Z_USERDLL fopenRetry( const char *filename, const char *mode, uint share, int *err_code )
{
    #define MAX_LOOPS    3
    int  err = 0 ;
	int  shflag = _SH_DENYRW ;

	switch ( share )
	{
		case sFILESHARE_READ:		shflag = _SH_DENYWR ; break ;
		case sFILESHARE_WRITE:		shflag = _SH_DENYRD ; break ;
		case sFILESHARE_READWRITE:	shflag = _SH_DENYNO ; break ;
	}

    FILE *fp = _fsopen( filename, mode, shflag ) ;
	if( fp != NULL )
	{
	    if( err_code )
		    *err_code = 0 ;
		return fp ;
	}

	BOOL readonly ;
	if( mode[0] == 'r' )
	{
		char m = mode[1] ;
		readonly = ( m==0 || ( mode[2]==0  &&  (m=='t' || m=='b') ) ) ;
	}
	else
		readonly = FALSE ;

	BOOL file_exist = TRUE ;
	switch( errno )
	{
		case ENOENT :
			if( readonly )
			{
				if( err_code )
					*err_code = ERR_NOSUCHFILE ;
				return NULL ;
			}
			file_exist = FALSE ;
			break ;

		case EACCES :	
			if( !readonly )
				clearReadonlyFileAttribute( filename ) ;
			break ;

		case EMFILE :	// too many open files
		case EINVAL:	// invalid flags
		case EEXIST:	// not existing file required, but it exist already
		    if( err_code )
				*err_code = ERR_OPENFILE ;
			return NULL ;
	}

	if( !file_exist )
	{
		char dir[1024] ;
		getFileDir( filename, dir ) ;
		if( !dirExist(dir) )
			makeDir( dir ) ;
		fp = _fsopen( filename, mode, shflag ) ;
	}

    if( !fp )
    {
        if( file_exist )
        {
            for( int j=0 ; j < MAX_LOOPS ; ++j )
            {
                delay( 150 ) ;
                if( (fp = _fsopen( filename, mode, shflag)) != NULL )
                    break ;
				if( GetLastError() == ERROR_ACCESS_DENIED )
					break ;
            }
            if( !fp )
				err = ERR_OPENFILE ;
        }
        else
            err = ERR_NOSUCHFILE ;
    }
    if( err_code )
        *err_code = err ;
    return  fp ;
}


/*------------------------------------------------------------------*/


static BOOL copyDirRec( const char *srcDir, const char *dstDir, BOOL delSrcDir = FALSE )
{
	char _srcPath[1024], _dstPath[1024];
	char **files=NULL;
	int  n_files= 0, retval;

	if( !dirExist( dstDir ) )
		if( makeDir( dstDir ) != 0 )
			return FALSE;

	createFileList( srcDir, NULL , A_PROPERDIRS , NULL , &files , &n_files );
	for( int i = 0; i < n_files; i++ )
	{
		concat( _srcPath, srcDir, "\\", files[i], NULL );
		concat( _dstPath, dstDir, "\\", files[i], NULL );
		if( !copyDirRec( _srcPath, _dstPath, delSrcDir ) )
		{
			freeStringList( (const char **)files );
			return FALSE;
		}
	}
	freeStringList( (const char **)files );

    files=NULL;
    n_files= 0; 
    createFileList( srcDir, NULL, A_FILES, NULL, &files, &n_files );
    for( i = 0; i < n_files; i++ )
	{
		concat( _srcPath, srcDir, "\\", files[i], NULL );
		concat( _dstPath, dstDir, "\\", files[i], NULL );
		if( delSrcDir )
			retval = rename( _srcPath, _dstPath );
		else
			retval = copyFile( _srcPath, _dstPath );
		if( retval != 0 )
		{
			freeStringList( (const char **)files );
			return FALSE;
		}
	}
    freeStringList( (const char **)files ) ;

	if( delSrcDir )
		if( delDir( srcDir ) == 0 )
			return TRUE;
		else
			return FALSE;

	return TRUE;
}


BOOL Z_USERDLL copyDir( const char *srcDir, const char *dstDir, BOOL delSrcDir )
{
	if( !dirExist( srcDir ) )
		return FALSE;

	return copyDirRec( srcDir, dstDir, delSrcDir );
}


BOOL Z_USERDLL renameDir( const char *srcDir, const char *dstDir, BOOL moveIfDstExist )
{
	if( !dirExist( srcDir ) )
		return FALSE;

	if( dirExist( dstDir ) )
	{
		if( moveIfDstExist )
			return copyDir( srcDir, dstDir, TRUE );
		else
			return FALSE;
	}

	if( rename( srcDir, dstDir ) != 0 )
			return FALSE;

	return TRUE;
}

BOOL Z_USERDLL getFileVersion( char *filename, char *version, int type )
{
	ASSERT( version );
	version[0] = 0;

	BOOL	ret = FALSE;
	DWORD	verInfoSize;
	DWORD	dummy;

	char buf[1024] ;
	if( filename == NULL )
	{
		GetModuleFileName( NULL, buf, sizeof(buf) ) ;
		filename = buf ;
	}

	verInfoSize = GetFileVersionInfoSize( filename, &dummy );
	if ( verInfoSize )
	{
		char	*verInfo;
		HANDLE	 mem;

		mem = GlobalAlloc( GMEM_MOVEABLE, verInfoSize );
		verInfo = (char*)GlobalLock( mem );

		if ( GetFileVersionInfo( filename, 0, verInfoSize, verInfo ) )
		{
			BOOL				 result;
			VS_FIXEDFILEINFO	*value;
			UINT				 valueLen;

			result = VerQueryValue( verInfo, "\\", (void**)&value, &valueLen );
			if ( result && value && valueLen )
			{
				switch(type)
				{
					case FULL_VERSION:
						sprintf( version, "%d.%02d.%02d.%02d",
							value->dwFileVersionMS >> 16,
							value->dwFileVersionMS & 0xffff,
							value->dwFileVersionLS >> 16,
							value->dwFileVersionLS & 0xffff );
						break;
					case UPPER_VERSION:
						sprintf( version, "%d.%02d",
							value->dwFileVersionMS >> 16,
							value->dwFileVersionMS & 0xffff );
						break;
				}
				ret = TRUE;
			}
		}

		GlobalUnlock( mem );
		GlobalFree( mem );
	}

	return ret;
}
