#ifndef 		__DLG_TOOL_HPP__
#define 		__DLG_TOOL_HPP__

#ifndef __INC_LOADDRES_HPP__
#include "loadRes.hpp"
#endif

//
// Class sSheetDialog provides 3 types of multipage dialogs	
//	1. WizardDlg	- Prev / Next pDialog
//	2. PagesDlg		- Dialog controlled by CTabCtrl
//	3. PropertyDlg	- Dialog controlled by CTreeCtrl
// Both CTabCtrl/CTreeCtrl are referred to as manager controls.
//
// Dialog resources needed will contain one container (host) dialog and several
// dialog pages (sheets). Container dialog must contain some prescribed controls,
// which are used as placeholders for sheet dialog elements such as tree control
// or place for dialog sheets. (The requirements depend on the dialog type.)
// Any number of additional controls are allowed; these controls are processed acc. to
// MFC standards - DoDataExchange() etc.
//
// Individual pages should be constructed as normal sDllDialog's and added
// to the sSheetDialog via addPage().
// Construction of the sDllDialog is very similar to standard CDialog; in fact
// the only substantial difference is in the constructor, where resource module
// containing dialog resources has to be supplied.
// The page dialog should behave acc. to MFC standards, i.e. override functions
// such as OnInitDialog(), DoDataExchange(), OnCommand() etc. This applies to both
// dialog sheets and the container dialog.
//
// addPage() returns reference to the page description structure sSheetInfo.
// This structure can be used to define tree hierarchy in PropertyDlg type dialog;
// it has little importance for other dialog types.
//
// To run the multipage dialog call DoModal() .
//

//
//------ Tree controlled Property sheet ------
//
// Steps to do:
// Create dialog resource <dialogId> with 2 controls:
//	treeCtrlId	= tree control with (suggested) styles:
//					Visible, Tab stop
//	pgCtrlId	= picture control for dialog sheets with (suggested) styles:
//					Visible, Tab stop
//
// Construct property dialog:
//	sSheetDialog dlg(
//		res_module,		// resource module containing dialogId; NULL if static resources used
//		PropertyDlg,	//
//		dialogId,		//
//		treeCtrlId,		// 
//		pgCtrlId,		// id for control which define page area (control will be hidden afte dlg init)
//		NULL ) ;		// or suitable parent window
//
// Construct individual pages (sDllDialog's) and add them to property dialog:
//		sSheetInfo &inf = dlg.addPage( myDlgPage, "pageTitle" ) ;
// Use the return value to define tree hierarchy: E.g. command
//		inf << inf1 << inf2 ;		// the same as: inf<<inf1; inf1<<inf2
// will define following hierarchy:
//
//		inf
//		 |
//		 |-- inf1
//			| 
//			|__inf2
//

//
//------ Tab controlled Property sheet ------
//
// Dialog resource <dialogId> should contain following control:
//	tabCtrlId	= tab control with (suggested) styles:
//					Visible, Tab stop
//
//	sSheetDialog dlg(
//		res_module,		// resource module containing dialogId; NULL if static resources used
//		PagesDlg,		//
//		dialogId,		//
//		tabCtrlId,		//
//		0,				//
//		NULL ) ;		// or suitable parent window
//
//

//------ Wizard constructor ------
//
// Dialog resource <dialogId> should contain following controls:
// <Cancel>		IDCANCEL
// <Finish>		IDOK
// <Next>		nextBtnId (contructor parameter)
// <Previous>	prevBtnId (contructor parameter)
// group box	areaId (contructor parameter) = place holder for pages
//
//	sSheetDialog dlg(
//		res_module,		// resource module containing dialogId; NULL if static resources used
//		WizardDlg,		//
//		dialogId,		//
//		areaId,			//
//		prevBtnId,		//
//		nextBtnId		//
//		NULL ) ;		// or suitable parent window
//
// You should override sSheetDialog::OnCommand() to process IDOK.
//

//
// Remarks:
//
// - To run the Properties dialog call
//		dlg.DoModal() ;
//
// - The pages can be located in different resource module as the Property sheet.
//
// - To define sheet icons define bitmap resource in the static resources and
//	 pass it to addPage(). (See addPage() description.)
//	 The icons can be used for PropertyDlg or PagesDlg dialog types.
//
// - The description is simplified; in the reality you should construct
//   sSheetDialog derived class and override standard CDialog overridables.
//
// - With carefull design page dialogs can be used as both property sheets and
//	 popup dialogs:
//	 Construct dialog resource as needed for popup dialog. (title, border, Ok button...)
//	 It is suggested that "Visible" style is off. (To minimize screen flicker.)
//	 In OnInitDialog() determine whether popup dialog or property sheet is used.
//	 (E.g. via some flag). In the latter case simply hide <Ok> button.
//


//----------------------------------------------------------------------------
// sSheetInfo
// (Provides basic information about sSheetDialog pages (sheets).)
//----------------------------------------------------------------------------


