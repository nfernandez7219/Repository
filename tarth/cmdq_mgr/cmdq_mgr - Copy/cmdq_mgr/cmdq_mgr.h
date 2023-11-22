#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern struct node *freelisthead;
//extern struct node *freelistlast;
extern int			freelist_cnt;

extern struct node *cmdqmgrqphead[8];
//extern struct node *cmdqmgrqplast[8];

extern struct node *cmdqmgractivehead[8];
//extern struct node *cmdqmgractivelast[8];

void cmdq_mgr_init(void);
void cmdq_mgr_freelist_init(struct node** flisthead);
struct node* cmdq_mgr_node_get(void);
int cmdq_mgr_tail_insert(struct node* newnode, 
					     int qp); 
void cmdq_mgr_insert_to_tail(struct node* newnode, 
							 struct node** cmdqmgrqp);
int cmdq_mgr_send_to_active(struct node** newnode,
	                                 int qp);
/*void cmdq_mgr_node_return(struct node **newnode, 
						  struct node *del,
						  int qp); 
						  */
int cmdq_mgr_node_return(struct node **newnode, 
						  int qp); 
void cmdq_mgr_freelist_return(struct node *active_queue, int qp);
