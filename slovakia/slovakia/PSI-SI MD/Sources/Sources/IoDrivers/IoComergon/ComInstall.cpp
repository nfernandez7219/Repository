
#include "tools2.hpp"
#include "card.h"

#include "resource.h"
#include "ComDvb.hpp"
#include "BaseRegistry.hpp"


/*
class InstallDialog : public CDialog
{
  protected:
	virtual void OnOK() ;

	virtual BOOL OnInitDialog()
	{
		BOOL ret = CDialog::OnInitDialog() ;
		return ret ;
	}
	InstallDialog( HWND parent ) : CDialog( sInstallDialog, CWnd::FromHandle(parent) )
	{
	}
} ;
*/

BOOL installDialog( HWND parent )
{
	const char *title = "Main Data, RS-422, ISA Card" ;
	char expl[512] ;
	if( !installDvbDrv( expl) )
	{
		char buf[1024] ;
		sprintf( buf, "Installation failed:\n%s", expl ) ;
		::MessageBox( parent, buf, title, MB_ICONERROR ) ;
		return FALSE ;
	}
	if( ::MessageBox( parent,
		"Installation succeeded.\n\n"
		"ISA Card needs to be configured.\n"
		"You can do it now if you press 'Yes' button, or later\n"
		"from the application Setup menu.\n\n"
		"Do you wish to configure the card now?",
		title, MB_YESNO|MB_ICONQUESTION) != IDYES )
	{
		::MessageBox( parent,
			"Configuration skipped.\n\n"
			"You can configure ISA card any time from the application Setup menu.",
			title, MB_OK|MB_ICONINFORMATION ) ;
		return TRUE ;
	}
	
	DvbSetupDialog( parent ) ;
	return TRUE ;
}

BOOL uninstallDialog( HWND parent )
{
	::MessageBox( parent, "...", "Uninstall", MB_OK ) ;
	return TRUE ;
}
