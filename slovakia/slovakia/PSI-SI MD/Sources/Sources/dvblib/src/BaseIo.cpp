/*
 *	Filename:		BaseIo.cpp
 *
 *	Version:		1.00
 *
 *	Description:	This file contains implementation of various data sender and receiver classes
 *					
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"
#include "MuxPacket.hpp"

#include <afxconv.h>
#include <errno.h>
#include "inbox.hpp"
#include "ClientCfg.hpp"
#include "ServCfg.hpp"
#include "..\\..\\IoDrivers\\IoHNet\\AppLib\\HNet.h"
#include "BaseRegistry.hpp"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DvbProtocolSetup dvbProtocolSetup ;

DvbProtocolSetup::DvbProtocolSetup()
{
	set( 4321, 0xED ) ;
}

BOOL DvbProtocolSetup::set( ushort udpPort, uchar ipProtocol )
{
	BOOL changed=FALSE ;
	udpPort = (udpPort<<8) | (udpPort>>8) ;
	if( _mpeUdpPort != udpPort )
	{
		_mpeUdpPort = udpPort ;
		changed = TRUE ;
	}
	if( _mpeHnetIpProtocol != ipProtocol )
	{
		_mpeHnetIpProtocol = ipProtocol ;
		changed = TRUE ;
	}
	return changed ;
}

void DvbProtocolSetup::get( ushort &udpPort, uchar &ipProtocol )
{
	udpPort    = (_mpeUdpPort<<8) | (_mpeUdpPort>>8) ;
	ipProtocol = _mpeHnetIpProtocol ;
}

// a = no aliasing
// g = global
// t = fast machine code
// y = no stack frame
#define OPTOPTIONS	"agt"
//#define OPTOPTIONS	"agty"
#pragma warning(disable:4083)	// disable 'expected string' warning

// Maximal number of MUX packets in 1 UDP packet (in case of IP/UDP/TS/MUX packet sending)
// Must not be higher than 7 (to avoid IP packet being bigger than Ethernet packet size (1500 bytes))
#define MAX_IPUDP_TSPACKETS	7	// 7 TS packets into 1 UDP => IP packet size is 7*188+28=1344

// Maximal number of MUX packets in 1 IP packet (in case of IP/MUX packet sending)
// Must not be higher than 7 (to avoid IP packet being bigger than Ethernet packet size (1500 bytes))
#define MAX_IP_MUXPACKETS	7

// MAC address used for broadcasting
// All receiving drivers should accept packets with this MAC in the MPE header
// It is calculated MAC from broadcast IP address 255.255.255.255
static __int64 broadcast_adr =  0xFFffFFffFFff ; // 0x0e4029c50200 ;

// if uncommented, MAC address of the outgoing MPE sections is calculated
// from the destination IP address of the outgoing IP datagram
// e.g. if IP is 197.41.64.14 => MAC will be 00-02-C5-29-40-0E
//		prefix 00-02 is standard supported by satellite reception drivers
//		for calculated unicast MAC addresses
//#define CALCULATE_MAC_FROM_IP

#define MAXSERVICESIZE	(65536+sizeof(ServiceHdr))


//-------------------------------------------------------------------------------
// IP related utilities
//-------------------------------------------------------------------------------


// UDP encapsulation
inline ushort ntols( ushort p )
{
	return (p<<8) | (p>>8) ;
}

static void RecalculateIPHeaderChecksum( IN IP_HEADER_TYPE *pip	)
{
	//zero checksum field
	pip-> ip_checksum = 0;

	//count new checksum
	USHORT *buf = (USHORT*)pip ;
	int nwords = IP_HLEN(pip) >> 1 ;

	unsigned long sum;
	for(sum=0; nwords>0; nwords--)
		sum += *buf++;

	sum  = (sum >> 16) + (sum &0xffff);		//add in carry
	sum += (sum >> 16);

	//set new checksum
	unsigned short Chsum;
	Chsum  = (short)sum ;
	Chsum ^= 0xFFFF;
	pip-> ip_checksum = Chsum;
}

static ushort udpcksum(IP_HEADER_TYPE *pip)
{
	UDP_HEADER_TYPE *pudp = (UDP_HEADER_TYPE *)pip->ip_data;
	ulong udpsum = 0;
	ushort *sptr = (unsigned short *)(&pip->ip_src);

	for(int i=0; i<IP_ALEN;++i)
		udpsum += *sptr++;

	ushort len = ntols(pudp-> udp_len);
	//len = ntols(pip->ip_len) - IP_HLEN(pip);

	udpsum += ntols(IPT_UDP) + ntols(len);

	sptr = (unsigned short *)pudp;
	if(len % 2)
	{
		len >>= 1;
		for(i=0;i<=len;++i)
		{
			if(i==len)
			{
				udpsum += (*sptr++)&0xff;
			}
			else
				udpsum += *sptr++;
		}
	}
	else {
		len >>= 1;
		for(i=0;i<len;++i)
			udpsum += *sptr++;
	}

	udpsum  = (udpsum >> 16) + (udpsum &0xffff);		//add in carry
	udpsum += (udpsum >> 16);

	return (short)(udpsum & 0xffff);
}

static void RecalculateUDPHeaderChecksum( IP_HEADER_TYPE *pip )
{
	UDP_HEADER_TYPE *pudp = (UDP_HEADER_TYPE *)pip->ip_data;
	pudp-> udp_checksum = 0;
	ushort Chsum = udpcksum(pip);
	Chsum ^= 0xFFFF;
	if(Chsum != 0)
		pudp-> udp_checksum = Chsum;
	else
		pudp-> udp_checksum = 0xFFFFU;
}


//-------------------------------------------------------------------------------
// protocol structures
//-------------------------------------------------------------------------------


#pragma optimize( OPTOPTIONS, on )

// IP/UDP/TS/MUX packet
struct IpUdpTsPacket
{
	// IP header
	UCHAR	ip_verlen;			//IP version and header length (in longs)
	UCHAR	ip_tos;				//type of service
	UCHAR	ip_len[2];			//total packet length
	USHORT	ip_id;				//datagram ID
	USHORT	ip_fragoff;			//fragment offset (in 8-octets)
	UCHAR	ip_ttl;				//time to live, in gatewat hops
	UCHAR	ip_proto;			//IP protocol
	USHORT	ip_checksum;		//header checksum
	IPaddr	ip_src;				//IP address of source
	IPaddr	ip_dst;				//IP address of destination

	// UDP header
	ushort	 srcPort ;
	ushort	 dstPort ;
	uchar	 length[2] ;		// data + header
	ushort	 checkSum ;

	TsPacket tsPacket[MAX_IPUDP_TSPACKETS] ;
	IpUdpTsPacket( )	{}
	void init( int nTSPackets )
	{
		int len = aggregatedLength( nTSPackets ) ;

		ip_verlen	= 0x45 ;
		ip_tos		= 0 ;	
		ip_len[0]	= len>>8 ;
		ip_len[1]	= (UCHAR)len;
		ip_id		= 0 ;	
		ip_fragoff	= 0;
		ip_ttl		= 0x80;	
		ip_proto	= 0x11 ;	// UDP
		ip_checksum	= 0 ;
		ip_src		= 0x0100007F;
		ip_dst		= 0xFFFFFFFF;

		RecalculateIPHeaderChecksum((IP_HEADER_TYPE *)this) ;

		len -= 20 ;
		
		srcPort		= 0 ;
		dstPort		= dvbProtocolSetup.mpeUdpPort() ;
		length[0]	= len>>8;
		length[1]	= (UCHAR)len;
		checkSum	= 0 ;
	}
	
	inline MuxPacket *prepareMux( int i, ushort convertedPid )	
	{ 
		TsPacket *tp = tsPacket+i ;

		tp->sync = TsPacketSyncByte ;
		tp->pid  = convertedPid ;
		tp->flags= 0x10 ;	// continuity counter ignored (packets are not intercepted by HW)

		return &tp->data ;
	}

	inline void finalize()	{ RecalculateUDPHeaderChecksum((IP_HEADER_TYPE *)this); }
	inline static int aggregatedLength( int nPackets )	{ return 28+nPackets*TSPACKET_SIZE ; }
} ;


// IP/MUX packet
struct IpMuxPacket
{
	// IP header
	UCHAR	ip_verlen;			//IP version and header length (in longs)
	UCHAR	ip_tos;				//type of service
	UCHAR	ip_len[2];			//total packet length
	USHORT	ip_id;				//datagram ID
	USHORT	ip_fragoff;			//fragment offset (in 8-octets)
	UCHAR	ip_ttl;				//time to live, in gatewat hops
	UCHAR	ip_proto;			//IP protocol
	USHORT	ip_checksum;		//header checksum
	IPaddr	ip_src;				//IP address of source
	IPaddr	ip_dst;				//IP address of destination

	MuxPacket muxPacket[MAX_IP_MUXPACKETS] ;
	IpMuxPacket( )	{}
	void init( int nMuxPackets )
	{
		int len = aggregatedLength( nMuxPackets ) ;

		ip_verlen	= 0x45 ;
		ip_tos		= 0 ;	
		ip_len[0]	= len>>8 ;
		ip_len[1]	= (UCHAR)len;
		ip_id		= 0 ;	
		ip_fragoff	= 0;
		ip_ttl		= 0x80;	
		ip_proto	= dvbProtocolSetup.mpeHnetIpProtocol() ;
		ip_checksum	= 0 ;
		ip_src		= 0x0100007F;
		ip_dst		= 0xFFFFFFFF;

		RecalculateIPHeaderChecksum((IP_HEADER_TYPE *)this) ;
	}
	
	inline MuxPacket *prepareMux( int i, ushort )	{ return muxPacket + i ; }
	inline void finalize()	{ }
	inline static int aggregatedLength( int nPackets )	{ return 20+nPackets*MUXPACKETSIZE ; }
} ;

#pragma optimize( "", on )			// restore original optimization options



//-------------------------------------------------------------------------------
// ServiceHdr
//-------------------------------------------------------------------------------


// service header holds information for data sent on service channel
struct ServiceHdr
{
	uchar	version ;
	ushort	flags ;			// same as packet flags
	ulong	reserved ;
	ulong	length ;		// length of data
	ulong	crc ;
	const char *name()
	{
		if( flags & MuxPacket::Message )
			return "" ;
		if( flags & MuxPacket::Upgrade )
			return "Upgrade" ;
		if( flags & MuxPacket::UserLog )
			return "UserLog" ;
		//if( flags & MuxPacket::UserTable )
		//	return "UserTable" ;
		return "Service" ;
	}
	int operator== ( const ServiceHdr &src )  { return memcmp(this,&src,sizeof(ServiceHdr)) == 0 ; }
	int operator!= ( const ServiceHdr &src )  { return !(*this == src) ; }

	ServiceHdr() { }
	ServiceHdr( ushort _flags, ulong _length )
	{
		version = 1 ;
		flags	= _flags ;
		reserved= 0 ;
		length	= _length ;
	}
} ;


//------------------------------------------------------------------------------
//	packet pool
//------------------------------------------------------------------------------


static long	_total_numfreepack	 =0;	// total # available packets
static long _IP_min_numfreepack	 =0;	// # packets guaranteed for internet if not working
static long _IP_distr_numfreepack=0;	// # packets guaranteed for internet if working
extern BOOL isInternetWorking ;

// set the # of free packets
void setNumfreepack( long total, long IP, long dIP )
{
	_total_numfreepack		= total ;
	_IP_min_numfreepack		= IP;
	_IP_distr_numfreepack	= dIP;
}


#pragma optimize( OPTOPTIONS, on )

// take one packet from packet pool
inline BOOL takePacketFromPacketPool( BOOL takeForInternet, int numPackets=1 )
{
	if( _total_numfreepack < numPackets )
		return FALSE ;

	if ( takeForInternet )
	{
		if( _total_numfreepack >= numPackets )			// negative value could happen if 2nd thread decrements the value
		{
			InterlockedExchangeAdd( &_total_numfreepack, -numPackets ) ;
			_IP_distr_numfreepack-= numPackets;
			_IP_min_numfreepack = __min( _IP_min_numfreepack, _IP_distr_numfreepack );
		}
		return _total_numfreepack >= 0 ;
	}

	// at this place numPackets is always = 1
	if ( isInternetWorking )
	{
		if ( _total_numfreepack <= _IP_distr_numfreepack )
			return FALSE;
	}
	else
	{
		if (_total_numfreepack <= _IP_min_numfreepack)
			return FALSE;
		else
		if (_total_numfreepack <= _IP_distr_numfreepack)
			_IP_distr_numfreepack-- ;
	}

	if( _total_numfreepack > 0 )			// negative value could happen if 2nd thread decrements the value
		InterlockedDecrement( &_total_numfreepack ) ;
	return _total_numfreepack >= 0 ;
}

#pragma optimize( "", on )			// restore original optimization options


//------------------------------------------------------------------------------
//	BaseSender - base class for sender classes
//------------------------------------------------------------------------------


BaseSender::BaseSender( MuxOutput *o, HANDLE _hKillEvent, HANDLE hNumFreePacketAvailable,
					    long *n_freepack )
{
	_jobId			= 0 ;
	muxOutput		= o ;
	channel			= -1 ;
	rebroadcastIndex= -1 ;
	hKillEvent		= _hKillEvent ;
	_hNumFreePacketAvailable = hNumFreePacketAvailable ;
	handleArray[0]	= hKillEvent;
	handleArray[1]	= hNumFreePacketAvailable;

	_numfreepack	= n_freepack ;
	usrId.makeInvalid() ;
	speed			= 0 ;
	packetInd		= 0;
	numPacketsSent	= 0 ;
}


#pragma optimize( OPTOPTIONS, on )

// main data sending function
// - guaranties the # of distributed packets
// - makes packet from given data with given flag and send it to the output
// - updates the speed of channel

int BaseSender::sendPacket( MuxPacket *mp, ushort flg, char *data, int len, PidStreamAttrib *pidStrAttr )
{
	if( streamFormat == CfgChannel::TSPIPE_PROTOCOL )
	{
		// _numfreepack can only be changed on this place or by distributeRecordRequests()
		EnterCriticalSection( &MuxChannel::_distrLock );
		if( *_numfreepack <= 0 )
		{	// no free packet wait for it
			ResetEvent( _hNumFreePacketAvailable ) ;
			LeaveCriticalSection( &MuxChannel::_distrLock );
			zeroSpeed() ;
			if( WaitForMultipleObjects( 2, handleArray, FALSE, INFINITE) == WAIT_OBJECT_0 )
				return FALSE ;			// kill
			EnterCriticalSection( &MuxChannel::_distrLock );
		}
		BOOL isHNet = isInternetChannel() ;
		while ( !takePacketFromPacketPool( isHNet) )
		{	// not available packet wait for it
			LeaveCriticalSection( &MuxChannel::_distrLock );
			zeroSpeed() ;
			Sleep( isHNet ? DELAY_WAITING_FOR_FREE_IP_PACKETS : DELAY_WAITING_FOR_FREE_PACKETS );
			EnterCriticalSection( &MuxChannel::_distrLock );
		}

		VERIFY( InterlockedDecrement( _numfreepack ) >= 0 );

		if( data != NULL )
		{
			if( !usrId.isValid() )
				mp->makeDataPacket( channel, flg, len, rebroadcastIndex, packetInd, data, _jobId ) ;
			else
				mp->makeUnicastPacket( channel, flg, len, rebroadcastIndex, packetInd, data, usrId, _jobId ) ;
		}
		muxOutput->put( mp, pidStrAttr, isHNet ) ;

		LeaveCriticalSection( &MuxChannel::_distrLock );
	}
	else
	{
		DSMCC_section section ;
		if( streamFormat == CfgChannel::MPE_HNET_PROTOCOL )
		{
			IpMuxPacket *ipMuxPackage = (IpMuxPacket *)section.data() ;
			ipMuxPackage->init( 1 ) ;
			MuxPacket *muxPacket = ipMuxPackage->prepareMux(0, pidStrAttr->getConvertedPid() ) ;

			if( data != NULL )
			{
				if( !usrId.isValid() )
					muxPacket->makeDataPacket( channel, flg, len, rebroadcastIndex, packetInd, data, _jobId ) ;
				else
					muxPacket->makeUnicastPacket( channel, flg, len, rebroadcastIndex, packetInd, data, usrId, _jobId ) ;
				*mp = *muxPacket ;
			}
			else
			{
				if( mp != NULL )
					*muxPacket = *mp ;
			}

			ipMuxPackage->finalize() ;
			section.setHeader( ipMuxPackage->aggregatedLength(1), broadcast_adr) ;
		}
		else	// MPE_UDP_PROTOCOL
		{
			IpUdpTsPacket *ipMuxPackage = (IpUdpTsPacket *)section.data() ;
			ipMuxPackage->init( 1 ) ;
			MuxPacket *muxPacket = ipMuxPackage->prepareMux(0, pidStrAttr->getConvertedPid() ) ;

			if( data != NULL )
			{
				if( !usrId.isValid() )
					muxPacket->makeDataPacket( channel, flg, len, rebroadcastIndex, packetInd, data, _jobId ) ;
				else
					muxPacket->makeUnicastPacket( channel, flg, len, rebroadcastIndex, packetInd, data, usrId, _jobId ) ;
				if( mp != NULL )
					*mp = *muxPacket ;
			}
			else
			{
				*muxPacket = *mp ;
			}

			ipMuxPackage->finalize() ;
			section.setHeader( ipMuxPackage->aggregatedLength(1), broadcast_adr) ;
		}
		sendMPESection( &section, pidStrAttr ) ;
	}

	if( data != NULL )
	{
		if( ++packetInd > 0xFFFFFF )
			packetInd = 0 ;
	}
	return 0 ;
}

int BaseSender::sendAgregatedPackets(  ushort flg, char *data, int nPackets, int packetSize, int &sentPackets, PidStreamAttrib *pidStrAttr )
{
	MuxPacket muxPacket ;
	if( streamFormat == CfgChannel::TSPIPE_PROTOCOL )
	{
		sentPackets = nPackets ;
		while (nPackets--)
		{
			sendPacket( &muxPacket, flg, data, packetSize, pidStrAttr) ;
			data += packetSize ;
		}
	}
	else
	{
		DSMCC_section section ;
		BOOL bUnicast = usrId.isValid() ;

		if( streamFormat == CfgChannel::MPE_HNET_PROTOCOL )
		{
			sentPackets = __min(nPackets, MAX_IP_MUXPACKETS) ;
			IpMuxPacket *ipMuxPackage = (IpMuxPacket *)section.data() ;
			ipMuxPackage->init( sentPackets ) ;
			ushort convPid = pidStrAttr->getConvertedPid() ;

			for ( int i=0; i < sentPackets; ++i )  
			{
				MuxPacket *muxPacket = ipMuxPackage->prepareMux(i, convPid ) ;
				if( bUnicast )
					muxPacket->makeUnicastPacket( channel, flg, packetSize, rebroadcastIndex, packetInd, data, usrId, _jobId ) ;
				else
					muxPacket->makeDataPacket( channel, flg, packetSize, rebroadcastIndex, packetInd, data, _jobId ) ;

				data += packetSize ;
				if( ++packetInd > 0xFFFFFF )
					packetInd = 0 ;
			}

			ipMuxPackage->finalize() ;
			section.setHeader( ipMuxPackage->aggregatedLength(sentPackets), broadcast_adr) ;
		}
		else
		{
			sentPackets = __min(nPackets, MAX_IPUDP_TSPACKETS) ;
			IpUdpTsPacket *ipMuxPackage = (IpUdpTsPacket *)section.data() ;
			ipMuxPackage->init(sentPackets) ;
			ushort convPid = pidStrAttr->getConvertedPid() ;

			for ( int i=0; i < sentPackets; ++i )  
			{
				MuxPacket *muxPacket = ipMuxPackage->prepareMux(i, convPid ) ;
				if( bUnicast )
					muxPacket->makeUnicastPacket( channel, flg, packetSize, rebroadcastIndex, packetInd, data, usrId, _jobId ) ;
				else
					muxPacket->makeDataPacket( channel, flg, packetSize, rebroadcastIndex, packetInd, data, _jobId ) ;

				data += packetSize ;
				if( ++packetInd > 0xFFFFFF )
					packetInd = 0 ;
			}

			ipMuxPackage->finalize() ;
			section.setHeader( ipMuxPackage->aggregatedLength(sentPackets), broadcast_adr) ;
		}
		sendMPESection( &section, pidStrAttr ) ;
	}

	return 0 ;
}

int BaseSender::sendServicePacket( ushort flg, uchar streamFmt, char *data, int len, PidStreamAttrib *pidStrAttr, const GlobalUserID& _usrId )
{
	if( streamFmt == CfgChannel::TSPIPE_PROTOCOL )
	{
		MuxPacket muxPacket ;
		if( !_usrId.isValid() )
			muxPacket.makeDataPacket( 0, flg, len, 0, 0, data, 0 ) ;
		else
			muxPacket.makeUnicastPacket( 0, flg, len, 0, 0, data, _usrId, 0) ;

		EnterCriticalSection( &MuxChannel::_distrLock );
		muxOutput->put( &muxPacket, pidStrAttr, FALSE ) ;
		LeaveCriticalSection( &MuxChannel::_distrLock );
	}
	else
	{
		DSMCC_section section ;

		if( streamFmt == CfgChannel::MPE_HNET_PROTOCOL )
		{
			IpMuxPacket *ipMuxPackage = (IpMuxPacket *)section.data() ;
			ipMuxPackage->init( 1 ) ;
			MuxPacket *muxPacket = ipMuxPackage->prepareMux(0, pidStrAttr->getConvertedPid() ) ;

			if( !_usrId.isValid() )
				muxPacket->makeDataPacket( 0, flg, len, 0, 0, data, 0 ) ;
			else
				muxPacket->makeUnicastPacket( 0, flg, len, 0, 0, data, _usrId, 0) ;

			ipMuxPackage->finalize() ;
			section.setHeader( ipMuxPackage->aggregatedLength(1), broadcast_adr) ;
		}
		else	// MPE_UDP_PROTOCOL
		{
			IpUdpTsPacket *ipMuxPackage = (IpUdpTsPacket *)section.data();
			ipMuxPackage->init( 1 ) ;
			MuxPacket *muxPacket = ipMuxPackage->prepareMux(0, pidStrAttr->getConvertedPid() ) ;

			if( !_usrId.isValid() )
				muxPacket->makeDataPacket( 0, flg, len, 0, 0, data, 0 ) ;
			else
				muxPacket->makeUnicastPacket( 0, flg, len, 0, 0, data, _usrId, 0) ;

			ipMuxPackage->finalize() ;
			section.setHeader( ipMuxPackage->aggregatedLength(1), broadcast_adr) ;
		}

		// we must add next section pointer byte before MPE section start (zero byte for one section per TS packet)
		int n_tsPackets = (section.getTotalLength()+1 + TSPAYLOAD_SIZE-1) / TSPAYLOAD_SIZE ;
		EnterCriticalSection( &MuxChannel::_distrLock );
		muxOutput->put( &section, n_tsPackets, pidStrAttr, FALSE ) ;
		LeaveCriticalSection( &MuxChannel::_distrLock );
	}
	return 0 ;
}


int BaseSender::sendMPESection( DSMCC_section *section, PidStreamAttrib *pidStrAttr )
{
	// compute the number of TS packets needed to carry the MPE section
	// we must add next section pointer byte before MPE section start (zero byte for one section per TS packet)
	int n_tsPackets = (section->getTotalLength()+1 + TSPAYLOAD_SIZE-1) / TSPAYLOAD_SIZE ;

	// _numfreepack can only be changed on this place or by distributeRecordRequests()
	EnterCriticalSection( &MuxChannel::_distrLock );
	if( *_numfreepack <= n_tsPackets )
	{	// no free packet wait for it
		ResetEvent( _hNumFreePacketAvailable ) ;
		LeaveCriticalSection( &MuxChannel::_distrLock );
		zeroSpeed() ;
		if( WaitForMultipleObjects( 2, handleArray, FALSE, INFINITE) == WAIT_OBJECT_0 )
			return FALSE ;			// kill
		EnterCriticalSection( &MuxChannel::_distrLock );
	}
	BOOL isHNet = isInternetChannel() ;
	while ( !takePacketFromPacketPool( isHNet, n_tsPackets) )
	{	// not available packet wait for it
		LeaveCriticalSection( &MuxChannel::_distrLock );
		zeroSpeed() ;
		Sleep( isHNet ? DELAY_WAITING_FOR_FREE_IP_PACKETS : DELAY_WAITING_FOR_FREE_PACKETS );
		EnterCriticalSection( &MuxChannel::_distrLock );
	}

	VERIFY( InterlockedExchangeAdd( _numfreepack, -n_tsPackets ) >= 0 );

	muxOutput->put( section, n_tsPackets, pidStrAttr, isHNet ) ;

	LeaveCriticalSection( &MuxChannel::_distrLock );
	// Not needed here - moreover, if called from sendPacket(),
	// then packetInd must not change.
	//packetInd = (packetInd+n_tsPackets) & 0xFFFFFF ;
	numPacketsSent += n_tsPackets;

	return 0 ;
}

#pragma optimize( "", on )			// restore original optimization options


//------------------------------------------------------------------------------
//	BaseReceiver - base class for receiver classes
//------------------------------------------------------------------------------


uchar BaseReceiver::bits[8]= { 1, 2, 4, 8, 16, 32, 64, 128 };
static uchar bitMasks[8]   = { 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff} ;

// TRUE iff all bits from 1. to the lastPacketNumber-th ( including ) are set
BOOL BaseReceiver::isReady( int lastPacketNumber )
{
	int  lastByteNumber= lastPacketNumber / 8;
	int  numLastBits   = lastPacketNumber % 8;
	if ( lastByteNumber > 0 )
	{
		if( _packetMap[0] != 0xfe )
			return FALSE ;
		for( int i=1; i < lastByteNumber; i++ )
			if( _packetMap[i] != 0xff )
				return FALSE ;
		if( numLastBits > 0 )
			if( _packetMap[lastByteNumber] != bitMasks[numLastBits] )
				return FALSE ;
	}
	else
	{
		if( numLastBits > 0 )
			if( _packetMap[lastByteNumber] != bitMasks[numLastBits] - 0x01 )
				return FALSE ;
	}
	return TRUE;
}

// reset the size of the receiving map to be able to hold informations on nPackets packets
// clear the map contents
void BaseReceiver::resetMap( int nPackets )
{
	int newSize = (nPackets+7)/8 ;
	if( newSize > _mapSize )
	{
		_mapSize = newSize + 128 ;
		_packetMap = (uchar*)realloc( _packetMap, _mapSize ) ;
	}
	_maxPackets = nPackets ;
	clearMap() ;
}

// resize the receiving map to be able to hold informations on nPackets packets
// map contents are preserved
void BaseReceiver::resizeMap( int nPackets )
{
	if( nPackets == _maxPackets )
		return ;

	if( nPackets > _maxPackets )
	{
		int newSize = (nPackets+7)/8 ;
		if( newSize > _mapSize )
		{
			int oldMapSize = _mapSize ;
			_mapSize = newSize + 128 ;
			_packetMap = (uchar*)realloc( _packetMap, _mapSize ) ;
			memset( _packetMap+oldMapSize, 0, _mapSize-oldMapSize ) ;
		}
	}
	_maxPackets = nPackets ;

	int   lastIndex= (_maxPackets+7)/8 ;
	_packetMap[lastIndex] &= bits[_maxPackets%8] ;
}

// write the map to a file - needed to be able to continue receiving of current file
void BaseReceiver::writeBitmap( FILE *bmpFp )
{
	if( bmpFp != NULL )
	{
		fwrite( &_mapSize   , sizeof(_mapSize)   , 1      , bmpFp);
		fwrite( &_maxPackets, sizeof(_maxPackets), 1      , bmpFp);
		fwrite( &_numPackets, sizeof(_numPackets), 1      , bmpFp);
		fwrite( _packetMap  , sizeof(uchar)      ,_mapSize, bmpFp );
	}
}

// read the previusly saved map from the file
BOOL BaseReceiver::readBitmap( FILE *bmpFp )
{	// TRUE iff bitmap was correctly readed
	if( bmpFp != NULL )
	{
		fread( &_mapSize   , sizeof(_mapSize)   , 1       , bmpFp);
		if( _mapSize <= NULL )
			return FALSE;
		_packetMap  = (uchar*)realloc( _packetMap, _mapSize ) ;
		clearMap();
		fread( &_maxPackets, sizeof(_maxPackets), 1      , bmpFp);
		fread( &_numPackets, sizeof(_numPackets), 1      , bmpFp);
		fread( _packetMap  , sizeof(uchar)      , _mapSize, bmpFp );

		// Compute _highestPacket

		// find last non-zero Byte
		for( int by=(_maxPackets+7)/8 ; by >= 0 ; by-- )
			if( _packetMap[by] != 0 )
				break ;
		if( by >= 0 )
		{
			// find last non-zero bit
			uchar mapByte = _packetMap[by] ;
			for( int bit=7 ; bit >= 0 ; bit-- )
			{
				if( mapByte & bits[bit] )
				{
					_highestPacket = 8*by + bit ;
					break ;
				}
			}
		}
	}
	return TRUE ;
}

int BaseReceiver::maxUnknownInterval()
{
	int  len=0 ;
	BOOL inside = FALSE ;
	int  start ;
	int  maxByte = (_maxPackets+7)/8 ;
	for( int by=0 ; by < maxByte ; ++by )
	{
		if( _packetMap[by] != 0xFF )
		{
			if( !inside )
			{
				inside = TRUE ;
				start  = by ;
			}
		}
		else
		if( inside )
		{
			inside = FALSE ;
			len = __max( len, by-start) ;
		}
	}
	if( inside )
		len = __max( len, by-start) ;
	return len*8 ;
}

BOOL BaseReceiver::isIntervalReady( int i1, int i2 )
{
	i2 = __min( i2, _maxPackets-1 ) ;

	int ind_by_i1 = i1/8 ;
	int ind_by_i2 = i2/8 ;
	int ind_bi_i1 = i1%8 ;
	int ind_bi_i2 = i2%8 ;

	if( ind_by_i1 == ind_by_i2 )
	{
		uchar by_i1 = _packetMap[ind_by_i1] ;
		while( ind_bi_i1 <= ind_bi_i2 )
		{
			if( (by_i1 & bits[ind_bi_i1]) == 0 )
				return FALSE ;
			ind_bi_i1++ ;
		}
		return TRUE ;
	}

	if( ind_bi_i1 != 0 )
	{
		static left_bytes[] = { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80 } ;
		uchar by_i1 = _packetMap[ind_by_i1] ;
		uchar by = left_bytes[ind_bi_i1] ;
		if( (by_i1 & by) != by )
			return FALSE ;
		ind_by_i1++ ;
		//ind_bi_i1 = 0 ;
		//i1 = 8*ind_by_i1 ;
	}

	if( ind_bi_i2 != 7 )
	{
		static right_bytes[] = { 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF } ;
		uchar by_i2 = _packetMap[ind_by_i2] ;
		uchar by = right_bytes[ind_bi_i2] ;
		if( (by_i2 & by) != by )
			return FALSE ;
		ind_by_i2-- ;
		//ind_bi_i2 = 7 ;
		//i2 = 8*ind_by_i2+7 ;
	}

	while( ind_by_i1 <= ind_by_i2 )
	{
		if( _packetMap[ind_by_i1] != 0xFF )
			return FALSE ;
		ind_by_i1++ ;
	}

	return TRUE ;
}


//------------------------------------------------------------------------------
//	DataSender - sender of different types of data stored in the memory
// sends data as service - with ServiceHdr
// given data are rebroadcasted on one run - rebroadcast by rebr.
// this do not allows mixing packets for this data with packets of other data
//------------------------------------------------------------------------------


// protocol:
//		<hdr> <data>

DataSender::DataSender( MuxOutput *o, HANDLE _hKillEvent, HANDLE hNumFreePacketAvailable,
					   long *n_freepack ) :
		BaseSender( o, _hKillEvent, hNumFreePacketAvailable, n_freepack )
{
	_status = Inactive ;
}

// send n_bytes bytes from data on channel <_channel>, with flags and _usrId set
// in all packets
// - data is sent as service
// - appends a service header to the start of data
// - provides dividing and packetizing of data and sending of this packets to the output
int DataSender::sendData( ushort flags, const char *data, int n_bytes,
			ushort _channel, uchar streamFmt, int numRebroadcasts, 
			PidStreamAttrib *pidStrAttr, const GlobalUserID& _usrId)
{
	int err = 0 ;
	ASSERT( isStarted() ) ;
	channel		= _channel ;
	streamFormat= streamFmt ;
	usrId		= _usrId ;
	_status		=  Sending ;

	nextJob() ;
	int packetSize = MUXDATASIZE - ((flags & MuxPacket::Unicast) ? sizeof(UnicastUserID) : 0);

	ServiceHdr	hdr(flags|MuxPacket::CrcComputed,n_bytes) ;
	char		rec1[256] ;
	int			rec1_len = packetSize-sizeof(ServiceHdr) ;
				rec1_len = __min( rec1_len, n_bytes ) ;

	hdr.crc = crc32( (const uchar*)data, n_bytes ) ;
	memcpy( rec1, &hdr, sizeof(ServiceHdr) ) ;
	memcpy( rec1+sizeof(ServiceHdr), data, rec1_len ) ;

	MuxPacket muxPacket ;
	for( rebroadcastIndex=numRebroadcasts ; rebroadcastIndex > 0 ; rebroadcastIndex-- )
	{
		packetInd = 0;
		err = sendPacket( &muxPacket, flags, rec1, rec1_len+sizeof(ServiceHdr), pidStrAttr ) ;
		for( int pos=rec1_len ; pos < n_bytes && err == 0; pos += packetSize )
		{
			int len = __min( packetSize, n_bytes-pos ) ;
			err = sendPacket( &muxPacket, flags, (char*)(data+pos), len, pidStrAttr ) ;
		}
	}
	_status = Ready ;
	if( err != 0 )
	{
		char buf[512];
		const char *msg = DvbEventText( err, buf );
		MfxPostMessage( EMsg_CommunicationError, 0, msg );
	}
	return err ;
}

#pragma optimize( OPTOPTIONS, on )

/*
// send internet data - same as sendData, only packetInd is not changed betveen rebroadcasts
int DataSender::sendInternetData( ushort flags, const char *data, int n_bytes,
			ushort _channel, int numRebroadcasts, PidStreamAttrib *pidStrAttr, const GlobalUserID& _usrId )
{
	int err = 0 ;
	ASSERT( isStarted() ) ;
	channel = _channel ;
	usrId   = _usrId ;
	_status = Sending ;
	nextJob() ;
	int packetSize = MUXDATASIZE - ((flags & MuxPacket::Unicast) ? sizeof(UnicastUserID) : 0);

	ServiceHdr	hdr(flags|MuxPacket::CrcComputed,n_bytes) ;
	char		rec1[256] ;
	int			rec1_len = packetSize-sizeof(ServiceHdr) ;
				rec1_len = __min( rec1_len, n_bytes ) ;

	hdr.crc = crc32( (const uchar*)data, n_bytes ) ;
	memcpy( rec1, &hdr, sizeof(ServiceHdr) ) ;
	memcpy( rec1+sizeof(ServiceHdr), data, rec1_len ) ;

	for( rebroadcastIndex=numRebroadcasts ; rebroadcastIndex > 0 ; rebroadcastIndex-- )
	{
		// Single difference to sendData().
		// Asi je to blbost, lebo Receiver ignoruje pakety pre 2. a dalsie reboadcasty.
		//packetInd = 0;
		err = sendPacket( flags, rec1, rec1_len+sizeof(ServiceHdr), pidStrAttr ) ;
		for( int pos=rec1_len ; pos < n_bytes && err == 0; pos += packetSize )
		{
			int len = __min( packetSize, n_bytes-pos ) ;
			err = sendPacket( flags, (char*)(data+pos), len, pidStrAttr ) ;
		}
	}
	_status = Ready ;
	if( err != 0 )
	{
		char buf[512];
		const char *msg = DvbEventText( err, buf );
		MfxPostMessage( EMsg_CommunicationError, 0, msg );
	}
	return err ;
}
*/

