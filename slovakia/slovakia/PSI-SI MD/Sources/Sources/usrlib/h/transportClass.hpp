#ifndef __INC_TRANSPORTSETUP_HPP__
#define __INC_TRANSPORTSETUP_HPP__

#ifndef __DLG_TOOL_HPP__
	#include "Dlg_tool.hpp"
#endif
#include "transport.hpp"

//
// TransportClass represents one end point of the communication channel -
// either client side or server side.
// Communication is done through transport medium. Currently available media are:
//	- tcp	connection oriented protocol; enables both send and reply
//	- MSMQ	connectionless protocol; enables only data sending
//
// Transport client selects one of these media, transport server is listening on either
// or both of them. Both client and server have to agree on the connection parameters,
// which are encoded in the "connectString".
//
// Setup data is stored permanently in the configuration file, which is read by the class
// constructor. Besides that the setup may be changed either programatically (functions
// such as setMediumConnectString()), or interactivelly.
// Interactive support includes either setup of the individual transport media
// (e.g. askUserConnectString()), or setup of the whole TransportClass object -
// this includes also medium selection.
//
// Once the connection is established, data exchange can start. Hereby the client can
// send the data (sendClientData()) and receive the reply (getClientReply()).
//
// Server starts listening with the call to startServerReceivingData().
// This function receives as the parameter your callback function of the prototype
//		BOOL callback( TransportMediumMsg *rcv, TransportMediumMsg *reply, LPARAM param )
// Here
//		rcv  = Data sent by the client via sendClientData().
//		reply= Reply structure to be filled (NULL for connectionless protocol);
//			   this data will be received by the client via getClientReply().
//		param= Parameter supplied to startServerReceivingData().
// Function has to return TRUE iff the reply data was generated.
//
// Remark:
// For the definition of TransportMediumMsg, resp. callback function type see transport.hpp.
//


class TransportClass ;

//
// Base class for medium-dependent setup dialogs.
// The dialog cannot be constructed by the user, but only retrieved via call to
//		TransportClass::getConnectionSetupDlg()
//
class UTMConnectionSetupDialog : public sDllDialog
{
  protected:
	TransportClass *_transportClass ;
	BOOL			_bUseAsSheetPage;

	virtual BOOL	OnInitDialog	( );
	virtual BOOL	OnCommand		( WPARAM wParam, LPARAM lParam );

	UTMConnectionSetupDialog( TransportClass *t, UINT dlgID, CWnd *parent, BOOL bUseAsSheetPage );

  public:
	virtual	void	getConnectString( char *str ) ;
	virtual	void	setDefaults		( ) ;

	// Server only
	virtual	BOOL	getMediumAllowed( ) ;
	virtual	void	setMediumAllowed( BOOL yes ) ;
} ;


//
// Setup dialog for transport connections.
// Can be used as:
//	- property page in the Preferences dialog
//	- standalone popup dialog (use DoModal())
//
// On the client side it selects single active connection.
// On the server side more connections can be enabled.
//
class TransportClassSetupDialog : public sSheetDialog
{
	TransportClass	*_transportClass ;
	void			*_typePage ;
	BOOL			 _runAsPopup ;

  protected:
    virtual BOOL OnInitDialog       ( ) ;
	virtual BOOL OnCommand          ( WPARAM wParam, LPARAM lParam) ;

  public:
    TransportClassSetupDialog		( TransportClass *t, CWnd *par=0 );

	void		setDefaults			( ) ;
	static UINT IDD					( ) ;		// dialog id
	virtual BOOL DoModal			( ) ;		// popup dialog
} ;


// 
//
// TCP connectString syntax:
//	[host]:port
//		host = IP-adress or machine name or name defined in the host file;
//			   compulsory for client, ignored for server
//		port = port number
// Example:
//		If the machine "tester" has IP 129.87.64.2 and the host file contains following line
//			129.87.64.2  CuttexAmaServer
//		then following connect strings are equivalent:
//			tester:12346
//			129.87.64.2:12346
//			CuttexAmaServer:12346
//
// MSMQ connectString syntax:
//	 machine\queueName[format_name]
//		machine		= name of computer containing the queue
//		queue_name	= queue name (you can find it using MSMQ Explorer - see documentation)
//					- it is case insensitive
//		format_name	= GUID of the queue 
//					- it is optional (can be empty)
//					- increases the speed of opening queue
// Example:
//			tester\AmaQueue[PUBLIC=823b40d1-e3c7-11d3-b346-00a0242d8fa3]
//
class TransportClass
{
	friend class TransportWnd ;

