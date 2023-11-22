
#ifndef INC_ENVX_HPP
#define INC_ENVX_HPP

class sENV_SECTION ;

class sENV_VAR
{
	friend class sENV_SECTION ;
  	char *   _varname ;
  	char *   _vartext ;
  public:
  	inline sENV_VAR( const char *name, const char *text )
	{
		_varname=STRDUP(name) ;
		_vartext=STRDUP(text?text:"") ;
	}
	inline ~sENV_VAR( )
	{
		FREE(_varname) ;
		FREE(_vartext) ;
	}
	inline	char *varname()  { return _varname ; }
	inline	char *vartext()  { return _vartext ; }
} ;

class sENV_EXTSECTION ;

class sENV_SECTION
{
	friend class sENV_EXTSECTION ;
	friend class BaseConfigClass ;
	friend class ConfigClass ;

	sENV_EXTSECTION *  _parent ;
	char               _changed ;     // 0-no changes
	uchar              _deleted ;
	sListPtr<sENV_VAR> _varList ;
	int       setvarval		( const char *name, const char *value ) ;     // -> 0, ERR_ALLOCERROR
	void      clear			( ) ;
	inline	void del		( )              { clear() ; _deleted=1 ; }
	BOOL      delVariable	( const char *varName ) ;
	sENV_VAR *getVariable	( const char *varName ) ;
	BOOL      isItMe		( const char *base, const char *usr ) ;
	void      addVariable	( const char *varName, const char *varTxt ) ;
	int       appendVars	( char ***varnames, int *n_varnames ) ;

  public :
	inline	BOOL hasVariable( const char *name )  { return getVariable(name) != NULL ; }
	inline	int  count      ( )                   { return _varList.count() ; }
	inline	BOOL changed    ( )                   { return _changed ; }
	inline	BOOL deleted    ( )                   { return _deleted ; }
	inline	BOOL isEmpty    ( )                   { return _varList.count() <= 0 ; }
	inline	sENV_EXTSECTION *parent( )            { return _parent ; }

	size_t  binarySize		( ) ;
	char   *loadFromBin		( char *bin ) ;			// = bin + binarySize(); NULL on error
	char   *saveToBin		( char *bin ) const ;	// = bin + binarySize()

    sENV_SECTION    ( sENV_EXTSECTION *parent ) ;
   ~sENV_SECTION    ( ) ;
} ;

#define     ENV_TOKENERR        3
#define     ENV_SECTNOTEXIST    4
								// stricmp without leading & closing spaces
#define     STRICMP( s1, s2)    (!textEqual( (s1), (s2)))


#endif
