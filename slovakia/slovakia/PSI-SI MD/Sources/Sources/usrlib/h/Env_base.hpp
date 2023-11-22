
#ifndef __INC_ENV_BASE_HPP__
#define __INC_ENV_BASE_HPP__

class sENV_SECTION ;

class sENV_EXTSECTION
{
  public :
    char              *baseName ;
    sENV_SECTION      *baseSect, *usrSect, *curSect ;
    sENV_EXTSECTION  ( const char *baseSectName ) ;
   ~sENV_EXTSECTION  ( ) ;

	void  del        ( ) ;
	BOOL  delVariable( const char *varName ) ;
	BOOL  changed    ( ) ;

	size_t  binarySize		( ) ;
	char   *loadFromBin		( char *bin ) ;			// = bin + binarySize(); NULL on error
	char   *saveToBin		( char *bin ) const ;	// = bin + binarySize()
} ;


#define siDEF_SECTION     (actSect ? (char*)((LPCTSTR)actSect->baseName):NULL)


class BaseConfigClass
{
	friend class sENV_SECTION ;

  protected :
	char *_username ;
    BOOL  _changed  ;
	sENV_EXTSECTION      *actSect ;
    sListPtr<sENV_EXTSECTION> *sectList;

    int    addSection     ( const char *base, const char *usr ) ;
    int    setActSection  ( const char *baseSectName ) ;
    int    setActSection  ( const char *base, const char *usr ) ;
    void   sectTitle      ( char *buf, const char *base, const char *usr ) ;
    int    getGetSection  ( const char *sectname, sENV_SECTION **sect ) ;
    int    getSetSection  ( const char *sectname, sENV_SECTION **sect ) ;
    void   decodeSectName ( char *sectName, char **base, char **usr ) ;
	void   computeChangeFlag( ) ;
	static void   convertCstringToString( const char *cstring, char *string ) ;

  public :
	//
	// Rules for section name:
	//		"#section"		... absolute name (unchanged)
	//		"section"		... decorated by the user name; eg. "section..user"
	//

   	BaseConfigClass( const char *filename=NULL ) ;
   ~BaseConfigClass( ) ;

	void            close( ) ;			// no (exc); no automatic save performed
	void			clear( ) ;			// no (exc); delete all data

	// TRUE if value os some variable changed since last open or save.
	inline BOOL isChanged( )		{ return  _changed ; }

	//----------------------------------------------------------------------
	// get/set functions
	//
	// set*() functions and printf() return error code (0/ERR_ALLOCERROR).
	// get*() functions fail (i.e. return NULL or FALSE) if the vaiable is not found or has bad syntax.
	//

	char             *get( const char *sect, const char *var ) ;
	inline char      *get( const char *var )              { return  get   ( siDEF_SECTION, var ) ; }

	BOOL	   getStrings( const char *sect, const char *var, sStringPtrArray &arr ) ;
	int		   setStrings( const char *sect, const char *var, const sStringPtrArray &arr ) ;

	BOOL		   getInt( const char *sect, const char *var, int *num ) ;
	inline int     getInt( const char *var, int *num )    { return  getInt( siDEF_SECTION, var, num ) ; }
	int            setInt( const char *sect, const char *var, int num ) ;

	BOOL         getFloat( const char *sect, const char *var, float *num ) ;
	inline int   getFloat( const char *var, float *num )    { return  getFloat( siDEF_SECTION, var, num ) ; }
	int          setFloat( const char *sect, const char *var, float num )  { return setDouble( sect, var, num ) ; }

	BOOL        getDouble( const char *sect, const char *var, double *num ) ;
	inline int  getDouble( const char *var, double *num )    { return  getDouble( siDEF_SECTION, var, num ) ; }
	int         setDouble( const char *sect, const char *var, double num ) ;

	BOOL          getFont( const char *sect, const char *var, LOGFONT *f ) ;
	inline int    getFont( const char *var, LOGFONT *f )    { return  getFont( siDEF_SECTION, var, f ) ; }
	int           setFont( const char *sect, const char *var, LOGFONT *f ) ;

	int               set( const char *sect, const char *var, const char *value ) ;
	int            printf( const char *sect, const char *var, const char *fmt ... ) ;

	//----------------------------------------------------------------------

	// Get list (NULL terminated array of strings) of all variables in the section.
	// The returned value must be destroyed via destroyVarList(*vars).
	// -> 0, ERR_ALLOCERROR
	int           varList( const char *sect, char ***vars, int *n_vars=NULL ) ;
	inline	int   varList( char ***vars, int *n_vars=NULL )
                                         { return  varList( siDEF_SECTION, vars, n_vars ) ; }
	static void   destroyVarList( char **vars ) ;

	// Get list of all sections matching given mask (mask allows */? wildcards).
	// Result must be deleted.
	sStringPtrArray *sectionList( const char *mask=NULL ) ;

	// Get list (NULL terminated array of strings) of variable tokens. (Works similarly to strtok().)
	// The returned value must be destroyed via destroyTokenList(*vars).
	// -> 0, ERR_ALLOCERROR
    int         tokenList( const char *sect, const char *var, char ***tokens, int *n_tokens=NULL, char *terms=NULL ) ;
    inline	int tokenList( const char *var, char ***tokens, int *n_tokens=NULL, char *terms=NULL )
                                         { return  tokenList( siDEF_SECTION, var, tokens, n_tokens, terms ) ; }
	static void destroyTokenList( char **tokens ) ;

	//----------------------------------------------------------------------

	// unless abs. section name is used the delete is performed on all subsections
	// (= base + derived subsections; eg. "section", "section..user")
	BOOL  delSection     ( const char *sectName ) ;
	BOOL  delVariable    ( const char *sectName, const char *varName ) ;

	inline 
	BOOL  existSection   ( const char *sectName )
	{
		sENV_SECTION *sect ;
		return  getGetSection( sectName, &sect) == 0 ;
	}

	//----------------------------------------------------------------------
	// linearization

	size_t  binarySize		( ) ;
	char   *loadFromBin		( char *bin ) ;			// = bin + binarySize(); NULL on error
	char   *saveToBin		( char *bin ) const ;	// = bin + binarySize()
} ;


#endif
