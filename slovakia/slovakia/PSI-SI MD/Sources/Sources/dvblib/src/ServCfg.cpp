/*
 *	Filename:		servcfg.cpp
 *
 *	Version:		1.00
 *
 *	Description: Contains basic classes for application setup.
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"
#include  <math.h>

#include "Inbox.hpp"
#include "DvbUser.hpp"
#include "servCfg.hpp"
#include "service.hpp"
#include "internet.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// These values are set to config file at first-time installation.
// NULL value means config section;
// Default section is NULL section.
static char *defaults[] = {
	// variable				  value						  comment
	//------------------------------------------------------------
	"autoStart"				, "1"						, "0/1; automatic sending after program start",
	"relPriorityPart"		, "40"						, "0-100; % of packets to distribute acc. to rel. priority of channels",
	"AliveSignal"			, "on; each 1 secs"			, "on/off; each 1-5 secs; alive signal interval",
	"ConnectString"			, "Dvb"						, "e.g. Dvb | 12345/tcp | 12345/udp | lpt1 | com1; which device to use in communication",
	"LogFile"			, "Log\\MD_DvbServer.log,1000"	, "file name, max. # of entries (0=infinite)",
	"sendCA_Table"			, "0-24"					, "e.g. 1:23-5:49;15-23; ...list of time intervals when CA Table is broadcast",
	"MonitorringPort"		, "12346"					, "tcp port used for remote monitoring",
	"SinglePID"				, "4137"					, "PID used in single PID mode",
	"UseSinglePID"			, "1"						, "1: all channels use the same PID; 0: more PID's allowed",

	"[MPE]"					, 0							, 0,
	"UseMultiSection"		, "0"						, "0-MPE section is padded with stuffing up to the packet boundary; 1-sections are merged",
	"UdpPort"				, "4321"					, "udp port used in MPE/udp protocol",
	"IpProtocol"			, "237"						, "IP protocol number used in MPE/HN protocol",

	"[Channeldefaults]"		, 0							, 0,
	"Inbox"					, "ToBeSent"				, "the inbox path relative to exe path",
	"AbsPriority"			, "10-"						, "1-10+/-; 1-max., +/- ... on/off",
	"RelPriority"			, "5+"						, "1-10+/-",
	"OutputRate"			, "0.0640-1.1000"			, "min-max output rate [Mbit/s]",
	"MaxVolumePerDay"		, "213-"					, "nnn+/-; max KB to send per day, -=ignore limit",
	"NumRebroadcasts"		, "3"						, "# of rebroadcasts per one file (0=infinite)",
	"RebroadcastDelay"		, "5"						, "[s] delay between rebroabcasts",
	"InboxSendDelay"		, "3"						, "[s] time to wait to send file after it is copied to inbox",
	"InboxSwitchOn"			, "1"						, "0-inactive, 1-active",
	"StreamProtocol"		, "Piping"					, "Piping / MPE_HNET / MPE_UDP",
	"ChannelPID"			, "257"						, "PID used for channel in multi-PID mode",
	"FEC"					, "-Medium; +35"			, "Forward error correction setup",

	"[CardState]"			, 0							, "setup for Card Control Program; better don't change this section",
	"Flags"					, "0x01"					, "0xnn: 0/1-Sender, 0/2-TS header, 0/4-make fill packets, 0/8-ACK timeout on, 0/10-PES header ",
	"Speed"					, "10000"					, "[kHz], frequency of the card",

	"[DriverState]"			, 0							, 0,
	"DriverMemory"			, "100"						, "queue size for the DVB driver (# of packets)",
	"Flags"					, "0"						, "1-dump packet contents, 2-dump packet id's"
};

#define n_defaults (sizeof(defaults)/sizeof(char*)/3)


//----------------------------------------------------------------------------
//	local utilities
//----------------------------------------------------------------------------


static char *freeAndStrdup( char *str, const char *newStr )
{
	if( str )
		FREE( str );
	if( newStr )
		return STRDUP( newStr );
	return NULL;
}


static void copyAndFormat( char *dest, const char *src, int size )
{
	int len = strlen( src ) ;
	while( len>0  &&  src[len-1] <= ' ' )  len-- ;
	if( size && len >= size )
		len  = size-1;

	strncpy( dest, src, len ) ;
	dest[len] = 0 ;
	strlwr( dest );
	dest[0] = (char)toupper( dest[0] );
}


static BOOL decodeSectionNames( const char *sect, char *str1, char *str2 )
{
	char *sep = strrchr( sect, '_' );
	int	  num;

	if( !sep )
		return FALSE;
	
	copyWithoutSpace( str2, sep+1, CfgAddress::TEXT_SIZE );
	num = ( sep-sect >= CfgAddress::TEXT_SIZE ) ? CfgAddress::TEXT_SIZE : sep-sect+1;
	copyWithoutSpace( str1, sect, num );
	return TRUE;
}


//----------------------------------------------------------------------------
//	CfgAddress
//  Stores all data which appear in the user address dialog.
//----------------------------------------------------------------------------


CfgAddress::CfgAddress( const char *srname, const char *nm )
{
	_clear() ;
	if( srname )
		copyAndFormat( _surname, srname, TEXT_SIZE );	
	if( nm )
		copyAndFormat( _name, nm, TEXT_SIZE );	
}

CfgAddress::CfgAddress( const CfgAddress &src ) : _channels()
{
	_copy( src ) ;
	_channels = src._channels ;
}

void CfgAddress::_copy( const CfgAddress &src )
{
	strcpy( _name			,src._name			);
	strcpy( _surname		,src._surname		);
	strcpy( _street			,src._street		); 
	strcpy( _zipCode		,src._zipCode		);
	strcpy( _city			,src._city			);
	strcpy( _country		,src._country		); 
	strcpy( _telephone		,src._telephone		);
	strcpy( _fax			,src._fax			);
	strcpy( _email			,src._email			);
	strcpy( _contactPerson	,src._contactPerson	);
	_id			= src._id ;
	_TCPAddress	= src._TCPAddress;
	_TCPport	= src._TCPport;
}


void CfgAddress::_clear()		// except name & surname
{
	_channels.clearList() ;
	//_name			[0] = 0 ;
	//_surname		[0] = 0 ;
	_street			[0] = 0 ; 
	_zipCode		[0] = 0 ;
	_city			[0] = 0 ;
	_country		[0] = 0 ; 
	_telephone		[0] = 0 ;
	_fax			[0] = 0 ;
	_email			[0] = 0 ;
	_contactPerson	[0] = 0 ;
	_TCPAddress			= (127<<24) | 1;
	_TCPport			= 12346;
}

LPCTSTR CfgAddress::ipAddressToString( LPTSTR lpBuff )
{
	sprintf( lpBuff, "%d.%d.%d.%d:%d", 
				(_TCPAddress&0xFF000000)>>24,
				(_TCPAddress&0x00FF0000)>>16,
				(_TCPAddress&0x0000FF00)>> 8,
				(_TCPAddress&0x000000FF),
				_TCPport );
	return lpBuff;
}

CfgAddress &CfgAddress::operator= ( const CfgAddress &src )
{
	_copy( src ) ;
	_channels = src._channels ;
	return *this;
}

// Set allowed channels
void CfgAddress::setChannels( short *channels, int count )
{
	_channels.clearList() ;
	for( int j=0 ; j < count ; ++j )
		_channels.add( channels[j] ) ;
}

// This is not ==, but test whether address denotes same users.
BOOL CfgAddress::isIdentical( const CfgAddress &adr ) const
{
	if( textEqual( _surname, adr.surname()) &&
		textEqual( _name,    adr.name()   ) && 
		textEqual( _street,  adr.street() ) &&
		textEqual( _city,    adr.city()   ) &&
		textEqual( _country, adr.country()) )
		return TRUE;
	return FALSE;
}

BOOL CfgAddress::operator==( const CfgAddress &adr ) const 
{
	if( !isIdentical(adr) )
		return FALSE ;
	if( !textEqual( _zipCode	  ,adr._zipCode		 )  ||
		!textEqual( _telephone	  ,adr._telephone	 )  ||
		!textEqual( _fax		  ,adr._fax			 )  ||
		!textEqual( _email		  ,adr._email		 )  ||
		!textEqual( _contactPerson,adr._contactPerson)  )
		return FALSE ;

	if( _TCPAddress	!= adr._TCPAddress	||
		_TCPport	!= adr._TCPport )
		return FALSE ;

	if( _channels.count() != adr._channels.count() )
		return FALSE ;
	if( memcmp( (ushort*)_channels, (ushort*)adr._channels, _channels.size()) != 0 )
		return FALSE ;
	return TRUE;
}

static char *nextMessageStart		= NULL;
static int   numNotExistingChannels	= 0;
static BOOL  someChannelIdTruncated = FALSE ;

// Used to define allowed channels for this user.
// str = text stored in config file (or edited in the dialog)
void CfgAddress::setChannelsFromText( const char *str )
{
	_channels.clearList() ;

	char	   *buf = strdup(str) ;
	const char *wrd = strtok( buf, " \t," ) ;

	CfgChannelsSetup *channels = MfxChannelsSetup();

	while( wrd != NULL )
	{
		int ind = atoi( wrd ) ;
		if( ind > 0 )
		{
			if ( channels && nextMessageStart )
			{
				int i;
				BOOL exist = FALSE;
				for ( i = 0; i < channels->numChannels(); i++ )
				{
					const CfgChannel *ch = channels->channel( i );
					if ( ch->serviceID() == ind )
					{
						exist = TRUE;
						break;
					}
				}
				if ( !exist )
				{
					nextMessageStart += sprintf( nextMessageStart, " %d,", ind );
					numNotExistingChannels++;
					if ( numNotExistingChannels % 15 == 0 )
						nextMessageStart += sprintf( nextMessageStart, "\n" );
				}
			}
			if( ind > USHRT_MAX  ||  ind < 0 )
				someChannelIdTruncated = TRUE ;
			_channels.add( (ushort)ind ) ;
		}
		wrd = strtok( NULL, " \t," ) ;
	}
	free( buf ) ;

	// delete duplicates
	for( int i=_channels.count()-1; i>=0; i-- )
		if( _channels.find( _channels[i] ) != i )
			_channels.del( i );
}

// Conversion of allowed channels to text.
const char *CfgAddress::channelsAsText( char *str )
{
	str[0] = 0 ;
	for( int j=0 ; j < _channels.count() ; ++j )
	{
		char  buf[80] ;
		if( j > 0 )
			strcat( str, " " ) ;
		itoa( _channels[j], buf, 10 ) ;
		strcat( str, buf ) ;
	}
	return str ;
}

// Config section used to store the data.
#define SECTION_NAME_SIZE (CfgAddress::TEXT_SIZE*2+2)
const char *CfgAddress :: getConfigSection( char *sect, int size ) const
{
	ASSERT( size >= SECTION_NAME_SIZE );
	sprintf( sect, "%s_%s", surname(), name() );
	return sect;
}

// data exchange between the object and config file
void CfgAddress::storage( CfgBaseSetup *cfg, BOOL saveFlag )
{
	VERIFY( cfg && !isEmpty() );

	char sect[SECTION_NAME_SIZE];
	getConfigSection( sect, SECTION_NAME_SIZE );
	if( saveFlag )
	{
		if( _street[0]	)		cfg->set( sect, "Street", _street  );
		if( _zipCode[0]	)		cfg->set( sect, "ZipCode",_zipCode );
		if( _city[0]	)		cfg->set( sect, "City",   _city    );
		if( _country[0]	)		cfg->set( sect, "Country",_country );
		if( _telephone[0])		cfg->set( sect, "Tel", _telephone  );
		if( _fax[0]		)		cfg->set( sect, "Fax", _fax        );
		if( _email[0]	)		cfg->set( sect, "EMail",_email     );
		if( _contactPerson[0])	cfg->set( sect, "ContPerson",_contactPerson );
		cfg->setInt( sect, "TCPAddress",_TCPAddress );
		cfg->setInt( sect, "TCPport",	_TCPport	);

		char vals[1024];
		channelsAsText( vals ) ;
		cfg->set( sect, "Channels", vals );
	}
	else
	{
		int i;

		_clear() ;
		cfg->get( sect, "Street" , _street, TEXT_SIZE );
		cfg->get( sect, "ZipCode", _zipCode,CODE_SIZE );
		cfg->get( sect, "City"   , _city   ,TEXT_SIZE );
		cfg->get( sect, "Country", _country,TEXT_SIZE );

		cfg->get( sect, "Tel"	 , _telephone,CODE_SIZE );
		cfg->get( sect, "Fax"	 , _fax    , CODE_SIZE  );
		cfg->get( sect, "EMail"	 , _email  , TEXT_SIZE*2  );
		cfg->get( sect, "ContPerson", _contactPerson, TEXT_SIZE*2 );

		cfg->getInt( sect, "TCPAddress",&i );
		_TCPAddress = i;
		cfg->getInt( sect, "TCPport",	&i	 );
		_TCPport	= i;

		char *str = cfg->get( sect, "Channels") ;
		if( str != NULL )
			setChannelsFromText( str ) ;
	}
}


//----------------------------------------------------------------------------
//	CfgProvidersSetup
//	Structure representing Providers definition dialog.
//----------------------------------------------------------------------------


static void deleteAddreses( CfgAddress **adr ) { delete *adr; };
CfgProvidersSetup :: CfgProvidersSetup( const char *file ) : CfgBaseSetup( file )
{
	_providers.setDelFunc( &deleteAddreses );
}

static int findAddress( void *adr2, CfgAddress **adr1 )
{
	return (*adr1)->isIdentical( *((CfgAddress*)adr2) ); 
}

// Checks for identical address. (via isIdentical())
// If found:
//		If modify==FALSE exception.
//		found provider is modified
// else
//		new provider is defined
//
// Also checks is adr channels are not assigned to another provider. (exception)
void CfgProvidersSetup::addOrModifyAddress( CfgAddress &adr, BOOL modify )
{
	// Test if adr already exist
	int index = _providers.find( (void*)&adr, &findAddress );  	
	if( index >= 0  &&  !modify )
		throw Msg( -1, "This provider is already defined." ) ;

	// Create bitmap of ex. channels excl. <adr> channels
	static uchar bits[] = { 1,2,4,8,16,32,64,128 } ;
	uchar bmp[9000] ;
	memset( bmp, 0, sizeof(bmp) ) ;
	for( int j=0 ; j < _providers.count() ; ++j )
	{
		const CfgAddress *prov = provider(j) ;
		if( prov->tmpId() == adr.tmpId() )
			continue ;
		const ushort *provChannels = prov->arrayOfChannels() ;
		for( int i=0 ; i < prov->numChannels() ; ++i )
		{
			int ind = provChannels[i] ;
			bmp[ind/8] |= bits[ind%8] ;
		}
	}

	// Check if <adr> uses some of occupied channels
	const ushort *adrChannels = adr.arrayOfChannels() ;
	for( int i=0 ; i < adr.numChannels() ; ++i )
	{
		int ind = adrChannels[i] ;
		if( bmp[ind/8] & bits[ind%8] )
			throw Msg( -1, "Channel %d is already assigned to another provider.", ind ) ;
	}

	if( index < 0 )
		_providers.add( &adr );
}

// delete passed address
BOOL CfgProvidersSetup::delAddress( CfgAddress &adr )
{
	int index = _providers.find( (void*)&adr, &findAddress );  	

	if( index != -1 )
	{
		openConfig();

		char sect[SECTION_NAME_SIZE];
		adr.getConfigSection( sect, SECTION_NAME_SIZE );
		config->delSection( sect ) ;
		_providers.del( index );
		return TRUE;
	}
	return FALSE;
}

// list of strings in the form "surname name; city"
void CfgProvidersSetup::shortAddressList( sStringPtrArray &list ) const
{
	char buffer[100];
	
	list.clearList();
	for( int i=0; i < _providers.count(); i++ )
	{
		CfgAddress *adr = _providers.item(i);

		sprintf( buffer, "%s %s; %s", adr->surname(), adr->name(), adr->city() );
		list.add( buffer );
	}
}

// data exchange between the object and config file
void CfgProvidersSetup::storage( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR lpErrBuffer )
{
	sStringPtrArray *list=NULL;

	if( lpErrBuffer )
		lpErrBuffer[0] = 0;

	if( saveFlag )
	{
		// Delete all
		cfg->cfg()->clear() ;
		for( int i=0; i<_providers.count(); i++ )
			_providers[i]->storage( cfg, saveFlag );
	}
	else
	{
		list = cfg->cfg()->sectionList();
		_providers.clearList();
		if( list )
		{
			char name   [CfgAddress::TEXT_SIZE+10];
			char surname[CfgAddress::TEXT_SIZE+10];

			char warningMsg[1024];
			nextMessageStart = warningMsg;
			*nextMessageStart = 0;
			nextMessageStart += sprintf( nextMessageStart, "Channel(s)" );
			numNotExistingChannels = 0;
			someChannelIdTruncated = FALSE ;

			for( int i=0; i<list->count(); i++ )
				if( decodeSectionNames( (*list)[i], surname, name ) )
				{
					CfgAddress *adr = new CfgAddress( surname, name );
					adr->storage( cfg, FALSE );
					try
					{
						addOrModifyAddress( *adr, FALSE );
					}
					catch ( Msg &msg1 )
					{
						Msg msg( -1, "Error in providers.cfg file.\r\n%s", msg1.shortString );
						delete adr;
						delete list;
						_providers.clearList();
						nextMessageStart = NULL;
						throw msg ;
					}
				}

			BOOL hasMessage = FALSE ;
			if( numNotExistingChannels )
			{
				nextMessageStart--;			// set position on the ','
				sprintf( nextMessageStart, "\nare registered by some providers but do not exist in\n"
					"the channel configuration file.\n"
					"\nThese channels were not created." );
				hasMessage = TRUE ;
			}
			if( someChannelIdTruncated )
			{
				if( hasMessage )
					strcat( warningMsg, "\n\n" ) ;
				else
					hasMessage = TRUE ;
				strcat( warningMsg, "1 or more channel id's were too long and were truncated." ) ;
			}

			if( hasMessage )
			{
				if( lpErrBuffer )
					strcpy( lpErrBuffer, warningMsg );
				else
					MessageBox( NULL, warningMsg, "Warning", MB_OK | MB_ICONWARNING | MB_TOPMOST );
			}
			nextMessageStart = NULL;
		}
	}
	delete list;
}


//----------------------------------------------------------------------------
//	CfgUser
//	Structure representing user or provider definition dialog.
//----------------------------------------------------------------------------


BOOL CfgUser::operator==( const CfgUser &usr ) const 
{
	if( _address != usr.address() )
		return FALSE;
	if( _userID != usr.userID()  ||  _canHNet != usr.canHybridNet()  ||
		_doHWFiltering != usr.doHWFiltering() )
		return FALSE;
	return TRUE;
}

// test if str contains hexa long number
inline BOOL isULong16( const char *str, ulong *x )
{
	ulong xx ;
	int   cnt ;
	if( sscanf( str, "%lx%n", &xx, &cnt) <= 0 )
		return FALSE ;
	for( str += cnt ; *str; ++str )
		if( *str > ' ' )
			return  FALSE ;
	if( x != NULL )
		*x = xx ;
	return TRUE ;
}

// data exchange between the object and config file
void CfgUser::storage( CfgBaseSetup *cfg, BOOL saveFlag )
{
	VERIFY( cfg && !isEmpty() );

	nextMessageStart = NULL;
	_address.storage( cfg, saveFlag );

	char sect[SECTION_NAME_SIZE];
	getConfigSection( sect, SECTION_NAME_SIZE );
	if( saveFlag )
	{
		cfg->set	( sect, "HardwareCACheck", _doHWFiltering ? "1" : "0" );
		cfg->set	( sect, "HybridNet", _canHNet ? "1" : "0" );
		char buf[20] ;
		_userID.asText( buf ) ;
		cfg->set	( sect, "UserID",	 buf  );
	}
	else
	{
		char *str;
		_doHWFiltering = 0 ;
		_userID.makeInvalid() ;
		_canHNet = 0;

		if( (str = cfg->get( sect, "HardwareCACheck" )) )	
			_doHWFiltering = atoi( str );
		if( (str = cfg->get( sect, "HybridNet" )) )	
			_canHNet = atoi( str );
		if( (str = cfg->get( sect, "UserID")) )
			_userID.fromText( str ) ;
	}
}

// Allocate memory (*table) for UserLog structure and fill it with information.
// This information will be decoded by the Receiver to update local user data.
ulong CfgUser::createUserCATable( char **table )
{
	static char *userCASig = USERCASIGNATURE;

	uchar	muxPacketDataSize = (uchar)(MUXDATASIZE - sizeof( UnicastUserID));

//#ifdef _DEBUG
//	int numOfChannelsInPacket	= ( muxPacketDataSize - sizeof( sUserCA ) + sizeof( ushort )  - sizeof(USERCASIGNATURE) ) / sizeof( ushort );
//	ASSERT( 0 < numOfChannelsInPacket & MAXNUMOFCHANNEL <= numOfChannelsInPacket );
//#endif
	int numOfChannels =	numChannels();
	int numOfPackets = numOfChannels / MAXNUMOFCHANNEL + ( ( numOfChannels % MAXNUMOFCHANNEL ) != 0 );
	if( numOfPackets == 0 )		// user have no channels
		numOfPackets = 1;

	ulong tabSize = numOfPackets * muxPacketDataSize;
	*table = (char *)MALLOC( tabSize );
	memset( *table, 0, tabSize ) ;

	sUserCA	*tab;
	if( numOfChannels > 0 )
	{
		int i = 0, k = 0;
		while( numOfChannels > 0 )
		{
			tab = (sUserCA *)( *table + i * muxPacketDataSize );

			tab->flags = 0;
			if( doHWFiltering() )
				tab->flags |= sUserCA::DO_FILTERING;
			if( canHybridNet() )
				tab->flags |= sUserCA::HNET_ALLOWED;

			if( MAXNUMOFCHANNEL < numOfChannels )
				tab->numChannels = MAXNUMOFCHANNEL;
			else
				tab->numChannels = (ushort)numOfChannels;
			numOfChannels -= tab->numChannels;

			for( int j = 0; j < tab->numChannels; j++, k++ )
			{
				ushort ch = channel( k );
				if( ch == 0xFFFF )			// exclude internet channel; should not happen
					tab->numChannels--;
				else
					tab->channels[j] = ch;
			}
			ASSERT( tab->numChannels >= 0 );

			if( tab->numChannels > 0 )
			{
				tab->fromChannel = tab->channels[0];
				tab->toChannel = tab->channels[tab->numChannels - 1];
			}
			else
			{
				tab->fromChannel = 0;
				tab->toChannel = 0;
			}

			int sigLength = strlen( userCASig );
			char *dest = (char *)tab + muxPacketDataSize - sigLength;
			memcpy( dest, userCASig, sigLength );

			i++;
		}
		ASSERT( i == numOfPackets );
	}
	else
	{
		tab = (sUserCA *)*table;

		tab->flags = 0;
		if( doHWFiltering() )
			tab->flags |= sUserCA::DO_FILTERING;
		if( canHybridNet() )
			tab->flags |= sUserCA::HNET_ALLOWED;

		tab->numChannels = 0;

		int sigLength = strlen( userCASig );
		char *dest = (char *)tab + muxPacketDataSize - sigLength;
		memcpy( dest, userCASig, sigLength );
	}

	tab = (sUserCA *)*table;
	tab->fromChannel = 0;

	tab = (sUserCA *)( *table + ( numOfPackets - 1 ) * muxPacketDataSize );
	tab->toChannel = 0xFFFF;

	return tabSize;
}


//----------------------------------------------------------------------------
//	CfgUsersSetup
//  class describing set of all users
//----------------------------------------------------------------------------


// local functions
static void _deleteUser( CfgUser **usr )
{
	delete *usr;
}
static int _findUser( void *adr2, CfgUser **adr1 )
{
	return (*adr1)->tmpId() == ((CfgUser*)adr2)->tmpId() ; 
}
static int _sortUsers( const void *e1, const void *e2 )
{
	CfgUser *u1 = *(CfgUser **)e1 ;
	CfgUser *u2 = *(CfgUser **)e2 ;
	if( u1->userID() <  u2->userID() )
		return -1 ;
	if( u1->userID() == u2->userID() )
		return 0 ;
	return 1 ;
}

#define RESETALIVESIGNAL()	(as_fromUser = -1)

CfgUsersSetup :: CfgUsersSetup( const char *file ) : CfgBaseSetup( file )
{
	users.setDelFunc( &_deleteUser );
	RESETALIVESIGNAL() ;
	InitializeCriticalSection( &_userServerSetupLock );
}

// modify=TRUE ... modification
// else ... new user added
BOOL CfgUsersSetup::addOrModifyUser( CfgUser &adr, BOOL modify )
{
	EnterCriticalSection( &_userServerSetupLock);

	RESETALIVESIGNAL() ;
	int index = users.find( (void*)&adr, &_findUser );
	if( index < 0 )
	{
		if( modify )				// should not happen: request to modify non-existing user
		{
			LeaveCriticalSection( &_userServerSetupLock);
			return FALSE ;
		}
		users.add( &adr );
	}
	users.sort( _sortUsers ) ;		// sort by id

	LeaveCriticalSection( &_userServerSetupLock);
	return TRUE;
}

const CfgUser *CfgUsersSetup::getUserById( const GlobalUserID& id, const CfgUser *exceptThisUser )
{
	EnterCriticalSection( &_userServerSetupLock);

	CfgUser *found_usr = NULL ;
	for( int i=0; i < users.count(); i++ )
	{
		CfgUser *usr = users.item(i) ;
		if( usr != exceptThisUser  &&  usr->userID() == id )
		{
			found_usr = usr ;
			break ;
		}
	}

	LeaveCriticalSection( &_userServerSetupLock);
	return found_usr ;
}
	
BOOL CfgUsersSetup::delUser( CfgUser &adr )
{
	EnterCriticalSection( &_userServerSetupLock);

	RESETALIVESIGNAL() ;
	BOOL ret = FALSE ;
	int  index = users.find( (void*)&adr, &_findUser );
	if(  index >= 0 )
	{
		openConfig();

		char sect[SECTION_NAME_SIZE];
		adr.getConfigSection( sect, SECTION_NAME_SIZE );
		config->delSection( sect ) ;
		users.del( index );
		ret = TRUE;
	}

	LeaveCriticalSection( &_userServerSetupLock);
	return ret;
}

// list of strings in the form "surname name; city"
void CfgUsersSetup::shortAddressList( sStringPtrArray &list )
{
	EnterCriticalSection( &_userServerSetupLock);
	
	list.clearList();
	for( int i=0; i < users.count(); i++ )
	{
		char buffer[100];
		CfgUser				*usr = users.item(i);
		const CfgAddress	*adr =&usr->address();
		sprintf( buffer, "%s %s; %s", adr->surname(), adr->name(), adr->city() );
		list.add( buffer );
	}

	LeaveCriticalSection( &_userServerSetupLock);
}	
		
// data exchange between the object and config file
void CfgUsersSetup::storage( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR lpErrBuffer )
{
	if( lpErrBuffer )
		lpErrBuffer[0] = 0;
	EnterCriticalSection( &_userServerSetupLock);

	sStringPtrArray *list=NULL;
	try
	{
		if( saveFlag )
		{
			// delete all sections
			cfg->cfg()->clear() ;

			// add sections
			for( int i=0; i<users.count(); i++ )
				users[i]->storage( cfg, saveFlag );
		}
		else
		{
			list = cfg->cfg()->sectionList();
			users.clearList();
			if( list )
			{
				char name   [CfgAddress::TEXT_SIZE+10];
				char surname[CfgAddress::TEXT_SIZE+10];

				for( int i=0; i<list->count(); i++ )
					if( decodeSectionNames( (*list)[i], surname, name ) )
					{
						CfgUser *adr = new CfgUser( surname, name );
						adr->storage( cfg, FALSE );
						users.add( adr );
					}
			}
			users.sort( _sortUsers ) ;		// sort by id
		}
	}
	catch( ... )
	{
		delete list;
		LeaveCriticalSection( &_userServerSetupLock);
		throw ;
	}
	delete list;
	LeaveCriticalSection( &_userServerSetupLock);
}


// Sends to MuxOutput array of packets bearing UserLog structure for particular user.
// (All users if userId is NULL)
BOOL CfgUsersSetup::sendUserTable( const GlobalUserID& userId )
{
	ulong	tabSize;
	char	*table;
	uchar	muxPacketDataSize = (uchar)(MUXDATASIZE - sizeof(UnicastUserID)) ;

//	MuxOutput		*out			= serverSetup->muxOutput() ;
	PidStreamAttrib	*servChannelPid = serverSetup->serviceChannelPid() ;
	ServiceChannel	*servChannel	= serverSetup->serviceChannel() ;
	DataSender		*dataSender		= servChannel->dataSender() ;

	tabSize	= createUserCATable( userId, &table );
	if( table != NULL )
	{
//		MuxPacket	muxPacket;
		char		*tab = table;

		while( tabSize > 0 )
		{
//			muxPacket.makeUnicastPacket( 0, MuxPacket::UserLog, muxPacketDataSize, 0, 0, tab, userId );
//			out->put( &muxPacket, servChannelPid ) ;
			dataSender->sendServicePacket( 
				MuxPacket::UserLog, servChannel->streamFormat(), tab, muxPacketDataSize,
				servChannelPid, userId ) ;

			tab = tab + muxPacketDataSize;
			tabSize = tabSize - muxPacketDataSize;
		}
		FREE( table );
	}

	return TRUE;
}


int CfgUsersSetup::sendUserTableForAllUser()
{
	for( int i = 0; i < users.count(); i++ )
		if( !sendUserTable( users[i]->userID()) )
			return FALSE ;

	return TRUE;
}


int CfgUsersSetup::sendAliveSignal( )
{
	// this is based on the DVB card requirements
	static uchar synchronisationBytes[] = { 0xAC, 0xEC, 0x38, 0xF0, 0xF0,
											0xAC, 0xEC, 0x38, 0xF0, 0xF0 };
	char xxx[TSPACKET_SIZE] ;
	char *ptr_xxx = (char*)xxx ;
	#define data	(*(UserIDs*)ptr_xxx)

	memset( &data, 0, sizeof(data) );
	const int	userIdsHdrSize		= sizeof(ushort) + 2*sizeof(GlobalUserID) ;
	const int   max_users_in_packet = (MUXDATASIZE - userIdsHdrSize - sizeof(synchronisationBytes)) / sizeof(GlobalUserID) ;

	EnterCriticalSection( &_userServerSetupLock);

	data.numOfID  = 0 ;
	if( as_fromUser < 0 )
		data.fromUser.makeInvalid() ;
	else
	{
		data.fromUser = user(as_fromUser)->userID() ;
		data.fromUser.next() ;
	}
	//data.fromUser = as_fromUser < 0 ? 0 : (user(as_fromUser)->userID() + 1);
	BOOL end_reached= FALSE ;
	while( 1 )
	{
		if( ++as_fromUser >= numUsers() )
		{
			end_reached = TRUE ;
			break ;
		}
		const CfgUser *usr = user(as_fromUser) ;
		data.ids[data.numOfID] = usr->userID();
		data.numOfID++ ;
		if( data.numOfID >= max_users_in_packet )
			break ;
	}
	if( end_reached )
	{
		data.toUser.makeMaximum() ;	// last possible user id
		RESETALIVESIGNAL() ;
	}
	else
		data.toUser = data.ids[data.numOfID - 1];

	LeaveCriticalSection( &_userServerSetupLock);
	// the last 10 byte carries synchronisation bytes
	memcpy( (char *)&data + MUXDATASIZE - sizeof(synchronisationBytes),
			synchronisationBytes, sizeof(synchronisationBytes) );

//	MuxPacket muxPacket ;
	//muxPacket.makeDataPacket( 0/*channel*/, MuxPacket::AliveSignal,
	//	userIdsHdrSize + data.numOfID*sizeof(long), 0, 0, (char*)&data ) ;
