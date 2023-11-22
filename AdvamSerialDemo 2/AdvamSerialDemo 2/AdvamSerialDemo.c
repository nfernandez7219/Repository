
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <process.h>
#include "windows.h"

#include "ProtocolMessages.h"
#include "TcpForward.h"
#include "MemQueue.h"

static TERMINAL_REQ Req;
static TERMINAL_RES Res;

static MEM_QUEUE msg_q_app2loop;
static HANDLE msg_q_app2loop_mutex;
static void QueueApp2Loop(void* data);
static MEM_NODE* CheckQueueApp2Loop();

static MEM_QUEUE msg_q_loop2app;
static HANDLE msg_q_loop2app_mutex;
static void QueueLoop2App(void* data);
static MEM_NODE* CheckQueueLoop2App();

static unsigned tid;
static HANDLE hThread;
static unsigned __stdcall thrAppLoop(void* p);
static void AppLoop(void);
static void WaitSyncLoop(TERMINAL_RES *term_res);

static void OnStartup(void);
static void CmdInit(void);
static void CmdLinkTest(void);
static void CmdPmtDet(void);
static void CmdTktRead(void);
static void CmdPmt(void);
static void CmdPmtConf(void);
static void CmdEject(void);
static void CmdDisable(void);
static void CmdSettlement(void);
static void CmdPowerOff(void);

static void DumpReq(TERMINAL_REQ* pReq);
static void DumpRes(TERMINAL_RES* pRes);

static BYTE pbyStanCurrent[6+1];

int main(void)
{
	BYTE byBufGeneric[256] = {0};

	strcpy(byBufGeneric, "AdvamSerialDemo - v0.0.2\n");
	printf(byBufGeneric);

	OnStartup();

	strcpy(byBufGeneric, "Press any key to continue to the Payment cycle\n");
	printf(byBufGeneric);
	scanf("%c", byBufGeneric);

	CmdInit();
	WaitSyncLoop(&Res);

	//CmdLinkTest();
	//WaitSyncLoop(&Res);

	//CmdSettlement();
	//WaitSyncLoop(&Res);

	//CmdPowerOff();
	//WaitSyncLoop(&Res);
	//WaitForSingleObject(hThread, INFINITE);

	while(1)
	{
		strcpy(byBufGeneric, "Press any key to continue to the Payment cycle\n");
		printf(byBufGeneric);
		scanf("%c", byBufGeneric);

		QueueFreeAll(&msg_q_loop2app);

		//CmdPmtDet();
		//do
		//{
		//	WaitSyncLoop(&Res);
		//}
		//while(Res.Cmd != CMD_PAYMENT_CARD_DET);

		CmdTktRead();
		do
		{
			WaitSyncLoop(&Res);
		}
		while(Res.Cmd != CMD_TICKET_READ);

		CmdPmt();
		do
		{
			WaitSyncLoop(&Res);
		}
		while(Res.Cmd != CMD_PAYMENT);

		if(Res.Stats == STATS_OK)
		{
			strcpy(pbyStanCurrent, Res.pbyStan);
			CmdPmtConf();
		}

		CmdEject();
		WaitSyncLoop(&Res);

		//CmdDisable();
		//WaitSyncLoop(&Res);
	}
	//WaitForSingleObject(hThread, INFINITE);

	PMDispose();

	return 0;
}

static void OnStartup(void)
{
	PMInit();

	memset(&msg_q_app2loop, 0, sizeof(msg_q_app2loop));
	QueueInit(&msg_q_app2loop);
	msg_q_app2loop_mutex = CreateMutex(NULL, FALSE, "msg_q_app2loop_mutex");

	hThread = (HANDLE) _beginthreadex(NULL, 0, thrAppLoop, NULL, 0, &tid);

	Sleep(2000);
}

static void CmdInit(void)
{
	memset(&Req, 0, sizeof(Req));

	Req.Cmd = CMD_INIT;
	Req.byVersion = PMGetVersion();

	QueueApp2Loop(&Req);
}

static void CmdLinkTest(void)
{
	memset(&Req, 0, sizeof(Req));

	Req.Cmd = CMD_LINK_TEST;
	
	QueueApp2Loop(&Req);
}

