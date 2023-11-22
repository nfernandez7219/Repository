#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <process.h>
#include "windows.h"

#include "ProtocolSerial.h"
#include "ProtocolMessages.h"
#include "TcpForward.h"

#define CUR_VERSN 1

#define STR_INIT_REQ "INIT_REQ"
#define STR_LINK_REQ "LINK_REQ"
#define STR_TKTREAD_REQ "TKTREAD_REQ"
#define STR_PMTDET_REQ "PMTDET_REQ"
#define STR_PAYMENT_REQ "PMT_REQ"
#define STR_PAYMENT_CONF "PMT_CONF"
#define STR_EJECT_REQ "EJECT_REQ"
#define STR_DISABLE_REQ "DISABLE_REQ"
#define STR_SETT_REQ "SETT_REQ"
#define STR_POWEROFF_REQ "POWEROFF_REQ"
#define STR_DISP_REQ "DISP_REQ"

#define STR_PMTDET_CAN "PMTDET_CAN"
#define STR_TKTREAD_CAN "TKTREAD_CAN"
#define STR_EJECT_CAN "EJECT_CAN"

#define STR_INIT_RES "INIT_RES"
#define STR_LINK_RES "LINK_RES"
#define STR_TKTREAD_RES "TKTREAD_RES"
#define STR_PMTDET_RES "PMTDET_RES"
#define STR_PAYMENT_RES "PMT_RES"
#define STR_EJECT_RES "EJECT_RES"
#define STR_DISABLE_RES "DISABLE_RES"
#define STR_SETT_RES "SETT_RES"

#define STR_TCPOPEN_REQ "TCPOPEN_REQ"
#define STR_TCPRECV_REQ "TCPRECV_REQ"
#define STR_TCPSEND_REQ "TCPSEND_REQ"
#define STR_TCPCLOSE_REQ "TCPCLOSE_REQ"

#define STR_TCPOPEN_RES "TCPOPEN_RES"
#define STR_TCPRECV_RES "TCPRECV_RES"
#define STR_TCPSEND_RES "TCPSEND_RES"
#define STR_TCPCLOSE_RES "TCPCLOSE_RES"

#define TAG_MSGID "MSGID"
#define TAG_VERSN "VERSN"
#define TAG_STATS "STATS"
#define TAG_AMNTX "AMTTX"
#define TAG_SURCH "SURCH"
#define TAG_TMOUT "TMOUT"
#define TAG_EQUIP "EQUIP"
#define TAG_TXREF "TXREF"
#define TAG_CRDHA "CRDHA"
#define TAG_CRDTD "CRDTD"
#define TAG_PRFIX "PRFIX"
#define TAG_ISPAY "ISPAY"
#define TAG_DISID "DISID"
#define TAG_DTEXT "DTEXT"
#define TAG_RCODE "RCODE"
#define TAG_RTEXT "RTEXT"
#define TAG_TSTAN "TSTAN"
#define TAG_TCARD "TCARD"
#define TAG_RECPT "RECPT"
#define TAG_SCHID "SCHID"
#define TAG_CTYPE "CTYPE"
#define TAG_TAUTH "TAUTH"
#define TAG_DHOST "DHOST"
#define TAG_DPORT "DPORT"
#define TAG_CONID "CONID"
#define TAG_EVENT "EVENT"
#define TAG_TCPMG "TCPMG"

