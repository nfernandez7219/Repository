#ifndef _SID_HPP
#define _SID_HPP

#ifdef _MSC_VER
#pragma pack( 1)
#endif

// sGlobalId() creates non-initialized object.
// Use create() to create an id.
// You can convert id to string via asString() and later restore from
// the string via sGlobalId(string).
// 
// Use isValid() to check whether id object was initialized.
class Z_USERDLL sGlobalId
{
	void   _init( ) ;
	BOOL   _getCountFromFile( ushort *c ) ;
    static ushort _tcp ;
    static ushort _cnt ;

  protected :
	time_t tim ;
	ushort cnt ;
	ushort tcp ;

  public:
	inline	sGlobalId( ushort ind, int )		{ cnt = 0 ; tim = 0 ; tcp = ind ; }
	inline	sGlobalId( )						{ cnt = 0 ; }
			sGlobalId( char *str ) ;

			char  *asString	  ( char *buf ) const ;	// 16 chars + \0
	inline	void  *asBinary	  ( )				{ return &tim ; }
	inline	int    binarySize ( )				{ return sizeof(tim) + sizeof(cnt) + sizeof(tcp) ; }
			void   reset	  ( ) ;
	inline	void   create	  ( )				{ reset( ) ; }
	inline	void   makeInvalid( )				{ cnt = 0  ; }
	inline	BOOL   isValid	  ( ) const			{ return cnt != 0 ; }
	inline	time_t time		  ( ) const			{ return tim ; }

	friend		  int operator== ( const sGlobalId &id1, const sGlobalId &id2 ) ;
	friend inline int operator!= ( const sGlobalId &id1, const sGlobalId &id2 )		{ return !(id1 == id2) ; }
	friend		  int operator<  ( const sGlobalId &id1, const sGlobalId &id2 ) ;
	friend inline int operator<= ( const sGlobalId &id1, const sGlobalId &id2 )		{ return  (id1 < id2)  ||  (id1 == id2) ; }
	friend inline int operator>  ( const sGlobalId &id1, const sGlobalId &id2 )		{ return !(id1 <= id2) ; }
	friend inline int operator>= ( const sGlobalId &id1, const sGlobalId &id2 )		{ return !(id1 < id2) ; }
} ;

#ifdef _MSC_VER
#pragma pack( )
#endif

#endif // __SID_HPP