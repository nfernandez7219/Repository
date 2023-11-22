
#ifndef            __SCRTOOLS_INCLUDED
#define            __SCRTOOLS_INCLUDED

#ifdef             __BORLANDC__
   #pragma option -w-
#endif


#include    <malloc.h>		// must precede <crtdbg.h>
#include    <stdlib.h>
#include	<limits.h>

#ifdef _DEBUG
	#ifdef _AFXDLL
		#define _CRTDBG_MAP_ALLOC
	#endif
	#include	<crtdbg.h>
#endif

#include    <string.h>
#include    <time.h>
#include    <dos.h>

#ifndef __AFX_H__
	#define ASSERT assert
	#include <assert.h>
	#ifdef _DEBUG
		void scr_trace( char* fmt, ... ) ;
		#define TRACE scr_trace
	#else
		#define TRACE (void)0
	#endif
#endif

#ifdef      _MSC_VER
#pragma warning( disable: 4251 )
#pragma warning( once: 4275 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef WINNT32
	#ifndef _WINDOWS
		#define _WINDOWS
	#endif
	#ifndef _WIN32
		#define _WIN32
	#endif
	#ifndef __NT__
	//	#define __NT__
	#endif
#endif

#ifdef Z_NTUSEDLL
	#ifndef _WINDOWS
		#define _WINDOWS
	#endif
	#ifndef _WIN32
		#define _WIN32
	#endif
	#ifndef __NT__
	//	#define __NT__
	#endif
#endif

#ifdef WINNT32DLL
	#ifndef _WINDOWS
		#define _WINDOWS
	#endif
	#ifndef _WIN32
		#define _WIN32
	#endif
	#ifndef __NT__
	//	#define __NT__
	#endif
	#ifndef Z_NTUSEDLL
		#define Z_NTUSEDLL
	#endif
	#ifndef Z_CLIDLL
		#define Z_CLIDLL
	#endif
	#ifndef __DLL__
		#define __DLL__
	#endif
#endif


#if defined(Z_NTUSEDLL)
	#ifdef Z_CLIDLL
		#ifdef _MSC_VER
			#define Z_USERDLL __declspec(dllexport)
		#else
			#define Z_USERDLL _export
		#endif
	#else
		#ifdef _MSC_VER
			#define Z_USERDLL __declspec(dllimport)
		#else
			#define Z_USERDLL _import
		#endif
	#endif
#else
	#define Z_USERDLL
#endif

#include    "screen.h"
#include    "general.h"
#include    "env.hpp"
#include    "stderror.h"
#include    "msglog.hpp"
#include    "sid.hpp"
#if !defined(c__COLORS)
#define c__COLORS


enum sCOLORS {
     cBLACK,          /* dark colors */
     cBLUE,
     cGREEN,
     cCYAN,
     cRED,
     cMAGENTA,
     cBROWN,
     cLIGHTGRAY,
     cDARKGRAY,       /* light colors */
     cLIGHTBLUE,
     cLIGHTGREEN,
     cLIGHTCYAN,
     cLIGHTRED,
     cLIGHTMAGENTA,
     cYELLOW,
     cWHITE
};
#endif

long Z_USERDLL colorIndexToRgb( int ind ) ;

// reads color names as "black", "red"... or hexa numbers ("0x........" )
BOOL Z_USERDLL colorNameToRgb( const char *name, long *rgb ) ;

// Converts rgb to hexa (so that colorNameToRgb() identifies original value)
char *Z_USERDLL rgbToColorName( char *name, long rgb ) ;

int  Z_USERDLL getBitsInPixel() ;


#define sGlbVars( v )  (&v)

#ifdef      _MSC_VER
#pragma warning( default: 4251 )
#pragma warning( default: 4275 )
#endif

#ifdef             __BORLANDC__
   #pragma option -w.
#endif

#endif