//	muxPacket.makeDataPacket( 0/*channel*/, MuxPacket::AliveSignal,
//		MUXDATASIZE, 0, 0, (char*)&data ) ;

//	MuxOutput		*out			= serverSetup->muxOutput() ;
	PidStreamAttrib	*servChannelPid = serverSetup->serviceChannelPid() ;
	ServiceChannel	*servChannel	= serverSetup->serviceChannel() ;
	DataSender		*dataSender		= servChannel->dataSender() ;

	dataSender->sendServicePacket( 
		MuxPacket::AliveSignal, servChannel->streamFormat(),(char*)&data,
		MUXDATASIZE, servChannelPid ) ;

	//	out->put( &muxPacket, servChannelPid ) ;
	//TRACE( "\nALIVE( %d users %lx..%lx)", data.numOfID, data.fromUser, data.toUser ) ;
	return 0 ;

	#undef data
}



//----------------------------------------------------------------------------
//	CfgChannel
//  Class describing data of a single channel.
//----------------------------------------------------------------------------
CfgChannel::CfgChannel( ushort chID, const char *nm)
{
	_serviceID = chID;
	_inboxPath = NULL;
	_setDefault();
	copyAndFormat( _serviceName, nm, TEXT_SIZE );
}

CfgChannel::~CfgChannel()
{ 
	if( _inboxPath ) 
		free(_inboxPath);
}


