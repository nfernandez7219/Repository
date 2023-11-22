#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define QP_MAX			8
#define QUEUE_MAX		256
#define	FREELIST_MAX	(QUEUE_MAX * QP_MAX)

#define MAXTEST			10