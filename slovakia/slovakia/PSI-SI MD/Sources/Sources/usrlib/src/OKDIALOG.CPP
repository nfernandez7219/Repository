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


#include "screen.msg"

#ifdef __BORLANDC__
extern "C"
{
    int _RTLENTRY _EXPFUNC getch( void ) ;      // Borland <conio.h> is in conflict in enum COLORS
}
#else
#include <conio.h>
#endif


/*
 * zMessage-based modal Modal dialogs (allow NULL title and parent)
 */


#define HLP_ID		0

static int _txtStdMessage( char *title, int style, char *fmt, va_list ap )
{
    printf( "\n" ) ;
    if( title  &&  *title )
        printf( "\n[%s]", title ) ;

    switch( style )
    {
	    case STD_WARNDIALOGSTYLE :
            printf( "\n(%s) ", SCRMSG( SCRMSG_Warning) ) ;
            break ;
        case STD_ERRORDIALOGSTYLE :
            printf( "\n(%s) ", SCRMSG( SCRMSG_Error) ) ;
            break ;
        default :
            printf( "\n" ) ;
    }
    vprintf( fmt, ap ) ;

    int  kod ;
    switch( style )
    {
    	case STD_INFODIALOGSTYLE :
            break ;
	    case STD_OKDIALOGSTYLE :
	    case STD_WARNDIALOGSTYLE :
        case STD_ERRORDIALOGSTYLE :
            printf( "\n%s ", SCRMSG( SCRMSG_PressAnyKey) ) ;
            getch( ) ;
            break ;
	    case STD_OKCANCELDIALOGSTYLE :
            printf( "\n%s ", SCRMSG( SCRMSG_SelectYesNo) ) ;
            kod = getch( ) ;
            return  kod == '\r' ;
	    case STD_YESNODIALOGSTYLE :
            printf( "\n%s ", SCRMSG( SCRMSG_SelectOkCancel) ) ;
            kod = getch( ) ;
            return  kod == '\r' ;
    }
    return  0 ;
}

static int _stdMessage( char *title, int style, char *fmt, va_list ap )
{
    static time_t last_time=0 ;
    //if( getGlbLogin()->isTextApp() )
    //    return  _txtStdMessage( title, style, fmt, ap ) ;
	char buf[ 2000] ;
    int  ret = 0 ;
    if( time(NULL) - last_time <= 1 )
        delay( 100 ) ;          // to enable proper closing of previous message

	vsprintf( buf, fmt, ap ) ;

    if( !title )
        title = "" ;

    ret = MessageBox( NULL, buf, title, style | MB_TASKMODAL ) ;
    switch( ret )
    {
        case IDABORT :
        case IDCANCEL:
        case IDIGNORE:
        case IDNO    :
        case IDRETRY :
        case 0       :
            ret = 0  ;
            break ;
        case IDOK    :
        case IDYES   :
            ret = 1  ;
            break ;
    }
    last_time = time(NULL) ;
	return  ret ;
}

int Z_USERDLL stdMessage( char *title, int style, char *fmt, ... )
{
	va_list  ap ;
	va_start( ap, fmt ) ;
    return  _stdMessage( title, style, fmt, ap ) ;
}


//------------------------------------------------------------------------------


void Z_USERDLL stdOkDialog( char *fmt,...)
{
	va_list  ap ;
	va_start( ap, fmt ) ;
	_stdMessage( "...", STD_OKDIALOGSTYLE, fmt, ap ) ;
}

void Z_USERDLL stdInfoDialog( char *fmt,...)
{
	va_list  ap ;
	va_start( ap, fmt ) ;
	//_stdMessage( "", STD_INFODIALOGSTYLE, fmt, ap ) ;
	_stdMessage( "...", 0, fmt, ap ) ;
}

void Z_USERDLL stdWarningDialog( char *fmt,...)
{
	va_list  ap ;
	va_start( ap, fmt ) ;
	_stdMessage( "!?", STD_WARNDIALOGSTYLE, fmt, ap ) ;
}

void Z_USERDLL stdErrorDialog( char *fmt,...)
{
	va_list  ap ;
	va_start( ap, fmt ) ;
	_stdMessage( "!", STD_ERRORDIALOGSTYLE, fmt, ap ) ;
}

int Z_USERDLL askOkDialog( char *fmt,...)
{
	va_list  ap ;
	va_start( ap, fmt ) ;
	return  _stdMessage( "?", MB_OKCANCEL, fmt, ap ) ;
}

int Z_USERDLL askYesDialog( char *fmt,...)
{
	va_list  ap ;
	va_start( ap, fmt ) ;
	return  _stdMessage( "?", MB_YESNO, fmt, ap ) ;
}

int Z_USERDLL askYesNoCancel( char *title, const char *fmt, ... )
{
	char buf[5000] ;
	va_list args ;
	va_start( args, fmt ) ;
	vsprintf( buf, fmt, args ) ;
	va_end( args ) ;
	if( title == NULL )
		title = "?" ;
	return MessageBox( NULL, buf, title, MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON1 | MB_TASKMODAL) ;
}
