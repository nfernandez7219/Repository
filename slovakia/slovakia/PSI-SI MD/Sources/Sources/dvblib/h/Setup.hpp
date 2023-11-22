#ifndef __INC_SETUP_HPP__
#define __INC_SETUP_HPP__

// Sender tuning:
// Under normal circumstances the delay takes place on 3 distinct places:
// 1. When waiting for data.
// 2. When waiting for multiplexor to assign the output capacity.
// 3. When packets are waiting in the output queue for flush - this is done periiodically.
#define DELAY_WAITING_FOR_HNET_DATA			1		/* msec; sleep when no data from HNet available */
#define DELAY_WAITING_FOR_FREE_PACKETS		50		/* msec; non-HNet channel sleep when waiting for multiplexor to assign free packets*/
#define DELAY_WAITING_FOR_FREE_IP_PACKETS	5		/* msec; the same for Internet channel*/
#define MUXOUTPUT_FLUSHINTERVAL				10		/* msec; interval between consecutive write's to output */

// Receiver tuning:
// The receiver immediatelly processes each incoming packet. The only place where the delay
// is possible is the sleep when no input data is available.
#define NO_INPUT_DATA_DELAY					1		/* msec; DVB connection only */

//----------------- DEBUG TOOLS -----------------------------------------------------
// Uncomment following #define's to receive extra debug information:

// To write received corrupted packets to "badPacket.dmp":
//#define _DUMP_RECEIVED_BAD_PACKETS

// To write handshake packets into "cardSrv/Rcv.dmp":
//#define _DUMP_DVBCARD_HANDSHAKE

// To write all received packets into "cardRcv.dmp":
//#define _DUMP_DVBCARD_RCVPACKETS

// To redirect TRACE macro to "TraceFile.dump":
#ifndef _DEBUG
//#define _PRIVATETRACE	
#endif

// See for additional debug tools in the file Internet.cpp
//-----------------------------------------------------------------------------------

#endif