int DataSender::sendHNetDataAsMPE( const char *data, int n_bytes, int numRebroadcasts, PidStreamAttrib *pidStrAttr )
{
	int err = 0 ;

	ASSERT( isStarted() ) ;
//	channel = _channel ;
//	usrId   = _usrId ;
	_status = Sending ;
//	nextJob() ;

	DSMCC_section section ;
	// get the number of App packets in the buffer
	int n_remAppPackets = *(int*)data ;
	const char *ptrData = data + 4 ;

	while (n_remAppPackets--)
	{
		ASSERT((int)(ptrData-data) < n_bytes) ;
		// unpack IP datagram
		APP_PACKET_TYPE *appPacket = (APP_PACKET_TYPE*)ptrData ;
		ETH_HEADER_TYPE *ethPacket = (ETH_HEADER_TYPE*)(appPacket->Data) ;
		IP_HEADER_TYPE	*ipPacket  = (IP_HEADER_TYPE*)(ethPacket->Data) ;
		
		ASSERT(ipPacket->ip_verlen=='\x45' && appPacket->PacketLen<0x10000) ;
		ptrData = ((char*)ethPacket) + appPacket->PacketLen ;

		ushort			 ipDataSize= ipPacket->ip_len ;
		ipDataSize = ipDataSize>>8 | ipDataSize<<8 ;

		if (ipDataSize <= MAX_IP_DATA_SIZE)
		{
			// IP datagram is fits into one DSMCC section
			// Finalize MPE section
#pragma message("CALCULATE_MAC_FROM_IP")
			#ifdef CALCULATE_MAC_FROM_IP
				__int64 mac = 0x200 ;	// calculated MAC for unicast
				memcpy( ((uchar*)&mac)+2, &ipPacket->ip_dst, 4 ) ;
				section.setHeader( ipDataSize, mac) ;
			#else
				section.setHeader( ipDataSize, appPacket->HN_MAC) ;
			#endif
			memcpy( section.data(), ipPacket, ipDataSize ) ;

			for (int i = numRebroadcasts; i--;)
			{
				// Copy to transport stream
				err = sendMPESection(&section, pidStrAttr) ;
			}
		}
		else
		{
			// IP datagram is too big to fit into one DSMCC section
			// We must divide it into more sections
			// Following code is not completed, but probably we shall not need
			// this branch as HNet uses Ethernet packets (approx. 1500 By).
			TRACE("\nDiscarded IP datagram - size (%d kB) is too big to fit DSMCC section\n", ipDataSize>>10) ;

/*			uchar newIpPacket[MAX_IP_DATA_SIZE] ;

			ushort ipHeaderSize = (ipPacket->ip_verlen&0x0f)<<2 ;
			memcpy(newIpPacket, ipPacket, ipHeaderSize) ;

			ushort maxIpDataSize = (MAX_IP_DATA_SIZE-ipHeaderSize)&0xFFFC ;	// must be multiple of 4

			uchar *newIpData = newIpPacket+ipHeaderSize ;

			int lastSection = (ipDataSize-ipHeaderSize) / (maxIpDataSize) ;
			if ( ipDataSize-ipHeaderSize== lastSection*maxIpDataSize)
				--lastSection ;

			for (int i = numRebroadcasts; i--;)
			{
				int sectionNumber = 0 ;
				uchar *ipData = ((uchar*)ipPacket)+ipHeaderSize ;
				ushort dataSize = ipDataSize-ipHeaderSize ;
				newIpPacket->

				while ( sectionNumber <= lastSection )
				{
					// set more fragments flag (except the last fragment) 
					// and set the fragment offset in the IP header
					// ...
					//RecalculateIPHeaderChecksum(newIpPacket) ;

					ushort size = (dataSize<=maxIpDataSize?dataSize:maxIpDataSize) ;
					memcpy(newIpData, ipData, size ) ;

					// finalize MPE section
					section.setData(
							newIpPacket, 
							size+ipHeaderSize, 
							appPacket->HN_MAC, 
							sectionNumber, 
							lastSection) ;
					// copy to transport stream
					err = sendMPESection(&section, pidStrAttr) ;

					dataSize -= size ;
					ipData	 += size ;
					++sectionNumber ;
				}
			}
*/		}
	}

	_status = Ready ;
	if( err != 0 )
	{
		char buf[512];
		const char *msg = DvbEventText( err, buf );
		MfxPostMessage( EMsg_CommunicationError, 0, msg );
	}
	return err ;
}

