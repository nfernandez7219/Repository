/*++ BUILD Version: 0004    // Increment this if a change has global effects

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    devioctl.h

Abstract:

    This module contains

Author:

    Andre Vachon (andreva) 21-Feb-1992


Revision History:


--*/

// begin_winioctl

#ifndef _DEVIOCTL_
#define _DEVIOCTL_

// begin_ntddk begin_nthal
//
// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.
//

#define DEVICE_TYPE ULONG

#define FILE_DEVICE_BEEP                0x00000001
#define FILE_DEVICE_CD_ROM              0x00000002
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003
#define FILE_DEVICE_CONTROLLER          0x00000004
#define FILE_DEVICE_DATALINK            0x00000005
#define FILE_DEVICE_DFS                 0x00000006
#define FILE_DEVICE_DISK                0x00000007
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008
#define FILE_DEVICE_FILE_SYSTEM         0x00000009
#define FILE_DEVICE_INPORT_PORT         0x0000000a
#define FILE_DEVICE_KEYBOARD            0x0000000b
#define FILE_DEVICE_MAILSLOT            0x0000000c
#define FILE_DEVICE_MIDI_IN             0x0000000d
#define FILE_DEVICE_MIDI_OUT            0x0000000e
#define FILE_DEVICE_MOUSE               0x0000000f
#define FILE_DEVICE_MULTI_UNC_PROVIDER  0x00000010
#define FILE_DEVICE_NAMED_PIPE          0x00000011
#define FILE_DEVICE_NETWORK             0x00000012
#define FILE_DEVICE_NETWORK_BROWSER     0x00000013
#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014
#define FILE_DEVICE_NULL                0x00000015
#define FILE_DEVICE_PARALLEL_PORT       0x00000016
#define FILE_DEVICE_PHYSICAL_NETCARD    0x00000017
#define FILE_DEVICE_PRINTER             0x00000018
#define FILE_DEVICE_SCANNER             0x00000019
#define FILE_DEVICE_SERIAL_MOUSE_PORT   0x0000001a
#define FILE_DEVICE_SERIAL_PORT         0x0000001b
#define FILE_DEVICE_SCREEN              0x0000001c
#define FILE_DEVICE_SOUND               0x0000001d
#define FILE_DEVICE_STREAMS             0x0000001e
#define FILE_DEVICE_TAPE                0x0000001f
#define FILE_DEVICE_TAPE_FILE_SYSTEM    0x00000020
#define FILE_DEVICE_TRANSPORT           0x00000021
#define FILE_DEVICE_UNKNOWN             0x00000022
#define FILE_DEVICE_VIDEO               0x00000023
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_WAVE_IN             0x00000025
#define FILE_DEVICE_WAVE_OUT            0x00000026
#define FILE_DEVICE_8042_PORT           0x00000027
#define FILE_DEVICE_NETWORK_REDIRECTOR  0x00000028

//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

//
// Define the method codes for how buffers are passed for I/O and FS controls
//

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

//
// Define the access check value for any access
//
//
// The FILE_READ_ACCESS and FILE_WRITE_ACCESS constants are also defined in
// ntioapi.h as FILE_READ_DATA and FILE_WRITE_DATA. The values for these
// constants *MUST* always be in sync.
//


#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe
#define MaxPlxDmaChannels 2	// Channels are assigned permanently to transfer directions (if supported by hardware)
#define TxDir 0
#define RxDir 1
// OptionFlags bit specifications
#define RLSetHigh 1 // HotLink RL pin set HIGH during normal receive operation (otherwise it's LOW)
#define FrSz204 2 // Transmit interframe stuffing generated for 204 byte packet (otherwise 188 byte)
#define PktSyncEnbl 4  // Enable Transport Stream packet synchronization for receive

#define FILE_DEVICE_DVB_PORT 32768	 
// Read channel statistics, zero counters, initialize minimums
#define IOCTL_DVB_RD_ST CTL_CODE(FILE_DEVICE_DVB_PORT,2048,METHOD_BUFFERED,FILE_ANY_ACCESS)
// read channel statistics, don't zero counters
#define IOCTL_DVB_RD_ST_NZ CTL_CODE(FILE_DEVICE_DVB_PORT,2050,METHOD_BUFFERED,FILE_ANY_ACCESS)
// read channel configuration
#define IOCTL_DVB_RD_CFG CTL_CODE(FILE_DEVICE_DVB_PORT,2051,METHOD_BUFFERED,FILE_ANY_ACCESS)
// set channel configuration
#define IOCTL_DVB_SET_CFG CTL_CODE(FILE_DEVICE_DVB_PORT,2049,METHOD_BUFFERED,FILE_ANY_ACCESS)
// read PLX global status (do not use)
#define IOCTL_DVB_GET_STATUS CTL_CODE(FILE_DEVICE_DVB_PORT,2052,METHOD_BUFFERED,FILE_ANY_ACCESS)
// write PLX global configuration (do not use)
#define IOCTL_DVB_SET_COMMAND CTL_CODE(FILE_DEVICE_DVB_PORT,2053,METHOD_BUFFERED,FILE_ANY_ACCESS)
// software reset of PLX channel and re-program channel, clear framing error, etc.
#define IOCTL_DVB_RESET_REFRAME CTL_CODE(FILE_DEVICE_DVB_PORT,2056,METHOD_BUFFERED,FILE_ANY_ACCESS)
// returned by  IOCTL_DVB_RD_ST,IOCTL_DVB_RD_ST_NZ
struct DVBStats {
	ULONG StartDma[MaxPlxDmaChannels] // The channel Dma controller was started
		,NumLost // The ISR could not start the DSP (Deferred Service Procedure)
		,NumExErr // Dma completion error (should never occur !)
		,MaxDspIntCount[MaxPlxDmaChannels] // The DSP for this channel has been started by the ISR
		,NumPciAbts[MaxPlxDmaChannels] // Pci abort interrupt signalled from hardware
		,MinNumPend[MaxPlxDmaChannels] // Minimum NumPend occuring since start or last IOCTL_DVB_RD_ST
		,NumPend[MaxPlxDmaChannels] // Number of buffers active for Dma transfer and pending completion
		,NumInts[MaxPlxDmaChannels] // Number of Dma interrupts generated by the channel
		,NumFifoErrs[MaxPlxDmaChannels] // Interrupt count at which FIFO overflow occured (0= no errors)
		,NumQued[MaxPlxDmaChannels]; // Number of buffers queued for Dma activation
 };
// set/returned by IOCTL_DVB_SET_CFG/IOCTL_DVB_RD_CFG
struct DVBCfg {
	BOOLEAN DirSupported[MaxPlxDmaChannels]; // Hardware supports transfer in this direction
	int DVB_opens // Number of open handles on this channel
		,OptionFlags // Bit flags controlling additional BOOLEAN selections
		,MaxTransferSize[MaxPlxDmaChannels] // Maximum Dma transfer size allowed for this channel
		,MaxBuffers[MaxPlxDmaChannels] // Maximum number of buffers that may be simultaneously active on this
		// channel (additional buffers will be queued)
		,FifoAe[MaxPlxDmaChannels] // Hardware almost empty limit for FIFO's (do not change)
		,FifoAf[MaxPlxDmaChannels] // Hardware almost full limit for FIFO's (do not change)
		,Stuffing  // Hardware transmit stuffing control parameter
		,Reserved1;
 };

#endif // _DEVIOCTL_

