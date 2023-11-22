#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdq_mgr.h"
#include "main.h"


struct circlehead	freelistqueue;			// Freelist Queue Head 
struct circlehead	qpqueue[QP_MAX];		// QP Queue Head 
struct circlehead	activequeue[QP_MAX];	// Active Queue Head 


void cmdq_mgr_init(void)
{
	int i, j;

	//cmdq_mgr_queue_init(&freelistqueue);
	CIRCLEQ_INIT(&freelistqueue);

	// init freelist
//	for (i = 0; i < FREELIST_MAX; i++)
	for (i = 0; i < MAXTEST*QP_MAX; i++)
	{
		//cmdq_mgr_freelist_init(&freelistqueue);

		qentry = malloc(sizeof(struct qentry_struct));
		freelist_cnt++;

		qentry->cnt = freelist_cnt;
		CIRCLEQ_INSERT_TAIL(&freelistqueue, qentry, entry);
	}

	for (j = 0; j < (QP_MAX); j++)
	{	
		CIRCLEQ_INIT(&qpqueue[j]);
		CIRCLEQ_INIT(&activequeue[j]);
		//cmdq_mgr_queue_init(&qpqueue[j]);
		//cmdq_mgr_queue_init(&activequeue[j]);
	}
}

struct node* cmdq_mgr_node_get(void)
{
	struct qentry_struct *newnode;

	newnode = malloc(sizeof(struct qentry_struct));

	// returns the first item on the queue
	newnode = CIRCLEQ_FIRST(&freelistqueue);

	CIRCLEQ_REMOVE(&freelistqueue, newnode, entry);

	return newnode;
}

int cmdq_mgr_tail_insert(struct qentry_struct **newnode, int qp)
{
	CIRCLEQ_INSERT_TAIL(&qpqueue[qp], *newnode, entry);

	return 0;
}

void cmdq_mgr_send_to_active(struct node **newnode, int qp)
{
	struct qentry_struct *tempnode;

	tempnode = malloc(sizeof(struct qentry_struct));

	tempnode = CIRCLEQ_FIRST(&qpqueue[qp]);

	CIRCLEQ_REMOVE(&qpqueue[qp], tempnode, entry);

	CIRCLEQ_INSERT_TAIL(&activequeue[qp], tempnode, entry);
}

int cmdq_mgr_node_return(struct qentry_struct **returnqueue, int qp) 						  
{	

	CIRCLEQ_REMOVE(&activequeue[qp], *returnqueue, entry);

	CIRCLEQ_INSERT_TAIL(&freelistqueue, *returnqueue, entry);

	return 0;
}