#pragma optimize( "", on )			// restore original optimization options


//------------------------------------------------------------------------------
//	DataReceiver - receiver for different data types, data are received to the memory
// if a job is comming the full job must come before next job starts
// if next job is started before prev. job is finished, this means that prev. job is incomplette
//------------------------------------------------------------------------------

#define MAX_PACKETS	(MAXSERVICESIZE/(MUXDATASIZE - sizeof(long)) + 1)

// start receiving   
void DataReceiver::start( )
{
	if( _isPaused )
	{
		_isPaused = FALSE;
		return;
	}
	ASSERT( !isStarted() );

	_status	= Ready ;
	_jobId	= 0 ;
	// allocate max. possible space
	_data	= (uchar*)calloc( (MAX_PACKETS+1)*MUXDATASIZE, 1 ) ;

	expPacketIndex = 0;
	lostPackets = 0;
	firstPacket = TRUE;
}

// stop receiving
void DataReceiver::stop( )
{
	ASSERT( isStarted() );
	free( _data ) ;
	_data	= NULL ;
	_status	= Inactive ;
	_jobId	= 0 ;
}

// push one packet's data to the job's data
// test the continuity of data and the job index
// if new job index comes start receiving this new job and clear the prev. job
void DataReceiver::push( MuxPacket *packet )
{
	if ( !isStarted() )
		return;

	if( packet->packetIndex() > (int)(MAX_PACKETS) )			// may by corrupted packet
		return;

	if( packet->jobId() != _jobId )			// another job
	{
		if( _jobId != 0 )					// clear previous job
		{
			clearMap() ;
		}
		_jobId      = packet->jobId() ;
	}

	int packetInd = packet->packetIndex() ;
	int packetSize= MUXDATASIZE - (packet->isUnicastPacket() ? sizeof(UnicastUserID) : 0);
	if( packetInd == 0 )					// header packet
	{
		if( !hasHeader() )
		{
			ServiceHdr *hdr = (ServiceHdr *)packet->data() ;
			resizeMap( (hdr->length +packetSize-1 + sizeof(ServiceHdr)) / packetSize ) ;
			if( hdr->flags & MuxPacket::Message )
				MfxClientSetup()->incNumMessagesTransferred();
		}
	}
	else					// allow accepting packets without header (i.e. unknown size)
	if( maxPackets() <= packetInd )
		resizeMap( packetInd+10 ) ;		// we set map size to the forbidden value

	// packet control
	if( firstPacket )
	{
		firstPacket = FALSE;
		expPacketIndex = (ulong)packetInd;
	}
	if( expPacketIndex != (ulong)packetInd )
	{
		if( expPacketIndex > (ulong)packetInd )
			lostPackets += ( 0xffffff - expPacketIndex ) + packetInd;
		else
			lostPackets += packetInd - expPacketIndex;
		expPacketIndex = packetInd;
	}
	expPacketIndex++;
	if( expPacketIndex > 0xffffff )
		expPacketIndex = 0;

	if( !isReady()  &&  packetInd < maxPackets()  &&  !isMapSet(packetInd) )
	{
		_status = Receiving ;
		memcpy( _data+packetInd * packetSize, packet->data(), packet->numDataBytes()) ;

		setMap( packetInd );

		if( isReady() )
		{
			_status = Ready;
			uchar *data   = _data+sizeof(ServiceHdr) ;
			int    n_bytes= ((ServiceHdr*)_data)->length ;

			if( (((ServiceHdr*)_data)->flags & MuxPacket::CrcComputed) == 0  ||
				 ((ServiceHdr*)_data)->crc == crc32( data, n_bytes) )
				processData( ((ServiceHdr*)_data)->flags, data, n_bytes, packet->channel() );
			else
				clearMap() ;		// corrupted data; try again
		}
	}
}