class sSheetInfo;
typedef sTemplateArray<sSheetInfo*>  sSheetInfoPtrArray;

class sSheetInfo
{
	friend class sSheetDialog ;

	sSheetInfoPtrArray	 aChildPages;
	sSheetInfo			*pParentSheet;
	sDllDialog			*pDialog;
	HTREEITEM			 hTreeItem;
	CString 			 cTitle ;
	UINT				 uImageID;
	long				 lValue;

	void	clear();				

  public:
	// subpage support
	inline	int			getSubpageCount() const			{ return aChildPages.count(); }
	inline	sSheetInfo *getSubpage	( int i ) const		{ return i<0 && i>=aChildPages.count() ? NULL : aChildPages[i]; }


	// Base access values
	inline	UINT		getDialogID	() const			{ return pDialog ? pDialog->getDlgID() : 0;}	// get pDialog resourse ID
	inline	sDllDialog *getDialog	() const			{ return pDialog;		}			// get pointer to sDllDialog
	inline	sSheetInfo *getParent	() const			{ return pParentSheet;	};			// get pParentSheet sheet structure
	inline	LPCTSTR		getTitle	() const			{ return cTitle;		};			// get page cTitle
	inline	BOOL		isPageEmpty	() const			{ return !pDialog || !getDialogID(); }

	// User defined value submitted via sSheetDialog::addPage()
	inline	long		getLValue	() const			{ return lValue; }
	inline	void		setLValue	( long l)			{ lValue=l; }

	// Add child page
	inline  sSheetInfo &addNewSheet	( sSheetInfo &item)	{ aChildPages.add( &item ); item.pParentSheet=this; return *this; }
	inline  sSheetInfo &operator << ( sSheetInfo &item)	{ return addNewSheet( item ); }

	sSheetInfo( sDllDialog *dlg=NULL,LPCTSTR titl=NULL, UINT imgID=0 ) ;
} ;


//----------------------------------------------------------------------------
//	sSheetDialog
//----------------------------------------------------------------------------


class sSheetDialog	: public sDllDialog
{
	friend class SheetDialogControlAsDialog;

  public:
	enum PageWndType {	WizardDlg,		// Prev / Next pDialog
						PagesDlg,		// Dialog width CTabCtrl
						PropertyDlg,	// Dialog width CTreeCtrl
						UserDefinedDlg};// used internally for SheetDialogControlAsDialog()

  private:
	sSheetInfoPtrArray	aPageList  ;
	sTemplateArray<UINT> aImageIDList;
	BOOL				bInProtectMode;
	BOOL				bInInitMode;
	UINT 				uPrevButtonId ;
	UINT				uNextButtonId ;
	int 				iActivePage	;

	CImageList			m_ImagesList ;
	PageWndType			m_eDialogType ;
	CRect				m_rPagesAreaRect  ;
	CWnd			   *m_pMngControl ;
	UINT				m_uMngControlID; 
	UINT				m_uPageAreaCtrlID ;

	// private functions
	void				enableNextPrev	( );
	void				destroyPageDlg	( sSheetInfo *inf);
	void				createTreeItem	( sSheetInfo *pPageInfo );
	void				createMngItem	( sSheetInfo *pPageInfo );
	void				initialize		( PageWndType type, UINT ctrlID, UINT areaID );
	BOOL				showPage		( sSheetInfo *pPageInfo, BOOL swShow, BOOL callOnPageShowed );
	void				expandAll		( BOOL expand=TRUE );	// PropertyDlg only

	// find functions
	int 				findPageInfo	( sSheetInfo *page )	{ return page ? aPageList.find( page ) : -1; }	
	int					findImageByID	( UINT uImageID );

	// notifications
	void				onPageShowed	( sSheetInfo *cur, BOOL show );

  protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE( sSheetDialog )

  protected:
	// By default pages are created dynamically - on as needed basis.
	// These calls can be used to explicitly control page creation (to be called in OnInitDialog() or later):
			void		 createAllPages   ( ) ;
			BOOL		 createPage		  ( sSheetInfo *info );

	// MFC framework
	afx_msg void		 OnDestroy		  ( ) ;
	virtual BOOL		 OnNotify		  ( WPARAM wParam, LPARAM lParam, LRESULT* pResult );
	virtual BOOL		 OnCommand		  ( WPARAM wParam, LPARAM lParam	);
	virtual BOOL		 OnInitDialog	  ( ) ;

	// Called by the framework when the page is about to change.
	// showNextPage = TRUE/FALSE in WizardDlg depending on which button is pressed,
	//				  undefined in remaining dialog types
	// Default implementation returns return value from updateData() for shown page.
	virtual BOOL	 isPageChangingAllowed( BOOL showNextPage ) { return updateData( TRUE, TRUE); }

	// These functions realize page exchange.
	// Override to add some extra processing, but do not forget to call base class implementation.
	// -1 <= oldPage, newPage < getNumPages()   (-1 is used for undefined page)
	virtual void		 onPageChanged	  ( int oldPage, int newPage);
	virtual void		 onPageChanged	  ( sSheetInfo *oldP, sSheetInfo *newP);

