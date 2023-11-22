#ifndef __DvbUser_hpp__
#define __DvbUser_hpp__


#include "serialNum.hpp"

//
// There are three services concerned with sending user CA data:
//
//	1. Alive packets are used to send user list (part by part).
//	   For this purpose UserIDs structure is used.
//
//	2. UserLog packet is unicast packet sent to particular user
//	   containing sUserCA structure. (Possibly more MuxPackets are used
//	   to transfer sUserCA object.)
//
//	3. UserTable is constructed as series of UserLog packets followed
//	   by a list of channel names (sChannelsTable structure).
//


//--------------------------------------------------------------
//	UserIDs
//	Structure used in sending alive signal
//--------------------------------------------------------------

struct UserIDs
{
	ushort			numOfID;
	GlobalUserID	fromUser;
	GlobalUserID	toUser;
	GlobalUserID	ids[1];
};



//--------------------------------------------------------------
//	sUserCA
//	Data of the UserLog packet
//--------------------------------------------------------------


#define MAXNUMOFCHANNEL		70
#define USERCASIGNATURE		"MD UserCA"

struct sUserCA
{
	enum {
		NONE			= 0x0000,
		DO_FILTERING	= 0x0001,
		HNET_ALLOWED	= 0x0002,
		ALL				= 0x0003
	};
	ushort	flags;			// do filtering, HNET allowed
	ushort	numChannels;	// number of channels is 0..MAXNUMOFCHANNEL
	ushort	fromChannel;
	ushort	toChannel;
	ushort	channels[1];	// channels[numChannels], all from the interval fromChannel-toChannel
};


//--------------------------------------------------------------
//	sChannelsTable
//	Sent as part of UserTable packet:
//		1. UserLog packets for all users
//		2. sChannelsTable
//--------------------------------------------------------------


struct sUserChannel
{
	enum { TEXT_SIZE=32,} ;
	ushort	channelID;
	char	channelName[TEXT_SIZE + 1];
};


struct	sChannelsTable
{
	ushort			numChannels;
	sUserChannel	channels[1];
};


//--------------------------------------------------------------
//	DvbUserLog
//	Stores permissions for current user. (Receiver structure)
//--------------------------------------------------------------


class DvbUserLog
{
  private:
	CRITICAL_SECTION	_userCALock;

	GlobalUserID			_userID;
	ushort					_flags;			// do filtering, HNET allowed
	sTemplateArray<ushort>	_channels;		// array of channels
	sTemplateArray<char *>	_channelNames;
	char					*_fileName;
	BOOL					_changed;


	int			find( ushort channel );		// return -1 if not fount, else return position
  public:
	// if save fails, throw Msg
	// FileErr_OpenError | FileErr_WriteError | FileErr_UnknownError
	void		save();
	// if load fails, throw Msg
	// FileErr_OpenError | FileErr_ReadError | DvbErr_BadUserRights | FileErr_UnknownError
	void		load( const GlobalUserID &userId );

	DvbUserLog	( const char *fileName );
	~DvbUserLog	();

	inline const GlobalUserID &userID() const				{ return _userID; };

	ushort		nChannels			();
	ushort		channels			( ushort i );

	const char	*getChannelNameByID	( ushort channel );
	void		setChannelNameByID	( ushort channel, char *channelName );
	const char	*getChannelName		( ushort index );
	void		setChannelName		( ushort index, char *channelName );

	inline BOOL isHWFilteringAllowed()		{ return _flags & sUserCA::DO_FILTERING; }
	inline BOOL	isHNetAllowed		()		{ return _flags & sUserCA::HNET_ALLOWED; }

	BOOL		hasChannel			( ushort channel );
	void		removeAllChannels	()						{ reset(); };

	BOOL		reset				();
	//	add or remove channels with respect on userCA. in updatedChannels array is stored all added and deleted channels
	//  return TRUE if some channels changed
	BOOL		updateCA			( sUserCA *userCA, sTemplateArray<ushort> *updatedChannels );
	// check if data in userCA is corect
	BOOL		isUserCAOk			( sUserCA *userCA );

	void		fillBitmapForCard( void *bitmap );
	//  return TRUE if some channels changed
	BOOL		synchChannelsWithCard( void *bitmap, sTemplateArray<ushort> *updatedChannels );
};




#endif