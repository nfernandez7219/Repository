#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"

#include "MemQueue.h"

int QueueIsEmpty(MEM_QUEUE* q)
{
	if(q == NULL)
	{
		return TRUE;
	}
	else if (q->rear == NULL)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void QueueInit(MEM_QUEUE* q)
{
	printf("MEM_QUEUE - QueueInit\n");

	q->front = NULL;
	q->rear = NULL;
	q->size = 0;
	q->node_count = 0;
}

int QueueSize(MEM_QUEUE* q)
{
	return q->size;
}

int QueueNodeCount(MEM_QUEUE* q)
{
	return q->node_count;
}

void QueueInitMemNode(MEM_NODE* m)
{
	if(m == NULL)
	{
		return;
	}

	m->data = NULL;
	m->next = NULL;
	m->capacity = 0;
	m->size_orginal = 0;
}

MEM_NODE* QueueAllocMemNode(int capacity)
{
	MEM_NODE* m = (MEM_NODE*)malloc(sizeof(MEM_NODE));

	printf("MEM_QUEUE - Alloc with capacity: %d\n", capacity);

	if(m == NULL)
	{
		printf("%s - Alloc failed, return null\n", __FUNCTION__);
		return NULL;
	}

	QueueInitMemNode(m);

	m->data = malloc(capacity);

	if(m->data == NULL)
	{
		printf("%s - Cannot allocate memory with size: %d, return null\n", __FUNCTION__, capacity);
		return NULL;
	}

	memset(m->data, 0, capacity);

	m->capacity = capacity;

	m->end = m->consume_start = m->base = (unsigned char *)m->data;

	return m;
}

void QueueFreeMemNode(MEM_NODE* m)
{
	printf("MEM_QUEUE::QueueFreeMemNode - capacity: %d\n", m->capacity);

	if( m == NULL)
	{
		return;
	}

	if(m->data != NULL)
		free(m->data);

	free(m);
}

int QueueGetNodeActualDataSize(MEM_NODE* m)
{
	return (int)(m->end - m->consume_start);
}

void QueuePut(MEM_QUEUE* q, MEM_NODE* m)
{
	printf("MEM_QUEUE::Put - Bulk capacity = %d, q size = %d\n", m->capacity, q->size);

	if(QueueIsEmpty(q))
	{
		q->rear = m;
		q->front = m;
	}
	else
	{
		q->rear->next = m;
		q->rear = m;
	}

	q->size+=m->size_orginal;

	q->node_count++;

	printf("MEM_QUEUE::Put Done - q node_count = %d, size = %d\n",q->node_count, q->size);
}

MEM_NODE* QueuePeek(MEM_QUEUE* q)
{
//	printf("MEM_QUEUE::Peek\n");

	return q->front;
}

MEM_NODE* QueueRemove(MEM_QUEUE* q)
{
	MEM_NODE* m = NULL;

//	printf("MEM_QUEUE::Remove\n");

	m = q->front;

	if(q->front == q->rear)
	{
		printf("MEM_QUEUE::Remove - Queue is empty now.\n");

		QueueInit(q);
	}
	else
	{
		q->front = m->next;

		q->size -= QueueGetNodeActualDataSize(m);

		q->node_count--;
	}

	return m;
}


void QueueFreeAll(MEM_QUEUE* q)
{
	MEM_NODE* m = NULL;

	printf("MEM_QUEUE::QueueFreeAll START - Current bulk number: %d, size: %d\n", QueueNodeCount(q), QueueSize(q));

	while ( (m = QueueRemove(q)) != NULL)
	{
		QueueFreeMemNode(m);
	}

	printf("MEM_QUEUE::QueueFreeAll DONE - Current bulk number: %d, size: %d\n", QueueNodeCount(q), QueueSize(q));
}
