#include <Winsock2.h>

#include "TcpForward.h"

static void HexEncode(char* dest, unsigned char* src, int src_len);
static int HexDecode(unsigned char* dest, char* src);

static SOCKET sd = NULL;
static WSADATA wsaData;

void ConnectionOpen(BYTE* pbyDestHost, BYTE* pbyDestPort)
{
	int ret;
	struct sockaddr_in SockAddr;
	u_long iMode = 1;
	BYTE byDestIpAddr[64+1];

	if ((ret = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", ret);
		WSACleanup();
		return;
	}

	// todo: use pbyDestHost, dns lookup etc.
	//strcpy(byDestIpAddr, "10.88.235.11");
	//strcpy(byDestIpAddr, "118.151.216.16"); // demo-tms.advam.com
	//strcpy(byDestIpAddr, "118.151.216.17"); // demo-tms2.advam.com
	//strcpy(byDestIpAddr, "118.151.216.16"); // demo-tss.advam.com
	//strcpy(byDestIpAddr, "118.151.216.17"); // demo-tss2.advam.com

	if( strcmp(pbyDestHost, "demo-tms.advam.com") == 0 )
	{
		printf("%s - demo-tms.advam.com\n", __FUNCTION__);
		strcpy(byDestIpAddr, "118.151.216.16");
	}
	else if( strcmp(pbyDestHost, "demo-tms2.advam.com") == 0 )
	{
		printf("%s - demo-tms2.advam.com\n", __FUNCTION__);
		strcpy(byDestIpAddr, "118.151.216.17");
	}
	else if( strcmp(pbyDestHost, "demo-tss.advam.com") == 0 )
	{
		printf("%s - demo-tss.advam.com\n", __FUNCTION__);
		strcpy(byDestIpAddr, "118.151.216.16");
	}
	else if( strcmp(pbyDestHost, "demo-tss2.advam.com") == 0 )
	{
		printf("%s - demo-tss2.advam.com\n", __FUNCTION__);
		strcpy(byDestIpAddr, "118.151.216.17");
	}
	else if( strcmp(pbyDestHost, "tms.advam.com") == 0 )
	{
		printf("%s - tms.advam.com\n", __FUNCTION__);
		strcpy(byDestIpAddr, "118.151.216.128");
	}
	else if( strcmp(pbyDestHost, "tms2.advam.com") == 0 )
	{
		printf("%s - tms2.advam.com\n", __FUNCTION__);
		strcpy(byDestIpAddr, "118.151.218.128");
	}
	else if( strcmp(pbyDestHost, "tss.advam.com") == 0 )
	{
		printf("%s - tss.advam.com\n", __FUNCTION__);
		strcpy(byDestIpAddr, "118.151.216.128");
	}
	else if( strcmp(pbyDestHost, "tss2.advam.com") == 0 )
	{
		printf("%s - tss2.advam.com\n", __FUNCTION__);
		strcpy(byDestIpAddr, "118.151.218.128");
	}
	else
	{
		printf("%s - baked gateway\n", __FUNCTION__);
		strcpy(byDestIpAddr, "10.88.235.11");
	}


	sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sd != INVALID_SOCKET)
	{
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_port = htons((u_short)atoi((char*)pbyDestPort));
		SockAddr.sin_addr.s_addr = inet_addr((char*)byDestIpAddr);
		if (connect(sd, (struct sockaddr*)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
		{
			sd = INVALID_SOCKET;
			printf("SOCKET_ERROR");
			WSACleanup();
			return;
		} 
	}

	//printf("%s - ******* test delay *******\n", __FUNCTION__);
	//Sleep(40 * 1000);
	printf("ConnectionOpen OK\n");

	ioctlsocket(sd, FIONBIO, &iMode);
}

void SendMsg(BYTE* pbyMsgAsciiSend)
{
	int ret;
	char buff[3072];

	//printf("%s - ******* test delay *******\n", __FUNCTION__);
	//Sleep(15 * 1000);

	memset(buff, 0, sizeof(buff));
	ret = HexDecode(buff, pbyMsgAsciiSend);
	send(sd, buff, ret, 0);
}

int RecvMsg(BYTE* pbyMsgAsciiRecv)
{
	int ret;
	//char buff[1460]; // Don't exceed 1460 when sending to terminal.
	char buff[1460];

	memset(buff, 0, sizeof(buff));
	ret = recv(sd, buff, sizeof(buff), 0);

	//printf("%s - ******* test delay *******\n", __FUNCTION__);
	//Sleep(15 * 1000);

	if(ret > 0)
	{
		HexEncode(pbyMsgAsciiRecv, buff, ret);
		return 1;
	}
	else
	{
		return 0;
	}
}

void ConnectionClose()
{
		closesocket(sd);
		WSACleanup();

		printf("ConnectionClose OK\n");
}

static void HexEncode(char* dest, unsigned char* src, int src_len)
{
	// Input: src: the hex byte string to be converted to plain ascii.
	// Output: dest: the zero terminated ascii string representing the hex byte string.

	char hex[3+1];
	int i;

	for(i = 0; i < src_len; i++)
	{
		sprintf(hex,"%02X", src[i]);
		strcat(dest, hex);
	}
}

static int HexDecode(unsigned char* dest, char* src)
{
	// Input: src: the zero terminated ascii string to be converted to hex bytes.
	// Output: dest: hex byte string.
	// Returns: length of the hex byte string.

	// ascii '0' - '9' --> 0x30 - 0x39
	// "gap" of 7      --> 0x3A - 0x40
	// ascii 'A' - 'F' --> 0x41 - 0x46

	int i, src_len;
	char hinibble;
	char lonibble;
	unsigned char* pdest;

	src_len = (int)strlen(src);
	pdest = dest;

	for(i = 0; i < src_len; i += 2)
	{
		hinibble = src[i];
		lonibble = src[i + 1];

		hinibble -= '0';
		if(hinibble > 9)
			hinibble -= 7;

		lonibble -= '0';
		if(lonibble > 9)
			lonibble -= 7;

		*pdest = (hinibble << 4) + lonibble;
		pdest++;
	}

	return i / 2; // length of binary
}