/*
 *	Filename:		dvbuser.cpp
 *
 *	Version:		1.00
 *
 *	Description: implemention of DvbUserLog class
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"
#include "ProtectedFile.hpp"
#include "ClientCfg.hpp"
#include "DvbUser.hpp"
#include "DvbUser.h"
#include "MfxGlobals.hpp"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


__inline static void setChannel( DvbSetCAUser *bitmap, ushort channel )
{
	bitmap->channelBitmap[channel / 8] |= ( (uchar)0x01 << ( channel % 8 ) );
}


DvbUserLog::DvbUserLog( const char *fileName )
{
	ASSERT( fileName != NULL );

	_userID.makeInvalid() ;
	_flags			= sUserCA::NONE;
	_fileName		= STRDUP( fileName );
	_changed		= FALSE;

	InitializeCriticalSection( &_userCALock );
	// after creating must load
}


DvbUserLog::~DvbUserLog()
{
	DeleteCriticalSection( &_userCALock );

	FREE( _fileName ) ;
	for( int i = 0; i < _channelNames.count(); i++ )
		if( _channelNames.item( i ) != NULL )
			FREE( (char *)_channelNames.item( i ) );
}


ushort DvbUserLog::nChannels()
{
	EnterCriticalSection( &_userCALock );

	ushort retval = _channels.count();

	LeaveCriticalSection( &_userCALock );
	return retval;
}

ushort DvbUserLog::channels( ushort i )
{
	EnterCriticalSection( &_userCALock );

	ushort retval = _channels.item( i );

	LeaveCriticalSection( &_userCALock );
	return retval;
}

// binary searching
int DvbUserLog::find( ushort channel )
{
	long left	= 0;
	long right	= _channels.count() - 1;
	if( right < left )
		return -1;

	ushort *ids = (ushort*)_channels ;

	if( channel < ids[left] )
		return -1;
	if( channel > ids[right] )
		return -1;

	while( left <= right )
	{
		long middle = ( left + right ) / 2;

		if( channel == ids[middle] )
			return middle;
		else
		if( channel < ids[middle] )
			right = middle - 1;
		else
		if( channel > ids[middle] )
			left = middle + 1;
	}
	return -1;
}


BOOL DvbUserLog::hasChannel( USHORT channel )
{
	EnterCriticalSection( &_userCALock );

	BOOL retval	= find( channel ) != -1;

	LeaveCriticalSection( &_userCALock );
	return retval;
}

const char *DvbUserLog::getChannelNameByID( ushort channel )
{
	EnterCriticalSection( &_userCALock );

	const char *retval = NULL;
	int i = find( channel );
	if( i != -1  )
		retval = _channelNames.item( i );

	LeaveCriticalSection( &_userCALock );
	return retval;
}


void DvbUserLog::setChannelNameByID( ushort channel, char *channelName )
{
	EnterCriticalSection( &_userCALock );

	int i = find( channel );
	if( i != -1 )
	{
		if( _channelNames.item( i ) != NULL )
			FREE( (char *)_channelNames.item( i ) );

		_channelNames.item( i ) = STRDUP( channelName );
		_changed = TRUE;
	}

	LeaveCriticalSection( &_userCALock );
};


const char *DvbUserLog::getChannelName( ushort index )
{
	EnterCriticalSection( &_userCALock );

	ASSERT( 0 <= index && index < _channels.count() );
	const char *retval = _channelNames.item( index );

	LeaveCriticalSection( &_userCALock );
	return retval;
};


void DvbUserLog::setChannelName( ushort index, char *channelName )
{
	EnterCriticalSection( &_userCALock );

	ASSERT( 0 <= index && index < _channels.count() );
	if( _channelNames.item( index ) != NULL )
	{
		if( strcmp( _channelNames.item( index ), channelName ) != 0 )
		{
			FREE( (char *)_channelNames.item( index ) );
			_channelNames.item( index ) = STRDUP( channelName );
			_changed = TRUE;
		}
	}
	else
	{
		_channelNames.item( index ) = STRDUP( channelName );
		_changed = TRUE;
	}

	LeaveCriticalSection( &_userCALock );
};


BOOL DvbUserLog::reset()
{
	EnterCriticalSection( &_userCALock );

	BOOL retval = FALSE;
	if( _flags != sUserCA::NONE || _channels.count() != 0 )
	{
		_flags = sUserCA::NONE;
		_channels.clearList();
		_channelNames.clearList();
		_changed = TRUE;
		retval = TRUE;
		MfxPostMessage( EMsg_SCAChanged );
	}
	LeaveCriticalSection( &_userCALock );
	return retval;
}


BOOL DvbUserLog::updateCA( sUserCA *userCA, sTemplateArray<ushort> *updatedChannels )
{
	EnterCriticalSection( &_userCALock );

	ushort channel;
	int i = 0;
	int j = 0;
	while( i < _channels.count() && _channels.item( i ) < userCA->fromChannel )
		i++;
	while( i < _channels.count() && j < userCA->numChannels )
	{
		if( userCA->channels[j] == 0 )
		{
			j++;
		}
		else if( _channels.item( i ) == userCA->channels[j] )
		{
			i++;
			j++;
		}
		else if( _channels.item( i ) < userCA->channels[j] )
		{
			_changed = TRUE;
			channel = _channels.item( i );
			_channels.del( i );
			if( _channelNames.item( i ) != NULL )
				FREE( _channelNames.item( i ) );
			_channelNames.del( i );
			updatedChannels->add( channel );
			MfxPostMessage( EMsg_ChannelDeleted, channel );
		}
		else if( _channels.item( i ) > userCA->channels[j] )
		{
			_changed = TRUE;
			channel = userCA->channels[j];
			_channels.insert( i, channel );
			_channelNames.insert( i, NULL );
			updatedChannels->add( channel );
			MfxPostMessage( EMsg_ChannelAdded, channel );
			i++;
			j++;
		}

	}
	while( i < _channels.count() && _channels.item( i ) < userCA->toChannel )
	{
		_changed = TRUE;
		channel = _channels.item( i );
		_channels.del( i );
		if( _channelNames.item( i ) != NULL )
			FREE( _channelNames.item( i ) );
		_channelNames.del( i );
		updatedChannels->add( channel );
		MfxPostMessage( EMsg_ChannelDeleted, channel );
	}
	while( j < userCA->numChannels )
	{
		_changed = TRUE;
		channel = userCA->channels[j];

		if( channel != 0 )
		{
			_channels.add( channel );
			_channelNames.add( NULL );
			updatedChannels->add( channel );
			MfxPostMessage( EMsg_ChannelAdded, channel );
		}
		j++;
	}

	BOOL changed = _changed;
	if( _flags != userCA->flags )
	{
		_changed = TRUE;
		_flags	= userCA->flags;
	}

	if( _changed )
		MfxPostMessage( EMsg_SCAChanged );

	LeaveCriticalSection( &_userCALock );
	return changed;
}


BOOL DvbUserLog::isUserCAOk( sUserCA *userCA )
{
	if( userCA->flags > sUserCA::ALL )
		return FALSE;

	if( userCA->fromChannel > userCA->toChannel )
		return FALSE;

	if( userCA->numChannels > MAXNUMOFCHANNEL )
		return FALSE;

	int i = 0;
	int j = 1;
	while( j < userCA->numChannels )
		if( userCA->channels[i++] > userCA->channels[j++] )
			return FALSE;

	return TRUE;
}



void DvbUserLog::fillBitmapForCard( void *btmp )
{
	DvbSetCAUser *bitmap = (DvbSetCAUser *)btmp;

	memset( bitmap, 0, sizeof(DvbSetCAUser) );
	bitmap->flags = (uchar)_flags;
	for( ushort i = 0; i < _channels.count(); i++ )
		setChannel( bitmap, _channels.item( i ) );
}


BOOL DvbUserLog::synchChannelsWithCard( void *bitmp, sTemplateArray<ushort> *updatedChannels )
{
	EnterCriticalSection( &_userCALock );

	DvbSetCAUser *bitmap = (DvbSetCAUser *)bitmp;
	uchar	*btmp = bitmap->channelBitmap;
	ushort	index = 0;

	for( ushort i = 0; i < 8192; i++ )
	{
		if( *btmp != 0 )
		{
			ushort j = 0;
			uchar  c = *btmp;
			while( c )
			{
				if( c & (uchar)0x01 )
				{
					ushort channel = 8 * i + j;

					if( channel != 0x0000 && channel != 0xffff )
					{
						while( index < _channels.count() && _channels.item( index ) < channel )
						{
							_changed = TRUE;
							ushort ch = _channels.item( index );
							_channels.del( index );
							if( _channelNames.item( index ) != NULL )
								FREE( _channelNames.item( index ) );
							_channelNames.del( index );
							updatedChannels->add( ch );
							MfxPostMessage( EMsg_ChannelDeleted, ch );
						}
						if( index < _channels.count() )
						{
							if( _channels.item( index ) != channel )
							{
								_changed = TRUE;
								_channels.insert( index, channel );
								_channelNames.insert( index, NULL );
								updatedChannels->add( channel );
								MfxPostMessage( EMsg_ChannelAdded, channel );
							}
						}
						else
						{
							_changed = TRUE;
							_channels.add( channel );
							_channelNames.add( NULL );
							updatedChannels->add( channel );
							MfxPostMessage( EMsg_ChannelAdded, channel );
						}
						index++;
					}
				}
				c = c >> 1;
				j++;
			}
		}
		btmp++;
	}
	while( index < _channels.count() )
	{
		_changed = TRUE;
		ushort ch = _channels.item( index );
		_channels.del( index );
		if( _channelNames.item( index ) != NULL )
			FREE( _channelNames.item( index ) );
		_channelNames.del( index );
		updatedChannels->add( ch );
		MfxPostMessage( EMsg_ChannelDeleted, ch );
	}

	BOOL changed = _changed;
	if( _flags != bitmap->flags )
	{
		_flags = bitmap->flags;
		_changed = TRUE;
	}

	if( _changed )
		MfxPostMessage( EMsg_SCAChanged );
	
	LeaveCriticalSection( &_userCALock );
	return changed;
}

static int uUserCfgLogVersion = 1;
void DvbUserLog::save()
{
	EnterCriticalSection( &_userCALock );

	BOOL	error = FALSE;
	int		msg;
	char	buff[512];
	
	if( _changed )
	{
		ProtectedFile	userLogFile( _fileName, ProtectedFile::P_DATA );
		int				err;

		if( ( err = userLogFile.open( ProtectedFile::F_WRITE ) ) )
		{
			error = TRUE;
			switch( err )
			{
			case ERR_READFILE:
				msg = FileErr_ReadError;
				sprintf( buff, "Can't read file %s", _fileName );
				break;
			default:
				msg = FileErr_OpenError;
				sprintf( buff, "Can't open file %s", _fileName );
				break;
			}
		}

		if( !error )
		{
			ushort			numChannels =_channels.count();
			GlobalUserID	id			= userID();

			msg = FileErr_WriteError;
			sprintf( buff, "Can't write file %s", _fileName );

			if( userLogFile.fWrite( &uUserCfgLogVersion,sizeof( int ),		  1 ) != 1 ||
				userLogFile.fWrite( &id,				sizeof( GlobalUserID),1 ) != 1 ||
			    userLogFile.fWrite( &_flags,			sizeof( ushort ),	  1 ) != 1 ||
				userLogFile.fWrite( &numChannels,		sizeof( ushort ),	  1 ) != 1 )
			{
				error = TRUE;
			}

			if( !error && numChannels != 0 )
			{
				char	*channelName;
				ushort	slen;
				ushort	channel;

				for( ushort i = 0; i < numChannels; i++ )
				{
					channel = _channels.item( i );
					if( userLogFile.fWrite( &channel, sizeof( ushort ), 1 ) != 1 )
					{
						error = TRUE;
						break;
					}

					channelName = (char *)_channelNames.item( i );
					if( channelName != NULL )
						slen = strlen( channelName ) + 1;
					else
						slen = 0;

					if( userLogFile.fWrite( &slen, sizeof( ushort ), 1 ) != 1 )
					{
						error = TRUE;
						break;
					}

					if( slen != 0 )
					{
						if( userLogFile.fWrite( channelName, slen, 1 ) != 1 )
						{
							error = TRUE;
							break;
						}
					}
				}
			}
		}

		userLogFile.close();
		_changed = FALSE;
	}

	LeaveCriticalSection( &_userCALock );

	if( error )
		throw Msg( msg, "Error when saving user access rights:\n%s", buff );
}

void DvbUserLog::load( const GlobalUserID &userId )
{
	EnterCriticalSection( &_userCALock );

	BOOL	error = FALSE;
	int		msg;

	ProtectedFile	userLogFile( _fileName, ProtectedFile::P_DATA );

	_changed = FALSE;

	if( userLogFile.test() != ProtectedFile::P_NONE )
		error		= TRUE;
	else
	if( userLogFile.open( ProtectedFile::F_READ) != 0 )
		error = TRUE;
	else
	{
		ushort			numChannels;
		int				uLogVersion;
		GlobalUserID	id;

		if( userLogFile.fRead( &uLogVersion,	sizeof( int ),			1 ) != 1 ||
			userLogFile.fRead( &id,				sizeof( GlobalUserID),	1 ) != 1 ||
			userLogFile.fRead( &_flags,			sizeof( ushort ),		1 ) != 1 ||
			userLogFile.fRead( &numChannels,	sizeof( ushort ),		1 ) != 1 ) 
			error = TRUE ;
		else
		if( uLogVersion	!= uUserCfgLogVersion )
			error = TRUE ;
		else
		if( id != userId )
			error = TRUE ;
		else
		{
			_channels.clearList();
			_channelNames.clearList();

			for( ushort i = 0; i < numChannels; i++ )
			{
				char	*channelName;
				ushort	slen;
				ushort	channel;

				if( userLogFile.fRead( &channel, sizeof( ushort ), 1 ) != 1 )
					break;

				_channels.add( channel );
				if( userLogFile.fRead( &slen, sizeof( ushort ), 1 ) != 1 )
					break;

				if( slen != 0 )
				{
					channelName = (char *)MALLOC( slen );
					if( userLogFile.fRead( channelName, slen, 1 ) != 1 )
					{
						FREE( channelName );
						_channelNames.add( NULL );
						break;
					}
					_channelNames.add( channelName );
				}
				else
					_channelNames.add( NULL );
			}

			if( i != numChannels )
				error = TRUE;
		}
		userLogFile.close();
	}

	LeaveCriticalSection( &_userCALock );

	_userID = userId ;
	if( error )
	{
		_changed = TRUE ;
		throw Msg( -1, 
			"File containing user access rights is missing or corrupted.\n"
			"All DVB channels (except service BNMS channel) were deleted.\n"
			"\n"
			"This can happen if you change f.e. your input connection.\n"
			"Contact your Service Provider or wait until new access rights will be broadcasted."
			);
	}
}