  public:
	// Dialog page has following attributes (defined during addPage() call):
	//		id		refers to the dialog resource id (sDllDialog::getDlgID())
	//		page	page itself (sDllDialog)
	//		label	displayed in the tree, or tab control, or as dialog title (Wizard)
	// Pages need not be located in the same resource module. This implies that page id
	// need not be unique.

	inline	int 		 getNumPages	( ) const				{ return aPageList.count();}; 

	// Page search
	// Remark: it is the user reponsibility to ensure that id and title are unique.
			sSheetInfo	*getPageInfo	( UINT id );		// find page by dialog resource id
			sSheetInfo	*getPageInfo	( LPCTSTR label);	// find page by page label
	inline	sDllDialog	*getPageDialog	( UINT id )			{ sSheetInfo *pInf=getPageInfo(id);		return pInf ? pInf->getDialog() : NULL; }
	inline	sDllDialog	*getPageDialog	( LPCTSTR label)		{ sSheetInfo *pInf=getPageInfo(label);	return pInf ? pInf->getDialog() : NULL; }

	// Active page
			sSheetInfo	*getShownPageInfo ( );
	inline	sDllDialog	*getShownPageDlg  ( )					{ sSheetInfo *pInf=getShownPageInfo();	return pInf ? pInf->getDialog()  :NULL; }
	inline  UINT		 getShownPageDlgId( )					{ sSheetInfo *pInf=getShownPageInfo();	return pInf ? pInf->getDialogID():NULL; }

	// Show page - explicit commands
	virtual BOOL		 showNextPage	( ) ;
	virtual BOOL		 showPrevPage	( ) ;
	inline	BOOL		 showPage		( sSheetInfo *page )	{ return showPage( page, TRUE, TRUE ); }
			BOOL		 showPage		( UINT id ) ;

	// add/delete page
	// add/delete subpage has meaning only for structured PropertyDlg
	//		
	//	 pParent	- create page for this parent (NULL mean that page will be in root)
	//	 page		- in PropertyDlg tree item without dialog sheet; ignored for other dialog types
	//	 label		- page title (tab or tree item label)
	//	 lValue		- user defined value accessible via sSheetInfo::getLValue() (e.g. in onPageChanged())
	//	 uImageID	- bitmap resource id (16x16, 16 colors) placed in static resources
			sSheetInfo  &addSubpage		( sSheetInfo *pParent, sDllDialog *page, LPCTSTR label= 0, long lValue = 0, UINT uImageID= 0 ) ;
	inline	sSheetInfo  &addPage		(					   sDllDialog *page, LPCTSTR label= 0, long lValue = 0, UINT uImageID= 0 ) { return addSubpage( NULL, page, label, lValue, uImageID ) ; }
			BOOL		 delPage		( sSheetInfo *i );	// can be used dynamically (within DoModal())
			BOOL		 delAllSubpages	( sSheetInfo *i );	// PropertyDlg only (other dialogs have no hierarchy)

	// Change page label dynamically
			BOOL		 setPageLabel	( sSheetInfo *i, LPCTSTR newLabel ) ;

	// Sending Windows message
			void	     sendMsgToAllPages( UINT message, WPARAM wParam=0, LPARAM lParam=0 );

	// Send IDOK to all pages; return TRUE iff all pages return TRUE.
	// The page which returns FALSE should display the reason in the message box.
			BOOL		 sendOkToAllPages( ) ;

	// Call UpdateData() for <this> dialog and visible sheet (if onlyShownPage=TRUE) or
	// all sheets (otherwise).
	// Return FALSE if at least one of the calls to UpdateData() returns FALSE.
	virtual BOOL		 updateData		( BOOL bSaveAndValidate=TRUE, BOOL onlyShownPage=FALSE );

	// constructor for PropertyDlg and PagesDlg
	sSheetDialog( ResDllDesc *mod,		// Resource module where <dlgID> is located
				  PageWndType type, 	// sSheetDialog::PropertyDlg or PagesDlg
				  UINT dlgID,			// host dialog id
				  UINT tabCtrlID,		// tree or tab control id
				  UINT areaID,			// PropertyDlg: control used to hold pages, PagesDlg: 0
				  CWnd* pParent ) ;		// dialog parent (NULL allowed)

	// WizardDlg constructor
	sSheetDialog( ResDllDesc *mod,		// Resource module where <dlgID> is located
				  PageWndType type, 	// sSheetDialog::WizardDlg
				  UINT	dlgID,			// host dialog id
				  UINT	areaID, 		// control id of the page placeholder (0 if dlgId area is to be used)
				  int	prevBtnId,		// control Id of <Previous> button (0 if not defined)
				  int	nextBtnId,		// control Id of <Next> button (0 if not defined)
				  CWnd* pParent ) ;		// dialog parent (NULL allowed)
};

#endif
