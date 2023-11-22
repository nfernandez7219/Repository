/*
 *	Filename:		clientcfg.cpp
 *
 *	Version:		1.00
 *
 *	Description: Main receiver config and setup classes.
 *
 *	History:
*/

#include "tools2.hpp"
#include "Mux.hpp"

#include "ClientCfg.hpp"
#include "MfxGlobals.hpp"
#include "ProtectedFile.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern BigComIO *bigComIO ;

#define MAX_NUMBER_OF_ENTRIES	15000



//////////////////////////////////////////////////////
////////////////     DvbClientCfg     ////////////////
//////////////////////////////////////////////////////


// These values are set to config file at first-time installation.
// NULL value means config section;
// Default section is NULL section.
static char *defaults[] = {
	// variable				  value						  comment
	//------------------------------------------------------------
	"LogFile"			, "Log\\MD_DvbReceiver.log,1000", "file name,entry number; this file is used to log events, holds maximum entry number entries (0=infinite)",
	"ConnectString"			, "Dvb"						, "Dvb / 12345/tcp / 12345/udp / lpt1 / com1; wich device to use in communication",
	"DeleteSubdirectory"	, "0"						, "0/1; delete the channels directory with all entries if channel is no longer valid? (0 - no, 1 - yes)",

	"[CardState]"			, 0							, "Setup for ISA-422 Receiver Card",
	"Flags"					, "0xe2"					, "0xnn",
	"ACKtimeout"			, "10"						, "[ms]",
	"Speed"					, "2000"					, "[kHz]",

	"[DriverState]"			, 0							, "Setup for ISA-422 Receiver Card",
	"DriverMemory"			, "100"						, "# of packets",
	"Flags"					, "0"						, 0
};

#define n_defaults (sizeof(defaults)/sizeof(char*)/3)


DvbClientCfg::DvbClientCfg( char *fileName ) : CfgBaseSetup( fileName )
{
	_logFileName	= STRDUP( "Log\\md_Receiver.log" );
	_maxEntries		= 1000;
	_changed		= TRUE;
	_connectString	= NULL;
	_deleteSubdirectory = 0;
	USE_PESHEADER	= 0 ;
}

DvbClientCfg::~DvbClientCfg()
{
	if( _logFileName )
		FREE( _logFileName );
	if( _connectString )
		FREE( _connectString );
}

void DvbClientCfg::setLogFileName( const char *name )
{
	if( _logFileName )
	{
		if( stricmp(_logFileName,name) == 0 )
			return ;
		FREE(_logFileName) ;
	}
	_logFileName = STRDUP(name);
	_changed = TRUE;
}

void DvbClientCfg::setMaxEntries( ulong maxEntries )
{
	if( _maxEntries != maxEntries )
	{
		_maxEntries = maxEntries ;
		_changed = TRUE;
	}
}

void DvbClientCfg::setDeleteSubdirectory( int state )
{
	if( _deleteSubdirectory != state )
	{
		_deleteSubdirectory = state;
		_changed = TRUE;
	}
}


	
void DvbClientCfg::storage( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR lpErrBuffer )
{
	if( saveFlag )
	{
		if( fileExist( fileName ) && !_changed )
			return;

		_maxEntries = __max( _maxEntries, 0 );
		_maxEntries = __min( _maxEntries, MAX_NUMBER_OF_ENTRIES );
		config->printf( "", "LogFile", "%s,%d", _logFileName, _maxEntries ) ;

		set( "", "ConnectString", _connectString );
		setInt( "", "DeleteSubdirectory" , _deleteSubdirectory );
	}
	else
	{
		char warningMsg[1024];
		char *nextWarning = warningMsg;
		*nextWarning = 0;

		if( !fileExist( fileName ) || !_changed )
			return;

		char *str;

		// read LogFile
		str=get( "", "LogFile" );
		if( str )
		{
			char *end = strchr( str, ',' );

			if( _logFileName )
				FREE( _logFileName );
			_logFileName = STRDUP( str );
			if( end )
			{
				_logFileName[end-str] = 0;
				int maxEntries=1000;		// default
				if( !isInt( end+1, &maxEntries ) )
					nextWarning += sprintf( nextWarning, "\n\nIllegal 'LogFile' specification" ) ;
				_maxEntries = maxEntries;
			}
			if( _maxEntries  < 0 )
				_maxEntries = 1000;
			else if( _maxEntries > MAX_NUMBER_OF_ENTRIES )
				_maxEntries = MAX_NUMBER_OF_ENTRIES;
		}


		str = get( "", "ConnectString" );
		if( str != NULL )
		{
			FREE( _connectString );
			_connectString = STRDUP( str );
		}

		char expl[256] ;
		BOOL connectStringOk = BigComIO::isConnectStringOk( _connectString, expl) ;
		if( expl[0] != 0 )
			nextWarning += sprintf( nextWarning, "\n\n%s", expl ) ;

		int delSubdir = 0;
		getInt( "", "DeleteSubdirectory" , &delSubdir );
		if( delSubdir != 0 )
			_deleteSubdirectory = 1;
		else
			_deleteSubdirectory = 0;

		if ( nextWarning != warningMsg )
		{
			char buf[1024] ;
			sprintf( buf, "Following setting(s) were found illegal or suspicious:%s\n"
					"\nCorrect in the configuration file (%s), please.",
					warningMsg, config->fileName() ) ;
			MessageBox( NULL, buf, "Warning", MB_OK | MB_ICONINFORMATION | MB_TOPMOST );
		}
	}

	_changed  = FALSE;
}