static void CmdPmtDet(void)
{
	memset(&Req, 0, sizeof(Req));

	Req.Cmd = CMD_PAYMENT_CARD_DET;
	Req.byTimeout = 60;
	strcpy(Req.pbyAmount, "1.00");
	//strcpy(Req.pbyAmount, "16.80");
	strcpy(Req.pbySurcharge, ""); // No surcharge set in this example.
	
	QueueApp2Loop(&Req);
}

static void CmdTktRead(void)
{
	memset(&Req, 0, sizeof(Req));

	Req.Cmd = CMD_TICKET_READ;
	Req.byTimeout = 30;
	
	QueueApp2Loop(&Req);
}

static void CmdPmt(void)
{
	memset(&Req, 0, sizeof(Req));

	Req.Cmd = CMD_PAYMENT;
	strcpy(Req.pbyAmount, "1.00");
	//strcpy(Req.pbyAmount, "16.80");
	strcpy(Req.pbySurcharge, ""); // No surcharge set in this example.
	strcpy(Req.pbyEquipId, "SPRINGER");
	strcpy(Req.pbyTxRef, "aabbccddeeff");
	
	QueueApp2Loop(&Req);
}

static void CmdPmtConf(void)
{
	memset(&Req, 0, sizeof(Req));

	Req.Cmd = CMD_PAYMENT_CONF;
	strcpy(Req.pbyStan, pbyStanCurrent);
	Req.Stats = STATS_OK;
	
	QueueApp2Loop(&Req);
}

static void CmdEject(void)
{
	memset(&Req, 0, sizeof(Req));

	Req.Cmd = CMD_CARD_EJECT;
	Req.byTimeout = 60;
	
	QueueApp2Loop(&Req);
}

static void CmdDisable(void)
{
	memset(&Req, 0, sizeof(Req));

	Req.Cmd = CMD_DISABLE_READER;
	
	QueueApp2Loop(&Req);
}

static void CmdSettlement(void)
{
	memset(&Req, 0, sizeof(Req));

	Req.Cmd = CMD_SETTLEMENT;
	
	QueueApp2Loop(&Req);
}

static void CmdPowerOff(void)
{
	memset(&Req, 0, sizeof(Req));

	Req.Cmd = CMD_POWEROFF;
	
	QueueApp2Loop(&Req);
}

static void DumpReq(TERMINAL_REQ* pReq)
{
	// todo
}

static void DumpRes(TERMINAL_RES* pRes)
{
	// todo
}

static void QueueApp2Loop(void* data)
{
	MEM_NODE* m;

	printf("%s -\n", __FUNCTION__);

	WaitForSingleObject(msg_q_app2loop_mutex, INFINITE);

	m = QueueAllocMemNode(4096);
	memcpy(m->data, data, 4096);
	m->size_orginal = 4096;

	QueuePut(&msg_q_app2loop, m);

	ReleaseMutex(msg_q_app2loop_mutex);
}

static MEM_NODE* CheckQueueApp2Loop()
{
	MEM_NODE* m;

	//printf("%s -\n", __FUNCTION__);

	WaitForSingleObject(msg_q_app2loop_mutex, INFINITE);

	m = NULL;
	if(QueuePeek(&msg_q_app2loop) != NULL)
	{
		m = QueueRemove(&msg_q_app2loop);
	}

	ReleaseMutex(msg_q_app2loop_mutex);

	return m;
}

static void QueueLoop2App(void* data)
{
	MEM_NODE* m;

	printf("%s -\n", __FUNCTION__);

	WaitForSingleObject(msg_q_loop2app_mutex, INFINITE);

	m = QueueAllocMemNode(4096);
	memcpy(m->data, data, 4096);
	m->size_orginal = 4096;

	QueuePut(&msg_q_loop2app, m);

	ReleaseMutex(msg_q_loop2app_mutex);
}

static MEM_NODE* CheckQueueLoop2App()
{
	MEM_NODE* m;

	//printf("%s -\n", __FUNCTION__);

	WaitForSingleObject(msg_q_loop2app_mutex, INFINITE);

	m = NULL;
	if(QueuePeek(&msg_q_loop2app) != NULL)
	{
		m = QueueRemove(&msg_q_loop2app);
	}

	ReleaseMutex(msg_q_loop2app_mutex);

	return m;
}

static unsigned __stdcall thrAppLoop(void* p)
{
	AppLoop();

	_endthreadex(0);
	return 0;
}

