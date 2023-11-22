#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdq_mgr.h"
#include "main.h"



//struct node *freelisthead;
struct node *freelisthead = NULL;
//struct node *freelistlast = NULL;
int			freelist_cnt = 0;

struct node *cmdqmgrqphead[8];
//struct node *cmdqmgrqplast[8];

struct node *cmdqmgractivehead[8];
//struct node *cmdqmgractivelast[8];


void main() 
{
	int key1 = 1234;
	int key2 = 0xdeaddead;
	int data1 = 0xffff1234;
	int data2 = 0x1234ffff;
	int i, j;
	struct node *newnode[8][10];
	int data = 0xffff1234;
	int key = 1234;

	// should be called to initialized the dll storage like:
	// freelist, active queue, normal queue
	cmdq_mgr_init();

	// test getting the head node of freelist
	// test up to maxtest in all the qp
	for (i = 0; i < QP_MAX; i++)
	{
		for (j = 0; j < MAXTEST; j++)
		{
			struct node *nnode = (struct node*)malloc(sizeof(struct node));
			newnode[i][j] = nnode;

			newnode[i][j] = cmdq_mgr_node_get();

			if (newnode[i][j] == NULL)
			{
				while(1);	// no more freelist
			}

			data += 1;
			key += 1;
			newnode[i][j]->data = data;
			newnode[i][j]->key = key;
		}
	}

	for (i = 0; i < QP_MAX; i++)
	{
		for (j = 0; j < MAXTEST; j++)
		{
			if (cmdq_mgr_tail_insert(newnode[i][j], i) != 0)
			{
				// queue to be inserted is null
				while (1);
			}
		}
	}

	for (i = 0; i < QP_MAX; i++)
	{
		for (j = 0; j < MAXTEST; j++)
		{
			if (cmdq_mgr_send_to_active(&newnode[i][j], i) != 0)
			{
				// qp to be deleted is null
				while(1);
			}
		}
	}

	for (i = 0; i < QP_MAX; i++)
	{
		for (j = 0; j < MAXTEST; j++)
		{
//			cmdq_mgr_node_return(&newnode[i][j], newnode[i][j]->next, i);
			cmdq_mgr_node_return(&newnode[i][j], i);
		}
	}
}