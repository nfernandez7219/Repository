#ifndef __INC_LOADDRES_HPP__
#define __INC_LOADDRES_HPP__

#pragma warning( disable: 4100 )

/*
ResDllDesc class defines interface to the resource objects irrespectivelly whether they
are stored in the static or dynamic resources.

Access to following application files:
	XXX_?_rc.dll located in
		1. current program directory
		2. %CS_DIR%\apps\programs
		3. standard system directories
	XXX_?.hlp located in
		1. ..\hlp directory
		2. %CS_DIR%\apps\hlp directory
Here '?' is short language suffix (e, d... ) and XXX stands for module name.

Example of the usage (written for applib library):

1. Declare the instance of corresp. ResDllDesc object:
	ResDllDesc applibResModule( "app", "1.23" ) ;

2. In the DLL resource, in the Version resource, set
	FILEVERSION 1,23,0,0

During the load time it will be checked that app_?_rc.dll has version at least 1.23.
The dll language is supplied as the load() function argument. Default loading takes
language specified in "getGlbLogin()->language".

(By overriding checkVersion() it is possible to change this default version checking.)

Remarks:
	- If NULL module_name is used, then no DLL is loaded and STATIC RESOURCES ARE USED instead.
	- To create resource DLL follow these steps:
		-- From MSVC create empty Win32 Dll project
		-- add you resources to the project
		-- adjust project options:
			--- select No MFC support,
			--- add linker switch /OPT:REF, and
			--- change output path to the directory where your application (exe) resides
*/


class ResDllDesc
{
	HINSTANCE _LoadLibrary( const char *lib_name, char *errStr ) ;
	BOOL	  _loadModule ( const char *dir, const char *libName, char *errStr ) ;
  protected:
	// These items are input to the constructor:
	char    _moduleName[30] ;		// "app"
	BOOL    _versionCheckRequired ;	// DLL must have version info
	char    _minVersion[12] ;		// "1.23" ... means "app_?_rc.dll" version 1.23

	// defined by load():
	HMODULE _mod ;
	char    _country[4] ;			// really loaded language
	char    _version[12];			// real version
	char	_libname[30];
	char    _useDLL ;
	char    _useGlbHelp ;
	char   *_helpFile ;

  public:
	// Constructor for specific resource module.
	// (To load resources from code dll dynamically linking MFC use CWinAppobject.m_hInstance.)
	ResDllDesc(
		const char *module_name,		// used on error in exception text (as the library name)
		HMODULE mod						// module used for loading resources
		) ;

	// Constructor for using language dependent resources
	ResDllDesc(
		const char *module_name,		// name without language suffix, eg. "APP"
		const char *min_version=0,		// min. version loaded from version resource
		BOOL versionCheckRequired=FALSE,// if TRUE then no version check is performed
		BOOL useGlobalHelpFile=FALSE	// used only in helpFile
		) ;

   ~ResDllDesc()
	{
	   free( _helpFile ) ;
	}

	inline operator		HMODULE		() const		{ return _mod ; }
	inline const char *	moduleName	() const		{ return _moduleName ; }
	inline const char *	version		() const		{ return _version ; }
	inline const char *	country		() const		{ return _country ; }

	//
	// This function is used in sDllDialog on following occasions:
	//		BOOL sDllDialog::OnHelpInfo( HELPINFO* pHelpInfo )
	//		void sDllDialog::OnContextMenu( CWnd* pWnd, CPoint point )
	// Both these functions call ::WinHelp() handing over file returned from helpFile().
	//
	// Default implementation constructs file name as follows:
	//		1. <getGlbLogin()->basePath>\..\Hlp\<hlpBaseName>_<language>.hlp
	//		2. <getGlbLogin()->getSysTskDir()>\..\Hlp\<hlpBaseName>_<language>.hlp
	// where
	//		hlpBaseName = useGlobalHelpFile ? getGlbLogin()->prgName : moduleName().
	//
	// This search goes first for language = <country()>, then for language = "E".
	// First file found in the search is returned (passed to ::Winhelp() in sDllDialog).
	//
	virtual const char* helpFile    () ;