void CfgChannel::copy( const CfgChannel &adr, BOOL withPID )
{
	FREE( _inboxPath );

	strcpy( _serviceName,  adr._serviceName);
	_serviceID			= adr._serviceID	;
	_useFec				= adr._useFec		;
	_fecLevel			= adr._fecLevel		;
	_useFecRebr			= adr._useFecRebr	;
	_maxFecRebrSize		= adr._maxFecRebrSize;
	_absPriority		= adr._absPriority	;
	_relPriority		= adr._relPriority	;
	_outputRateMin		= adr._outputRateMin;	
	_outputRateMax		= adr._outputRateMax;	
	_channelSwitchOn	= adr._channelSwitchOn;
	_numRebroadcasts	= adr._numRebroadcasts;
	_maxVolumePerDay	= adr._maxVolumePerDay;
	_inboxSendDelay		= adr._inboxSendDelay;	
	_rebroadcastDelay	= adr._rebroadcastDelay;
	_absPriorityOn		= adr._absPriorityOn	;
	_relPriorityOn		= adr._relPriorityOn	;
	_maxVolumePerDayOn	= adr._maxVolumePerDayOn;
	_streamFormat		= adr._streamFormat;
	_inboxPath			= freeAndStrdup( NULL, adr.inboxPath() );
	_schedulers			= adr._schedulers;
	if( withPID )
		_channelPID		= adr._channelPID;
}

void CfgChannel::setInboxPath( const char *dir )
{
	_inboxPath = freeAndStrdup( _inboxPath, dir );
}

void CfgChannel::_setDefault( )
{
	_inboxPath		  = freeAndStrdup( _inboxPath, isSpecialChannel() ? "" : "ToBeSent" );
	_maxVolumePerDayOn= 0 ;
	_outputRateMin	  = 0.064f;
	_outputRateMax	  = 1.1f ;
	_numRebroadcasts  = 3 ;
	_maxVolumePerDay  = 213 ;
	_inboxSendDelay   = 3 ;
	_rebroadcastDelay = 5 ;
	_channelSwitchOn  = TRUE ;
	_absPriorityOn    = 0 ;
	_relPriorityOn    = 1 ;
	_useFec			  = 0 ;
	_fecLevel		  = MediumFEC ;
	_useFecRebr		  = 1 ;
	_maxFecRebrSize	  = 35;
	_absPriority	  = 10;
	_relPriority	  = 5 ;
	_channelPID		  = 257;
	_streamFormat	  = TSPIPE_PROTOCOL ;
}

void CfgChannel::setDefaults()
{
	const CfgChannel *ch = MfxDvbSetup()->channelDefaults();

	_inboxPath		  = freeAndStrdup( _inboxPath, "ToBeSent\\" );
	_channelSwitchOn  = ch->_channelSwitchOn  ;
	_absPriorityOn    = ch->_absPriorityOn    ;
	_relPriorityOn    = ch->_relPriorityOn    ;
	_maxVolumePerDayOn= ch->_maxVolumePerDayOn;
	_fecLevel		  = ch->_fecLevel		  ;
	_useFec			  = ch->_useFec			  ;
	_useFecRebr		  = ch->_useFecRebr		  ;
	_maxFecRebrSize	  = ch->_maxFecRebrSize	  ;
	_absPriority	  = ch->_absPriority	  ;
	_relPriority	  = ch->_relPriority	  ;
	_outputRateMin	  = ch->_outputRateMin	  ;
	_outputRateMax	  = ch->_outputRateMax	  ;
	_numRebroadcasts  = ch->_numRebroadcasts  ;
	_maxVolumePerDay  = ch->_maxVolumePerDay  ;
	_inboxSendDelay   = ch->_inboxSendDelay   ;
	_rebroadcastDelay = ch->_rebroadcastDelay ;
	_channelPID		  = ch->_channelPID;
	_streamFormat	  = ch->_streamFormat;
}


BOOL CfgChannel::equals( const CfgChannel &adr, BOOL withPID ) const 
{
	if( stricmp( _serviceName, adr._serviceName) != 0 )
		return FALSE ;

	#define CMP(x)	if( x != adr.x ) return FALSE ;
	CMP( _serviceID		);
	CMP( _streamFormat	);
	CMP( _useFec		);
	CMP( _fecLevel		);
	CMP( _useFecRebr	);
	CMP( _maxFecRebrSize);
	CMP( _absPriority	);
	CMP( _relPriority	);
	CMP( _outputRateMin	);
	CMP( _outputRateMax	);
	CMP( _channelSwitchOn );
	CMP( _numRebroadcasts );
	CMP( _maxVolumePerDay );
	CMP( _inboxSendDelay  );
	CMP( _rebroadcastDelay) ;		// [s] 0=no delay, else delay between rebroadcasts
	CMP( _absPriorityOn	) ;
	CMP( _relPriorityOn	) ;
	CMP( _maxVolumePerDayOn ) ;
	if( withPID )
		CMP( _channelPID ) ;

	if( _inboxPath  &&  adr._inboxPath )
	{
		if( stricmp( _inboxPath, adr._inboxPath) )
			return FALSE;
	}
	else
	if( _inboxPath  !=  adr._inboxPath )
		return FALSE;

	// For the time being ignored
	//CfgOutputSchedulerPtrArray _schedulers;
	return TRUE;
}

// Config section used to store the data.
#define SECTION_NAME_SIZE1	(CfgAddress::TEXT_SIZE+40)
const char *CfgChannel :: getConfigSection( char *sect, int size ) const
{
	ASSERT( size >= SECTION_NAME_SIZE1 );
	sprintf( sect, "%s_%d", _serviceName, _serviceID );
	return sect;
}

