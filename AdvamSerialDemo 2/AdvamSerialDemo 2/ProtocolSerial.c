#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"

#include "CRC16.h"
#include "ProtocolSerial.h"

#define STX		0x02
#define ETX		0x03
#define ACK		0x06
#define NAK		0x15
#define WACK	0x13

static HANDLE hSerial;
static BYTE byPrevSeqNum;
static BYTE byPrevSentSeqNum;

static BOOL CheckCRC(BYTE* pbyBuf);
static USHORT CalcCRC(BYTE* pbyBuf);
static void ExtractPayload(BYTE* pbyBuf, char* pcPayLoad);

void PSInit(void)
{
	DCB dcbSerialParams = {0};
	COMMTIMEOUTS timeouts = {0};

	printf("PSInit -\n");

	hSerial = CreateFile("COM1",
			  GENERIC_READ | GENERIC_WRITE,
			  0,
			  0,
			  OPEN_EXISTING,
			  FILE_ATTRIBUTE_NORMAL,
			  0);

	if(hSerial == INVALID_HANDLE_VALUE)
	{
		if( GetLastError() == ERROR_FILE_NOT_FOUND )
		{
			//serial port does not exist. Inform user.
		}
	//some other error occurred. Inform user.
	}

	if(!GetCommState(hSerial, &dcbSerialParams))
	{
		//error getting state
	}
	
	//dcbSerialParams.BaudRate=CBR_9600;
	dcbSerialParams.BaudRate=CBR_115200;
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=ONESTOPBIT;
	dcbSerialParams.Parity=NOPARITY;
	
	if(!SetCommState(hSerial, &dcbSerialParams))
	{
		//error setting serial port state
	}
	
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	
	if(!SetCommTimeouts(hSerial, &timeouts))
	{
		printf("%s - SetCommTimeouts not ok!\n", __FUNCTION__);
	}

	//GetCommTimeouts(hSerial, &timeouts);

	// Set message sequence number to the startup value
	byPrevSeqNum = 255;
	byPrevSentSeqNum = 255;
}

void PSDispose(void)
{
	CloseHandle(hSerial);
}

void PSWrite(BYTE* pbyMessage)
{
	// Format:
	// STX | NUM | ... data bytes ... | ETX | CRC1 (lowbyte) | CRC2 (highbyte)

	BYTE pbyBuf[4096];
	BYTE byCurrSeqNum;
	DWORD dwBytesRead;
	DWORD dwBytesWritten;
	USHORT CRC;
	BYTE CRC1, CRC2;
	USHORT mask, tmp;
	unsigned int i, j;
	BYTE c;
	DWORD dwTickStart;
	DWORD dwTickCheck;
	DWORD dwTickDiff;
	BOOL bDone;

	memset(pbyBuf, 0, sizeof(pbyBuf));
	i = 0;

	// First char must be STX
	pbyBuf[i++] = STX;

	// Next is Sequence Number
	switch(byPrevSentSeqNum)
	{
	case 254:
		// Roll over to 0
		byCurrSeqNum = 0;
		byPrevSentSeqNum = 0;
		break;

	case 255:
		// Startup value, set current to this value...
		byCurrSeqNum = 255;
		// and decrement previous sent value, so that the next time it will be 0
		byPrevSentSeqNum--;
		break;

	default:
		byCurrSeqNum = ++byPrevSentSeqNum;
		break;
	}

	pbyBuf[i++] = byCurrSeqNum;

	// Data bytes
	for(j = 0; j < strlen(pbyMessage); j++)
	{
		pbyBuf[i++] = pbyMessage[j];
	}

	// Append ETX
	pbyBuf[i++] = ETX;

	// Calculate CRC
	CRC = CalcCRC(pbyBuf);

	// CRC1 (lowbyte)
	mask = 0x00FF;
	tmp = CRC;
	tmp &= mask;
	CRC1 = (UCHAR)tmp;

	// CRC2 (highbyte)
	tmp = CRC >> 8;
	CRC2 = (UCHAR)tmp;

	// Append CRC
	pbyBuf[i++] = CRC1;
	pbyBuf[i++] = CRC2;

	// OK complete message built, send off
	WriteFile(hSerial, pbyBuf, i, &dwBytesWritten, NULL);

	bDone = FALSE;

	while(!bDone)
	{
		// Wait for reception of ACK, NAK or WACK during Answer Timeout of 1500 ms
		dwTickCheck = 1500;
		dwTickStart = GetTickCount();

		do
		{
			ReadFile(hSerial, &c, 1, &dwBytesRead, NULL);
			dwTickDiff = GetTickCount() - dwTickStart;
		}
		while(!dwBytesRead && dwTickDiff < dwTickCheck);

		if(!dwBytesRead)
		{
			printf("PSWrite - Answer Timeout happened, resending\n");
			WriteFile(hSerial, pbyBuf, i, &dwBytesWritten, NULL);
			continue;
		}

		switch(c)
		{
		case ACK:
			// Read Sequence Number
			ReadFile(hSerial, &c, 1, &dwBytesRead, NULL);

			if(dwBytesRead)
			{
				// Check Sequence Number
				if(c == byCurrSeqNum)
				{
					bDone = TRUE;
				}
				else
				{
					printf("PSWrite - Received incorrect message number %d, keep listening\n", c);
				}
			}
			else
			{
				printf("PSWrite - ICT while expecting Sequence Number\n");
			}

			break;

		case NAK:
			printf("PSWrite - Received a NAK, resending\n");
			WriteFile(hSerial, pbyBuf, i, &dwBytesWritten, NULL);
			break;

		case WACK:
			printf("PSWrite - Received a Wait_ACK, keep listening\n");
			break;

		default:
			printf("PSWrite - Received unknown response %d, keep listening\n", c);
			break;
		}
	}
}

