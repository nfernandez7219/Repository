
#ifndef            __TOOLS2_INCLUDED
#define            __TOOLS2_INCLUDED

#undef new

#ifdef _DEBUG
	#include    <malloc.h>		// must precede <crtdbg.h>
	#include    <stdlib.h>
	
	#ifdef _AFXDLL
		#define _CRTDBG_MAP_ALLOC
	#endif
	#include	<crtdbg.h>
#endif

#include    <afx.h>
#include    <afxtempl.h>
#include    <afxwin.h>
#include    <afxext.h>
#include    <afxmt.h>
#include    <afxcmn.h>

#include    "scrtools.hpp"


class Z_USERDLL SysValues
{
  public:
    int cxborder ;      // window frame width/height
    int cyborder ;
    int cytitlebar ;    // height of title bar
    int cxsizeborder ;
    int cysizeborder ;
    int cxvscroll ;     // scroll bar width/height
    int cyhscroll ;
    int cxscreen ;      // screen width/height
    int cyscreen ;
    int cymenu ;        // menu bar height
    void reset() ;      // update all values

    inline SysValues() { reset() ; }
} ;


#endif

