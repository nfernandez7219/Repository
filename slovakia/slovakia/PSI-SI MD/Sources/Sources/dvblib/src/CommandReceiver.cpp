#include "tools2.hpp"
#include "mux.hpp"

#include "ClientCfg.hpp"
#include "FileIO.hpp"
#include "DvbUser.h"


//-------------------------------------------------------------------------------
//	CommandReceiver
//	called by ComInp::processPacket() to process statistics/CAUser commands
//  This class is used by the Receiver only.
//-------------------------------------------------------------------------------


CommandReceiver::CommandReceiver()
{
	_CAUser = MALLOC( sizeof(DvbSetCAUser) );
}

// Called when 1st CA-table packet is coming to clear the CA table.
void CommandReceiver::clearCAUser()
{
	numPakcets = sizeof(DvbSetCAUser) / MUXDATASIZE;
	numBytesInLastPacket = sizeof(DvbSetCAUser) - numPakcets * MUXDATASIZE;
	if( numBytesInLastPacket == 0 )
	{
		numPakcets--;
		numBytesInLastPacket = MUXDATASIZE;
	}
	lastRecievedPacketIndex = -1;
	isBitmapOK = TRUE;
	memset( _CAUser, 0, sizeof(DvbSetCAUser) );
}

// Processing of 1 command packet.
void CommandReceiver::push( MuxPacket *packet )
{
	switch ( packet->crc16() )
	{
		// accept new CA-table
		case DVB_SETCAUSER:
		{
			int index			= packet->packetIndex();
			int numBytesToCopy	= MUXDATASIZE;

			if( index == 0 )
				clearCAUser();
			else if( !isBitmapOK )
				return;

			if( index > numPakcets )
				return;

			if( index == numPakcets )
				numBytesToCopy = numBytesInLastPacket;

			lastRecievedPacketIndex++;
			if( lastRecievedPacketIndex == index )
				memcpy( (char *)_CAUser + index * MUXDATASIZE, packet->data(), numBytesToCopy );
			else
				isBitmapOK = FALSE;

			if( isBitmapOK && index == numPakcets )
			{
				ComInp *com = ((BigComInp*)bigComIO)->com();

				com->synchUserCAWithCard( _CAUser );
			}
			break;
		}
	}
}
