#ifndef __INC_APP_RC_HPP__
#define __INC_APP_RC_HPP__


#ifndef __INC_LOADDRES_HPP__
#include "loadRes.hpp"
#endif


extern ResDllDesc applibResModule ;

inline HGLOBAL loadDlgResource( int id )
{
	return applibResModule.loadDialog( id ) ;
}

inline HMENU loadMenuResource( int id )
{
	return applibResModule.loadMenu( id ) ;
}

inline HCURSOR loadCursorResource( int id )
{
	return applibResModule.loadCursor( id ) ;
}

inline int loadStringResource( int id, LPTSTR buf, int maxChars )
{
	buf[0] = 0 ;
	return  applibResModule.loadString( id, buf, maxChars ) ;
}

#endif