//////////////////////////////////////////////////////
////////////////    DvbClientSetup    ////////////////
//////////////////////////////////////////////////////


DvbClientSetup::DvbClientSetup()
{
	struct	tm		*gmt;
			BOOL	corruptedFile, newFile;
			char	path[1024], drive[20], dir[1024];

	GetModuleFileName( NULL, path, 1024 ) ;
	_splitpath( path, drive, dir, NULL, NULL ) ;
	_drive	= STRDUP( drive );
	_dir	= STRDUP( dir );

	// load client configuration
	_makepath( path, _drive, _dir, "Config\\ContextR.cfg", NULL ) ;
	_context   = new CfgContext( path )		 ;

	_makepath( path, _drive, _dir, "Config\\DvbClient.cfg", NULL );
	_clientCfg = new DvbClientCfg( path );
	try
	{
		//_clientCfg->openConfig();
		_clientCfg->cfg()->install( (const char**)defaults, n_defaults ) ;
	}
	catch( Msg &msg )
	{
		stdErrorDialog( "(DvbClient.cfg)\n\n%s", msg.shortString ) ;
	}
	catch( ... )
	{
	}
	//_clientCfg->load();

	// load user configuration
	_makepath( path, _drive, _dir, "Config\\DvbUser.cfg", NULL );
	_userLog = new DvbUserLog( path );

	_comInp	= new BigComInp;
	bigComIO= _comInp ;

	_numPackets							= 0;
	_numSuccPackets						= 0;
	_numProcessedPackets				= 0;
	_numInternetPackets					= 0;

	_numFilesTransferred				= 0;
	_numFilesAcceptedSuccessfully		= 0;
	_numRebroadcast						= 0;
	_totalFileSizeTransferred			= 0;

	_numMessagesTransferred				= 0;
	_numMessagesAcceptedSuccessfully	= 0;

	time( &_startTime );

	// activities log file
	_makepath( path, _drive, _dir, "Activities.log", NULL );
	_activitiesLogFile = new ProtectedFile( path, ProtectedFile::P_DATA );
	if( access( path, 0 ) == 0 )			//if exist file exist
	{
		corruptedFile = _activitiesLogFile->test() != 0;
		if( corruptedFile )
		{
			char buff1[1024], buff2[1024];

			_makepath( buff1, _drive, _dir, "Activities.log.old", NULL );
			if( access( buff1, 0 ) == 0 )
			{
				_makepath( buff2, _drive, _dir, "Activities.log.old.old", NULL );
				if( access( buff2, 0 ) == 0 )
				{
					::SetFileAttributes( buff2, FILE_ATTRIBUTE_NORMAL );
					remove( buff2 );
				}
				::SetFileAttributes( buff1, FILE_ATTRIBUTE_NORMAL );
				rename( buff1, buff2 );
				::SetFileAttributes( path, FILE_ATTRIBUTE_NORMAL );
				rename( path, buff1 );
			}
			else
				rename( path, buff1 );
			_activitiesLogFile->open( ProtectedFile::F_WRITE );
		}
		else
		{
			_activitiesLogFile->open( ProtectedFile::F_MODIFY );
			_activitiesLogFile->fSeek( 0, SEEK_END );
		}
		newFile = FALSE;
	}
	else
	{
		_activitiesLogFile->open( ProtectedFile::F_WRITE );
		corruptedFile = FALSE;
		newFile = TRUE;
	}

	sprintf( path, "\r\n====================================================================================================\r\n" );
	_activitiesLogFile->fWrite( path, strlen( path ), 1 );
	gmt = localtime( &_startTime );
	strftime( dir, 1024, "%A, %B %d, %Y", gmt );
	sprintf( path, "Date: %s\r\n", dir );
	_activitiesLogFile->fWrite( path, strlen( path ), 1 );
	if( newFile )
	{
		sprintf( path, "Activities.log file not found, new log file created.\r\n" );
		_activitiesLogFile->fWrite( path, strlen( path ), 1 );
	}
	if( corruptedFile )
	{
		sprintf( path, "Old Activities.log file damaged, new log file created.\r\n" );
		_activitiesLogFile->fWrite( path, strlen( path ), 1 );
	}
	gmt = localtime( &_startTime );
	strftime( dir, 1024, "%H:%M:%S", gmt );
	sprintf( path, "Application started at       %s\r\n", dir );
	_activitiesLogFile->fWrite( path, strlen( path ), 1 );
	_activitiesLogFile->fFlush();
}


