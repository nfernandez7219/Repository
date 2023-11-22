/* 2.01 version */
enum enumerrors
{
// general error messages

 errorsMSG_EOF                , /* unexpected End-Of-File encountered \n%s*/
 errorsMSG_FILENAME    	      , /* incorrect file name \n%s*/
 errorsMSG_NOSUCHFILE  	      , /* can't find file \n%s*/
 errorsMSG_NOSUCHDIR	      , /* can't find directory \n%s*/
 errorsMSG_OPENFILE    	      , /* can't open file \n%s*/
 errorsMSG_READFILE    	      , /* error at reading file \n%s*/
 errorsMSG_WRITEFILE   	      , /* error at writing file \n%s*/
 errorsMSG_NOTEMPFILE  	      , /* can't create temporary file*/
 errorsMSG_DIRNOTEMPTY 	      , /* directory not empty \n%s*/
 errorsMSG_MAKEDIR	      , /* can't make directory \n%s*/
 errorsMSG_NOSUCHDEV          , /* device %s does not exist*/
 errorsMSG_INVDRIVE           , /* invalid drive %s*/
 errorsMSG_DISKFULL           , /* disk %s is full */
 errorsMSG_WRITEFAULT         , /* write fault */
 errorsMSG_READFAULT          , /* read fault */

 errorsMSG_INTERRUPT          , /* %s interrupted */
 errorsMSG_USRCANCEL          , /* %s interrupted by the user */
 errorsMSG_ABORT              , /* %s aborted */
 errorsMSG_FAIL               , /* %s failed */
 errorsMSG_BUSY               , /* %s busy */
 errorsMSG_NOTREADY           , /* %s not ready */
 errorsMSG_NOACCESS           , /* access denied*/
 errorsMSG_NOTSUPPORTED       , /* %s not supported */
 errorsMSG_NOTINITIALIZED     , /* %s not initialized */
 errorsMSG_NOTREGISTERED      , /* %s not registered */
 errorsMSG_BADVERSION  	      , /* bad version*/
 errorsMSG_BADGOAL            , /* bad goal */
 errorsMSG_FATAL              , /* fatal error \n%s*/
 errorsMSG_INTERNALERROR      , /* internal error \n%s*/

 errorsMSG_OVERFLOW           , /* overflow */
 errorsMSG_OUTOFRANGE         , /* out of range */
 errorsMSG_NOSOLUTION         , /* no solution */
 errorsMSG_SYNTAXERROR        , /* syntax error */
 errorsMSG_ALLOCERROR         , /* no memory */
 errorsMSG_SHARINGVIOLATION   , /* sharing violation */

 errorsMSG_INVDATE            , /* invalid date */
 errorsMSG_INVARG             , /* invalid argument */
 errorsMSG_INVPARS        	  , /* invalid parameters */
 errorsMSG_INVDATA            , /* invalid data */
 errorsMSG_NOPARS             , /* no parameters */
 errorsMSG_NODATA             , /* no data */
 errorsMSG_MOREDATA           , /* more data expected */
 errorsMSG_BUFFERFULL         , /* buffer full */
 errorsMSG_BUFFEROVF          , /* buffer overflow */
 errorsMSG_TIMEOUT            , /* timeout */

 errorsMSG_INVCMD             , /* invalid command */
 errorsMSG_INVICON            , /* invalid icon */
 errorsMSG_INVBITMAP          , /* invalid bitmap */
 errorsMSG_INVRECORD          , /* invalid record */
 errorsMSG_INVID              , /* invalid identity number */
 errorsMSG_INVFORMAT          , /* invalid format */
 errorsMSG_INVPASSWORD        , /* invalid password */

 errorsMSG_OUTOFPAPER         , /* out of paper */
 errorsMSG_PRINTQFULL         , /* print queue full */
 errorsMSG_PRINTCANCELLED     , /* print cancelled */
 errorsMSG_PRINTFAILED        , /* print failed */

 errorsMSG_LOCKVIOLATION      , /* lock violation */
 errorsMSG_NOTLOCKED          , /* not locked */
 errorsMSG_LOCKFAILED         , /* lock failed */
 errorsMSG_UNLOCKFAILED       , /* unlock failed */

