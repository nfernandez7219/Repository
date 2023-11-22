#ifndef __ClientCfg_hpp__
#define __ClientCfg_hpp__

#ifndef __DvbUser_hpp__
#include "DvbUser.hpp"
#endif

#ifndef __BASECFG_HPP__
#include "BaseCfg.hpp"
#endif

struct DvbMuxPacketStatistics ;
class  ProtectedFile ;


//////////////////////////////////////////////////////
////////////////     DvbClientCfg     ////////////////
//////////////////////////////////////////////////////
class DvbClientCfg : public CfgBaseSetup
{
	friend class DvbClientSetup;

  private:
	char	*_logFileName;
	char	*_connectString;
	uint	_maxEntries;
	int		_deleteSubdirectory;
	BOOL	_changed;

  public:
	DvbClientCfg( char *fileName );
   ~DvbClientCfg() ;

	void		storage				 ( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR lpErrBuffer=NULL );
	BOOL		dvbDriverDumpOn		 () const ;
	inline BOOL	isChanged			 ()						{ return _changed; }

	void		setLogFileName		 ( const char *name ) ;
	void		setMaxEntries		 ( ulong maxEntries ) ;
	void		setDeleteSubdirectory( int state ) ;
};


//////////////////////////////////////////////////////
////////////////    DvbClientSetup    ////////////////
//////////////////////////////////////////////////////
class DvbClientSetup
{
	friend void getMuxPacketStatistics( DvbMuxPacketStatistics *st ) ;

  private:
	// statistics
	time_t					_startTime;
	ulong					_numPackets;
	ulong					_numSuccPackets;
	ulong					_numProcessedPackets;
	ulong					_numInternetPackets;
	ulong					_numFilesTransferred;
	ulong					_numFilesAcceptedSuccessfully;
	ulong					_numRebroadcast;
	ulong					_totalFileSizeTransferred;
	ulong					_numMessagesTransferred;
	ulong					_numMessagesAcceptedSuccessfully;

	DvbClientCfg			*_clientCfg;
	CfgContext				*_context;
	DvbUserLog				*_userLog;
	BigComInp				*_comInp;

	char					*_drive;
	char					*_dir;

	ProtectedFile			*_activitiesLogFile;		// activities log file

  public:
	DvbClientSetup				();
   ~DvbClientSetup				();

	inline  char	*drive				()						{ return _drive; };
	inline  char	*dir				()						{ return _dir; };

			void	load				();
			void	save				();

	// DvbUserLog access functions ----------------------------------
	inline  DvbClientCfg	*clientCfg	()						{ return _clientCfg; }
	inline  DvbUserLog		*userLog	()						{ return _userLog; }
	inline  BigComInp		*comInp		()						{ return _comInp ; }
	inline  CfgContext		*context	()						{ return _context; }

	inline  const GlobalUserID &userID	() const				{ return _userLog->userID(); }
	inline  BOOL	isHNetAllowed		()						{ return _userLog->isHNetAllowed(); }
	inline  BOOL	isHWFilteringAllowed()						{ return _userLog->isHWFilteringAllowed(); }
	inline  UINT	nChannels			()						{ return _userLog->nChannels(); }
	inline  USHORT	channels			( ushort i )			{ return _userLog->channels( i ); }
	inline  BOOL	hasChannel			( ushort ch )			{ return _userLog->hasChannel( ch ); }

	inline  const char *getChannelNameByID( ushort c )			{ return _userLog->getChannelNameByID( c ); }
	inline  void	setChannelNameByID	( ushort c, char *name)	{ _userLog->setChannelNameByID( c, name ); }
	inline  const char	*getChannelName	( ushort i )			{ return _userLog->getChannelName( i ); }
	inline  void	setChannelName		( ushort i, char *name)	{ _userLog->setChannelName( i, name ); }

	// comInp access functions -------------------------------------
			void	openComInp			( void (*hook)()=0 );
			void	closeComInp			();
			BOOL	startComInp			();
			void	stopComInp			();

	// log file ----------------------------------------------------
	inline void		setLogFileName		( const char *name )	{ _clientCfg->setLogFileName(name) ; }
	inline char	   *getLogFileName		()						{ return _clientCfg->_logFileName; }

	inline ulong	getMaxEntries		()						{ return _clientCfg->_maxEntries; }
	inline void		setMaxEntries		( ulong maxEntries )	{ _clientCfg->setMaxEntries(maxEntries) ; }

	inline int		deleteSubdirectory	()						{ return _clientCfg->_deleteSubdirectory; }
	inline void	   setDeleteSubdirectory( int state )			{ _clientCfg->setDeleteSubdirectory( state); }

	// mux packets statistics ---------------------------------------
		   void clearStatistics			();
		   void saveStatistics			( const char *prompt=NULL );

	inline void incPackets				()								{ _numPackets++; };
	inline void decPackets				()								{ _numPackets--; };
	inline void incSuccPackets			()								{ _numSuccPackets++; };
	inline void incProcessedPackets		()								{ _numProcessedPackets++; };
	inline void incInternetPackets		()								{ _numInternetPackets++; };

	inline void incNumFilesTransferred				()					{ _numFilesTransferred++; };
	inline void incNumFilesAcceptedSuccessfully		()					{ _numFilesAcceptedSuccessfully++; };
	inline void incNumRebroadcast					( ulong num = 1 )	{ _numRebroadcast += num; };
	inline void addToTotalFileSizeTransferred		( ulong size )		{ _totalFileSizeTransferred += size; };
	inline void incNumMessagesTransferred			()					{ _numMessagesTransferred++; };
	inline void incNumMessagesAcceptedSuccessfully	()					{ _numMessagesAcceptedSuccessfully++; };

	inline ulong getNumPackets						()					{ return _numPackets; };
	inline ulong getNumInternetPackets				()					{ return _numInternetPackets; };
};


extern DvbClientSetup *clientSetup;

inline DvbClientSetup *MfxClientSetup()		{ return clientSetup; };


#endif