// data exchange between the object and config file
void CfgChannel::storage( CfgBaseSetup *cfg, BOOL saveFlag, const char *sectionName )
{
	VERIFY( cfg && !isEmpty() );

	char sect[SECTION_NAME_SIZE1];
	if( sectionName != NULL )
	{
		strncpy( sect, sectionName, SECTION_NAME_SIZE1 ) ;
		sect[SECTION_NAME_SIZE1-1] = 0 ;
	}
	else
		getConfigSection( sect, SECTION_NAME_SIZE1 );

	ConfigClass *c = cfg->cfg() ;
	if( saveFlag )
	{
		if( _inboxPath )	
			cfg->set( sect, "Inbox", _inboxPath );

		char *str ;
		switch( _fecLevel )
		{
			case HighFEC : str = "High" ; break ;
			case MediumFEC : str = "Medium" ; break ;
			default : str = "Low" ; break ;
		}
		c->printf  ( sect, "FEC", "%c%s; %c%d",
			_useFec     ? '+' : '-', str,
			_useFecRebr ? '+' : '-', _maxFecRebrSize );

		c->printf  ( sect, "AbsPriority", "%d%c"	 , _absPriority    , _absPriorityOn ? '+' : '-' );
		c->printf  ( sect, "RelPriority", "%d%c"	 , _relPriority    , _relPriorityOn ? '+' : '-' );
		c->printf  ( sect, "OutputRate" , "%.4f-%.4f", _outputRateMin  , _outputRateMax );
		c->printf  ( sect, "MaxVolumePerDay" , "%d%c", _maxVolumePerDay, _maxVolumePerDayOn ? '+' : '-' );
		cfg->setInt( sect, "NumRebroadcasts", _numRebroadcasts );
		cfg->setInt( sect, "RebroadcastDelay",_rebroadcastDelay);
		cfg->setInt( sect, "InboxSendDelay",  _inboxSendDelay  );
		cfg->setInt( sect, "InboxSwitchOn",   _channelSwitchOn );
		cfg->setInt( sect, "ChannelPID",	  _channelPID	   );
		char *stream ;
		switch( _streamFormat )
		{
			case MPE_UDP_PROTOCOL:
				stream = "MPE_UDP" ;
				break ;
			case MPE_HNET_PROTOCOL:
				stream = "MPE_HNET" ;
				break ;
			default:
				stream = "Piping" ;
				break ;
		}
		cfg->set( sect, "StreamProtocol", stream) ;
	}
	else
	{
		#define IDENT_SIZE  (TEXT_SIZE+sizeof(short))		//*name + ID*/
		char *str;
		char  key;
		
		_setDefault( );

		if( _serviceID != 0xffff)
		{
			if( (str = c->get( sect, "FEC")) != NULL )
			{
				if( str[0] == '+' )
					_useFec = 1 , str++ ;
				else
				if( str[0] == '-' )
					_useFec = 0 , str++ ;
				else
					_useFec = 0 ;

				if( strnicmp( str, "High", 4) == 0 )
				{
					str += 4 ;
					_fecLevel = HighFEC ;
				}
				else
				if( strnicmp( str, "Medium", 6) == 0 )
				{
					str += 6 ;
					_fecLevel = MediumFEC ;
				}
				else
				if( strnicmp( str, "Low", 3) == 0 )
				{
					str += 3 ;
					_fecLevel = LowFEC ;
				}
				else
					_useFec = 0 ;

				if( str[0] == ';' )
				{
					str++ ;
					while( *str == ' '  ||  *str == '\t' )
						str++ ;
					if( str[0] == '+' )
						_useFecRebr = 1 , str++ ;
					else
					if( str[0] == '-' )
						_useFecRebr = 0 , str++ ;
					else
						_useFecRebr = 0 ;
					_maxFecRebrSize = atoi( str ) ;
				}
			}
			if( _serviceID != 0xffff )
				if((str = c->get( sect, "Inbox" )) )			
					_inboxPath   = freeAndStrdup( _inboxPath, str );
		}

		str = cfg->get( sect, "AbsPriority"		);
		if( str != NULL )
		{
			if( sscanf( str, "%hd%c", &_absPriority, &key) == 2 )
				_absPriorityOn = (key == '+' ? 1 : 0) ;
		}
		str = cfg->get( sect, "RelPriority"		);
		if( str != NULL )
		{
			if( sscanf( str, "%hd%c", &_relPriority, &key) == 2 )
				_relPriorityOn = (key == '+' ? 1 : 0) ;
		}
		str = cfg->get( sect, "MaxVolumePerDay"	);
		if( str != NULL )
		{
			if( sscanf( str, "%d%c", &_maxVolumePerDay, &key) == 2 )
				_maxVolumePerDayOn = (key == '+' ? 1 : 0) ;
		}

		c->getInt( sect, "NumRebroadcasts" , &_numRebroadcasts );
		c->getInt( sect, "RebroadcastDelay", &_rebroadcastDelay);
		c->getInt( sect, "InboxSendDelay"  , &_inboxSendDelay  );
		int ch_id ;
		c->getInt( sect, "ChannelPID",		 &ch_id );
		_channelPID = (ushort)ch_id ;
		if ( _serviceID == 0 || _serviceID >= 0xfff0 )
		{
			_channelSwitchOn = TRUE;
			if ( _serviceID == 0 )
				_inboxPath = freeAndStrdup( _inboxPath, "ToBeSent" );
		}
		else
			c->getInt( sect, "InboxSwitchOn"   , &_channelSwitchOn );
		if( (str = cfg->get( sect,"OutputRate")) )
		{
			sscanf( str, "%f-%f", &_outputRateMin, &_outputRateMax );
		}

		_streamFormat = TSPIPE_PROTOCOL ;
		if ( GetProgramLevel() != ProgramLevel_Basic )
		{
			char *stream = cfg->get( sect, "StreamProtocol") ;
			if( stream != NULL )
			{
				if( stricmp(stream, "MPE_HNET") == 0 )
					_streamFormat = MPE_HNET_PROTOCOL ;
				else
				if( stricmp(stream, "MPE_UDP") == 0 )
					_streamFormat = MPE_UDP_PROTOCOL ;
			}
		}
	}

	// scheduler
	strcat( sect, "_SCH" ) ;
	_schedulers.storage( cfg, sect, NULL, saveFlag ); 
	if( saveFlag==FALSE )
	{
		// delete all schedular items which are not in switch mode
		for( int i=_schedulers.count()-1; i>=0; i-- )
		{
			if( _schedulers[i]->inSwitchMode() == FALSE )
				_schedulers.del( i );
		}
	}
}


//----------------------------------------------------------------------------
//	CfgChannelsSetup
//  class representing data for all channels
//----------------------------------------------------------------------------


// local functions
static void _deleteChannel( CfgChannel **adr )
{
	delete *adr;
}
static int _findChannel( void *adr2, CfgChannel **adr1 )
{
	return !stricmp( (*adr1)->serviceName(), (char *)adr2 ); 
}
static int _sortChannels( const void *e1, const void *e2 )
{
	CfgChannel *c1 = *(CfgChannel **)e1 ;
	CfgChannel *c2 = *(CfgChannel **)e2 ;
	if( c1->serviceID() < c2->serviceID() )
		return -1 ;
	if( c1->serviceID() == c2->serviceID() )
		return 0 ;
	return 1 ;
}


CfgChannelsSetup :: CfgChannelsSetup( const char *file ) : CfgBaseSetup( file )
{
	InitializeCriticalSection( &_channelServerSetupLock );
	channels.setDelFunc( &_deleteChannel );
}

BOOL CfgChannelsSetup::addOrModifyChannel( CfgChannel &adr, BOOL modify )
{
	int index = channels.find( (void*)adr.serviceName(), &_findChannel );
	if( index < 0 )
		if( modify )						// modify non-existing channel
			return FALSE;

	EnterCriticalSection( &_channelServerSetupLock);

	if( index < 0 )
		channels.add( &adr );
	channels.sort( _sortChannels ) ;		// sort by id

	LeaveCriticalSection( &_channelServerSetupLock);
	return TRUE;
}
	
BOOL CfgChannelsSetup::delChannel( CfgChannel &adr, CfgUsersSetup *userSetup, CfgProvidersSetup *providerSetup )
{
	int index = channels.find( (void*)&adr, &_findChannel );
	if( index < 0 )
		return FALSE ;

	openConfig();

	EnterCriticalSection( &_channelServerSetupLock);

	if( userSetup != NULL )
	{
		for( int j=0 ; j < userSetup->numUsers() ; ++j )
		{
			CfgUser *usr = (CfgUser *)userSetup->user(j) ;
			usr->delChannel( adr.serviceID() ) ;
		}
	}

	if( providerSetup != NULL )
	{
		for( int j=0 ; j < providerSetup->numProviders() ; ++j )
		{
			CfgAddress *provider = (CfgAddress *)providerSetup->provider( j );
			provider->delChannel( adr.serviceID() );
		}
	}

	char sect[SECTION_NAME_SIZE1+10];
	adr.getConfigSection( sect, SECTION_NAME_SIZE1 );
	config->delSection( sect ) ;
	strcat( sect, "_SCH" ) ;
	config->delSection( sect ) ;
	channels.del( index );

	LeaveCriticalSection( &_channelServerSetupLock);
	return TRUE;
}

void CfgChannelsSetup::shortChannelList( sStringPtrArray &list )
{
	EnterCriticalSection( &_channelServerSetupLock);
	
	list.clearList();
	for( int i=0; i < channels.count(); i++ )
	{
		CfgChannel *adr = channels.item(i);

		char buffer[100];
		sprintf( buffer, "%-3d %s", adr->serviceID(), adr->serviceName() );
		list.add( buffer );
	}

	LeaveCriticalSection( &_channelServerSetupLock);
}	

void CfgChannelsSetup::channelIdsList( sTemplateArray<int> &list )
{
	EnterCriticalSection( &_channelServerSetupLock);

	list.clearList();
	for( int i=0; i < channels.count(); i++ )
	{
		CfgChannel *adr = channels.item(i);
		list.add( adr->serviceID() );
	}

	LeaveCriticalSection( &_channelServerSetupLock);
}

const CfgChannel *CfgChannelsSetup::getChannelById( ushort id )
{
	EnterCriticalSection( &_channelServerSetupLock);

	CfgChannel *foundCh=NULL ;
	for( int i=0; i < channels.count(); i++ )
	{
		CfgChannel *adr = channels.item(i);
		if( adr->serviceID() == id )
		{
			foundCh = adr ;
			break ;
		}
	}

	LeaveCriticalSection( &_channelServerSetupLock);
	return foundCh ;
}

const CfgChannel *CfgChannelsSetup::getChannelByName(const char *name)
{
	EnterCriticalSection( &_channelServerSetupLock);

	CfgChannel *foundCh=NULL ;
	for( int i=0; i < channels.count(); i++ )
	{
		CfgChannel *adr = channels.item(i);
		if( !stricmp( adr->serviceName(), name) )
		{
			foundCh = adr ;
			break ;
		}
	}

	LeaveCriticalSection( &_channelServerSetupLock);
	return foundCh ;
}

// data exchange between the object and config file
void CfgChannelsSetup::storage( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR /*lpErrBuffer*/ )
{
	EnterCriticalSection( &_channelServerSetupLock);
	char			 errStr[4196]="";
	sStringPtrArray *lst = NULL;
	try
	{
		if( saveFlag )
		{
#ifndef EMPTY_VERSION
			for( int i=0; i< channels.count(); i++ )
				channels[i]->storage( cfg, saveFlag );
#endif
		}
		else
		{
			BOOL dvbIpGateway = runAsDvbIpGateway() ;
#ifndef EMPTY_VERSION
			lst = cfg->cfg()->sectionList();
			if( lst )
			{
				char name[CfgAddress::TEXT_SIZE],
					 num [CfgAddress::TEXT_SIZE];

				sTemplateArray<int>	idArray;
				sStringPtrArray		nameArray;

				// prepare channel info
				for( int i=0; i<lst->count(); i++ )
				{
					LPCTSTR lpSection = (*lst)[i];

					if( strcmp( lpSection+strlen(lpSection)-4, "_SCH" ) &&
						decodeSectionNames( lpSection, name, num) )
					{
						int iNum = atoi( num );
						if (!dvbIpGateway || iNum==0xFFFF)
						{
							idArray.add( iNum );
							nameArray.add( name );
						}
					}
				}

				// delete not used channals
				for( i=channels.count()-1; i>=0; i-- )
				{
					for( int j=idArray.count()-1; j>=0; j-- )
						if( idArray[j] == channels[i]->serviceID() )
							break;
					if( j==-1 )
						channels.del(i);
				}

				// add or modify channels
				for( i=0; i<idArray.count(); i++ )
				{
					int	id = idArray[i];
					
					strcpy( name, nameArray[i] );
					if ( id == 0 )
					{
						if( !textEqual( name, "Service channel" ) )
							strcpy( name, "Service channel" );
					}
					else
					if( textEqual( name, "Service channel" ) )
						continue;

					CfgChannel *adr = (CfgChannel*)getChannelById((ushort)id);
					int			numRebr;
					
					if( adr==NULL )
					{
						for( int j=channels.count()-1; j>=0; j-- )
						{
							if( textEqual( channels[j]->serviceName(), name ) )
							{
								int errLen = strlen(errStr);
								if( errLen<4000 )
									sprintf( errStr+errLen, "\"%s\" channel has the name which is used by another channel.", name );
								break;
							}
						}
						if( j!=-1 && channels.count()>0 )
							continue;
						adr = new CfgChannel( (ushort)id, name );
						channels.add( adr );
					}

					adr->storage( cfg, FALSE );
					numRebr = adr->numRebroadcasts() ;
					if( numRebr < 0 || 15 < numRebr )
					{
						int errLen = strlen(errStr);
						numRebr = 0;
						if( errLen<4000 )
							sprintf( errStr+errLen, "Rebroadcast value for channel %s has to be from the interval <0,15>.\n", adr->serviceName() ) ;
					}
					/*					
					for( int j=0 ; j < channels.count() ; ++j )
					{
						const CfgChannel *ch = channels[j] ;
						if( textEqual( ch->inboxPath(), adr->inboxPath() ) &&
							ch->serviceID() > 0 && ch->serviceID() < 0xfff0 &&
							adr->serviceID()> 0 && adr->serviceID() < 0xfff0 )||
							textEqual( ch->serviceName(), adr->serviceName() ) )
						{
							int errLen = strlen(errStr);
							if( errLen<4000 )
								sprintf( errStr+errLen, "\"%s\" channel uses name, ID or inbox used by another channel.", adr->serviceName() );
						}
					}
					*/
				}
			}
			if( !dvbIpGateway && getChannelById(0) == NULL )
				channels.add( new CfgChannel( 0, "Service channel" ) );
#endif
			if( getChannelById(0xFFFF) == NULL )
				channels.add( new CfgChannel( 0xffff, "Internet" ) );

			channels.sort( _sortChannels ) ;
			// if there was error(s) -> write it
			if( errStr[0] )
				throw Msg( -1, "Error(s) in \"channels.cfg\" file.\n%s", errStr );
		}
	}
	//catch( Msg &msg )
	//{
	//	delete lst;
	//	LeaveCriticalSection( &_channelServerSetupLock);
	//	throw msg;
	//}
	catch( ... )
	{
		delete lst;
		LeaveCriticalSection( &_channelServerSetupLock);
		throw ;
	}
	delete lst;
	LeaveCriticalSection( &_channelServerSetupLock);
}

