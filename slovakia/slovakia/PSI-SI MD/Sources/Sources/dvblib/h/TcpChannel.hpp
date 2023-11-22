class TcpChannel : public MuxChannel
{
	HANDLE		   _sendingThread ;

  public:
	TcpChannel( Mux *mux, const MuxChannelSetup *s, BOOL online=TRUE ) ;
   ~TcpChannel( ) ;
	virtual BOOL  start		  () ;
	virtual BOOL  initiateStop() ;
	virtual BOOL  stop		  () ;

	inline void	  setFatalError( DWORD err ) ;		// immediate stop
	BOOL   _isKilled ;
	inline int	  sendData( const char *buf, int cnt, const CardSerialNumber& usrId ) ;
};
