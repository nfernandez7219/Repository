/*
 *	Filename:		basecfg.cpp
 *
 *	Version:		1.00
 *
 *	Description: Base class for access to the user config files.
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"

#include "BaseCfg.hpp"


#define IS_SPACE(x)  (x==' ' || x=='\t')

void copyWithoutSpace ( char *dest, const char *src, int size )
{
	while( IS_SPACE( *src ) )	src++;
	for( int len=strlen(src); len>0 && IS_SPACE(src[len-1]);)
		 len--;
	
	if( size && len >= size )
		len  = size-1;
	strncpy( dest, src, len );
	dest[len] = 0;
}


//----------------------------------------------------------------------------
//	CfgBaseSetup
//----------------------------------------------------------------------------


CfgBaseSetup :: CfgBaseSetup( const char *file )
{
	fileName = STRDUP( file );
	config=NULL; 
}

void CfgBaseSetup::openConfig()
{
	if( config != NULL )
		return;

	config = new ConfigClass( fileName );
	if( config->open( NULL, FALSE, TRUE ) )
	{
		Msg msg( -1, config->errorMessage() ) ;
		delete config;
		config = NULL;
		throw msg ;
	}
}

void CfgBaseSetup::get( const char *sect, const char *var, char *value,int sz) 
{ 
	char *str=NULL;
	value[0] = 0;
	if( config && (str=config->get( sect, var ))  )
	{
		copyWithoutSpace( value, str, sz );
		value[sz-1] = 0;
	}
}

void CfgBaseSetup::save( LPTSTR lpErrBuffer )
{
	openConfig();
	storage( this, TRUE, lpErrBuffer );
	config->save( TRUE );
}

void CfgBaseSetup::load( LPTSTR lpErrBuffer )
{
	openConfig();
	storage( this, FALSE, lpErrBuffer );
}

void CfgBaseSetup::load( BaseConfigClass *pConfig, LPTSTR lpErrBuff )
{
	int	  iBufSize  = pConfig->binarySize();
	char *lpBuff	= (char*)malloc( iBufSize );
	
	try
	{
		// copy loaded config to dvb setup class
		if( pConfig->saveToBin( lpBuff ) != NULL &&
			cfg()->loadFromBin( lpBuff ) != NULL )
				storage( this, FALSE, lpErrBuff );
		free( lpBuff );
	}
	catch(...)
	{
		free( lpBuff );
		throw;
	}
}

CfgBaseSetup::~CfgBaseSetup()
{
	if( config )
		delete config ;
	if(fileName)
		FREE( fileName);
}