// Allocate memory for *table and fill it with channle names (used in sending CA table)
ulong CfgChannelsSetup::createChannelNamesTable( char **table )
{
	sChannelsTable		*channelsTable;
	ulong				sizeOfTable;

	EnterCriticalSection( &_channelServerSetupLock);

	// | ushort nChannels | array of UserChannels |		except channel 0
	short numOfChannels = channels.count() - 1;
	ASSERT( numOfChannels );
	sizeOfTable = sizeof( ushort ) + ( numOfChannels ) * sizeof( sUserChannel );

	*table			= (char *)CALLOC( sizeOfTable, 1 );
	channelsTable	= (sChannelsTable *)*table;

	// fill channels
	channelsTable->numChannels = numOfChannels;
	for( int i = 0, j = 1; i < numOfChannels; i++, j++ )
	{
		const CfgChannel *ch = channels[j];
		const char *name  = ch->serviceName() ;

		channelsTable->channels[i].channelID = ch->serviceID();
		if( name != NULL )
			strcpy( channelsTable->channels[i].channelName, name );
	}

	LeaveCriticalSection( &_channelServerSetupLock);
	return sizeOfTable;
}


char CfgChannelsSetup::globalStreamFormat()
{
	CfgChannel *ch0 = channels[0] ;
	char streamFmt = ch0->streamFormat() ;
	// skip HNet channel
	for( int i=channels.count()-2 ; i > 0 ; i-- )
	{
		const CfgChannel *ch = channels[i] ;
		if( ch->streamFormat() != streamFmt )
		{
			return -1 ;
			break ;
		}
	}
	return streamFmt ;
}

BOOL CfgChannelsSetup::setGlobalStreamFormat( char f, char *warning )
{
	BOOL changed = FALSE ;
	*warning = 0 ;
	EnterCriticalSection( &_channelServerSetupLock);

	// skip HNet channel
	BOOL warn = FALSE ;
	for( int i=channels.count()-2 ; i >= 0 ; i-- )
	{
		CfgChannel *ch = channels[i] ;
		if( ch->streamFormat() != f )
		{
			ch->_streamFormat = f ;
			changed = TRUE ;
			if( serverSetup != NULL )
				if( !serverSetup->resetMuxChannel( ch) )
					warn = TRUE ;
		}
	}
	if( !warn )
		*warning = 0 ;
	else
		strcpy( warning,
			"Some channel(s) are now sending the data.\n"
			"The change in the send protocol will be applied after this job completes." ) ;

	LeaveCriticalSection( &_channelServerSetupLock);
	return changed ;
}


//----------------------------------------------------------------------------
//	CfgOutputScheduler
//----------------------------------------------------------------------------


// Get rate for given time.
// 12:20 means 12:20:00
float CfgOutputScheduler::getRate( const tm *tm, time_t *how_long )
{
	int	tim = tm->tm_hour*60 + tm->tm_min ;
	CfgOutputRate *rt = NULL ;
	for( int j=0 ; j < _outputRates.count() ; ++j )
	{
		rt = &_outputRates.item(j) ;
		time_t from = rt->timeFrom() ;
		if( tim < from )
		{
			*how_long = 60*(from - tim) - tm->tm_sec ;
			return 0 ;
		}
		time_t to = rt->timeTo() ;
		if( tim < to )
		{
			*how_long = 60*(to - tim) - tm->tm_sec ;
			return  rt->rate ;
		}
	}

	time_t tilMidnight = (23 - tm->tm_hour)*3600 + (59 - tm->tm_min)*60 + (60 - tm->tm_sec) ;
	*how_long = tilMidnight + 1 ;
	return 0 ;
}

// get CfgOutputRate valid for given time (NULL if not found)
CfgOutputRate *CfgOutputScheduler::getScheduler( const tm *tm )
{
	int	tim = tm->tm_hour*60 + tm->tm_min ;
	CfgOutputRate *rt = NULL ;
	for( int j=0 ; j < _outputRates.count() ; ++j )
	{
		rt = &_outputRates.item(j) ;
		if( rt->timeFrom() <= tim  &&  tim < rt->timeTo() )
			return rt ;
	}
	if( rt != NULL  &&  tim == rt->timeTo() )
		return rt ;
	return NULL ;
}

// If not then some day period is undefined (transfer will stop).
BOOL CfgOutputScheduler::fullDayDefined( )
{
	int lastTime=0 ;
	for( int i=0; i<_outputRates.count(); i++ )
	{
		CfgOutputRate &rt = _outputRates.item(i);
		if( rt.timeFrom() != lastTime )
			return FALSE ;
		lastTime = rt.timeTo() ;
	}
	return lastTime == (24*60) ;
}