	// Function loads resource DLL into memory.
	// For NULL (or empty) country current country (getGlbLogin()->language) is taken first
	// and then (if failed) default english version.
	// For non-zero country default english version is not tested.
	//
	// The function is automatically called by all load*() functions. (If needed.)
	// Re-load to a different language is possible. (On failure original language is retained.)
	//
	// Return:
	//	TRUE - all O.K.
	//	FALSE- default (english) version loaded (warning is displayed if say_warning=TRUE)
	//  Msg type exception is thrown if DLL could not be loaded.
	//
	BOOL    load( const char *country=NULL, BOOL say_warning=TRUE ) ;
	inline BOOL	isLoaded() const					{ return _mod != 0 ; }

	// This function is used to check the validity of the resource DLL.
	// Default implementation simply checks that
	//		atof(version) >= atof(minVersion)
	// where
	//		version is read from the resource (string resource versionStringId)
	//		min_version is given in the constructor
	virtual BOOL checkVersion( const char *loadedDllVersion ) ;

	// Access to specific resources (for help see WIN32 functions with equal names).
	// In case of failure all load functions throw an Msg exception.
	int     loadString		( UINT uID,	LPTSTR lpBuffer, int nBufferMax ) ;
	HACCEL  loadAccelerators( WORD id ) ;	// freed automatically
	HCURSOR loadCursor		( WORD id )	;	// cursor id must be unique among all resources
	HICON   loadIcon		( WORD id )	;	// loads icon of standard size SM_CXICON x SM_CYICON; no delete needed (use loadImage() to scale)
	HBITMAP loadBitmap		( WORD id )	;	// DeleteObject(hBitmap) needed
	HMENU   loadMenu		( WORD id )	;	// DestroyMenu(hMenu) needed
	HGLOBAL loadResource	( WORD id , LPCSTR type ) ;		// FreeResource() needed for W95
	BOOL	loadToolBar		( CToolBarCtrl *ctrl, UINT uID ) ;	// load toolbar uID into given control
	inline HGLOBAL loadDialog( WORD id )		{ return loadResource(id,RT_DIALOG) ; }

	// Instead of an exception, the function returns "?" on failure
	const char *loadSafeString( UINT uID, LPTSTR buf, int max_chars=0 ) ;

	// Supplied name is decorated by the language suffix, (e.g. "marker" -> "marker_e")
	// and corresp. help file (marker_e.hlp") is located:
	//	1. in local directory (appDir\..\hlp)
	//	2. CS_DIR help directory (appDir\..\hlp)
	// If not successfull then the same procedure is tried for default language.
	static BOOL ResDllDesc::getHelpFile( const char *name, char *filePath ) ;

	// Get tool tip text.
	// Can be used to implement tool tips e.g. for toolbars; see following example:
 	//		BOOL MyCDialogBar::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
	//		{
	//			return resMod->OnToolTipNotify( id, pNMHDR, pResult ) ;
	//		}
	BOOL OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult ) ;
} ;

//
// Support for dialogs working with ResDllDesc resources.
//
// DoModal()
//	Loads the dialog and calls
//			return CDialog::DoModal().
//	If res_mod==NULL, then static resources are taken.
//	In case of any uncaught exception DoModal() is properly closed, user message displayed,
//	and -1 returned.
//
// To implement the tooltips, either
//	implement function getToolTipText() returning tool tip for given control,
// or
//	define for every control a string resource with the same ID (as the control has),
// If both ways are implemented, then getToolTipText() has the priority.
//
// To implement context help, switch on (in the resource editor) Help ID flag
// for each control. Use automatically generated help id to create an entry in the
// resource module help file.  CONTROL ID'S MUST BE UNIQUE!
// Out of these id's the system automatically creates help map. However, you can supply
// your own help map if you override getHelpArray() virtual function.
// The syntax of the help map is as shown below:
//	DWORD helpMap[] = {
//    sStdC_FindDown,         HsStdC_FindDown,			// control id, corresp. help id
//    sStdC_FindHistory,      HsStdC_FindHistory,           
//		...
//    0,0
//	};
// The help file used is derived either from the module name or from the application name.
// (depending on the value useGlobalHelpFile supplied to the resource module constructor).
// It can be located either in application or cs help directory
//
class sDllDialog : public CDialog
{
	char	_txt[512] ;
	int		_DoModal() ;
	void    _load() ;