static void AppLoop(void)
{
	int res_recv_serial;
	int res_recv_tcp;
	int flag_check_tcp_recv = 0;
	MEM_NODE *m_app2loop;
	TERMINAL_REQ term_req;
	TERMINAL_RES term_res;
	BYTE byTcpMsgRecv[3072];
	int flag_serial_in_progress = 0;

	printf("%s - start looping\n", __FUNCTION__);

	while(1)
	{
		m_app2loop = NULL;
		res_recv_serial = 0;
		res_recv_tcp = 0;

		memset(&term_res, 0, sizeof(term_res));

		do
		{
			m_app2loop = CheckQueueApp2Loop();

			res_recv_serial = PMRecv(&term_res);

			if(flag_check_tcp_recv && !flag_serial_in_progress)
			{
				memset(byTcpMsgRecv, 0, sizeof(byTcpMsgRecv));
				res_recv_tcp = RecvMsg(byTcpMsgRecv);
			}
		}
		while(m_app2loop == NULL && !res_recv_serial && !res_recv_tcp);

		if(m_app2loop != NULL)
		{
			printf("%s - got an app queued request\n", __FUNCTION__);

			memset(&term_req, 0, sizeof(term_req));
			memcpy(&term_req, m_app2loop->data, sizeof(term_req));
			QueueFreeMemNode(m_app2loop);
			PMSend(&term_req);
		}

		if(res_recv_serial)
		{
			printf("%s - got a command from serial\n", __FUNCTION__);

			memset(&term_req, 0, sizeof(term_req));
			flag_serial_in_progress = 0;

			term_req.Cmd = term_res.Cmd;

			switch(term_res.Cmd)
			{
				case CMD_TCP_OPEN: 
					ConnectionOpen(term_res.pbyDestHost, term_res.pbyDestPort); // todo: obtain a con_id
					{
						// Don't post in queue but send terminal serial response ("request") right here.
						term_req.Stats = STATS_OK; // todo: hard coded
						//term_req.Stats = STATS_FAIL; // For testing scenario
						//return; // For testing scenario
						strcpy(term_req.pbyConId, "1234");
						PMSend(&term_req);
					}
					flag_check_tcp_recv = 1;
					break;

				case CMD_TCP_SEND:
					//printf("%s - MASM DEBUG CMD_TCP_SEND: %s\n", __FUNCTION__, term_res.pbyTcpMsg);
					SendMsg(term_res.pbyTcpMsg); // todo: use the con_id
					{
						// Don't post in queue but send terminal serial response ("request") right here.
						term_req.Stats = STATS_OK; // todo: hard coded
						//term_req.Stats = STATS_FAIL;
						PMSend(&term_req);
					}
					break;

				case CMD_TCP_CLOSE:
					flag_check_tcp_recv = 0;
					ConnectionClose(); // todo: use the con_id
					{
						// Don't post in queue but send terminal serial response ("request") right here.
						term_req.Stats = STATS_OK; // todo: hard coded
						PMSend(&term_req);
					}
					break;

				//case CMD_INIT:
				//	// hack...
				//	//term_res.Cmd = CMD_INIT;
				//	//term_res.byVersion = 2;
				//	//PMSendRes(&term_res);
				//	break;

				default:
					QueueLoop2App(&term_res);
					break;
			}
		}

		if(res_recv_tcp)
		{
			printf("%s - got a message from tcp\n", __FUNCTION__);

			memset(&term_req, 0, sizeof(term_req));
			term_req.Cmd = CMD_TCP_RECV;
			strcpy(term_req.pbyConId, "1234"); // todo: dummy for now, not yet used by terminal.
			strcpy(term_req.pbyEvent, "01"); // todo: dummy for now, not yet used by terminal.
			strcpy(term_req.pbyTcpMsg, byTcpMsgRecv);
			PMSend(&term_req);

			flag_serial_in_progress = 1;
		}
		
		
		
	}

}

static void WaitSyncLoop(TERMINAL_RES *term_res)
{
	MEM_NODE *m_loop2app;

	m_loop2app = NULL;
	memset(term_res, 0, sizeof(TERMINAL_RES));

	do
	{
		m_loop2app = CheckQueueLoop2App();
	}
	while(m_loop2app == NULL);

	printf("%s - got an app loop queued response\n", __FUNCTION__);

	memcpy(term_res, m_loop2app->data, sizeof(TERMINAL_RES));
	QueueFreeMemNode(m_loop2app);
}