 errorsMSG_NETWORKERROR       , /* network error */
 errorsMSG_NONETACCESS        , /* network access denied */
 errorsMSG_NETWRITEFAULT      , /* network write fault */
 errorsMSG_BADNETNAME         , /* bad network name */
 errorsMSG_BADNETPATH         , /* bad network path */

// outdated
 errorsMSG_DbExec             , /* DBS */
 errorsMSG_DbRowTooLong       , /* DBS */
 errorsMSG_DbInternErr        , /* DBS */
 errorsMSG_DbConnect          , /* DBS */
 errorsMSG_DbDisconnect       , /* DBS */
 errorsMSG_DbCancel           , /* DBS */


// screen.lib special error codes have these message equivalents

 errorsMSG_FILELIST        	  , /* error creating file list*/
 errorsMSG_DATESECTNOTFOUND   , /* date section not found in configuration file */
 errorsMSG_DATESYNTAXERROR    , /* syntax error in date section in configuration file */
 errorsMSG_CFG_DUPLICATESECT  , /* duplicate section in configuration file found */
 errorsMSG_CFG_SYNTAXERROR    , /* syntax error in configuration file */
 errorsMSG_CFG_NOACCESS       , /* no run-time permission to change the configuration */

// screen.lib internal error messages

 errorsMSG_BADDLGARG          , /* Bad argument type in dialog stream operator\n"
                                 " (Item no. %d declared as @%d.%d%c)"*/
 errorsMSG_DLGSYNTAXERROR     , /* Syntax error in dialog format analysis:\n%s*/
 errorsMSG_LogCfgSetup        , /* Message system initialization failed -\n"
                                 "  config section\n    %s\n  not found or empty"*/
 errorsMSG_LogCfgDevSetup     , /* Message system initialization failed -\n"
                                 "  config file error in device filter:\n  [%s].%s = %s"*/

 errorsMSG_MENURESOURCE       , /* Error loading menu resource\n  %s*/
 errorsMSG_USRNOTREGISTERED   , /* user %s.%s not registered*/

 errorsMSG_NoHelpFile         ,/*Help file\n  %s\nnot found.*/

// language file errors

 errorsMSG_LANG_NOSUCHFILE    , /* can't find language file %s */
 errorsMSG_LANG_OPENFILE      , /* can't open language file %s */
 errorsMSG_LANG_BADVERSION    , /*
                                "\nOld language file %s"
                                "\nmin. version should be %.2f \n"*/
 errorsMSG_LANG_READERROR     , /*
                                "\n\n language file %s - "
                                "\n error at or before line no. %d "
                                "\n (message %s)\n"*/
 errorsMSG_LANG_DEFAULT	      , /*
		                        "\n  -- WARNING --\n"
		                        "\nLocal language file error:\n%s\n\n"
		                        "\nLoading default english version."*/

// configuration file errors

 errorsMSG_ENV_NOTEMPFILE     , /* Can't open temporary file to save the config file */
 errorsMSG_ENV_NOSUCHFILE     , /* Can't find config file:\n %s */
 errorsMSG_ENV_OPENFILE       , /* Can't open config file:\n %s */
 errorsMSG_ENV_WRITEFILE      , /* Can't write config file:\n %s */
 errorsMSG_ENV_SYNTAXERROR    , /* Syntax error in config file:\n  [%s]\n  (line %d) */
 errorsMSG_ENV_DUPLICATESECT  , /* same section name used twice in config file\n  [%s]\n  (line %d) */
 errorsMSG_ENV_CONFIGERROR    , /* Error reading config file\n  %s\n  (line %d */
 errorsMSG_ENV_READFILE       , /* Can't read config file:\n %s */
 
 errorsMSG_UNKNOWNERR         , /* Unknown error %d */

// sserver
 errorsMSG_IndexfileFull	  , /*indexfile full*/
 errorsMSG_NameAlreadyUsed	  , /*'%s' - name already used*/
 errorsMSG_ServerHasBadRecs	  , /*'%s' - Server contains invalid records.*/

// ComPort
 errorsMSG_PortOpen			  , /*Can't open port %s:\n%s*/
 errorsMSG_PortLine			  , /*%s - incorrect port settings:\n   %d Bd, bits:%d, stop bits:%s, parity:%s*/
 errorsMSG_PortTimeout		  , /*%s - timeout failed*/
 errorsMSG_PortSetControl	  , /*%s - control line error*/
 errorsMSG_PortWrite		  , /*%s - write error*/
 errorsMSG_PortRead			  , /*%s - read error*/

 errorsMSG_UnknownError       , /* Unknown error */
} ;