  protected:
	HGLOBAL		 m_hResource ;
	ResDllDesc  *m_resMod ;
	UINT		 m_dlgId ;
	CWnd		*m_parent ;
	const char  *m_title ;

	BOOL OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult ) ;
	BOOL OnNotify		( WPARAM wParam, LPARAM lParam, LRESULT* pResult ) ;

	virtual DWORD      *getHelpArray  ( )								{ return NULL ; }
	virtual const char *getToolTipText( UINT ctrlId, char *txt )		{ return NULL ; }
            BOOL		OnHelpInfo    ( HELPINFO* pHelpInfo) ;
            void		OnContextMenu ( CWnd* pWnd, CPoint point); 
	virtual BOOL		OnInitDialog  ( ) ;

  public:
    inline	UINT        getDlgID      ( ) const                         { return m_dlgId; }    
	inline	ResDllDesc *getResMod     ( ) const                         { return m_resMod;}
	inline	const char *getDlgTitle	  ( ) const							{ return m_title ; }

	void enableAllControls( BOOL enable ) ;
	BOOL Create( ResDllDesc *res_mod=NULL, UINT dlg_id=0, CWnd *pParent=NULL ) ;
    
	sDllDialog( ) ;
	sDllDialog(
		ResDllDesc *res_mod,	// resource module with dialog resource; NULL for current module
		UINT dlg_id,			// dialog resource id
		CWnd *pParent=NULL,		// window parent
		const char *title=NULL	// optional title which overrides the window title in the resources
		) ;
   ~sDllDialog() ;
	virtual int DoModal() ;

    DECLARE_MESSAGE_MAP()
   	DECLARE_DYNCREATE( sDllDialog )
} ;


//
// Support for dynamic change of the application language
// Add all your language modules (string tables or resource dll's) to 
// sLanguageObjectList. Later, by call to setLanguage(), the language
// of all submitted modules is changed.
//
class sLanguageObjectList
{
  private:
	sTemplateArray<ResDllDesc*>  dllList ;
	sTemplateArray<StringTable*> tblList ;
	char	_lang[20] ;
	char ** _longLanguages  ;
	int    n_longLanguages  ;
	char ** _shortLanguages ;
	int    n_shortLanguages ;
	void    _loadLanguages();
	BOOL	_forReports ;

  public:
    void clear( ) ;

	void add( ResDllDesc  *dll )	{ dllList.add(dll) ; }
	void add( StringTable *tbl )	{ tblList.add(tbl) ; }
	BOOL del( ResDllDesc  *dll )	{ return dllList.delObj(dll) ; }
	BOOL del( StringTable *tbl )	{ return tblList.delObj(tbl) ; }

	//
	// Conversion between short and long language name (e.g. "E" <-> "English").
	// Corresp. strings are taken from getGlbConfig(), section [Languages].
	const char  *shortToLongLanguage( const char *shortL) ;	// NULL if not found
	const char  *longToShortLanguage( const char *longL ) ;
	const char **longLanguages		( )					// NULL-terminated; always non empty
	{
		_loadLanguages() ;
		return (const char **)_longLanguages ;
	}

	const char *language()		{ return _lang ; }		// short language ("E")
	BOOL setLanguage( const char *lang, char *err_message ) ;
	sLanguageObjectList( BOOL forReports=FALSE )		// for application(=default) or reports
	{
		_forReports		 = forReports ;
		_longLanguages   = 0 ;
		n_longLanguages  = 0 ;
		_shortLanguages  = 0 ;
		n_shortLanguages = 0 ;
		_lang[0]		 = 0 ;
	}
   ~sLanguageObjectList( )			{ clear() ; }
	void init( ) ;
} ;

#endif
