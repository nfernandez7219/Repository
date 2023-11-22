
#ifndef INC_ENV_HPP
#define INC_ENV_HPP

#include "Env_base.hpp"

class Z_USERDLL sENV_SECTION ;

class Z_USERDLL ConfigClass : public BaseConfigClass
{
	friend class sENV_SECTION ;

  private :
    int   _err      ;
    char  _message[ 180] ;
    char *_filename ;
	BOOL  _sayError ;
	int   _errLine  ;

	void   errorReport    ( int errnum, char *filename ) ;
	int    readFromFile   ( sENV_SECTION *thisSectionOnly=NULL, BOOL allowNewFile=FALSE, BOOL delInvFile=FALSE ) ;
	int    _save          ( sENV_SECTION *thisSectionOnly, const char *_sectname=NULL ) ;
    int    writeToFile    ( FILE *src, FILE *dst, sENV_SECTION *thisSectionOnly, const char *thisSectName )  ;
    int    writeSectToFile( FILE *fp, sENV_SECTION *sect, const char *base, const char *usr ) ;
    int    isMySection    ( const char *usr ) ;
    int    getLineText    ( char *line, char **txt, int *cont ) ;
    int    readLine       ( char *line, char **sect, char **var, char **txt, int *cont ) ;

  public :
   	ConfigClass( const char *filename=NULL ) ;
   ~ConfigClass( ) ;

	// Config file name; can be set in the constructor or explicitly via setFileName().
	// Used only during save/load operations.
	const char  *fileName( )							{ return _filename ; }
	void	  setFileName( const char *filename ) ;

	// Returns error code (0=success).
	// If allowNewFile==0 then non-existence of the config file is considered as an error.
	// (Despite that config object can be used further and eventually saved at the end.)
	int              open( const char *usr=NULL, BOOL say_error=0, BOOL allowNewFile=FALSE, BOOL delInvalidFile=FALSE ) ;

	// Adds sections and/or variables into existing or new config file. (No matter whether opened or not.)
	// (exc) on error
	//
	// Example:
	//	Following code
	//
	//		char *defaults[] = {
	//			"[marker]"   , 0  , 0,				// section
	//			"drawFlaws"  , "1",	"comment",		// 
	//			"drawColors" , "1",	0,				// variable without comment
	//		} ;
	//		install( defaults, 3 ) ;
	//
	//	results in
	//
	//		[marker]
	//		drawFlaws=1			* comment
	//		drawColors=1
	//
    void          install( const char **defaults, int n_defaults, int installNewSectionsOnly=0 ) ;

	void           reload( ) ;			// (exc); on error old contents preserved

	// Returns error code (0=success) and sets class error, which can be retrieved
	// via error functions.
	// If say_error is TRUE, the error message is displayed.
	// If !isChanged() then the call is ignored.
	int              save( BOOL say_error ) ;

	//----------------------------------------------------------------------

	// save/reload only for abs. sections; (exc) on error
	void		  saveSection    ( const char *sectname ) ;
	void		  reloadSection  ( const char *sectname ) ;

    inline	int   error          ( )          { return  _err ; }
			void  clearError     ( ) ;
    inline	char *errorMessage   ( )          { return  _err ? _message : NULL ; }
} ;


ConfigClass Z_USERDLL *getGlbConfig() ;

#endif
