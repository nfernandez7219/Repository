#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "main.h"
#include "cmdq_mgr.h"



int main()
{
   //head h;   
   //init_head(&h);

	int key1 = 1234;
	int key2 = 0xdeaddead;
	int data1 = 0xffff1234;
	int data2 = 0x1234ffff;
	int i, j;
	int data = 0xffff1234;
	int key = 1234;
	//head *node[8][10];
	head newnode[8][10];

	cmdq_mgr_init();

	// test getting the head node of freelist
	// test up to maxtest in all the qp
	for (i = 0; i < QP_MAX; i++)
	{
		for (j = 0; j < MAXTEST; j++)
		{
			//head *nnode = (struct node*)malloc(sizeof(struct node));
			//newnode[i][j] = nnode;

			if (cmdq_mgr_node_get() == NULL)
			{
				while(1);	// no more freelist
			}

			data += 1;
			key += 1;
			newnode[i][j].data = data;
			newnode[i][j].nodecnt = key;
		}
	}

	for (i = 0; i < QP_MAX; i++)
	{
		for (j = 0; j < MAXTEST; j++)
		{
			if (cmdq_mgr_tail_insert(&newnode[i][j], i) != 0)
			{
				while(1);
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


   return 0;
}