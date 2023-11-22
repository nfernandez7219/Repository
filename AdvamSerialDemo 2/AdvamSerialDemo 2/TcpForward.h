#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "windows.h"

void ConnectionOpen(BYTE* pbyDestHost, BYTE* pbyDestPort);
void SendMsg(BYTE* pbyMsgAsciiSend);
int RecvMsg(BYTE* pbyMsgAsciiRecv);
void ConnectionClose();