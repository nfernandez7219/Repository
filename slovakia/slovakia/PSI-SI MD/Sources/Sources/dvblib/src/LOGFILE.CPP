
#ifdef __BORLANDC__
#ifndef _COMMAND_LINE
#pragma  hdrfile "os2\obj\screen.sym"
#endif
#endif
#include "tools.hpp"
#ifdef _MSC_VER
#pragma  hdrstop ("winnt\obj\screen.pch")
#else
#pragma  hdrstop
#endif

//#define TRACE
#include "trace.h"


/*
 *  Journal maintenance
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


class LogfileClass
{
    int   _appendMode ;
    int   _logLevel ;
    char  _logFile[ 256] ;
    FILE *_fp ;
  public :
    void close  ( void ) ;
    int  open   ( char *name, int level, int append_mode, char *path ) ;
    int  vprintf( char *fmt, va_list arg ) ;
    int  printf ( int level, char *fmt, ... ) ;
    int  printf ( char *fmt, ... ) ;
} ;


int LogfileClass :: vprintf( char *fmt, va_list arg )
{
    if( _appendMode )
        _fp = fopen( _logFile, "a+" ) ;

    if( !_fp )
        return -1 ;
    int  ret = vfprintf( _fp, fmt, arg ) ;

    if( _appendMode )
    {
        fclose( _fp ) ;
        _fp = NULL ;
    }
    else
        fflush( _fp ) ;
    return  ret ;
}


int LogfileClass :: printf( int level, char *fmt, ... )
{
    va_list  arg ;

    if( level > _logLevel )
        return  0 ;

    va_start( arg, fmt ) ;
    return  vprintf( fmt, arg ) ;
}


int LogfileClass :: printf( char *fmt, ... )
{
    va_list  arg ;

    va_start( arg, fmt ) ;
    return  vprintf( fmt, arg ) ;
}


int LogfileClass :: open( char *name, int level, int append_mode, char *path )
{
    char    *ptr, pat[ 40], mask[ 40], fname[ 40] ;
    time_t  t ;

    close( ) ;
    _appendMode = append_mode ;
    _logLevel   = level ;
    if( !name ) return 0 ;

    trace( "logfile;\n path='%s'  name='%s'", path, name ) ;
    strcpy( fname, name ) ;
    if( (ptr = strchr( fname, '@')) != NULL )
    {
        char    *usr = sGlbVars(login)->usrName ;

        do
            *ptr++ = *usr ? *usr++ : '_' ;
        while( *ptr == '@' ) ;
    }
    trace( "logfile; fname1=\n'%s'", fname ) ;

    if( !strchr( fname, '.') )        // if no extension -> generate new version
    {
        concat ( pat , fname, ".$$$", NULL ) ;
        concat ( mask, fname, ".*"  , NULL ) ;
        long  version =  hiFilePat( path, mask, pat) + 1 ;
        trace( "logfile;\n path='%s'  mask='%s'\n -> version=%ld", path, mask, version ) ;
        if( version >= 0 )
            sprintf( fname+strlen(fname), ".%03.3ld", version ) ;
    }
    _makepath( _logFile, NULL, path, fname, NULL ) ;
    trace( "logfile; path=\n'%s'", _logFile ) ;

    if( !_appendMode )
        if( (_fp = fopen( _logFile, "w")) != NULL )
            return  0 ;

    time( &t ) ;
    printf( "  [%s]             level %d              %s\n",
        sGlbVars(login)->usrName, _logLevel, ctime( &t) ) ;
    return  1 ;
}


void LogfileClass :: close( void )
{
    time_t  t = time( NULL ) ;
    printf( "\n\n *** closed at  %s ***\n", ctime( &t) ) ;
    if( _fp )
    {
        fclose( _fp ) ;
        _fp = NULL ;
    }
    _logFile[ 0] = 0 ;
}