DvbClientSetup::~DvbClientSetup()
{
	closeComInp();
	save();

	time_t actualTime;
	char buffer[200], time_buf1[20], time_buf2[20], time_buf3[20];

	time( &actualTime );
	struct tm *gmt = localtime( &actualTime );
	strftime( time_buf2, 20, "%H:%M:%S", gmt );

	saveStatistics( "Application stopped at" );
	
	gmt = localtime( &_startTime );
	strftime( time_buf1, 20, "%H:%M:%S", gmt );
	actualTime -= _startTime;
	gmt = gmtime( &actualTime );
	strftime( time_buf3, 20, "%H:%M:%S", gmt );
	sprintf( buffer, "\r\nRunning time:\t%s - %s ( %s )\r\n", time_buf1, time_buf2, time_buf3 );
	_activitiesLogFile->fWrite( buffer, strlen( buffer ), 1 );

	_activitiesLogFile->close();

	bigComIO = NULL ;
	delete _comInp;
	delete _userLog;
	delete _clientCfg;
	delete _context;
	delete _activitiesLogFile;
	FREE( _drive );
	FREE( _dir );
}


void DvbClientSetup::load()
{
	CATCH_AND_DISPLAY_EXCEPTION( _clientCfg->load() );
//	CATCH_AND_DISPLAY_EXCEPTION( _userLog->load() );
	CATCH_AND_DISPLAY_EXCEPTION( _context->load() );
}


void DvbClientSetup::save()
{
	CATCH_AND_DISPLAY_EXCEPTION( _clientCfg->save() );
	CATCH_AND_DISPLAY_EXCEPTION( _userLog->save() );
	CATCH_AND_DISPLAY_EXCEPTION( _context->save() );
}


//------------------------------------------------------------------------------
// Basic control of the input communication channel.
//------------------------------------------------------------------------------


void DvbClientSetup::openComInp( void (*hook)() )
{
	char expl[512];
	char shortMsg[512] ;

	const char *connectStr = _clientCfg->_connectString ;
	if( !_comInp->create( connectStr, expl) )
	{
		sprintf( shortMsg, "%s: %s", connectStr, expl ) ;
		throw Msg( -1, "Can't open comunication protocol\n%s\n\nApplication will be shutdown.", shortMsg ) ;
	}

	int comErr = _comInp->open( clientCfg()->cfg(), hook );
	if( comErr == 0 )
		return ;

	DvbEventText( comErr, expl) ;

	sprintf( shortMsg, "%s: %s", connectStr, expl ) ;
	MfxPostMessage( EMsg_CommunicationError, 0, shortMsg ) ;

	char buf[1024] ;
	sprintf( buf,
		"Can't open comunication protocol \"%s\" because of\n"
		"    \"%s\"\n"
		"\n"
		"No data can be received.\n"
		"To correct the problem you may try following:\n"
		"    - check the installation;\n"
		"    - run Setup dialog;\n"
		"    - edit configuration file config\\DvbClient.cfg.",
		connectStr, expl ) ;

	// Don't use AfxMessageBox() because it will have splash window as parent -> disappears.
	::MessageBox( NULL, buf, "Main Data DVB Receiver", MB_OK|MB_ICONERROR|MB_TOPMOST ) ;
}


