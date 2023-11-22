#include "tools2.hpp"
#include "Mux.hpp"
#include "SerialNum.hpp"

/*
BOOL GlobalUserID::fromText( const char *nptr )
{
	while ( isspace((int)(unsigned char)*nptr) )
		++nptr;

	__int64 total=0;	// current total
	int c ;
	int nChars = 0 ;
	while( (c = (int)(unsigned char)*nptr++) != 0 )
	{
		if( c >= '0'  &&  c <= '9' )
		{
			total = 16 * total + (c - '0');
		}
		else
		{
			switch( c )
			{
				case 'A' :
				case 'B' :
				case 'C' :
				case 'D' :
				case 'E' :
				case 'F' :
					total = 16 * total + (c - 'A' + 10);
					break ;
				case 'a' :
				case 'b' :
				case 'c' :
				case 'd' :
				case 'e' :
				case 'f' :
					total = 16 * total + (c - 'a' + 10);
					break ;
				default:
					while( isspace(c) )
						c = (int)(unsigned char)*nptr++ ;
					if( c != 0 )
						goto labelFailed ;
					goto labelEnd ;
			}
		}
		if( nChars > 12 )
			goto labelFailed ;
	}

  labelEnd:
	*this = total ;
	return TRUE ;

  labelFailed:
	makeInvalid() ;
	return FALSE ;
}
*/

/*
BOOL GlobalUserID::fromText( const char *nptr )
{
	while ( isspace((int)(unsigned char)*nptr) )
		++nptr;

	__int64 total=0;	// current total
	int c ;
	int nChars = 0 ;
	while( (c = (int)(unsigned char)*nptr++) != 0 )
	{
		if( c >= '0'  &&  c <= '9' )
		{
			total = 16 * total + (c - '0');
		}
		else
		{
			switch( c )
			{
				case 'A' :
				case 'B' :
				case 'C' :
				case 'D' :
				case 'E' :
				case 'F' :
					total = 16 * total + (c - 'A' + 10);
					break ;
				case 'a' :
				case 'b' :
				case 'c' :
				case 'd' :
				case 'e' :
				case 'f' :
					total = 16 * total + (c - 'a' + 10);
					break ;
				default:
					while( isspace(c) )
						c = (int)(unsigned char)*nptr++ ;
					if( c != 0 )
						goto labelFailed ;
					goto labelEnd ;
			}
		}
		if( nChars > 12 )
			goto labelFailed ;
	}

  labelEnd:
	set( total ) ;
	return TRUE ;

  labelFailed:
	makeInvalid() ;
	return FALSE ;
}
*/

BOOL GlobalUserID::fromText( const char *nptr )
{
	// Remove leading and closing white spaces and extend the number
	// from the left to 12 chars by prefixing with '0' characters.
	while ( isspace((int)(unsigned char)*nptr) )
		++nptr;
	int len = strlen( nptr ) ;
	while( len > 0  &&  nptr[len-1] <= ' ' )
		len-- ;
	if( len == 0  ||  len > 12 )
		goto labelFailed ;

	unsigned char buf[13] ;
	memset( buf, '0', sizeof(buf) ) ;
	strncpy( (char*)(buf+12-len), nptr, len ) ;
	buf[12] = 0 ;
	strupr( (char*)buf ) ;

	// Convert 12 digit string to Byte sequence
	{
		for( int j=0 ; j < 6 ; ++j )
		{
			unsigned char val1, val2 ;

			int c = buf[2*j] ;
			if( c >= '0'  &&  c <= '9' )
				val1 = c - '0' ;
			else
			if( c >= 'A'  &&  c <= 'F' )
				val1 = c + 10 - 'A' ;
			else
				goto labelFailed ;

			c = buf[2*j+1] ;
			if( c >= '0'  &&  c <= '9' )
				val2 = c - '0' ;
			else
			if( c >= 'A'  &&  c <= 'F' )
				val2 = c + 10 - 'A' ;
			else
				goto labelFailed ;

			bytes[5-j] = val1*16 + val2 ;
		}
	}
	return TRUE ;

  labelFailed:
	makeInvalid() ;
	return FALSE ;
}

char *GlobalUserID::asText( char *buf ) const
{
	char *b = buf ;
	for( int j=0 ; j < 6 ; ++j )
	{
		#define CHR(c) (c < 10 ? (c+'0') : (c-10+'A'))

		unsigned char bb = bytes[5-j] ;
		unsigned char c  = bb/16 ;
		*b++ = CHR(c) ;
		c = bb%16 ;
		*b++ = CHR(c) ;
	}
	*b = 0 ;
	return buf ;
}

BOOL GlobalUserID::operator< ( const GlobalUserID& num ) const
{
	for( int j=0 ; j < 6 ; ++j )
	{
		if( bytes[j] < num.bytes[j] )
			return TRUE ;
		if( bytes[j] > num.bytes[j] )
			return FALSE ;
	}
	return FALSE ;
}

BOOL GlobalUserID::operator<= ( const GlobalUserID& num ) const
{
	for( int j=0 ; j < 6 ; ++j )
	{
		if( bytes[j] < num.bytes[j] )
			return TRUE ;
		if( bytes[j] > num.bytes[j] )
			return FALSE ;
	}
	return TRUE ;
}

void GlobalUserID::next()
{
	for( int j=5 ; j >= 0 ; j-- )
	{
		bytes[j]++ ;
		if( bytes[j] != 0 )
			break ;
	}
}