int PSRead(BYTE* pbyMessage)
{
	// Format:
	// STX | NUM | ... data bytes ... | ETX | CRC1 (lowbyte) | CRC2 (highbyte)

	BYTE pbyBuf[4096];
	BYTE pbyBufAck[3];
	DWORD dwBytesRead;
	DWORD dwBytesWritten;
	int i, j;
	BYTE c;
	BYTE byCurrSeqNum;
	BYTE byExpSeqNum;

	while(TRUE)
	{
		memset(pbyBuf, 0, sizeof(pbyBuf));
		i = 0;

		// Listen for STX
		ReadFile(hSerial, &c, 1, &dwBytesRead, NULL);

		if(!dwBytesRead)
		{
			//continue;
			return 0;
		}

		if(c != STX)
		{
			printf("PSRead - Need STX but got %02X, discarding\n", c);
			continue;
		}

		pbyBuf[i] = c;

		// Read in Sequence Number
		ReadFile(hSerial, &c, 1, &dwBytesRead, NULL);

		// Check if Inter Character Timer timeout occurred
		if(!dwBytesRead)
		{
			printf("PSRead - ICT while expecting Sequence Number\n");
			continue;
		}

		pbyBuf[++i] = c;

		// Read in remainder of message up to and including ETX using
		// Inter Character Timer timeout value
		do
		{
			ReadFile(hSerial, &c, 1, &dwBytesRead, NULL);
			pbyBuf[++i] = c;
		}
		while(pbyBuf[i] != ETX && dwBytesRead);

		// Check if Inter Character Timer timeout occurred
		if(!dwBytesRead)
		{
			printf("PSRead - ICT while reading in bulk of message\n");
			continue;
		}

		// Read in the two CRC bytes
		for(j = 0; j < 2; j++)
		{
			ReadFile(hSerial, &c, 1, &dwBytesRead, NULL);
			pbyBuf[++i] = c;
			if(!dwBytesRead)
				break;
		}

		// Check if Inter Character Timer timeout occurred
		if(!dwBytesRead)
		{
			printf("PSRead - ICT while reading in CRC bytes\n");
			continue;
		}

		// OK complete message received
		byCurrSeqNum = pbyBuf[1];

		//Check CRC Validity
		memset(pbyBufAck, 0, sizeof(pbyBufAck));

		if(CheckCRC(pbyBuf))
		{
			// Send ACK + Sequence Number
			sprintf(pbyBufAck, "%c%c", ACK, byCurrSeqNum);
			WriteFile(hSerial, pbyBufAck, 2, &dwBytesWritten, NULL);
		}
		else
		{
			// Send NAK
			sprintf(pbyBufAck, "%c", NAK, byCurrSeqNum);
			WriteFile(hSerial, pbyBufAck, 1, &dwBytesWritten, NULL);
			printf("PSRead - CRC check failed\n");
			continue;
		}

		// Check Sequence Number Validity
		switch(byPrevSeqNum)
		{
		case 254:
			byExpSeqNum = 0;
			break;

		default:
			byExpSeqNum = byPrevSeqNum + 1;
			break;
		}

		if(byCurrSeqNum >= byExpSeqNum || byCurrSeqNum == 255)
		{
			// Extract payload
			ExtractPayload(pbyBuf, pbyMessage);

			// Update Previous Sequence Number
			byPrevSeqNum = byCurrSeqNum;

			// Break out of loop
			break;
		}
		else if(byCurrSeqNum == byPrevSeqNum)
		{
			// Ignore when current same as previous
			printf("PSRead - Message number same as previous, ignoring message\n");
			break;
		}
		else
		{
			// Doc says this is an "error" but says nothing about how to handle it...
			printf("PSRead - Message number not expected\n");
			break;
		}

	}

	return 1;
}

