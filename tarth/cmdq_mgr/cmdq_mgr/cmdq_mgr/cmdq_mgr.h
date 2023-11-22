#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "queue.h"

struct node {
   int data;
   int key;
	
   struct node *next;
   struct node *prev;
};

#define QP_MAX			8
#define QUEUE_MAX		256
#define	FREELIST_MAX	(QUEUE_MAX * QP_MAX)
#define MAXTEST			10

CIRCLEQ_HEAD(circlehead, qentry_struct); //freelistqueue;
//struct circleq *headp;

// nats
struct r_bth {
	unsigned char  opcode;
	unsigned char  flags;
	unsigned short pkey;
	unsigned int   qpn;
	unsigned int   apsn;
};

struct r_deth {
	unsigned int qkey;
	unsigned int sqp;
};

struct bth_pkt_struct {
	struct r_bth bth;
	struct r_reth reth;
	unsigned char dta[1024];
};

struct qentry_struct {
	unsigned int cnt;

	unsigned int qpn;
	unsigned int cqn;
	unsigned int bth_opcode;
	unsigned int wqe[16];
	struct bth_pkt_struct pkt;

	CIRCLEQ_ENTRY(qentry_struct) entry;	// Queue
} *qentry;
// nats

struct queue_struct {
	unsigned int cnt;
	CIRCLEQ_HEAD(cmdqueue, qentry_struct) cmdq;
};



//struct node *freelistqueue;
//struct node *qpqueue[8];
//struct node *activequeue[8];

static int			freelist_cnt = 0;


void cmdq_mgr_init(void);
struct node* cmdq_mgr_node_get(void);
int cmdq_mgr_tail_insert(struct qentry_struct **newnode, 
					     int qp); 
void cmdq_mgr_send_to_active(struct qentry_struct **newnode,
	                                 int qp);
int cmdq_mgr_node_return(struct qentry_struct **newnode, 
						  int qp); 
