#include "tools2.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


sLoginClass Z_USERDLL login ;

sLoginClass Z_USERDLL *getGlbLogin( void )
{
	return  &login ;
}

