
/* 2.73 version */

enum  enumScreen
{
    SCRMSG_OK			   	   ,/* Ok. */
    SCRMSG_CANCEL		   	   ,/* Cancel */
    SCRMSG_YES			   	   ,/* Yes */
    SCRMSG_NO			   	   ,/* No */
    SCRMSG_HELP	               ,/* Help */
    SCRMSG_ERROR               ,/* Error */
    SCRMSG_USERNAME            ,/* Username */
    SCRMSG_PASSWORD            ,/* Password */
    SCRMSG_LOGIN               ,/* Login */
    SCRMSG_PressAnyKey         ,/* Press any key to continue ... */
    SCRMSG_SelectYesNo         ,/* Select  ENTER = Yes  ,  any other key = No ... */
    SCRMSG_SelectOkCancel      ,/* Select  ENTER = Ok  ,  any other key = Cancel ... */
    SCRMSG_FatalError			,/* fatal error */
    SCRMSG_Information			,/* information */
    SCRMSG_Error				,/* error */
    SCRMSG_Warning				,/* warning */
    SCRMSG_Message				,/* message */
    SCRMSG_AskSaveConfig		,/* Configuration has changed; save the changes ?" */

// env1.cpp
	SCRMSG_ErrSaveCfgFile		,/*Error saving configuration file*/
	SCRMSG_CantCreateCfgFile	,/*Can't create new configuration file*/

// env.cpp
	SCRMSG_ReloadBadSect		,/*Reload of not existing section required*/
	SCRMSG_ErrReadCfg			,/*Error reading configuration file*/

// sserver.cpp/hpp
    SCRMSG_AskDeleteThem		,/*Server contains invalid records.\nDelete them?*/
    SCRMSG_NumRecsCleaned		,/*# of records deleted*/
    SCRMSG_AskRetryMakeDir		,/*Can't make directory.\nTry to remove the reason.\n\nRetry ?*/
    SCRMSG_InvalidIndexFile		,/*Indexfile contains invalid data.*/
    SCRMSG_CompVersionsFmt		,/*IndexFile has version %d,\nwhereas the application expects version %d.*/
    SCRMSG_ErrReadData			,/*Error reading data*/
    SCRMSG_Server				,/*Server*/
    SCRMSG_BadIndexfile			,/*Bad indexfile*/
    SCRMSG_AskRecreateIndex		,/*Should it be recreated?\n\n(All data will be lost.*/

// stripes.cpp
	ScrMsg_UndefinedStripes		,/*Stripes/Checks: undefined*/
	ScrMsg_Stripes				,/*Stripes*/
	ScrMsg_Checks				,/*Checks*/
	ScrMsg_UpToEnd				,/*up to end*/
	ScrMsg_StripeError			,/*Error in stripe line description*/
	ScrMsg_CheckError			,/*Error in check line description*/
	ScrMsg_IllegalStripes		,/*Illegal stripe description*/


// LoadRes.cpp (AppLib)
	SCRMSG_DlgFailed			, /*The dialog failed*/
	SCRMSG_DlgFailedUnknown		, /*Unknown error during loading of the dialog.*/
	SCRMSG_ResLoadFailed		, /*Failure at loading the resource*/
	SCRMSG_NativeDllFailed		, /*Load of native resource DLL failed.*/
	SCRMSG_DefaultDllLoaded		, /*Default (english) version loaded.*/
	SCRMSG_LoadResDllFailed		, /*Load of resource DLL failed*/
	SCRMSG_CheckWhereIsDll		, /*\n\nCheck whether you have this DLL and whether it is located\nin %s directory\nor in the PATH.*/
	SCRMSG_BadDllVersion		, /*The DLL has incorrect version: '%s'
								   "\n('%s' or higher expected.)"
								   "\n\nDll version can be checked e.g. via Windows Explorer (File Properties)."*/
	SCRMSG_LoadError			, /*Load error.*/
	SCRMSG_NoDllVersion			, /*DLL version missing.*/

// Lang.cpp (AppLib)
	SCRMSG_Failed				, /*failed*/
	SCRMSG_ResourceDll			, /*resource DLL*/
	SCRMSG_StringTable			, /*string table*/
	SCRMSG_OnlyEngResources		, /*Only english version is available for following resources*/

// sBaseServerObject.cpp
	SCRMSG_ObjNameTooLong		, /*Object name too long.*/
	SCRMSG_NoObjectAccess		, /*No privilege to access the object.*/
	SCRMSG_NoReadAccess			, /*No privilege to read the object.*/
	SCRMSG_CantMakeAdr			, /*Can't create address */
	SCRMSG_ObjectIsInUse		, /*Object is in use.*/
	SCRMSG_CantLocateObj		, /*Data server could not locate required object.*/
	SCRMSG_CantOpenObjects		, /*Some object(s) could not be opened:*/
	SCRMSG_UnknownError			, /*Unknown error*/
	SCRMSG_InvPrivSettings		, /*Invalid privilege settings*/

// texColor.cpp
	SCRMSG_Black				, /*Black*/
	SCRMSG_Green				, /*Green*/
	SCRMSG_Blue					, /*Blue*/
	SCRMSG_Cyan					, /*Cyan*/
	SCRMSG_Red					, /*Red*/
	SCRMSG_Magenta				, /*Magenta*/
	SCRMSG_Brown				, /*Brown*/
	SCRMSG_LightGray			, /*Light gray*/
	SCRMSG_DarkGray				, /*Dark gray*/
	SCRMSG_LightBlue			, /*Light blue*/
	SCRMSG_LightGreen			, /*Light green*/
	SCRMSG_LightCyan			, /*Light cyan*/
	SCRMSG_LightRed				, /*Light red*/
	SCRMSG_LightMagenta			, /*Light magenta*/
	SCRMSG_Yellow				, /*Yellow*/
	SCRMSG_White				, /*White*/
} ;