static BOOL CheckCRC(BYTE* pbyBuf)
{
	// Packet format:
	// STX | NUM | ... data bytes ... | ETX | CRC1 (lowbyte) | CRC2 (highbyte)
	// The CRC must be calculated over:
	// NUM | ... data bytes ... | ETX

	int i, j;
	BYTE pbyTest[4096];
	BYTE CRC1, CRC2;
	USHORT recvCRC, calcCRC;

	memset(pbyTest, 0, sizeof(pbyTest));

	i = 0;
	j = 1;

	// Copy NUM
	pbyTest[i] = pbyBuf[j];

	// Copy data bytes
	do
	{
		i++;
		j++;

		pbyTest[i] = pbyBuf[j];
	}
	while(pbyBuf[j] != ETX);

	// Copy ETX byte
	pbyTest[i++] = ETX;

	// CRC1 (lowbyte)
	CRC1 = pbyBuf[++j];

	// CRC2 (highbyte)
	CRC2 = pbyBuf[++j];

	recvCRC = (USHORT)CRC2 << 8;
	recvCRC += CRC1;

	// Calculate checksum
	setPoly(DIN);
	calcCRC = getCRC16(pbyTest, i, 0);

	if(recvCRC == calcCRC)
	{
		printf("CRC match, OK\n");
		return TRUE;
	}
	else
	{
		printf("CRC no match, FAIL\n");
		return FALSE;
	}
}

static USHORT CalcCRC(BYTE* pbyBuf)
{
	// Packet format:
	// STX | NUM | ... data bytes ... | ETX | CRC1 (lowbyte) | CRC2 (highbyte)
	// The CRC must be calculated over:
	// NUM | ... data bytes ... | ETX

	int i, j;
	BYTE pbyTest[4096];
	USHORT calcCRC;

	memset(pbyTest, 0, sizeof(pbyTest));

	i = 0;
	j = 1;

	// Copy NUM
	pbyTest[i] = pbyBuf[j];

	// Copy data bytes
	do
	{
		i++;
		j++;

		pbyTest[i] = pbyBuf[j];
	}
	while(pbyBuf[j] != ETX);

	// Copy ETX byte
	pbyTest[i++] = ETX;

	// Calculate checksum
	setPoly(DIN);
	calcCRC = getCRC16(pbyTest, i, 0);

	return calcCRC;
}

static void ExtractPayload(BYTE* pbyBuf, char* pcPayLoad)
{
	// Message format:
	// STX | NUM | ... data bytes ... | ETX | CRC1 (lowbyte) | CRC2 (highbyte)
	// Here we need to extract the data bytes

	int i, j;

	i = 0;
	j = 2;

	while(pbyBuf[j] != ETX)
	{
		pcPayLoad[i++] = pbyBuf[j++];
    }
}
