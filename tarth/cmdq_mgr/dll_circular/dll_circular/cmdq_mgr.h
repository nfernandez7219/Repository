#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

struct node {
   int data;
   int nodecnt;
   struct node *prev;
   struct node *next;
};

typedef struct node head;
typedef struct node entry;

struct node *freelist;
struct node *cmdqmgrqp[8];
struct node *cmdqmgractive[8];

void cmdq_mgr_init(void);
void cmdq_mgr_init_head(head **h);
void cmdq_mgr_init_entry(head *e, int data, int cnt);
void cmdq_mgr_init_queue(head **queue);
head* cmdq_mgr_alloc_entry(int data, int nc);
void cmdq_mgr_insert_at_tail(head **h, entry **e);
head* insert_at_last(head **h, entry **e);

head *cmdq_mgr_node_get(void);
int cmdq_mgr_tail_insert(head *newnode,
	 				     head *cmdqstorage);
int cmdq_mgr_send_to_active(head *newnode, int qp);
int cmdq_mgr_node_return(head *activeq,
						 int qp);