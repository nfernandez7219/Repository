#ifndef MEMQUEUE_H_
#define MEMQUEUE_H_

/**
 * A memory structure representing a node of a queue
 */
typedef struct MEM_NODE
{
	void* data; 			/** The pointer of the actual data held by the node */
	struct MEM_NODE *next;  /** The pointer of the next mode in the queue */
	int capacity; 			/** The maximum allowed size of data held by the the node */
	int size_orginal;       /** The original size of the data */

	unsigned char *base;
	unsigned char *consume_start;
	unsigned char *end;

} MEM_NODE;

/**
 * A memory structure representing a memory queue
 */
typedef struct {
	MEM_NODE* front; /** The pointer of the first node in the queue */
	MEM_NODE* rear;  /** The pointer of the last node in the queue */
	int size;        /** The total size of the queue memory */
	int node_count;

} MEM_QUEUE;


/**
 * @brief Initializes a queue structure
 *
 * @param q
 */
void QueueInit(MEM_QUEUE* q);

/**
 * @brief Initializes a MEM_NODE structure
 *
 * @param node The pointer of the memory node that will be initialized
 */
void QueueInitMemNode(MEM_NODE* node);

/**
 * @brief Creates a new node with specified size,
 *
 * If successful, the node structure will be initialized and its memory will be reset with binary zeros
 * Please DO NOT forget to free the allocated memory node when it is no longer needed, freeing
 * a memory node can be achieved by calling QueueFreeMemNode
 *
 * @param size The expected memory size of the new node
 * @return The pointer of the newly created node, NULL if allocation fails
 */
MEM_NODE* QueueAllocMemNode(int size);

/**
 * @brief Puts a new memory node at the end of a queue
 *
 * The size and the node count of queue will also be increased
 *
 * @param q The queue
 * @param m The memory node
 */
void QueuePut(MEM_QUEUE* q, MEM_NODE* m);

/**
 * @brief Checks if a queue is empty
 *
 * @param q The queue to be checked
 * @return true if the queue has at least one node, false otherwise, including the case
 * that the queue is NULL
 */
int QueueIsEmpty(MEM_QUEUE* q);


/**
 * @brief Returns the object at the beginning of the Queue without removing it.
 *
 * @param q
 * @return
 */
MEM_NODE* QueuePeek(MEM_QUEUE* q);

/**
 * @brief Removes and returns the object at the beginning of the Queue.
 *
 * @param q The queue
 * @return
 */
MEM_NODE* QueueRemove(MEM_QUEUE* q);

/**
 * @brief Gets the size of available data in the queue
 *
 * @param q The queue
 * @return the size of available data in the queue
 */
int QueueSize(MEM_QUEUE* q);

/**
 * @brief Gets the number of nodes in the queue
 *
 * @param q the queue
 * @return the number of nodes in the queue
 */
int QueueNodeCount(MEM_QUEUE* q);

/**
 * @brief Frees the memory of all the nodes contained in the queue, the 'size' value
 * and the node number of this queue will also be decreased
 *
 * @param q
 */
void QueueFreeAll(MEM_QUEUE* q);


/**
 * @brief Frees the specified memory node
 *
 * @param node The pointer of the memory node that will be freed
 */
void QueueFreeMemNode(MEM_NODE* node);

int QueueGetNodeActualDataSize(MEM_NODE* node);

/**
 * @brief Example code :
 *
 * @code
 * 	//declare and initialize a queue
	MEM_QUEUE my_q;

	memset(&my_q, 0, sizeof(my_q));

	QueueInit(&my_q);

	//create a new node
	MEM_NODE* m = QueueAllocMemNode(sizeof(EQUIPMENT_RES));
	memset(m->data, 0, sizeof(EQUIPMENT_RES));

	//put meaningful data in the node

	memcpy(m->data, resp, sizeof(EQUIPMENT_RES));
	m->size_orginal = sizeof(EQUIPMENT_RES);

	//put the mode in the queue
	QueuePut(&my_q, m);


	//consume the node in a queue
	if(QueuePeek(&my_q) != NULL)
	{
		m = QueueRemove(&my_q);

		//do something here to make use of the data in the mode

		//free the node
		QueueFreeMemNode(m);
	}

	//free all nodes in the queue
	QueueFreeAll(my_q);

	@endcode
 */


#endif /* MEMQUEUE_H_ */
