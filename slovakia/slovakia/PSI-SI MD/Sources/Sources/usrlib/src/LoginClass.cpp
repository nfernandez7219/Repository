#include "tools2.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

sLoginClass::sLoginClass( )
{
	memset( this, 0, sizeof(sLoginClass) ) ;
}


void sLoginClass::close( )
{
	if( argv )
	{
		freeStringList( (const char**)argv, argc ) ;
		argv = NULL ;
		argc = 0 ;
	}
	if(   basePath  )
	{
		FREE(   basePath ) ;
		basePath = NULL ;
	}
	if ( prgName )
	{
		FREE( prgName ) ;
		prgName = NULL ;
	}
}