BOOL DvbClientSetup::startComInp()
{
	if( _comInp->isStarted() )
		return TRUE ;
	if( !_comInp->start() )
		return FALSE ;

			time_t	actualTime;
	struct	tm		*gmt;
			char	buffer[50], time_buff[20];

	time( &actualTime );
	gmt = localtime( &actualTime );
	strftime( time_buff, 50, "%H:%M:%S", gmt );
	sprintf( buffer, "Communication started at     %s\r\n", time_buff );
	_activitiesLogFile->fWrite( buffer, strlen( buffer ), 1 );
	_activitiesLogFile->fFlush();
	return TRUE ;
}


void DvbClientSetup::stopComInp()
{
	if( !_comInp->isStarted() )
		return ;

			time_t	actualTime;
	struct	tm		*gmt;
			char	buffer[50], time_buff[20];

	_comInp->stop();
	time( &actualTime );
	gmt = localtime( &actualTime );
	strftime( time_buff, 50, "%H:%M:%S", gmt );
	sprintf( buffer, "Communication stopped at     %s\r\n", time_buff );
	_activitiesLogFile->fWrite( buffer, strlen( buffer ), 1 );
	_activitiesLogFile->fFlush();
}


void DvbClientSetup::closeComInp()
{
	if( _comInp == NULL )
		return;

	//ComInp *com = _comInp->com();
	//if( com != NULL && com->isOpened() )
	_comInp->close();
}


void DvbClientSetup::clearStatistics()
{
	saveStatistics( "Application statistics cleared at" );

	_numPackets							= 0;
	_numSuccPackets						= 0;
	_numProcessedPackets				= 0;
	_numInternetPackets					= 0;

	_numFilesTransferred				= 0;
	_numFilesAcceptedSuccessfully		= 0;
	_numRebroadcast						= 0;
	_totalFileSizeTransferred			= 0;

	_numMessagesTransferred				= 0;
	_numMessagesAcceptedSuccessfully	= 0;

	if( _comInp != NULL )
		_comInp->clearStatistics() ;
}

void DvbClientSetup::saveStatistics( const char *prompt )
{
	if( prompt == NULL )
		prompt = "Statistics saved at" ;

	struct	tm		*gmt;
			char	buf1[256], buf2[20];
	time_t  timeNow;

	sprintf( buf1, "\r\n----------------------------------------------------------------------------------------------------\r\n" );
	_activitiesLogFile->fWrite( buf1, strlen( buf1 ), 1 );

	time( &timeNow );
	gmt = localtime( &timeNow );
	strftime( buf2, 1024, "%H:%M:%S", gmt );
	sprintf( buf1, "%s         %s\r\n", prompt, buf2 );
	_activitiesLogFile->fWrite( buf1, strlen( buf1 ), 1 );

	char buffer[512];
	int  len ;

	// Files
	if( _numFilesTransferred == 0 )
		len = sprintf( buffer, "Files: -\r\n" ) ;
	else
	{
		len = sprintf( buffer, "Files:\t\tOK: %lu / %lu ; %.4f rebr./file ; total size transf. %.4fK\r\n",
			_numFilesAcceptedSuccessfully, _numFilesTransferred,
			(float)_numRebroadcast / (float)_numFilesTransferred,
			(float)_totalFileSizeTransferred  / 1024.0f );
	}
	_activitiesLogFile->fWrite( buffer, len, 1 );

	// Messages
	sprintf( buffer, "Messages:\tOK %lu / %lu\r\n", _numMessagesAcceptedSuccessfully, _numMessagesTransferred );
	_activitiesLogFile->fWrite( buffer, strlen( buffer ), 1 );

	// App packets
	len = sprintf( buffer, "App packets:\t%lu / %lu ; filtered:", _numSuccPackets, _numPackets );

	if( isHWFilteringAllowed() )
		len += sprintf( buffer+len, " N/A ; " );
	else
		len += sprintf( buffer+len, " %lu ; ", _numSuccPackets - _numProcessedPackets );

	if( isHNetAllowed() )
		len += sprintf( buffer+len, " IP : %lu\r\n", _numInternetPackets );
	else
		len += sprintf( buffer+len, " IP: N/A\r\n" );

	_activitiesLogFile->fWrite( buffer, len, 1 );

	// DVB card
	//if( _comInp->getStatisticsLog( buffer) )
	//	_activitiesLogFile->fWrite( buffer, strlen(buffer), 1 );

	// Card packets
	if( _comInp->getStatisticsLog( buffer) )
		_activitiesLogFile->fWrite( buffer, strlen( buffer ), 1 );

	_activitiesLogFile->fFlush();
}
