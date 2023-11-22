#ifndef  __INC_CARD_H__
#define  __INC_CARD_H__

#include "..\\..\\drivers\\cardNT\\CardDrv.h"
#include "serialNum.hpp"

#pragma pack(1)

#define	DVBDRVNAME			"md_DVBdriver.sys"
#define DVB_CARDPASSWORD	"MD Dvb DSP Card"


//
// packet format: [TSheader] [PESheader] data
//		TSheader	= Transport Stream header (4By)
//		PESheader	= (optional) Program Stream header (6By)
//		data		= 178/184 By
//
// Following possibilities are allowed:
//
//						raw data		Transport Stream
//	--------------------------------------------------------
//	data piping			<data184>		<TS4><data184>
//	data streaming		<data178>		<TS4><PES6><data178>
//


// DvbOpen::flags
// Remaining bits are interpreted solelly by the Card
#define	DVBFLAG_IsSender		 0x01	// 1 - Sender (else Receiver),
#define	DVBFLAG_StreamFormatPES	 0x08	// (Receiver input) 1- TS4+PES6+Mux178;  0- TS4+Mux184
#define	DVBFLAG_ClockSource		 0x40	// 0-Card (i.e. card determines output rate), 1-external

typedef struct tagDvbOpen {
	unsigned short		appVersion ;				// receiver program version
	unsigned short		cardPrgVersion ;			// 0	version of the card program; 0 if the program is not available
	unsigned short		PID ;						// N/A	PID used in Transport Stream data packets (to eliminate MPEG system tables) ???
	unsigned char		flags ;						// DVBFLAG_*
	unsigned char		reserved ;					// 0
	unsigned short		speed;						// [kHz] (Relevant only if the Card generates clock signal, i.e. external clock is independent) 
	unsigned short		ackTimeout ;				// [ms] Receiver only: card waits this time if less than 31 packets available when driver requests data
	unsigned short		statisticsTimeout ;			// [ms] Receiver only: time interval between CardStatistics packets (default 1000 => 1sec)
} DvbOpen ;

typedef struct tagDvbCardInfo {
	unsigned char		name[20] ;					// DVB_CARDPASSWORD (NULL terminated)
	unsigned short		cardPrgVersion ;			// version of the card program
	unsigned short		cardMemory ;				// card memory size (kB)
	CardSerialNumber	serialNum ;					// B5=highest, B0=lowest
	unsigned char		programOK ;					// 0 - failed, 1 - ok
	unsigned short		status ;					// valid only if programOK
													// 0x04 - 1-opened as server
	// debug information for program download
	/*unsigned char		results[119] ;
	unsigned char		counts[18];
	unsigned char		nPacketsSent;
	unsigned char		nPacketsAnalyzed;*/
} DvbCardInfo ;

// (Relevant only if the Card generates clock signal,  i.e. external clock is independent)
typedef struct tagDvbSetSpeed {
	unsigned short		speed;						// [kHz]
} DvbSetSpeed;

typedef struct tagDvbCardStatistics {
	unsigned short		corrupted;
	unsigned short		arrived;
	unsigned short		filtered;
	unsigned short		lost;
	unsigned char		flags;
} DvbCardStatistics;

//#define	STATFLAG_ComsatAlive		0x01	// Clock present (connection to demultiplexor alive)
//#define	STATFLAG_ComsatSynchronized	0x02	// Card is synchronised with demultiplexor  (??? Co ked karta dava clock ???)
//#define	STATFLAG_BufferOVF			0x04	// Card buffer overflow

#pragma pack()

#ifdef __cplusplus


//
// All following functions returning int return DVB error codes.
//

extern "C"
{

BOOL testDrvRunning();
BOOL installDvbDrv( char *expl );
BOOL uninstallDvbDrv( char *expl ) ;

BOOL restartDvbDrv( char *expl ) ;

BOOL lockDVB( char *reason ) ;
void unlockDVB( ) ;

int  dvbDrvOpen ( DvbDrvMode *m, DvbDrvInfo *inf );
BOOL dvbDrvOpened() ;
int  dvbDrvClose( void );

int  dvbDrvRead ( void *buf, int maxPackets2read , int *numPacketsRead );
int  dvbDrvWrite( void *buf, int maxPackets2write, int *numPacketsWritten );
int  dvbDrvStatistics( DvbDrvStatistics *stat );

USHORT dvbDrvDbgState( OUT void *b);
USHORT dvbDrvDump( char manually = 0 );

USHORT dvbDrvDbgStateDlg( HINSTANCE hRes, BOOL dumpAllowed );
void   StopDrvStateDialog();

}		// end extern "C"

#endif	// __cplusplus

#endif
