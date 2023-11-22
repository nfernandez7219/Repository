#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdq_mgr.h"
#include "main.h"



void cmdq_mgr_init(void)
{
	int i, j, k;

	// init freelist
//	for (i = 0; i < FREELIST_MAX; i++)
	for (i = 0; i < MAXTEST*QP_MAX; i++)
	{
		cmdq_mgr_freelist_init(&freelisthead);
	}

	for (j = 0; j < (QP_MAX); j++)
	{	
		struct node *newqp = (struct node*) malloc(sizeof(struct node));
		//cmdqmgrqplast[j] = newqp;
		cmdqmgrqphead[j] = newqp;
		//cmdqmgrqplast[j] = NULL;
		cmdqmgrqphead[j] = NULL;
	}

	for (k = 0; k < (QP_MAX); k++)
	{	
		struct node *newactive = (struct node*) malloc(sizeof(struct node));
		//cmdqmgractivelast[k] = newactive;
		cmdqmgractivehead[k] = newactive;
		//cmdqmgractivelast[k] = NULL;
		cmdqmgractivehead[k] = NULL;
	}
}

void cmdq_mgr_freelist_init(struct node** flisthead)
{
	// allocate node
	struct node *newnode = (struct node*)malloc(sizeof(struct node));
	struct node *lastnode = *flisthead;

	freelist_cnt++;

	newnode->key = freelist_cnt;
	newnode->data = 0xdeadbeef;

	// link the old list off the new node
	newnode->next = NULL;

	// change prev of head node to new node
	if ((*flisthead) == NULL)
	{
		newnode->prev = NULL;
		*flisthead = newnode;
		return;
	}

	while (lastnode->next != NULL)
	{
		lastnode = lastnode->next;
	}

	// change the next of last node
	lastnode->next = newnode;

	// make last node as previous of new node
	newnode->prev = lastnode;

	return;
}

struct node* cmdq_mgr_delete(struct node** cmdq_mgr_head, struct node* del)
{
	// base case
	if (*cmdq_mgr_head == NULL || del == NULL)
	{
		return NULL;
	}

	// if node to be deleted is head node
	if (*cmdq_mgr_head == del)
	{
		*cmdq_mgr_head = del->next;
	}

	// change next only if node to be deleted is not the last node
	if (del->next != NULL)
	{
		del->next->prev = del->prev;
	}

	// change prev only if node to be deleted is not the first node
	if (del->prev != NULL)
	{
		del->prev->next = del->next;
	}

	del->next = NULL;

	return del;
}

struct node* cmdq_mgr_node_get(void)
{
	struct node *newnode = (struct node*)malloc(sizeof(struct node));
	newnode = cmdq_mgr_delete(&freelisthead, freelisthead);

	return newnode;
}

void cmdq_mgr_insert_to_tail(struct node* newnode,			// node to insert
							 struct node** cmdqmgrstorage)	// dll storage
{
	struct node *tempnode = *cmdqmgrstorage;

	// link the old list off the new node
	newnode->next = NULL;//(*flisthead);

	// change prev of head node to new node
	if ((*cmdqmgrstorage) == NULL)
	{
		newnode->prev = NULL;
		(*cmdqmgrstorage) = newnode;
//		(*newnode)->next = (*newnode)->prev = (*newnode);
//		(*cmdqmgrstorage) = (*newnode);
		return;
	}

	
	while (tempnode->next != NULL)
	{
		tempnode = tempnode->next;
	}

	// change the next of last node
	tempnode->next = newnode;

	// make last node as previous of new node
	newnode->prev = tempnode;
}

int cmdq_mgr_tail_insert(struct node* newnode, int qp)
{
	cmdq_mgr_insert_to_tail(newnode, &cmdqmgrqphead[qp]);

	return 0;
}


int cmdq_mgr_send_to_active(struct node **newnode, int qp)
{
	struct node *qpnode = (struct node*)malloc(sizeof(struct node));
	qpnode = cmdq_mgr_delete(&cmdqmgrqphead[qp], cmdqmgrqphead[qp]);

	if (qpnode == NULL)
	{
		return 1;
	}

	cmdq_mgr_insert_to_tail(qpnode, &cmdqmgractivehead[qp]);

	return 0;
}

int cmdq_mgr_node_return(struct node **activequeue, int qp) 
						  
{	
	struct node *newnode = (struct node*)malloc(sizeof(struct node));
	newnode = cmdq_mgr_delete(&cmdqmgractivehead[qp], cmdqmgractivehead[qp]);

	if (newnode == NULL)
	{
		return 1;
	}

	cmdq_mgr_insert_to_tail(&newnode, &freelisthead);

	return 0;
}

void cmdq_mgr_freelist_return(struct node *active_queue, int qp)
{
	// tail freelist
	cmdq_mgr_insert_to_tail(&active_queue, qp);		
}