  private:
	const char				  *_name ;
	const char				  *_key ;
	ConfigClass				  *_cfg ;
	BOOL					   _isServer ;
	sStringPtrArray			   _mediumNames ;
	BOOL					   _blocked ;
	USHORT					   _defTcpPort ;
	CCriticalSection		   _lock ;

	// client data
	int							clientReferences ;
	UniversalTransportMedium*	clientConnection ;
	TransportMediumHandshakeFun _handshakeFnc ;
	LPARAM						_handshakeParam ;
	TransportMediumMsg			_handshakeSendMsg ;

	// server data
	CWnd					  *_wnd ;
	TransportMediumRcvFun	   _srvRcvFnc ;
	LPARAM					   _srvParam ;
	UniversalTransportMedium*	serverConnection[2] ;
	BOOL					   _mediumModuleInstalled[2] ;

	BOOL startReceivingMediumData ( UTM_TYPE mediumType, char *errReason ) ;
	void stopReceivingMediumData  ( UTM_TYPE mediumType ) ;
	BOOL openAsClient			  (	UTM_TYPE mediumType, char *errReason, BOOL interactivelly ) ;

	void startTimer				  ( ) ;
	void stopTimer				  ( ) ;
	void sendAliveSignal		  ( ) ;

	// e.g. during resetServerConnection()
	inline void setBlocked		  ( BOOL yesNo )		{ _blocked = yesNo ; }
	inline BOOL isBlocked			  ( ) const				{ return _blocked ; }

  public:

	enum	MediumMask	{ mTCP = 1, mMSMQ = 2, } ;

	// name = language dependent string used for presentation purposes (e.g. message box title).
	//		  Example: "Automarker Queue"
	// key  = language independent string used as the name of the config file and MSMQ queue
	//		  Example: For "AMQueue" corresp. config file will be "\cs\mfc\cfg\AMQueue.cfg"
	//		  and MSMQ queue will be called "AMQueue".
	// defTcpPort = tcp port used on setDefaults()
	// mediumMask - you can disable some transport medium using this flag 
	//		  by default both TCP and MSMQ are supported ( mTCP|mMSMQ value )
	TransportClass( const char *name, const char *key, BOOL is_server, USHORT defTcpPort=0, int mediumMask = mTCP|mMSMQ ) ;
   ~TransportClass( ) ;

	// List all available media.
	inline const
	sStringPtrArray &getMediumNames	  ( ) const			{ return _mediumNames ; }

	inline const char	 *name		  ( ) const			{ return _name ; }
	inline BOOL			  isServer	  ( ) const			{ return _isServer ; }

	//
	// ----------------- setup ----------------------------
	//

	// TransportClass setup dialog (modal popup dialog).
	// Performs complete setup of the class. (all media + medium selection)
	BOOL		setupDialog			  ( CWnd *parent ) ;

	// Setup dialog for given medium.
	// It returns new dialog object, which can be either
	// - added to the sSheetDialog (via addPage(); don't delete in this case!), or
	// - used as popup dialog (via DoModal(); must be deleted)
	// - deleted.
	//
	UTMConnectionSetupDialog *
				getConnectionSetupDlg ( UTM_TYPE medium, CWnd *parent, BOOL bUseAsSheetPage  ) ;
	UTMConnectionSetupDialog *
				getConnectionSetupDlg ( const char *mediumName, CWnd *parent, BOOL bUseAsSheetPage ) ;

	// Independent popup window specifying connection setup for given medium.
	// Returns FALSE iff user cancels the setup dialog.
	BOOL		askUserConnectString  ( UTM_TYPE medium, char *connectStr, CWnd *parent ) ;

	// The medium is fully qualified as <mediumType,connectString>;
	// syntax of the connect string is described above the class definition.
	const char *getMediumConnectString( UTM_TYPE medium, char *buf ) ;
	void		setMediumConnectString( UTM_TYPE medium, const char *buf ) ;

	// returns medium's peer computer name or IP (or clears buffer if medium isn't opened)
	// parameter medium is used only if trasport class is opened as server (otherwise clientConnection is used)
	// locationBuffer must be large enough
	void		getPeerLocation		  ( char *locationBuffer, UTM_TYPE medium = UniversalTransportMedium::UnknownMedium ) ;

