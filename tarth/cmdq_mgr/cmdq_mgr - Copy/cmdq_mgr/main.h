#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