static void Serialize(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializeRes(TERMINAL_RES* pRes, BYTE* pbyMsg);
static void SerializeInitReq(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializeInitRes(TERMINAL_RES* pRes, BYTE* pbyMsg);
static void SerializeLinkTestReq(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializePmtDetReq(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializeTktReadReq(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializePmtReq(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializePmtConf(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializeEjectReq(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializeDisableReq(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializeSettlementReq(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializePowerOffReq(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializeTcpOpenRes(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializeTcpSendRes(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializeTcpRecvReq(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void SerializeTcpCloseRes(TERMINAL_REQ* pReq, BYTE* pbyMsg);
static void DeSerialize(BYTE* pbyMsg, TERMINAL_RES* pRes);
static WORD ConstructTLV(BYTE* pbyTag, BYTE* pbyValue, BYTE* pbyTLV);
static void ConvertStr2Cmd(BYTE* pbyStr, TERMINAL_CMD* pCmd);

void PMInit(void)
{
	printf("vPMInit -\n");

	PSInit();
}

void PMDispose(void)
{
	PSDispose();
}

BYTE PMGetVersion(void)
{
	return CUR_VERSN;
}

void PMSend(TERMINAL_REQ* pReq)
{
	BYTE pbyMsg[4096];

	memset(pbyMsg, 0, sizeof(pbyMsg));
	Serialize(pReq, pbyMsg);
	printf("Sending: %s\n", pbyMsg);
	PSWrite(pbyMsg);
}

void PMSendRes(TERMINAL_RES* pRes)
{
	// todo: hack, INIT has become bi-directional, so maybe rename PMSend to PMSendReq etc...

	BYTE pbyMsg[4096];

	memset(pbyMsg, 0, sizeof(pbyMsg));
	SerializeRes(pRes, pbyMsg);
	printf("Sending: %s\n", pbyMsg);
	PSWrite(pbyMsg);
}

int PMRecv(TERMINAL_RES* pRes)
{
	BYTE pbyMsg[4096];

	memset(pbyMsg, 0, sizeof(pbyMsg));
	if(PSRead(pbyMsg))
	{
		printf("Received: %s\n", pbyMsg);
		DeSerialize(pbyMsg, pRes);
		return 1;
	}
	else
	{
		return 0;
	}
}

static void Serialize(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	switch(pReq->Cmd)
	{
	case CMD_INIT:
		SerializeInitReq(pReq, pbyMsg);
		break;

	case CMD_LINK_TEST:
		SerializeLinkTestReq(pReq, pbyMsg);
		break;

	case CMD_PAYMENT_CARD_DET:
		SerializePmtDetReq(pReq, pbyMsg);
		break;

	case CMD_TICKET_READ:
		SerializeTktReadReq(pReq, pbyMsg);
		break;

	case CMD_PAYMENT:
		SerializePmtReq(pReq, pbyMsg);
		break;

	case CMD_PAYMENT_CONF:
		SerializePmtConf(pReq, pbyMsg);
		break;

	case CMD_CARD_EJECT:
		SerializeEjectReq(pReq, pbyMsg);
		break;

	case CMD_DISABLE_READER:
		SerializeDisableReq(pReq, pbyMsg);
		break;

	case CMD_SETTLEMENT:
		SerializeSettlementReq(pReq, pbyMsg);
		break;

	case CMD_POWEROFF:
		SerializePowerOffReq(pReq, pbyMsg);
		break;

	case CMD_TCP_OPEN:
		SerializeTcpOpenRes(pReq, pbyMsg);
		break;

	case CMD_TCP_SEND:
		SerializeTcpSendRes(pReq, pbyMsg);
		break;

	case CMD_TCP_RECV:
		SerializeTcpRecvReq(pReq, pbyMsg);
		break;

	case CMD_TCP_CLOSE:
		SerializeTcpCloseRes(pReq, pbyMsg);
		break;

	default:
		break;
	}
}

static void SerializeRes(TERMINAL_RES* pRes, BYTE* pbyMsg)
{
	// hack continued...

	switch(pRes->Cmd)
	{
	case CMD_INIT:
		SerializeInitRes(pRes, pbyMsg);
		break;

	default:
		break;
	}
}

static void SerializeInitReq(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE pbyValue[16];
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	pbyTLV += ConstructTLV(TAG_MSGID, STR_INIT_REQ, pbyTLV);

	// TAG_VERSN
	sprintf(pbyValue, "%d", pReq->byVersion);
	ConstructTLV(TAG_VERSN, pbyValue, pbyTLV);
}

static void SerializeInitRes(TERMINAL_RES* pRes, BYTE* pbyMsg)
{
	BYTE pbyValue[16];
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	pbyTLV += ConstructTLV(TAG_MSGID, STR_INIT_RES, pbyTLV);

	// TAG_VERSN
	sprintf(pbyValue, "%d", pRes->byVersion);
	ConstructTLV(TAG_VERSN, pbyValue, pbyTLV);
}

static void SerializeLinkTestReq(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	ConstructTLV(TAG_MSGID, STR_LINK_REQ, pbyTLV);
}

static void SerializePmtDetReq(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE pbyValue[16];
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	pbyTLV += ConstructTLV(TAG_MSGID, STR_PMTDET_REQ, pbyTLV);

	// TAG_TMOUT
	sprintf(pbyValue, "%d", pReq->byTimeout);
	pbyTLV += ConstructTLV(TAG_TMOUT, pbyValue, pbyTLV);

	// TAG_AMNTX
	pbyTLV += ConstructTLV(TAG_AMNTX, pReq->pbyAmount, pbyTLV);

	// TAG_SURCH
	ConstructTLV(TAG_SURCH, pReq->pbySurcharge, pbyTLV);
}

static void SerializeTktReadReq(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE pbyValue[16];
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	pbyTLV += ConstructTLV(TAG_MSGID, STR_TKTREAD_REQ, pbyTLV);

	// TAG_TMOUT
	sprintf(pbyValue, "%d", pReq->byTimeout);
	ConstructTLV(TAG_TMOUT, pbyValue, pbyTLV);
}

static void SerializePmtReq(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	pbyTLV += ConstructTLV(TAG_MSGID, STR_PAYMENT_REQ, pbyTLV);

	// TAG_AMNTX
	pbyTLV += ConstructTLV(TAG_AMNTX, pReq->pbyAmount, pbyTLV);

	// TAG_SURCH
	pbyTLV += ConstructTLV(TAG_SURCH, pReq->pbySurcharge, pbyTLV);

	// TAG_EQUIP
	pbyTLV += ConstructTLV(TAG_EQUIP, pReq->pbyEquipId, pbyTLV);

	// TAG_TXREF
	ConstructTLV(TAG_TXREF, pReq->pbyTxRef, pbyTLV);
}

static void SerializePmtConf(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE pbyValue[16];
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	pbyTLV += ConstructTLV(TAG_MSGID, STR_PAYMENT_CONF, pbyTLV);

	// TAG_TSTAN
	pbyTLV += ConstructTLV(TAG_TSTAN, pReq->pbyStan, pbyTLV);

	// TAG_STATS
	sprintf(pbyValue, "%d", pReq->Stats);
	ConstructTLV(TAG_STATS, pbyValue, pbyTLV);
}

static void SerializeEjectReq(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE pbyValue[16];
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	pbyTLV += ConstructTLV(TAG_MSGID, STR_EJECT_REQ, pbyTLV);

	// TAG_TMOUT
	sprintf(pbyValue, "%d", pReq->byTimeout);
	ConstructTLV(TAG_TMOUT, pbyValue, pbyTLV);
}

static void SerializeDisableReq(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	ConstructTLV(TAG_MSGID, STR_DISABLE_REQ, pbyTLV);
}

static void SerializeSettlementReq(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	ConstructTLV(TAG_MSGID, STR_SETT_REQ, pbyTLV);
}

static void SerializePowerOffReq(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	ConstructTLV(TAG_MSGID, STR_POWEROFF_REQ, pbyTLV);
}

static void SerializeTcpOpenRes(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE pbyValue[16];
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	pbyTLV += ConstructTLV(TAG_MSGID, STR_TCPOPEN_RES, pbyTLV);

	// TAG_STATS
	sprintf(pbyValue, "%d", pReq->Stats);
	pbyTLV += ConstructTLV(TAG_STATS, pbyValue, pbyTLV);

	// TAG_CONID
	ConstructTLV(TAG_CONID, pReq->pbyConId, pbyTLV);
}

static void SerializeTcpSendRes(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE pbyValue[16];
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	pbyTLV += ConstructTLV(TAG_MSGID, STR_TCPSEND_RES, pbyTLV);

	// TAG_STATS
	sprintf(pbyValue, "%d", pReq->Stats);
	ConstructTLV(TAG_STATS, pbyValue, pbyTLV);
}

static void SerializeTcpRecvReq(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	pbyTLV += ConstructTLV(TAG_MSGID, STR_TCPRECV_REQ, pbyTLV);

	// TAG_CONID
	pbyTLV += ConstructTLV(TAG_CONID, pReq->pbyConId, pbyTLV);

	// TAG_EVENT
	pbyTLV += ConstructTLV(TAG_EVENT, pReq->pbyEvent, pbyTLV);

	// TAG_TCPMG
	ConstructTLV(TAG_TCPMG, pReq->pbyTcpMsg, pbyTLV);
}

static void SerializeTcpCloseRes(TERMINAL_REQ* pReq, BYTE* pbyMsg)
{
	BYTE pbyValue[16];
	BYTE* pbyTLV;

	pbyTLV = pbyMsg;

	// TAG_MSGID	
	pbyTLV += ConstructTLV(TAG_MSGID, STR_TCPCLOSE_RES, pbyTLV);

	// TAG_STATS
	sprintf(pbyValue, "%d", pReq->Stats);
	ConstructTLV(TAG_STATS, pbyValue, pbyTLV);
}

static void DeSerialize(BYTE* pbyMsg, TERMINAL_RES* pRes)
{
	BYTE* pbyBuf;
	BYTE pbyTag[5+1];
	BYTE pbyLen[3+1];
	BYTE pbyValue[4096];
	DWORD dwLen;
	WORD i;

	i = 0;
	pbyBuf = pbyMsg;

	while(*pbyBuf != 0x00)
	{
		memset(pbyTag, 0, sizeof(pbyTag));
		memset(pbyLen, 0, sizeof(pbyLen));
		memset(pbyValue, 0, sizeof(pbyValue));

		// Get tag
		strncpy(pbyTag, pbyBuf, 5);
		pbyBuf += 5;
		printf("Found tag: %s\n", pbyTag);
		i++;

		// Get length (as string)
		strncpy(pbyLen, pbyBuf, 3);
		pbyBuf += 3;
		printf("With length (as string): %s\n", pbyLen);

		// Convert length to number
		sscanf(pbyLen, "%03X", &dwLen);
		printf("With length (as number): %d\n", dwLen);

		// Get value
		strncpy(pbyValue, pbyBuf, dwLen);
		pbyBuf += dwLen;
		printf("With value: %s\n", pbyValue);

		if(strcmp(pbyTag, TAG_MSGID) == 0)
		{
			ConvertStr2Cmd(pbyValue, &pRes->Cmd);
		}
		else if(strcmp(pbyTag, TAG_VERSN) == 0)
		{
			pRes->byVersion = atoi(pbyValue);
		}
		//else if(strcmp(pbyTag, TAG_AMNTX) == 0)
		//{
		//}
		//else if(strcmp(pbyTag, TAG_SURCH) == 0)
		//{
		//}
		//else if(strcmp(pbyTag, TAG_EQUIP) == 0)
		//{
		//}
		//else if(strcmp(pbyTag, TAG_TXREF) == 0)
		//{
		//}
		else if(strcmp(pbyTag, TAG_CRDHA) == 0)
		{
		}
		else if(strcmp(pbyTag, TAG_CRDTD) == 0)
		{
		}
		else if(strcmp(pbyTag, TAG_PRFIX) == 0)
		{
		}
		else if(strcmp(pbyTag, TAG_ISPAY) == 0)
		{
		}
		else if(strcmp(pbyTag, TAG_DISID) == 0)
		{
			pRes->byDisplayId = atoi(pbyValue);
		}
		else if(strcmp(pbyTag, TAG_DTEXT) == 0)
		{
			strcpy(pRes->pbyDisplayText, pbyValue);
		}
		else if(strcmp(pbyTag, TAG_RCODE) == 0)
		{
			strcpy(pRes->pbyResponseCode, pbyValue);
		}
		else if(strcmp(pbyTag, TAG_RTEXT) == 0)
		{
			strcpy(pRes->pbyResponseText, pbyValue);
		}
		else if(strcmp(pbyTag, TAG_TSTAN) == 0)
		{
			strcpy(pRes->pbyStan, pbyValue);
		}
		else if(strcmp(pbyTag, TAG_TCARD) == 0)
		{
			strcpy(pRes->pbyTruncCardData, pbyValue);
		}
		else if(strcmp(pbyTag, TAG_RECPT) == 0)
		{
			strcpy(pRes->pbyReceipt, pbyValue);
		}
		else if(strcmp(pbyTag, TAG_SCHID) == 0)
		{
			strcpy(pRes->pbySurchargeId, pbyValue);
		}
		else if(strcmp(pbyTag, TAG_CTYPE) == 0)
		{
			strcpy(pRes->pbyCardType, pbyValue);
		}
		else if(strcmp(pbyTag, TAG_TAUTH) == 0)
		{
			strcpy(pRes->pbyAuth, pbyValue);
		}
		else if(strcmp(pbyTag, TAG_STATS) == 0)
		{
			pRes->Stats = atoi(pbyValue);
		}
		else if(strcmp(pbyTag, TAG_DHOST) == 0)
		{
			strcpy(pRes->pbyDestHost, pbyValue);
			printf("pRes->pbyDestHost: %s\n", pRes->pbyDestHost);
		}
		else if(strcmp(pbyTag, TAG_DPORT) == 0)
		{
			strcpy(pRes->pbyDestPort, pbyValue);
			printf("pRes->pbyDestPort: %s\n", pRes->pbyDestPort);
		}
		else if(strcmp(pbyTag, TAG_TCPMG) == 0)
		{
			strcpy(pRes->pbyTcpMsg, pbyValue);
			printf("pRes->pbyTcpMsg: %s\n", pRes->pbyTcpMsg);
		}
		else
		{
			printf("Unknown tag found: %s\n", pbyTag);
		}
	}

	printf("Number of tags found: %d\n", i);
}

static WORD ConstructTLV(BYTE* pbyTag, BYTE* pbyValue, BYTE* pbyTLV)
{
	WORD wLen;

	wLen = (WORD)strlen(pbyValue);

	sprintf(pbyTLV, "%s%03x%s", pbyTag, wLen, pbyValue);

	wLen = (WORD)strlen(pbyTLV);

	return wLen;
}

static void ConvertStr2Cmd(BYTE* pbyStr, TERMINAL_CMD* pCmd)
{
	if(strcmp(pbyStr, STR_INIT_RES) == 0)
	{
		*pCmd = CMD_INIT;
	}
	else if(strcmp(pbyStr, STR_LINK_RES) == 0)
	{
		*pCmd = CMD_LINK_TEST;
	}
	else if(strcmp(pbyStr, STR_TKTREAD_RES) == 0)
	{
		*pCmd = CMD_TICKET_READ;
	}
	else if(strcmp(pbyStr, STR_PMTDET_RES) == 0)
	{
		*pCmd = CMD_PAYMENT_CARD_DET;
	}
	else if(strcmp(pbyStr, STR_PAYMENT_RES) == 0)
	{
		*pCmd = CMD_PAYMENT;
	}
	else if(strcmp(pbyStr, STR_EJECT_RES) == 0)
	{
		*pCmd = CMD_CARD_EJECT;
	}
	else if(strcmp(pbyStr, STR_DISABLE_RES) == 0)
	{
		*pCmd = CMD_DISABLE_READER;
	}
	else if(strcmp(pbyStr, STR_SETT_RES) == 0)
	{
		*pCmd = CMD_SETTLEMENT;
	}
	else if(strcmp(pbyStr, STR_TCPOPEN_REQ) == 0)
	{
		*pCmd = CMD_TCP_OPEN;
	}
	else if(strcmp(pbyStr, STR_TCPSEND_REQ) == 0)
	{
		*pCmd = CMD_TCP_SEND;
	}
	else if(strcmp(pbyStr, STR_TCPRECV_RES) == 0)
	{
		*pCmd = CMD_TCP_RECV;
	}
	else if(strcmp(pbyStr, STR_TCPCLOSE_REQ) == 0)
	{
		*pCmd = CMD_TCP_CLOSE;
	}
	else if(strcmp(pbyStr, STR_INIT_REQ) == 0)
	{
		*pCmd = CMD_INIT;
	}
	else
	{
		printf("Unknown string found: %s\n", pbyStr);
	}
}