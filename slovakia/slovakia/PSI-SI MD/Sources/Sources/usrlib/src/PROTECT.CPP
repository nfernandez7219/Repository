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

#include "ProtectedFile.hpp"

#define STAMP		"SF1.0"
#define STAMPSIZE	8

typedef struct
{
	char    stamp[STAMPSIZE] ;
	ushort  crc  ;
	short	pmode;
	time_t	time ;
	char	computer[32] ;
	char	path[128] ;
	char	user[32] ;
}
	HEADER ;

#define HDR(crc)		(((HEADER*)(_hdr))->crc)
#define HDR(stamp)		(((HEADER*)(_hdr))->stamp)
#define HDR(pmode)		(((HEADER*)(_hdr))->pmode)
#define HDR(time)		(((HEADER*)(_hdr))->time)
#define HDR(computer)	(((HEADER*)(_hdr))->computer)
#define HDR(path)		(((HEADER*)(_hdr))->path)
#define HDR(user)		(((HEADER*)(_hdr))->user)

#define ENCRYPT_HEADER		crcEncrypt( (char*)&HDR(crc), sizeof(HEADER)-STAMPSIZE )
#define DECRYPT_HEADER		crcDecrypt( (char*)&HDR(crc), sizeof(HEADER)-STAMPSIZE )
//#define ENCRYPT_HEADER
//#define DECRYPT_HEADER

static BOOL isAsciiString( char *s )
{
	for( ; *s ; s++ )
		if( !isprint(*s) )
			return FALSE ;
	return  TRUE ;
}

ProtectedFile::ProtectedFile( char *filename, int pmode )
{
	strcpy( _filename, filename ) ;
	memset( _hdr, 0, sizeof(HEADER) ) ;
	_fp     = NULL ;
	_pmode  = pmode ;
        _myFmode= F_READ ;
	_hdr[0] = 0 ;
	_pos0   = sizeof(HEADER) ;
	_changed= 0 ;
}


void ProtectedFile::_setVars( int pmode )
{
	time = 0 ;
	computer[0]  = path[0]   = user[0]  = 0 ;

	if( pmode & P_FILETIME )
		time = HDR(time) ;
	if( pmode & P_COMPUTER )
		strncpy( computer, HDR(computer), 32 ) ;
	if( pmode & P_FILEPATH )
		strncpy( path, HDR(path), 128 ) ;
	if( pmode & P_USER )
		strncpy( user, HDR(user), 32 ) ;
	computer[31] = path[127] = user[31] = 0 ;
}

int ProtectedFile::open( Fmode fmode )
{
	int   err=0, virgin=1 ;
	int   exist = fileExist( _filename ) ;
	_changed = 0 ;
	_myFmode = fmode ;
	_ftime   = exist ? fileTime( _filename) : 0 ;

	_setVars( P_NONE ) ;

	if( exist && ( fmode == F_WRITE || fmode == F_MODIFY ) )
	{
		DWORD fattr = GetFileAttributes( _filename ); 
		if( fattr & FILE_ATTRIBUTE_READONLY )
			SetFileAttributes( _filename, fattr ^ FILE_ATTRIBUTE_READONLY ); 
	}

	switch( fmode )
	{
		case F_READ   :
			if( (_fp = fopen( _filename, "rb") ) == NULL )
				err = exist ? ERR_OPENFILE : ERR_NOSUCHFILE ;
			else
				virgin = 0 ;
			break ;
		case F_WRITE  :
			if( (_fp = fopen( _filename, "w+b") ) == NULL )
				err = ERR_OPENFILE ;
			break ;
		case F_MODIFY :
			if( exist )
				virgin = 0 ;
			if( (_fp = fopen( _filename, virgin ? "w+b" : "r+b") ) == NULL )
				err = ERR_OPENFILE ;
			break ;
		default :
			ASSERT( 0 ) ;	// invalid access mode for ProtectedFile
	}
	if( !err )
	{
		if( virgin )
		{
			memset( _hdr, 0, sizeof(HEADER) ) ;
			if( fwrite( _hdr, sizeof(HEADER), 1, _fp) != 1 )
				err = ERR_READFILE ;
			_changed = 1 ;
		}
		else
		{
			if( fread( &_hdr, sizeof(HEADER), 1, _fp) != 1 )
				err = ERR_READFILE ;
			else
			if( strncmp( HDR(stamp), STAMP, strlen(STAMP)) != 0 )
				err = ERR_BADVERSION ;
			else
			{
				DECRYPT_HEADER ;
				_setVars( (Pmode)( HDR(pmode)) ) ;
			}
		}
	}
	if( err )
	{
		if( _fp )  fclose( _fp) ;
		_fp = NULL ;
	}
	else
		fSeek( 0 );

	return  err ;
}


