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


StringTable :: StringTable( char *baseFileName, float min_version )
{
    minVersion = min_version ;
    strcpy( _baseFilename, baseFileName ? baseFileName : "" ) ;
    _offset = 0 ;
    type    = 0 ;
    ver     = 0.F ;
    lang[0] = 0 ;
    msgs    = NULL ;
    n_msgs  = 0 ;
    _languages = NULL ;
    n_languages= 0 ;
	_filename  = NULL ;
	_changed   = 0 ;
}

StringTable :: ~StringTable( )
{
    release( ) ;
    if( _languages )
        freeStringList( (const char**)_languages, n_languages ) ;
	FREE( _filename ) ;
}

void StringTable :: release( )
{
    if( msgs )
    {
        freeStringList( (const char **)msgs, n_msgs ) ;
        msgs = NULL ;
    }
    n_msgs  = 0 ;
    lang[0] = 0 ;
}
