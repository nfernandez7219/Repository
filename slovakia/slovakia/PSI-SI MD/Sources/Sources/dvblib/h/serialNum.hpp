#ifndef __SERIALNUM_HPP__
#define __SERIALNUM_HPP__

// structure received from card
// the order of bytes is reversed !!!

#ifndef __CardSerialNumberDefined__
#define __CardSerialNumberDefined__
	struct CardSerialNumber
	{
		unsigned char		bytes[6] ;		// bytes[5] - highest, bytes[0] - lowest
	} ;
#endif


// the user id, byte order is mixed (4 5 2 3 0 1)
// used in unicasted packets
struct UnicastUserID
{
	unsigned char		bytes[6] ;
	inline BOOL isValid() const
	{
		return bytes[0] != 0 || bytes[1]!=0 || bytes[2]!=0 || bytes[3]!=0 || bytes[4]!=0 || bytes[5]!=0 ;
	}
	inline void makeInvalid()
	{
		bytes[0] = bytes[1] = bytes[2] = bytes[3] = bytes[4] = bytes[5] = 0 ;
	}
	inline void makeMaximum()
	{
		bytes[0] = bytes[1] = bytes[2] = bytes[3] = bytes[4] = bytes[5] = 0xFF ;
	}
	inline void _copy( const UnicastUserID &num )
	{
		bytes[0] = num.bytes[0] ;
		bytes[1] = num.bytes[1] ;
		bytes[2] = num.bytes[2] ;
		bytes[3] = num.bytes[3] ;
		bytes[4] = num.bytes[4] ;
		bytes[5] = num.bytes[5] ;
	}
} ;

// the user id, byte order is mixed (4 5 2 3 0 1)
class GlobalUserID : public UnicastUserID
{
  public:
	// Default constructor makes invalid id (smaller than any other id)
	inline	GlobalUserID				( ) throw() ;
	inline	GlobalUserID				( const GlobalUserID& num ) throw() ;
	inline	GlobalUserID				( const UnicastUserID& num ) throw() ;
	inline	GlobalUserID				( const CardSerialNumber& num ) throw() ;

	inline	GlobalUserID& operator =	( const GlobalUserID& num ) ;
	inline	GlobalUserID& operator =	( const UnicastUserID& num ) ;
	inline	GlobalUserID& operator =	( const CardSerialNumber& num ) ;

			BOOL		  operator <	( const GlobalUserID& num ) const ;
			BOOL		  operator <=	( const GlobalUserID& num ) const ;

	inline	int			  operator ==	( const GlobalUserID& num ) const ;
	inline	int			  operator !=	( const GlobalUserID& num ) const ;

	inline	int			  operator ==	( const UnicastUserID& num ) const ;
	inline	int			  operator !=	( const UnicastUserID& num ) const ;

	void next() ;
	inline void set( __int64 num ) ;

	char *asText  ( char *str ) const ;
	BOOL  fromText( const char *str ) ;		// on failure <this> becomes invalid
} ;


//
// Constructors
//
inline GlobalUserID::GlobalUserID( const GlobalUserID& num ) throw()
{
	_copy( num ) ;
	//memcpy( this, &num, sizeof(GlobalUserID) ) ;
}
inline GlobalUserID::GlobalUserID( const UnicastUserID& num ) throw()
{
	_copy( num ) ;
	//memcpy( this, &num, sizeof(GlobalUserID) ) ;
}
inline GlobalUserID::GlobalUserID( const CardSerialNumber& num ) throw()
{
	bytes[5] = num.bytes[4] ;
	bytes[4] = num.bytes[5] ;
	bytes[3] = num.bytes[2] ;
	bytes[2] = num.bytes[3] ;
	bytes[1] = num.bytes[0] ;
	bytes[0] = num.bytes[1] ;
}
inline GlobalUserID::GlobalUserID( ) throw()
{
	bytes[0] = bytes[1] = bytes[2] = bytes[3] = bytes[4] = bytes[5] = 0 ;
}

inline void GlobalUserID::set( __int64 num )
{
	bytes[5] = (unsigned char)(num & 0xff) ;
	num >>= 8 ;
	bytes[4] = (unsigned char)(num & 0xff) ;
	num >>= 8 ;
	bytes[3] = (unsigned char)(num & 0xff) ;
	num >>= 8 ;
	bytes[2] = (unsigned char)(num & 0xff) ;
	num >>= 8 ;
	bytes[1] = (unsigned char)(num & 0xff) ;
	num >>= 8 ;
	bytes[0] = (unsigned char)(num & 0xff) ;
}


//
// Copying
//
inline GlobalUserID& GlobalUserID::operator =( const GlobalUserID& num )
{
	_copy( num ) ;
	return *this ;
}
inline GlobalUserID& GlobalUserID::operator =( const UnicastUserID& num )
{
	_copy( num ) ;
	return *this ;
}
inline GlobalUserID& GlobalUserID::operator =( const CardSerialNumber& num )
{
	bytes[5] = num.bytes[4] ;
	bytes[4] = num.bytes[5] ;
	bytes[3] = num.bytes[2] ;
	bytes[2] = num.bytes[3] ;
	bytes[1] = num.bytes[0] ;
	bytes[0] = num.bytes[1] ;
	return *this ;
}


//
// Comparisons
//

inline int GlobalUserID::operator ==( const GlobalUserID& num ) const
{
	return
	bytes[0] == num.bytes[0]  &&
	bytes[1] == num.bytes[1]  &&
	bytes[2] == num.bytes[2]  &&
	bytes[3] == num.bytes[3]  &&
	bytes[4] == num.bytes[4]  &&
	bytes[5] == num.bytes[5] ;
}
inline int GlobalUserID::operator !=( const GlobalUserID& num ) const
{
	return
	bytes[0] != num.bytes[0]  ||
	bytes[1] != num.bytes[1]  ||
	bytes[2] != num.bytes[2]  ||
	bytes[3] != num.bytes[3]  ||
	bytes[4] != num.bytes[4]  ||
	bytes[5] != num.bytes[5] ;
}

inline int GlobalUserID::operator ==( const UnicastUserID& num ) const
{
	return
	bytes[0] == num.bytes[0]  &&
	bytes[1] == num.bytes[1]  &&
	bytes[2] == num.bytes[2]  &&
	bytes[3] == num.bytes[3]  &&
	bytes[4] == num.bytes[4]  &&
	bytes[5] == num.bytes[5] ;
}
inline int GlobalUserID::operator !=( const UnicastUserID& num ) const
{
	return
	bytes[0] != num.bytes[0]  ||
	bytes[1] != num.bytes[1]  ||
	bytes[2] != num.bytes[2]  ||
	bytes[3] != num.bytes[3]  ||
	bytes[4] != num.bytes[4]  ||
	bytes[5] != num.bytes[5] ;
}

#endif // __SERIALNUM_HPP__
