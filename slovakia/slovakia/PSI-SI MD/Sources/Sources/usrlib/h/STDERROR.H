/*
 * standard error codes
 */

#ifndef INC_STDERROR_H

#define INC_STDERROR_H

        // Input/Output
#define ERR_EOF         	-1      /* %s -\n unexpected End-Of-File encountered */
#define ERR_FILENAME    	-2      /* incorrect file name -\n  %s */
#define ERR_NOSUCHFILE		-3      /* no such file -\n  %s*/
#define ERR_NOSUCHDIR		-4      /* %s\n  no such directory*/
#define ERR_OPENFILE    	-5      /* Can't open file\n  %s*/
#define ERR_READFILE    	-6      /* Error at reading file\n  %s*/
#define ERR_WRITEFILE   	-7      /* Error at writing file\n  %s*/
#define ERR_NOTEMPFILE		-8      /* Can't create temporary file*/
#define ERR_DIRNOTEMPTY		-9      /* Directory not empty*/
#define ERR_MAKEDIR	    	-10     /* Can't make directory\n  %s*/
#define ERR_NOSUCHDEV       -11
#define ERR_INVDRIVE        -12
#define ERR_DISKFULL        -13
#define ERR_WRITEFAULT      -14
#define ERR_READFAULT       -15
#define ERR_UNKNOWNERROR	-16

#define ERR_INTERRUPT       -30
#define ERR_USRCANCEL		-31     /* Interrupted by the user */
#define ERR_ABORT           -32
#define ERR_FAIL            -33
#define ERR_BUSY            -34
#define ERR_NOTREADY        -35
#define ERR_NOACCESS		-36     /* %s\n  access denied*/
#define ERR_NOTSUPPORTED    -37
#define ERR_NOTINITIALIZED  -38
#define ERR_NOTREGISTERED   -39
#define ERR_BADVERSION		-40     /* %s\n  bad version*/
#define ERR_BADGOAL         -41
#define ERR_FATAL		    -42     /* Fatal error\n  %s */
#define ERR_INTERNALERROR   -43

#define ERR_OVERFLOW         -60
#define ERR_OUTOFRANGE  	 -61
#define ERR_NOSOLUTION  	 -62
#define ERR_SYNTAXERROR		 -63
#define ERR_ALLOCERROR		 -64     /* no memory */
#define ERR_SHARINGVIOLATION -65

#define ERR_INVDATE          -79
#define ERR_INVARG           -80
#define ERR_INVPARS        	 -81
#define ERR_INVDATA          -82
#define ERR_NOPARS           -83
#define ERR_NODATA           -84
#define ERR_MOREDATA         -85
#define ERR_BUFFERFULL       -86
#define ERR_BUFFEROVF        -87
#define ERR_TIMEOUT          -88

#define ERR_INVCMD           -100
#define ERR_INVICON          -101
#define ERR_INVBITMAP        -102
#define ERR_INVRECORD        -103
#define ERR_INVID            -104
#define ERR_INVFORMAT        -106
#define ERR_INVPASSWORD      -107

#define ERR_OUTOFPAPER       -130
#define ERR_PRINTQFULL       -131
#define ERR_PRINTCANCELLED   -132
#define ERR_PRINTFAILED      -133

#define ERR_LOCKVIOLATION    -140
#define ERR_NOTLOCKED        -141
#define ERR_LOCKFAILED       -142
#define ERR_UNLOCKFAILED     -143

#define ERR_NETWORKERROR     -150
#define ERR_NONETACCESS      -151
#define ERR_NETWRITEFAULT    -152
#define ERR_BADNETNAME       -153
#define ERR_BADNETPATH       -154

#define ERR_DbExec           -201      /* database error; query execution failed */
#define ERR_DbRowTooLong     -202      /* database error; row is too long */
#define ERR_DbInternErr      -203      /* database error; internal container error */
#define ERR_DbConnect        -204      /* database error; connection failed */
#define ERR_DbDisconnect     -205      /* database error; disconnect failed */
#define ERR_DbCancel         -206      /* database error; command execution did not close corectly */

#define ERR_PortOpen         -221
#define ERR_PortLine         -222
#define ERR_PortTimeout      -223
#define ERR_PortSetControl   -224
#define ERR_PortWrite        -225
#define ERR_PortRead         -226

#define ERR_IndexFileFull	 -250
#define ERR_NameAlreadyUsed	 -251
#define ERR_ServerHasBadRecs -252

// screen.lib special error codes have these message equivalents

#define ERR_FILELIST    	  -900 /* error creating file list*/
#define ERR_DATESECTNOTFOUND  -901
#define ERR_DATESYNTAXERROR   -902
#define ERR_ENV_DUPLICATESECT -903
#define ERR_ENV_SYNTAXERROR   -904
#define ERR_ENV_NOACCESS      -905

// screen.lib internal error codes

#define ERR_BADDLGARG           -910
#define ERR_DLGSYNTAXERROR      -911
#define ERR_LogCfgSetup         -912
#define ERR_LogCfgDevSetup      -913
#define ERR_MENURESOURCE        -914
#define ERR_USRNOTREGISTERED    -915

#define ERR_LANG_NOSUCHFILE     -916
#define ERR_LANG_OPENFILE       -917
#define ERR_LANG_BADVERSION     -918
#define ERR_LANG_READERROR      -919
#define ERR_LANG_DEFAULT        -920

#define ERR_NOHELPFILE          -921


char Z_USERDLL *siStdErrorString( char *str, int err_code, ... ) ;
char Z_USERDLL *stdErrorString  ( char *str, int err_code, char *arg=NULL ) ;


#endif						/* INC_STDERROR_H */
