#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "main.h"
#include "cmdq_mgr.h"

int nodecount = 0;

void cmdq_mgr_init(void)
{
	int i;

	cmdq_mgr_init_head(&freelist);

	for (i = 0; i < MAXTEST*QP_MAX; i++)
	{
		cmdq_mgr_init_queue(&freelist);
	}

	for (i = 0; i < QP_MAX; i++)
	{
		cmdq_mgr_init_head(&cmdqmgrqp[i]);
		cmdq_mgr_init_queue(&cmdqmgrqp[i]);
	}

	for (i = 0; i < QP_MAX; i++)
	{
		cmdq_mgr_init_head(&cmdqmgractive[i]);
		cmdq_mgr_init_queue(&cmdqmgractive[i]);
	}
}

void cmdq_mgr_init_queue(head **queue)
{	
	head *newnode = malloc(sizeof(*newnode));

	nodecount++;
	newnode->data = 0xffffffff;
	newnode->nodecnt = nodecount;

	cmdq_mgr_insert_at_tail(&queue, &newnode);
}

void cmdq_mgr_init_head(head **h)
{
	head *newnode = malloc(sizeof(*newnode));
//	newnode->next = newnode;
//	newnode->prev = newnode;
	newnode->next = NULL;
	newnode->prev = NULL;
	*h = newnode;
   //(*h)->next = *h;
   //(*h)->prev = *h;
}

void cmdq_mgr_init_entry(head *e, int data, int cnt)
{
   e->data = data;
   e->nodecnt = cnt;
   e->next = NULL;
   e->prev = NULL;
}

head* cmdq_mgr_alloc_entry(int data, int nc)
{
   head *e = malloc(sizeof(*e));
   if (e != NULL) {
      cmdq_mgr_init_entry(e, data, nc);
   }
   return e;
}

void cmdq_mgr_insert_at_tail(head **h, entry **e)
{

	if (((*h)->next == NULL) || ((*h)->prev == NULL))
	{
		(*e)->next = (*e)->prev = *e;
		*h = e;
		return;
	}

	(*e)->prev = (*h)->prev;
	(*e)->next = *h;
	(*h)->prev->next = *e;
	(*h)->prev = *e;

	/*
   head *e = cmdq_mgr_alloc_entry(data, nc);
   insert_at_last(&h, &e);
   */
}

head* insert_at_last(head **h, entry **e)
{

//	if (((*h)->next == *h) || ((*h)->prev == *h))
	if (((*h)->next == NULL) || ((*h)->prev == NULL))
	{
		//head *newnode = malloc(sizeof(*newnode));
		//newnode->next = newnode->prev = newnode;
		//h = newnode;
		(*e)->next = (*e)->prev = *e;
		h = e;
		return;
	}

	(*e)->prev = (*h)->prev;
	(*e)->next = *h;
	(*h)->prev->next = *e;
	(*h)->prev = *e;
}

head *cmdq_mgr_node_get(void)
{/*
	head *newnode = freelist->next;

	newnode->next->prev = freelist;
	freelist->next = newnode->next;
*/
	return;// newnode;
}

int cmdq_mgr_tail_insert(head *newnode,
						 head *cmdqstorage)
{
//	head *tempnode = *cmdqstorage;
/*
	if (*cmdqstorage) == NULL)
	{
		return 1;
	}
*/
	return 0;
}

int cmdq_mgr_send_to_active(head *newnode, int qp)
{
/*
	if ((*newnode) == NULL)
	{
		return 1;
	}
*/
	return 0;
}

int cmdq_mgr_node_return(head *activeq,
						 int qp)
{
	return 0;
}


/*

int is_empty(head *h)
{
   return (h->next == h && h->prev == h);
}

void add_at_first(head *h, entry *e)
{
   e->next = h->next;
   e->prev = h;
   h->next->prev = e;
   h->next = e;
}

void add_at_last(head *h, entry *e)
{
   e->prev = h->prev;
   e->next = h;
   h->prev->next = e;
   h->prev = e;
}

int remove_first(head *h)
{
   entry *e = h->next;
   int data = e->data;

   e->next->prev = h;
   h->next = e->next;
   free(e);
   return data;
}

int remove_last(head *h)
{
   entry *e = h->prev;
   int data = e->data;

   e->prev->next = h;
   h->prev = e->prev;
   free(e);
   return data;
}

int list_remove(entry *e)
{
   int data = e->data;
   e->next->prev = e->prev;
   e->prev->next = e->next;
   free(e);
   return data;
}

void print_list_reverse(head *h)
{
   entry *e = h->prev;
   printf("Contents (Reverse): ");
   while (e != h) {
      printf("%d -> ", e->data);
      e = e->prev;
   }
   printf("\n");
}

void print_list(head *h)
{
   entry *e = h->next;
   printf("Contents (Forward): ");
   while (e != h) {
      printf("%d -> ", e->data);
      e = e->next;
   }
   printf("\n");
   print_list_reverse(h);
}


void list_add_first_test(head *h, int data)
{
   entry *e = alloc_entry(data);
   add_at_first(h, e);
   print_list(h);
}

void list_add_last_test(head *h, int data)
{
   entry *e = alloc_entry(data);
   add_at_last(h, e);
   print_list(h);
}

void list_remove_fist_test(head *h, int expected)
{
   int data = remove_first(h);
   assert(data == expected);
   print_list(h);
}

void list_remove_last_test(head *h, int expected)
{
   int data = remove_last(h);
   assert(data == expected);
   print_list(h);
}

void run_unit_tests(head *h)
{
   init_head(h);
   print_list(h);
   list_add_first_test(h, 10);
   list_add_last_test(h, 20);
   list_add_first_test(h, 30);
   list_add_last_test(h, 40);
   list_add_first_test(h, 50);
   list_add_last_test(h, 60);
   list_remove_fist_test(h, 50);
   list_remove_last_test(h, 60);
   list_remove(h->next->next);
   print_list(h);
}
*/