static const char *dayNames[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Monday-Sunday", "Monday-Friday", "Weekend" }; 

const char **CfgOutputScheduler::stdDates( int *n_names )
{
	*n_names = sizeof(dayNames) / sizeof(char*) ;
	return dayNames ;
}

inline const char *dayName2Str( CfgOutputScheduler::DayName day )
{
	return dayNames[day];
}

BOOL CfgOutputScheduler::isDateValid( const char *buf )
{
	if( isdigit( buf[0] ) )	//data
	{
		_viaDayName = FALSE;
		int			day, mon, year;

		if( sscanf( buf, "%d-%d-%d", &day, &mon, &year ) != 3 && 
			sscanf( buf, "%d/%d/%d", &day, &mon, &year ) != 3 )
			return FALSE;
		if( year <= 1970 )
			return  FALSE ;
		struct tm loc ;
		memset( &loc, 0, sizeof(tm) );
		loc.tm_mday = day ;
		loc.tm_mon  = mon - 1 ;
		loc.tm_year = year- 1900 ;
		loc.tm_hour = 12;
		if( int(_date = mktime( &loc )) == -1 )
			return FALSE;
		return TRUE;
	}
	else						//dayName
	{
		_viaDayName= TRUE;
		_dayName   = 0;
		for( int i=0; i<10; i++ )
			if( !stricmp( buf, dayName2Str( DayName(i))) )
			{
				_dayName = i;
				return TRUE ;
			}
	}
	return FALSE ;
}


void CfgOutputScheduler :: dateAsString( char *val ) const
{
	if( _viaDayName )
		strcpy( val, dayName2Str( DayName(_dayName) ));
	else
	{
		struct tm *newtime = localtime( &_date );
		strftime( val, 11, "%d-%m-%Y", newtime );
	}
}

static BOOL convertTime( const char *buf, int *hr, int *min )
{
	switch( sscanf( buf, "%d:%d", hr, min) )
	{
		case 2:
			break ;
		case 1:
			*min = 0 ;
			break ;
		default:
			return FALSE ;
	}
	if( *min == 60 )
	{
		*min = 0 ;
		*hr  = *hr + 1 ;
	}
	if( *hr < 0  ||  *hr > 24  ||  (*hr==24 && *min>0) )
		return FALSE ;
	if( *min < 0  ||  *min > 60 )
		return FALSE ;
	return TRUE ;
}

// present variables as text
#define		SwitchOnLen	4
LPCTSTR		lpSwitchOnStr = "On: ";
BOOL CfgOutputScheduler::ratesAsString( char *buf, int size ) const
{
	char item[100];
	int	 sz = 0 ;

	buf[0] =0;
	if( _bSwitchMode )
	{
		strcpy( buf, lpSwitchOnStr );
		sz = SwitchOnLen;
	}
	for( int i=0; i<_outputRates.count(); i++ )
	{
		CfgOutputRate &rt = _outputRates.item(i);
		int			   len;
	
		if( rt.minuteFrom != 0  &&  rt.minuteTo != 0 )
			len = sprintf( item, "%d:%d-%d:%d",  rt.hourFrom, rt.minuteFrom, rt.hourTo, rt.minuteTo );
		else if( rt.minuteFrom != 0 )
			len = sprintf( item, "%d:%d-%d", 	 rt.hourFrom, rt.minuteFrom, rt.hourTo );
		else if( rt.minuteTo != 0 )
			len = sprintf( item, "%d-%d:%d", 	 rt.hourFrom,				 rt.hourTo, rt.minuteTo );
		else
			len = sprintf( item, "%d-%d", 		 rt.hourFrom,				 rt.hourTo );

		if( !_bSwitchMode )
			len += sprintf( item+len, "<%.3f",	 rt.rate );
		if( sz + len+2 > size )
			return FALSE;

		if( i > 0 )
		{
			strcpy( buf+sz, "; " ) ;
			sz += 2 ;
		}
		strcpy( buf+sz, item );
		sz += len;
	}
	return TRUE;
}

BOOL CfgOutputScheduler::fromStringToRates( const char *data )
{
	char	buf[1024] ;
	int		lastTime = 0;
	int		ret= FALSE ;

	_outputRates.clearList();
	_bSwitchMode = FALSE;
	if( !strnicmp( data, lpSwitchOnStr, SwitchOnLen ) )
	{
		_bSwitchMode = TRUE;
		data += SwitchOnLen;
	}

	strncpy( buf, data, sizeof(buf) ) ;
	buf[sizeof(buf)-1] = 0 ;
	for( char *wrd=strtok(buf,";") ; wrd != NULL ; wrd=strtok(NULL,";") )
	{
		CfgOutputRate	rate	;
		char			tim1[10], tim2[10], restWrd[100] ;
		int				hFrom, mFrom, hTo, mTo ;

		if( _bSwitchMode )
		{
			if( sscanf( wrd, " %[0-9:] - %[0-9:]", tim1, tim2) != 2 )
				return FALSE;
			rate.rate = 0.f;
		}
		else
		{
			if( sscanf( wrd, " %[0-9:] - %[0-9:] < %f %100s", tim1, tim2, &rate.rate, restWrd) != 3 )
				return FALSE;
		}
		if( !convertTime( tim1, &hFrom, &mFrom)  ||  !convertTime( tim2, &hTo, &mTo) )
			return FALSE;

		rate.hourFrom	= (uchar)hFrom; 
		rate.minuteFrom	= (uchar)mFrom;
		rate.hourTo		= (uchar)hTo  ;
		rate.minuteTo	= (uchar)mTo  ;
		int tm=rate.timeFrom() ;
		if( tm >= rate.timeTo() || tm < lastTime )
			return FALSE;
		lastTime  = rate.timeTo();
		_outputRates.add( rate );
		ret = TRUE ;
	}
	return ret;
}

BOOL CfgOutputScheduler :: hasEqualDate( const CfgOutputScheduler &sch ) const
{
	if(	_viaDayName == sch.viaDayName()  && 
		 (( _viaDayName  &&  _dayName == sch.dayName()) ||
		  (!_viaDayName  &&  _date    == sch.date()   ) ))
		  return TRUE;
	return FALSE;
}
		
CfgOutputScheduler &CfgOutputScheduler::operator= ( const CfgOutputScheduler &usr)
{
	const CfgOutputRatePtrArray	&usrRate = usr.outputRates();

	_dayName	= usr.dayName();
	_viaDayName	= usr.viaDayName();
	_date		= usr.date();
	_bSwitchMode= usr._bSwitchMode;
	_outputRates.clearList();
	for( int i=0; i<usrRate.count(); i++ )
		_outputRates.add( usrRate.item(i) );
	return *this;
}

BOOL CfgOutputScheduler::operator==( const CfgOutputScheduler &usr) const
{
	const CfgOutputRatePtrArray	&usrRate = usr.outputRates();

	if( !hasEqualDate(usr) )
		return FALSE;
	if( _bSwitchMode != usr._bSwitchMode )
		return FALSE;

	if(	_outputRates.count() != usrRate.count() )
		return FALSE;
	for( int i=0; i<usrRate.count(); i++ )
	{
		CfgOutputRate &r1 = _outputRates.item(i);
		CfgOutputRate &r2 = usrRate.item(i);

		if( r1.timeFrom() != r2.timeFrom()	||
			r1.timeTo()	  != r2.timeTo()	||
			fabs(r1.rate - r2.rate) > 0.00001 )
			return FALSE;
	}
	return TRUE;
}

#define dvbSchedulerSection "TotalOutputRate"

// data exchange between the object and config file
void CfgOutputScheduler::storage( CfgBaseSetup *cfg, LPCTSTR lpSection, LPCTSTR value, BOOL bSave )
{
#ifndef EMPTY_VERSION
	#define HALFADAY 12*3600
	VERIFY( cfg );

	if( bSave )
	{
		char buf[1024];
		char val[20];

		dateAsString( val );
		if( !ratesAsString( buf, 1023 ) )
			throw Msg( -1, "Configuration error: Illegal Scheduler string. (\"%s\")", val );
		cfg->set( lpSection, val, buf);
	}
	else
	{
		char *str;

		str=cfg->get( lpSection,value);
		if(!setSchedular( value, str ) )
			throw Msg( -1, "Configuration error: Illegal Scheduler string. (\"%s\")", value );
	}
#endif
}

BOOL CfgOutputScheduler::setSchedular( LPCTSTR lpDate, LPCTSTR lpRate )
{
#ifndef EMPTY_VERSION
	_outputRates.clearList();
	_bSwitchMode = FALSE;
	if( !isDateValid( lpDate ) )
		return FALSE;
	if( lpRate )
		fromStringToRates( lpRate );
#endif
	return TRUE;
}

//------------------------------------------------------------------------------------
//	SchedulerManager
//  Background thread which takes care about multiplexor output rate.
//------------------------------------------------------------------------------------

/*
class SchedulerManager : public CWinThread
{
	HANDLE	_eventKill  ;
	HANDLE	_eventReset ;
	HANDLE	_handlers[2];

  protected:
	CfgOutputSchedulerPtrArray	*m_scheduler	;
	CfgOutputRate				*m_currentRate ;
	time_t						 m_timeInterval;	// [msec]
	float						 m_transferRate;

	float	updateScheduler	( time_t *how_long );		// = new rate

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC( SchedulerManager )

  public:
	BOOL	InitInstance	();
	int		ExitInstance	();

	float	transferRate	()	{ return m_transferRate; }

	void	start			()	{ CreateThread(); }
	void	reset			()	{ VERIFY( SetEvent(_eventReset) ); };
	void	kill			();
	SchedulerManager();
};

IMPLEMENT_DYNAMIC(SchedulerManager, CWinThread)

BEGIN_MESSAGE_MAP(SchedulerManager, CWinThread)
END_MESSAGE_MAP()

SchedulerManager::SchedulerManager()
{
	m_bAutoDelete   = FALSE;
	_eventKill		= CreateEvent(NULL, TRUE, FALSE, NULL);
	_eventReset		= CreateEvent(NULL, TRUE, FALSE, NULL);
	_handlers[0]    =_eventKill ;
	_handlers[1]    =_eventReset;
	m_currentRate   = NULL;
	m_scheduler	    = &MfxDvbSetup()->_schedulers ;
	m_transferRate  = 0;
	m_timeInterval  = 0 ;
}

BOOL SchedulerManager::InitInstance( )
{
	m_transferRate = updateScheduler( &m_timeInterval );
	serverSetup->mux()->setOutputRate( m_transferRate ) ;
	while(1)
	{
		// loop but check for kill notification
		// Wait until either killed or output rate changed.
		switch( WaitForMultipleObjects( 2, _handlers, FALSE, m_timeInterval) )
		{
			case WAIT_OBJECT_0+1:			// reset event
				ResetEvent( _eventReset ) ;
			case WAIT_TIMEOUT:				// scheduler timeout
			{
				float old_rate = m_transferRate ;
				m_transferRate = updateScheduler( &m_timeInterval );
				if( m_transferRate != old_rate )
				{
					MfxPostMessage( EMsg_CfgOutputRateChanged, *(long*)&m_transferRate );
					serverSetup->mux()->setOutputRate( m_transferRate ) ;
				}
				break;
			}
			case WAIT_OBJECT_0:				// kill event
				return FALSE;
		}
	}
	return FALSE;
}

int SchedulerManager::ExitInstance( )
{
	CloseHandle(_eventKill );
	CloseHandle(_eventReset);
	return CWinThread::ExitInstance();
}

void SchedulerManager::kill()
{
	VERIFY( SetEvent( _eventKill ) );
	SetThreadPriority	( THREAD_PRIORITY_ABOVE_NORMAL );
	WaitForSingleObject	( m_hThread, 500);
}


// return current rate and how long should be applied
float SchedulerManager::updateScheduler( time_t *msecs )
{
	CfgOutputScheduler  *foundSch=NULL;
	time_t timeNow ;
	tm	   ttm ;
	int	   i;		

	time( &timeNow );
	ttm = *localtime( &timeNow );
	// Find scheduler acc. to the date
	for( i=0; i<m_scheduler->count(); i++ )
	{
		CfgOutputScheduler *sch = (*m_scheduler)[i];

		if( sch->_viaDayName )
			continue;
		time_t  tDate = sch->_date;
		tm     *tmDate= localtime( &tDate );
		if( ttm.tm_year == tmDate->tm_year &&
			ttm.tm_mon  == tmDate->tm_mon  &&
			ttm.tm_mday == tmDate->tm_mday )
		{
			foundSch = sch ;
			break ;
		}
	}

	// If not found, try via day name.
	if( foundSch == NULL )
	{
		int day = ttm.tm_wday;
		for( i=0; i<m_scheduler->count(); i++ )
		{
			CfgOutputScheduler *sch = (*m_scheduler)[i];

			if( !sch->_viaDayName )
				continue;
			int schDay = (int)sch->dayName();
			
			if( schDay <= sch->Sat && day != schDay ||
			  ((schDay == sch->Mon_Fri && (day <1   || day > 5) ) || 
			   (schDay == sch->Weekend &&  day != 0 && day !=6)))
				continue;
			foundSch = sch ;
			break ;
		}
	}

	float rate ;
	if( foundSch == NULL )
	{
		// wait for tomorrow
		rate	   = 0 ;
		time_t tilMidnight = (23 - ttm.tm_hour)*3600 + (59 - ttm.tm_min)*60 + (60 - ttm.tm_sec) ;
		*msecs = tilMidnight + 1 ;
	}
	else
		rate = foundSch->getRate( &ttm, msecs ) ;
	*msecs *= 1000 ;
	return rate ;
}
*/

//----------------------------------------------------------------------------
//	CfgOutputSchedulerPtrArray
//	Dynamic array of schedulers
//----------------------------------------------------------------------------


// local functions
static void _deleteScheduler( CfgOutputScheduler **adr )
{
	delete *adr;
}
static int _findScheduler( void *adr2, CfgOutputScheduler **adr1 )
{
	return (*adr1)->hasEqualDate( *(CfgOutputScheduler*)adr2 ); 
}

CfgOutputSchedulerPtrArray:: CfgOutputSchedulerPtrArray()
						   : sTemplateArray<CfgOutputScheduler*>()
{
	bIgnoreSchedular = TRUE;
	setDelFunc( &_deleteScheduler );
}

CfgOutputSchedulerPtrArray &CfgOutputSchedulerPtrArray::operator = ( const CfgOutputSchedulerPtrArray &list)
{
	clearList();
	bIgnoreSchedular = list.bIgnoreSchedular;
	for( int i=0; i<list.count(); i++ )
		add( new CfgOutputScheduler( *list[i] ) );
	return *this;
} ;


// if TRUE - scheduler was copied and may be deleted
BOOL CfgOutputSchedulerPtrArray::addOrModify( CfgOutputScheduler &adr, int &index )
{
#ifndef EMPTY_VERSION
	index = find( (void*)&adr, &_findScheduler );  	

	if( index >= 0 )
	{
		*item(index) = adr ;
		return TRUE;
	}
	add( &adr );
	return FALSE;
#else
	return TRUE;
#endif
}

BOOL CfgOutputSchedulerPtrArray::delScheduler( CfgOutputScheduler &adr )
{
	int index = find( (void*)&adr, &_findScheduler ); 	
	if( index != -1 )
	{
		del( index );
		return TRUE;
	}
	return FALSE;
}

void CfgOutputSchedulerPtrArray::storage( CfgBaseSetup *cfg, LPCTSTR lpSection, LPCTSTR lpValue, BOOL bSave )
{
  #ifndef EMPTY_VERSION
	LPCTSTR lpIgnoreSchedular = "IgnoreSchedular";

	BaseConfigClass *c = cfg->cfg() ;
	if( bSave )
	{
		c->delSection( lpSection );
		c->setInt( lpSection, lpIgnoreSchedular, bIgnoreSchedular );
		for( int i=0; i<count(); i++ )
			item(i)->storage( cfg, lpSection, lpValue, bSave );
	}
	else
	{
		char  **list;
		int		n_vars;

		// read Scheduler
		clearList();
		c->getInt( lpSection, lpIgnoreSchedular, &bIgnoreSchedular );
		if( !c->varList( lpSection, &list, &n_vars ) )
		{
			char *vars;
			for( int i=0; i<n_vars; i++ )
			{
				vars = list[i];
				if( vars && stricmp( vars, lpIgnoreSchedular) )
				{
					CfgOutputScheduler *rate = new CfgOutputScheduler;
					
					rate->storage( cfg, lpSection, vars, bSave );
					add( rate );
				}
			}
			c->destroyVarList( list ) ;
			if( GetProgramLevel()==ProgramLevel_Basic )
			{
				for( int i=count()-1; i>=0; i-- )
				{
					CfgOutputScheduler *rate = item(i);
					char buff[64];
				
					rate->dateAsString( buff );
					if( stricmp( buff, "Monday-Sunday") )
						del( i );
				}
			}
		}
		
		if( count()==0 )
		{	// no vars found in cfg, set default
			CfgOutputScheduler *rate = new CfgOutputScheduler;
			
			rate->storage( cfg, lpSection, "Monday-Sunday", FALSE );
			rate->fromStringToRates( "0-24<1.000" );
			add( rate );
		}
	}
  #endif
}


//----------------------------------------------------------------------------
//	CfgDVBSetup
//----------------------------------------------------------------------------


void CfgDVBSetup ::setDefaults( )
{
	USE_PESHEADER	= 0 ;
	_absPriorityPart= 50 ;
	_aliveSgn		= TRUE ;
	_aliveInterval  = 5 ;
	_numLogEntries	= 1000 ;
	_autoStart		= TRUE ;
	_singlePID		= 0	   ;
	_useSinglePID	= TRUE;
	_useMultiSection= FALSE;
	_monitorringPort= 12346;
	memset( &_driverSrvProps, 0, sizeof(DvbDriverSrvProps) );
	_multiplePIDs.clearList();
}

CfgDVBSetup::CfgDVBSetup( const char *file ) : CfgBaseSetup( file )
{
	_logFile		= freeAndStrdup( NULL, "md_DvbServer.log" );
	_connectString	= NULL ;
	_channelDefaults= new CfgChannel( 0, "ChannelDefaults" ) ;
	setDefaults() ;
}

CfgDVBSetup::~CfgDVBSetup()
{
	free(_logFile);
	free(_connectString);
	delete _channelDefaults ;
}
	
BOOL CfgDVBSetup::initDriverProp()
{
	CfgChannelsSetup* chs= MfxChannelsSetup();

	_multiplePIDs.clearList();
	memset( &_driverSrvProps, 0, sizeof(DvbDriverSrvProps) );
	if( !chs || !bigComIO )
	{
		_driverSrvProps.outputRate	= 1.f;
		return FALSE;
	}
	for( int i=chs->numChannels()-1; i>=0; i-- )
	{
		UINT pid = chs->channel(i)->channelPID();

		if( _multiplePIDs.find(pid )==-1 )
			_multiplePIDs.add( pid );
	}
	((BigComOut*)bigComIO)->getDrvProperties( &_driverSrvProps ) ;
	return TRUE;
}

// data exchange between the object and config file
void CfgDVBSetup::storage( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR lpErrBuffer )
{
	LPCSTR	lpTemp = "MonitorTmp";

	if( lpErrBuffer )
		lpErrBuffer[0] = 0;

	_channelDefaults->storage(	cfg, saveFlag, _channelDefaults->serviceName() ) ;
	//_schedulers.storage(		cfg, dvbSchedulerSection, NULL, saveFlag ); 
	if( saveFlag )
	{
		char buf[80] ;
		int relPPart = 100 - _absPriorityPart;

		// DVB settings
		sprintf( buf, "%s; each %d secs", _aliveSgn ? "on" : "off", _aliveInterval ) ;
		set   ( "", "AliveSignal"  ,	buf );
		set   ( "", "ConnectString",	_connectString ? _connectString : "" ) ;
		setInt( "", "autoStart",		_autoStart ) ;
		setInt( "", "relPriorityPart",	relPPart	 );
		config->printf( "", "LogFile", "%s,%d", _logFile, _numLogEntries) ;

		if( sendCATableString() == NULL )
			config->set( "", "sendCA_Table", "0-24" ) ;
		setInt( "", "MonitorringPort",	_monitorringPort );
		setInt( "", "SinglePID",		_singlePID	 );
		setInt( "", "UseSinglePID",		_useSinglePID);

		// MPE settings
		uchar  proto ;
		ushort port ;
		dvbProtocolSetup.get( port, proto ) ;
		setInt( "MPE", "useMultiSection",_useMultiSection);
		setInt( "MPE", "UdpPort",		port);
		setInt( "MPE", "IpProtocol",	proto);

		// Temporarry stored for monitorring
		config->setFloat( lpTemp, "DvbOutRate",		_driverSrvProps.outputRate		);
		set				( lpTemp, "DrvCOMInterface",_driverSrvProps.commInterface	);
		setInt			( lpTemp, "DrvFlags",		(int)_driverSrvProps.flags		);
		if( _multiplePIDs.count() > 0 )
		{
			char pids[4196]="";
			int	 iLen = 0;

			for( int i=0; i<_multiplePIDs.count() && iLen<4180; i++ )
			{
				iLen = strlen(pids);
				sprintf( pids + iLen, "%ld,", _multiplePIDs[i] );
			}
			set( lpTemp, "MultiplePIDs",	pids );
		}
		else
			set( lpTemp, "MultiplePIDs",	""   );
	}
	else
	{
		char	warningMsg[1024];
		char*	nextWarning = warningMsg;
		char*	str;
		int		relPPart;
		
		*nextWarning = 0;

		// clear
		setDefaults() ;
		if( _connectString )
		{
			FREE( _connectString ) ;
			_connectString = NULL ;
		}

		// DVB settings
		config->getInt  ( "", "SinglePID",		&_singlePID	  );
		config->getInt  ( "", "UseSinglePID",	&_useSinglePID);
		config->getInt  ( "", "autoStart"      ,&_autoStart   );
		config->getInt  ( "", "relPriorityPart",&relPPart	  );
		_absPriorityPart = 100 - relPPart;

		// MPE settings
		int proto ;
		int port ;
		config->getInt  ( "MPE", "useMultiSection",&_useMultiSection);
		config->getInt  ( "MPE", "UdpPort",	   &port ) ;
		config->getInt  ( "MPE", "IpProtocol",	   &proto) ;
		dvbProtocolSetup.set( (ushort)port, (uchar)proto ) ;

		if( GetProgramLevel()==ProgramLevel_Basic || runAsDvbIpGateway() )
			_useSinglePID = TRUE;

		if( !initDriverProp() )
		{
			// Temporarry stored for monitorring
			if((str = get( lpTemp, "DrvCOMInterface" )) )
				strncpy( _driverSrvProps.commInterface, str, 32 );	
			if((str = get( lpTemp, "DvbOutRate" )) )
				sscanf( str, "%f", &_driverSrvProps.outputRate );
			if((str = get( lpTemp, "DrvFlags" )) )
				sscanf( str, "%ld", &_driverSrvProps.flags );
			if((str = get( lpTemp, "MultiplePIDs" )) && str[0] )
			{
				char *end;

				while( (end=strchr( str, ',')) )
				{
					end[0] = 0;
					_multiplePIDs.add( (UINT)atoi( str ) );
					end[0] = ',';
					str = end+1;
				}
			}
		}

		// scheduler
		{
			CfgOutputScheduler *rate = new CfgOutputScheduler;
			char	buff[64];

			_schedulers.clearList();
			_schedulers.add( rate );
			_schedulers.setIgnoreFlag( FALSE );
			sprintf( buff, "0-24<%.3f", _driverSrvProps.outputRate );
			rate->setSchedular( "Monday-Sunday", buff );
		}

		// read AliveSignal
		str=get( "", "AliveSignal");
		if( str != NULL )
		{
			char buf[256] ;
			strcpy( buf, str ) ;
			char *wrd = strtok( buf, " \t" ) ;
			if( wrd != NULL  &&  stricmp( wrd, "on;" ) == 0 )
			{
				_aliveSgn = TRUE;
				wrd = strtok( NULL, " \t" ) ;		// "each"
				if( wrd != NULL )
					wrd = strtok( NULL, " \t" ) ;	// "nnn"
				if( wrd != NULL )
					isInt( wrd, &_aliveInterval ) ;
			}
			else
			{
				_aliveSgn = FALSE;
				nextWarning += sprintf( nextWarning, "\nAliveSignal: signal is switched off. No alive packets will be sent.\n" );
			}
		}

		// read LogFile
		str=get( "", "LogFile" );
		if( str )
		{
			char *end = strchr( str, ',' );

			_logFile = freeAndStrdup( _logFile, str );
			if( end )
			{
				_logFile[end-str] = 0;
				if( !isInt( end+1, &_numLogEntries ) )
				{
					_numLogEntries = 1000;
					nextWarning += sprintf( nextWarning, "\nLogFile: value the number of log entries is incorrect. Using the default 1000.\n" );
				}
			}
		}

		// read ConnectString
		str=get( "", "ConnectString" );
		if( str )
			_connectString = freeAndStrdup( _connectString, str ) ;
		char expl[256] ;
		/*BOOL connectStringOk =*/ BigComIO::isConnectStringOk( _connectString, expl) ;
		if( expl[0] != 0 )
			nextWarning += sprintf( nextWarning, "\n\n%s", expl ) ;

		config->getInt( "", "MonitorringPort", &_monitorringPort);

		if( nextWarning != warningMsg )
		{
			char buf[1024];
			sprintf( buf, "Following config settings were found illegal or suspicious:\n%s\n"
					"\nCorrect in the configuration file (%s), please.",
					warningMsg, config->fileName() );
			if( lpErrBuffer )
				strcpy( lpErrBuffer, warningMsg );
			else
				MessageBox( NULL, buf, "Warning", MB_OK | MB_ICONINFORMATION | MB_TOPMOST );
		}
	}
}


//-------------------------------------------------------------------------------
//	DvbServerSetup
//-------------------------------------------------------------------------------


DvbServerSetup::DvbServerSetup()
{
	_serviceChannel = NULL ;
	_serviceChannelPid = NULL ;
	_serviceManager	= NULL;
	_dvbSetup		= NULL;
	_channelsSetup	= NULL;
	_providersSetup	= NULL;
	_usersSetup		= NULL;
	_bigComOut		= NULL;
	_muxSetup		= NULL;
	_muxOutput		= NULL;
	_schedManager   = NULL;
	_channelSchedular=NULL;
	_pidStreamManager=NULL;
	_mux			= NULL;
	_channelsStarted= 0 ;
	_usrName[0]		= 0 ;
	_thread			= NULL;

	char  buf[80] ;
	DWORD n_chars=80 ;
	if( GetUserName( buf, &n_chars) )
	{
		strncpy( _usrName, buf, sizeof(_usrName) ) ;
		_usrName[ sizeof(_usrName)-1] = 0 ;
	}
	else
		_usrName[0] = 0 ;

	char path[1024] ;
	MfxGetFullPath( "Config\\Context.cfg", path ) ;
	_context		= new CfgContext( path )		 ;

	MfxGetFullPath( "Config\\Dvb.cfg", path ) ;
	_dvbSetup		= new CfgDVBSetup( path )		 ;

	try
	{
		//_dvbSetup->openConfig();
		_dvbSetup->cfg()->install( (const char**)defaults, n_defaults ) ;
	}
	catch( Msg &msg )
	{
		stdErrorDialog( "(DvbClient.cfg)\n\n%s", msg.shortString ) ;
	}
	catch( ... )
	{
	}

	MfxGetFullPath( "Config\\Channels.cfg", path ) ;
	_channelsSetup	= new CfgChannelsSetup( path );	

	MfxGetFullPath( "Config\\Providers.cfg", path ) ;
	_providersSetup	= new CfgProvidersSetup( path );

	MfxGetFullPath( "Config\\Users.cfg", path ) ;
	_usersSetup		= new CfgUsersSetup( path ) ;

	InitializeCriticalSection( &_dvbServerSetupLock );
}

DvbServerSetup::~DvbServerSetup()
{
	bigComIO = NULL ;

	if ( _thread )
		WaitForSingleObject( _thread, INFINITE );
	if( _channelSchedular )
		delete _channelSchedular;
//	if ( _schedManager )
//		delete _schedManager	;
	if ( _pidStreamManager )
		delete _pidStreamManager ;
	if ( _mux )
		delete _mux				;
	if ( _muxOutput )
		delete _muxOutput		;
	if ( _muxSetup )
		delete _muxSetup		;
	if ( _bigComOut )
		delete _bigComOut		;
	if( _context )
		delete 	_context		;
	if ( _dvbSetup )
		delete _dvbSetup		;
	if ( _channelsSetup )
		delete _channelsSetup	;
	if ( _providersSetup )
		delete _providersSetup	;
	if ( _usersSetup )
		delete _usersSetup		;

	DeleteCriticalSection( &_dvbServerSetupLock );
}

void DvbServerSetup::resetScheduler()
{
#ifndef EMPTY_VERSION
	if( _channelSchedular )
		_channelSchedular->reset();
	//if( _schedManager )
	//	_schedManager->reset() ;
#endif
}

void DvbServerSetup::load()
{
	_dvbSetup		->load() ;
#ifndef EMPTY_VERSION
	_channelsSetup	->load() ;
	_providersSetup	->load() ;
	_usersSetup		->load() ;
#endif
	_context		->load() ;
}

void DvbServerSetup::save()
{
	char buf[3000] ;
	buf[0] = 0 ;
	try
	{
		_dvbSetup->save() ;
	}
	catch( Msg &msg1 )
	{
		sprintf( buf, "%46s%s\n", "", msg1.shortString ) ;
	}
#ifndef EMPTY_VERSION
	try
	{
		_channelsSetup->save() ;
	}
	catch( Msg &msg2 )
	{
		sprintf( buf+strlen(buf), "%46s%s\n", "", msg2.shortString ) ;
	}
	try
	{
		_providersSetup->save() ;
	}
	catch( Msg &msg3 )
	{
		sprintf( buf+strlen(buf), "%46s%s\n", "", msg3.shortString ) ;
	}
	try
	{
		_usersSetup->save() ;
	}
	catch( Msg &msg4 )
	{
		sprintf( buf+strlen(buf), "%46s%s\n", "", msg4.shortString ) ;
	}
#endif
	try
	{
		_context->save() ;
	}
	catch( Msg &msg4 )
	{
		sprintf( buf+strlen(buf), "%46s%s\n", "", msg4.shortString ) ;
	}
	if( buf[0] != 0 )
		throw Msg( -1, "Error when saving the configuration:\n%s", buf ) ;
}

void DvbServerSetup::open()
{
	const char *connectStr = _dvbSetup->connectString();

	_muxSetup	  = NULL;
	_muxOutput	  = NULL;
	_schedManager = NULL;
	_mux		  = NULL;
	_bigComOut	  = NULL;

	try
	{
		_bigComOut = new BigComOut( ) ;

		char expl[512] ;
		char shortMsg[512] ;

		if( !_bigComOut->create( connectStr, expl) )
		{
			sprintf( shortMsg, "%s: %s", connectStr, expl ) ;
			MfxPostMessage( EMsg_CommunicationError, 0, shortMsg ) ;

			throw Msg( -1, "Can't open comunication protocol\n%s\n\nApplication will be shutdown.", shortMsg ) ;
		}

		bigComIO   = _bigComOut ;
		int comErr = _bigComOut->open( _dvbSetup->cfg() ) ;
		if( comErr != 0 )
		{
			DvbEventText( comErr, expl) ;

			sprintf( shortMsg, "%s: %s", connectStr, expl ) ;
			MfxPostMessage( EMsg_CommunicationError, 0, shortMsg ) ;

			char buf[1024] ;
			sprintf( buf, "Can't open comunication protocol\n%s\n\nNo data will be sent. (Run Setup to correct the problem.)",
				shortMsg, expl ) ;
			AfxMessageBox( buf ) ;
		}
	}
	catch ( ... )
	{
		bigComIO = NULL ;
		delete _bigComOut;
		_bigComOut = NULL ;
		throw;
	}

	_pidStreamManager = new PidStreamAttribManager ;
	_muxSetup	  = new MuxSetup ();
	_muxOutput	  = new MuxOutput( _bigComOut->com(), _muxSetup ) ;
	//_schedManager = new SchedulerManager();
	_muxSetup->absPriorityPart = MfxDvbSetup()->absPriorityPart();
	_muxSetup->relPriorityPart = 100 - _muxSetup->absPriorityPart;
	_mux		  = new Mux		 ( _muxSetup, _muxOutput) ;
//	_schedManager->start();

	CfgChannelsPtrArray &arr = _channelsSetup->channels;

	for( int i=0; i<arr.count(); i++ )
	{
		CfgChannel *ch = arr[i];
		createMuxChannel( ch );
	}

	// channel schedular
	_channelSchedular = new ChannelScheduler;
	_channelSchedular->start();
}

void DvbServerSetup::close()
{
	if (!runAsDvbIpGateway())
		closeServiceChannel();

	for( int i=_muxChannels.count()-1; i>=0; i-- )
	{
		MuxChannel *muxCh = _muxChannels[i];

		muxCh->initiateStop();
		_mux->unregisterChannel( muxCh ) ;
		_muxChannels.delObj( muxCh );
		delete muxCh;
	}
//	if ( _schedManager != NULL )
//		_schedManager->kill();
	if ( _channelSchedular!=NULL )
		_channelSchedular->kill();
}

//-------------------------------------------------------------------------


void DvbServerSetup::getMuxChannelSetup( const CfgChannel *ch, MuxChannelSetup *s )
{
	EnterCriticalSection( &_dvbServerSetupLock);
	if( s->isEmpty() )
	{
		s->channel	 = ch->serviceID() ;
		s->inboxDir  = getAllocatedFullPath( ch->inboxPath() ) ;
		strncpy( s->name, ch->serviceName(), 31 ) ;
	}
	s->absPriority		= ch->absPriority()		;
	s->relPriority		= ch->relPriority()		;
	s->minRate			= ch->outputRateMin()	;
	s->maxRate			= ch->outputRateMax()	;
	s->numRebroadcasts	= ch->numRebroadcasts() ;
	s->fileSendDelay	= ch->inboxSendDelay()	;
	s->volumeLimitPerDay= ch->maxVolumePerDay() ;
	s->rebroadcastDelay = ch->rebroadcastDelay();
	s->streamFormat		= ch->streamFormat()	;
	s->fecLevel			= ch->FEClevel()		;
	if( s->fecLevel  &&  ch->useFECrebroadcast() )
		s->fecRebrSize = ch->maxFECrebrSize() ;
	else
		s->fecRebrSize = 0 ;

	int pid ;
	if( _dvbSetup->useSinglePID()  ||  (GetProgramLevel()==ProgramLevel_Basic) )
		pid = _dvbSetup->getSinglePID() ;
	else
		pid = ch->channelPID()		;
	
	if (s->channelPID)
		pidStreamManager()->releasePidStreamAttrib(s->channelPID) ;
	s->channelPID = pidStreamManager()->getPidStreamAttrib(pid) ;

	LeaveCriticalSection( &_dvbServerSetupLock);
}

//------- Channels --------------------------------------------------------

MuxChannel *DvbServerSetup::createMuxChannel( CfgChannel *ch, char *expl )
{
	ASSERT( ch );
	MuxChannelSetup	setup;
	getMuxChannelSetup( ch, &setup );		

	MuxChannel *muxCh=NULL ;
	try
	{
		if( ch->serviceID() == 0 )
		{
			muxCh = new ServiceChannel( _mux, &setup, ch->channelSwitchOn() );
			openServiceChannel( (ServiceChannel*)muxCh ) ;
		}
		else
		if( ch->serviceID() >= 0xfff0 )		// more Internet channels?
			muxCh = new InternetChannel( _mux, &setup, ch->channelSwitchOn() );
		else
			muxCh = new Inbox( _mux, &setup, ch->channelSwitchOn() );
		_muxChannels.add( muxCh );
		_mux->registerChannel( muxCh );
		return muxCh ;
	}
	catch( int err )
	{
		if( expl != NULL )
			DvbEventText( err, expl ) ;
	}
	catch( ... )
	{
		if( expl != NULL )
			strcpy( expl, "Unknown error during channel creation" ) ;
	}
	try
	{
		delete muxCh ;
	}
	catch(...) {}
	return NULL ;
}

void DvbServerSetup::deleteMuxChannel( CfgChannel *ch )
{
	ushort id = ch->serviceID() ;
	MuxChannel *muxCh = getMuxChannel( id );

	if( muxCh != NULL )
	{
		muxCh->initiateStop();
		_mux->unregisterChannel(muxCh ) ;
		_muxChannels.delObj( muxCh );
		delete muxCh;
		if( id != 0  &&  !muxCh->isInternetChannel() )
		{
			char path[1024], drive[20], dir[1024];

			GetModuleFileName( NULL, path, 1024 );
			_splitpath( path, drive, dir, NULL, NULL );
			_makepath ( path, drive, dir, ch->inboxPath(), NULL );
			if( dirExist(path) )
			{
				if( isDirEmpty(path)  ||
					askYesDialog( "Channel Inbox directory\n\t%s\nis nonempty.\n\nDelete anyway ?", path) )
					rmWholeDir(path) ;
			}
		}
	}
}

BOOL DvbServerSetup::resetMuxChannel( CfgChannel *ch )
{
	MuxChannel *muxCh = getMuxChannel( ch->serviceID() );
	if( muxCh == NULL )
		return FALSE ;

	MuxChannelSetup	setup;

	getMuxChannelSetup( ch, &setup );		
	muxCh->reset( &setup ) ;
	return !muxCh->isSending() ;
}

static int findMuxChannel( void *adr2, MuxChannel **adr1 )		{ return (*adr1)->channelID() == (ushort)adr2; };
MuxChannel *DvbServerSetup::getMuxChannel( ushort channelID )
{
	int index = _muxChannels.find( (void*)channelID, &findMuxChannel );  	
	if( index != -1 )
		return _muxChannels[index];
	return FALSE;
}

static DWORD WINAPI threadStartChannels( void *param )
{
	((DvbServerSetup *)param)->startAllChannelsThr();
	return 0;
}

BOOL DvbServerSetup::startAllChannels()
{
	if( !_bigComOut->isOpened() )
		return FALSE ;

	DWORD dwthrID;
	if ( _thread )
	{
		WaitForSingleObject( _thread, INFINITE );
		CloseHandle( _thread );
		_thread = NULL;
	}
	_thread = CreateThread( NULL, 0, threadStartChannels, this, 0, &dwthrID );
	return TRUE ;
}

void DvbServerSetup::startAllChannelsThr()
{
	int i;
	//for( i=1; i<_muxChannels.count(); i++ )
	//{
	//	MuxChannel *chan = _muxChannels[i];
	//	chan->create();
	//}
	for( i=1; i<_muxChannels.count(); i++ )
	{
		MuxChannel *chan = _muxChannels[i];
		chan->start();
	}
	_channelsStarted = TRUE ;
}

BOOL DvbServerSetup::startServiceChannel()
{
	if( !_bigComOut->isOpened() )
		return FALSE ;

	#ifndef EMPTY_VERSION
		MuxChannel *chan = _muxChannels[0];
		chan->start();
	#endif

	_mux->start();
	return TRUE ;
}

static DWORD WINAPI threadStopChannels( void *param )
{
	((DvbServerSetup *)param)->stopAllChannelsThr();
	return 0;
}

void DvbServerSetup::stopAllChannels()
{
	DWORD dwthrID;
	if ( _thread )
	{
		WaitForSingleObject( _thread, INFINITE );
		CloseHandle( _thread );
		_thread = NULL;
	}
	_thread = CreateThread( NULL, 0, threadStopChannels, this, 0, &dwthrID );
}

void DvbServerSetup::stopAllChannelsThr()
{
	int i;
	for( i=1; i<_muxChannels.count(); i++ )
	{
		MuxChannel *chan = _muxChannels[i];
		chan->initiateStop();
	}
	for( i=1; i<_muxChannels.count(); i++ )
	{
		MuxChannel *chan = _muxChannels[i];
		chan->stop();
	}
	_channelsStarted = FALSE ;
}

void DvbServerSetup::stopServiceChannel()
{
#ifndef EMPTY_VERSION
	if ( _thread )
	{
		WaitForSingleObject( _thread, INFINITE );
		CloseHandle( _thread );
		_thread = NULL;
	}
	MuxChannel *chan = _muxChannels[0];
	chan->stop();
#endif
	_mux->stop();
}


//------------------------------ sent CA Table -------------------------------

BOOL DvbServerSetup::addTimeInterval( sTimeInterval interval )
{
	if( interval.from == interval.to )
		return FALSE;

	if( _sendCATable.count() == 0 )
	{
		_sendCATable.add( interval );
		return TRUE;
	}

	int count = _sendCATable.count();
	int i = 0;
	while( i < count && _sendCATable[i].to <= interval.from )
		i++;

	if( i == count )
	{
		if( interval.from > interval.to )
		{
			if( _sendCATable[count - 1].from > _sendCATable[count - 1].to ||
				interval.to > _sendCATable[0].from )
				return FALSE;
		}
		_sendCATable.add( interval );
	}
	else
	{
		if( interval.from > interval.to ||
			interval.to > _sendCATable[i].from )
			return FALSE;
		_sendCATable.insert( i, interval );
	}
	return TRUE;
}

inline int get_number( const char **str, char len )
{
	int			retval = 0;
	const char	*s = *str;

	while( *s && ( *s == ' ' || *s == '\t' ) )
		s++;

	while( len > 0 && *s && isdigit( *s ) )
	{
		retval = retval * 10 + *s - '0';
		len--;
		s++;
	}

	while( *s && ( *s == ' ' || *s == '\t' ) )
		s++;

	*str = s;
	return retval;
}

long DvbServerSetup::getSendCATableWaitTime()
{
	if( !_sendCATable.count() )
		return -1;

	time_t actualTime;
	time( &actualTime );
	struct tm *gmt = localtime( &actualTime );
	long t = ( gmt->tm_hour * 60 + gmt->tm_min ) * 60 + gmt->tm_sec;

	int c = _sendCATable.count();
	int i = _actualTimeInterval;
	while( i < c &&  t > _sendCATable[i].from )
		i++;
	if( i == c )
		_actualTimeInterval = 0;
	else
		_actualTimeInterval = i;

	srand( (unsigned)time( NULL ) );
	sTimeInterval interval = _sendCATable[_actualTimeInterval++];
	if( _actualTimeInterval == _sendCATable.count() )
		_actualTimeInterval = 0;
	int diff = interval.to - interval.from;
	if( interval.from >= interval.to )
		diff += 86400;	// 24 * 60 * 60
	int random = interval.from + ( rand() + 1 ) % diff;


	t = random - t;
	if( t < 0 )
		t += 86400;	// 24 * 60 * 60
	else if( t == 0 )
		t += diff / ( ( rand() + 1 ) %  ( ( diff / 2 ) + 1 ) ) + 1;

	return t;
}

// Does some preparations for sending CA-tables.
// Call at the program begin before first send.
BOOL DvbServerSetup::initSendCATable()
{
	const char *senCA_Table = _dvbSetup->sendCATableString();

	_actualTimeInterval = 0;
	if ( !senCA_Table || !strlen( senCA_Table ) )
	{
		senCA_Table = "0-24";		// if not exist in cfg file create the default
		MessageBox( NULL, "The sendCA_Table variable is not defined in config file.\n"
						  "Using default 0-24 (Meaning CA permissions will be broadcasted once a day).\n\n"
						  "To change broadcast frequency modify configuration setting sendCA_Table manually.\n"
						  "To accept new value the program must not be running!", "Warning",
						  MB_OK | MB_ICONINFORMATION | MB_TOPMOST );
	}
	if( senCA_Table )
	{
		sTimeInterval	interval;
		const char 		*str = senCA_Table;
		uchar			hour, min;

		// ";" separated list of time intervals "hr1[:min1]-hr2[:min2]"
		while( *str )
		{
			// get time interval
			hour = get_number( &str, 2 );
			if( !*str )
				return FALSE;
			if( *str == ':' )
			{
				str++;
				min = get_number( &str, 2 );
			}
			else
				min = 0;
			if( !*str || *str != '-' )
				return FALSE;
			str++;
			if( hour > 24 || ( hour == 24 && min != 0 ) || min > 59 )
				return FALSE;
			if( hour == 24 )
				interval.from = 0;
			else
				interval.from = ( hour * 60 + min ) * 60;

			hour = get_number( &str, 2 );
			if( *str )
			{
				if( *str == ':' )
				{
					str++;
					min = get_number( &str, 2 );
				}
				else
					min = 0;

				if( *str && *str != ';' )
					return FALSE;
				else if( *str )
					str++;
			}
			else
				min = 0;
			if( hour > 24 || ( hour == 24 && min != 0 ) || min > 59 )
				return FALSE;
			interval.to = ( hour * 60 + min ) * 60;

			// add time interval
			if( !addTimeInterval( interval ) )
				return FALSE;
		}

		// set actual time interval
		time_t actualTime;
		time( &actualTime );
		struct tm *gmt = localtime( &actualTime );
		long t = ( gmt->tm_hour * 60 + gmt->tm_min ) * 60 + gmt->tm_sec;
		int i = 0;
		while( i < _sendCATable.count() && _sendCATable[i].from < t )
			i++;
		if( i == _sendCATable.count() )
			_actualTimeInterval = 0;
		else
			_actualTimeInterval = i;

	}
	return TRUE;
}