ushort ProtectedFile::_computeCrc( )
{
	char   buf[512] ;
	int    cnt ;
	ushort saved_crc = HDR(crc) ;
	HDR(crc) = 0 ;

	rewind( ) ;
	ushort crc = crc16( _hdr, sizeof(HEADER) ) ;
	while( (cnt = fRead( buf, 1, 512)) > 0 )
		crc = crc16( buf, cnt, crc ) ;
	HDR(crc) = saved_crc ;
	return  crc ;
}


int ProtectedFile::test( )
{
	if( open( F_READ ) )
        return P_ALL;
	int  violated = (_computeCrc() == HDR(crc)) ? 0 : (int)P_DATA ;
	close( ) ;

	int  mode = HDR(pmode) ;
	char buf[256] ;
	if( mode & P_FILEPATH )
	{
		char  absPath[256] ;
		_fullpath( absPath, _filename, 256 ) ;
		char *s = strchr( absPath, '\\' ) ;
		if( !isAsciiString(HDR(path)) )
			violated |= (int)P_DATA ;
		else
		if( stricmp( HDR(path), s) != 0 )
			violated |= P_FILEPATH ;
	}
	if( mode & P_COMPUTER )
	{
		ulong size = 32 ;
		char *pom = HDR(computer) ;
		if( !isAsciiString(HDR(computer)) )
			violated |= (int)P_DATA ;
		else
		if( GetComputerName( buf, &size)  &&
			stricmp( HDR(computer), buf) != 0 )
			violated |= P_COMPUTER ;
	}
	if( mode & P_USER )
	{
		ulong size = 32 ;
		if( !isAsciiString(HDR(user)) )
			violated |= (int)P_DATA ;
		else
		if( GetUserName( buf, &size)  &&
			stricmp( HDR(user), buf) != 0 )
			violated |= P_USER ;
	}

	if( mode & P_FILETIME )
	{
		time_t  t = fileTime(_filename) ;
		HEADER *h = (HEADER*)_hdr ;
		int dif = t - HDR(time) ;
		if( dif != 0  &&  dif != 3600  &&  dif != -3600 )		// daylight savings!
		{
			dif -= _timezone ;
			if( dif != 0  &&  dif != 3600  &&  dif != -3600 )
				violated |= P_FILETIME ;
		}
	}

	return  violated ;
}

void ProtectedFile::close( Cmode mode )
{
	if( !_fp )
		return ;
	if( _myFmode == F_READ )
	{
		fclose( _fp ) ;
		_fp = NULL ;
		return ;
	}

	if( HDR(time) == 0  ||  (changed()  &&  !(mode & C_PRESERVEFILETIME)) )
	{
		HDR(time)  = (::time(NULL) + 1) / 2 * 2 ;		// WINNT needs even # of seconds
		_changed   = 1 ;
	}
	if( changed() )
	{
		HDR(pmode) = _pmode ;
		strcpy( HDR(stamp), STAMP ) ;
		if( _pmode & P_FILEPATH )
		{
			char  absPath[256] ;
			_fullpath( absPath, _filename, 256 ) ;
			char *path = strchr( absPath, '\\' ) ;
			strcpy( HDR(path), path ) ;
		}
		if( _pmode & P_COMPUTER )
		{
			char  buf[MAX_COMPUTERNAME_LENGTH + 1] ;

			ulong size = MAX_COMPUTERNAME_LENGTH ;
			GetComputerName( buf, &size ) ;
			strncpy( HDR(computer), buf, sizeof(HDR(computer)) ) ;
			HDR(computer)[31] = 0 ;
		}
		if( _pmode & P_USER )
		{
			char  buf[32] ;
			ulong size = 31 ;
			if( GetUserName( buf, &size) )
				strcpy( HDR(user), buf ) ;
		}
		
		HDR(crc) = _computeCrc( ) ;
		::rewind( _fp ) ;
		ENCRYPT_HEADER ;
		fwrite( _hdr, sizeof(HEADER), 1, _fp ) ;
		DECRYPT_HEADER ;
		_setVars( _pmode ) ;
	}
	fclose( _fp ) ;
	_fp = NULL ;
	if( changed() )
		setFileTime( _filename, HDR(time) ) ;
}

