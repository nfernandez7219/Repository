#pragma once

typedef enum
{
	CMD_INIT,
	CMD_LINK_TEST,
	CMD_TICKET_READ,
	CMD_PAYMENT_CARD_DET,
	CMD_PAYMENT,
	CMD_PAYMENT_CONF,
	CMD_CARD_EJECT,
	CMD_DISABLE_READER,	
	CMD_SETTLEMENT,		
	CMD_POWEROFF,
    CMD_DISPLAY_MSG,
	CMD_TCP_OPEN,
	CMD_TCP_SEND,
	CMD_TCP_RECV,
	CMD_TCP_CLOSE
} TERMINAL_CMD;

typedef enum
{
	STATS_OK = 0,
	STATS_FAIL,
	STATS_TIMEOUT,
	STATS_CANCEL,
	STATS_CARDREADFAILED,
	STATS_READER_NOT_EMPTY,
	STATS_CARD_NOT_ALLOWED,
	STATS_TERMINAL_ERR = 9
} TERMINAL_STATS;

typedef struct
{
	TERMINAL_CMD Cmd;
	BYTE byVersion;
	BYTE byTimeout;
	BYTE pbyAmount[8+1];
	BYTE pbySurcharge[256+1];
	BYTE pbyEquipId[12+1];
	BYTE pbyTxRef[24+1];
	BYTE pbyStan[6+1];
	TERMINAL_STATS Stats;
	BYTE pbyConId[4+1];
	BYTE pbyEvent[2+1];
	BYTE pbyTcpMsg[3072];
} TERMINAL_REQ;

typedef struct
{
	TERMINAL_CMD Cmd;
	BYTE byVersion;
	TERMINAL_STATS Stats;
	BYTE byDisplayId;
	BYTE pbyDisplayText[64];
	BYTE pbyResponseCode[1+1];
	BYTE pbyResponseText[64];
	BYTE pbyStan[6+1];
	BYTE pbyAuth[64];
	BYTE pbyTruncCardData[19+1];
	BYTE pbySurchargeId[8+1];
	BYTE pbyCardType[64];
	BYTE pbyReceipt[2048];
	BYTE pbyDestHost[64];
	BYTE pbyDestPort[64];
	BYTE pbyTcpMsg[3072];
} TERMINAL_RES;


void PMInit(void);
void PMDispose(void);
BYTE PMGetVersion(void);

void PMSend(TERMINAL_REQ* pReq);
void PMSendRes(TERMINAL_RES* pRes); // hack...
int PMRecv(TERMINAL_RES* pRes);
