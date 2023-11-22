#ifndef __INC_PROTECTEDFILE_HPP__
#define __INC_PROTECTEDFILE_HPP__

// This class provides C-like file access while providing protection against illegal access.
// You cannot access the file with any external (non-class) function.

class ProtectedFile
{
  public :
	enum Pmode{ P_NONE=0,
				P_USER=1,		// protected against user change
				P_FILEPATH=2,	//                   copy to another path
				P_FILETIME=4,	//                   change of the file time
				P_COMPUTER=8,	//                   copy to another computer
				P_DATA=16,		//                   data change realized without this class
				P_ALL=255
			  } ;
	enum Cmode{ C_DONTCHANGE=0,			//
				C_PRESERVEFILETIME=1	// file creation time is preserved also for modification
			  } ;
	enum Fmode{ F_READ,			// reading existing file; no change allowed, time preserved
				F_WRITE,		// open new / rewrite old file (old settings lost)
				F_MODIFY		// read/write into existing file (old settings preserved
			  } ;				//   unless changed via later call to protMode())

  private:
	FILE  *_fp ;
	char   _filename[256] ;
	int    _pmode ;
	Fmode  _myFmode ;
	char   _hdr[256] ;
	long   _pos0 ;
	int    _changed ;
	time_t _ftime ;
	ushort _computeCrc( ) ;
	void   _setVars( int mode ) ;

  protected :
	// these variables are set in each read/close/test; they are 0/empty if corresp. data is undefined
	time_t	time ;
	char	computer[32] ;
	char	path[128] ;
	char	user[32] ;
	
	// use only when opened
	inline int   protMode( )									{ return (int)_pmode  ; }							// get current protection mode
	inline int   protMode( Pmode newMode )						{ int x=(int)_pmode ; _pmode=newMode ; return x ; }	// change mode; = previous mode

  public:

	ProtectedFile( char *filename, int m=P_FILETIME | P_COMPUTER | P_DATA ) ;
	inline ~ProtectedFile()										{ close() ; }

	inline const char *fileName()								{ return _filename ; }
	inline int		   changed( )								{ return _changed ; }

		// to test eventual access violation call this function (It does not open the file.)
	int   test  ( ) ;						// 0-OK, else combination of Pmode bits violated

		// to read/write from the file open() must be called
		// close() is done implicitly in the destructor, but if you want specific mode
		// (like preserving the file time during write) you must call it explicitly
	int			 open    ( Fmode m=F_READ ) ;		// ERR_OPENFILE, NOSUCHFILE, BADVERSION, READFILE
	inline BOOL  isOpened( )									{ return _fp != NULL ; }
	void		 close   ( Cmode m=C_DONTCHANGE ) ;

		// all remaining functions can be called on the open file only
		// they behave equally as their C counterparts
	inline int   fFlush  ( )                                    { return  fflush( _fp ); _commit( _fileno( _fp ) ); };
	inline long  fTell   ( )                                    { return  ftell (_fp) - _pos0 ; }
	inline int   fSeek   ( long pos, int orig=SEEK_SET )        { return  fseek(_fp, orig == SEEK_SET ? pos+_pos0 : pos, orig ) ; }
	inline void  rewind  ( ) 	                                { ::rewind(_fp) ; fSeek(0) ; }
	inline int   fWrite  ( void *buf, size_t n, size_t cnt=1 )  { _changed = 1 ; return fwrite(buf,n,cnt,_fp) ; }
	inline int   fRead   ( void *buf, size_t n, size_t cnt=1 )  { return  fread(buf,n,cnt,_fp) ; }
	inline int   fEof    ( )                                    { return  feof  (_fp) ; }
	inline int   fError  ( )                                    { return  ferror(_fp) ; }
	inline char *fGets   ( char *buf, int n )				    { return  fgets(buf,n,_fp) ; }
} ;

#endif