	// Support functions for setup dialog
	inline BOOL	isChanged			  ( )						{ return _cfg->isChanged() ; }
	inline BOOL save				  ( BOOL say_error=FALSE)	{ return _cfg->save(say_error) == 0 ; }

	// Returns FALSE if last open failed due to absence of medium supporting library.
	// (For example it might happen that MSMQ is not installed.)
	inline BOOL mediumModuleInstalled ( UTM_TYPE mediumType )	{ return _mediumModuleInstalled[mediumType] ; }	
	
		   BOOL undo				  ( ) ;
		   void setDefaults			  ( UTM_TYPE medium ) ;
		   void saveAndValidate		  ( ) ;

	//
	// ----------------- client interface -----------------
	//

	// Specify handshake procedure performed during each connection.
	// (By default no handshake is performed.)
	void		setClientHandshake	  (
		TransportMediumHandshakeFun f,		// function called to validate reply packet during handshake
		LPARAM param,						// parameter passed to hadshake function
		TransportMediumMsg *packet ) ;		// packet sent to server during handshake

	inline TransportMediumHandshakeFun
					 handshakeFunction( ) const		{ return _handshakeFnc ; }
	inline LPARAM    handshakeParam   ( ) const		{ return _handshakeParam ; }
	inline const TransportMediumMsg *
					 handshakeMessage ( ) const		{ return &_handshakeSendMsg ; }

	// Unlike server, client can activate only one medium. This can be done either via
	// medium type or via medium name.
	UTM_TYPE	getClientMediumType	  ( ) ;
	void		setClientMediumType	  ( UTM_TYPE medium ) ;
	const char *getClientMediumName   ( ) ;		// NULL if no medium set yet
	void		setClientMediumName   ( const char *mediumName ) ;

	// TRUE iff the connection points to the server running on the same computer.
	BOOL	   isLocalClientConnection( ) ;

	// String characterizing cliet connection
	const char *getClientConnectString( char *buf ) ;	// NULL if no medium set yet

	// Call after config change (e.g. after setClientMedium*())
	void	 resetClientConnection	  ( ) ;

	// Register before using send/reply.
	// Registering increments reference counter, unregistering decrements this counter.
	// When the counter drops to 0, the connection is closed.
	void registerClientConnection	  ( ) ;		// addRef()
	void unregisterClientConnection	  ( ) ;		// releaseRef()

	// isClientConnected() returns TRUE iff the connection is established.
	// (This is done on as needed basis - in first successfull sendClientData(),
	//  or in explicit call to connectClient().)
	BOOL isClientConnected			  ( ) ;

	// Explicit request to connect. The client is connected implicitly in sendClientData().
	// In the interactive mode before failure is returned an attempt to setup
	// the connection (a dialog) is made.
	BOOL connectClient				  ( char *reason, BOOL interactivelly=TRUE ) ;

	// FALSE for connection-less protocols
	BOOL canDoReply					  ( ) ;

	// Send message to the server.
	BOOL sendClientData				  (
		const TransportMediumMsg &snd,		//    data to be sent
		char *errReason=0,					//    explanation in case of failure
		BOOL interactivelly=TRUE ) ;		//    TRUE: connection dialog opens if needed

	// Receive reply; fails if !canDoReply().
	BOOL getClientReply				  (
		DWORD sentPacketId,					//   snd.packetId() (snd = packet being replied)
		TransportMediumMsg &reply,			//   data to be sent
		char *errReason=0 ) ;				//   explanation in case of failure

	//
	// ----------------- server interface -----------------
	//

	// For server more media can be allowed (depending on client needs)
	BOOL isServerMediumAllowed		  ( UTM_TYPE medium ) ;
	void setServerMediumAllowed		  ( UTM_TYPE medium, BOOL allow ) ;

	// Override to receive notifications of connect/disconnect events.
	virtual void onServerMediumConnect( UTM_TYPE medium, BOOL connect )		{}

	// Call after config change (e.g. after setServerMediumAllowed())
	BOOL resetServerConnection		  ( char *reason ) ;

	// Start receiving data on all allowed media.
	// Returns:
	//	TRUE - at least 1 medium was started (errReason - if nonempty - contains warning)
	//	FALSE- no medium started; errReason contains error message
	BOOL startServerReceivingData	  (
			TransportMediumRcvFun fun,		// callback called for received packets
			LPARAM param,					// 3rd parameter passed to callback function
			char  *errReason				// error text on failure or warning on success
		) ;
	void stopServerReceivingData	  ( ) ;
} ;


#endif // __INC_TRANSPORTSETUP_HPP__
