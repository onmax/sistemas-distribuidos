#include <stddef.h>

/*
 * Incluya en este fichero todas las definiciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */

struct Node
{
	char *msg;
	struct Node *next;
};

typedef struct
{
	char *name;
	struct Node *head;
	struct Node *tail;
} QUEUE;

typedef struct
{
	QUEUE *array;
	int size;
} QUEUES;

int queue_create(QUEUE *q, const char *name);
int queue_destroy(QUEUE *q);
int queue_push(QUEUE *q, const char *msg);
int queue_pop(QUEUE *q, void **msg, size_t *tam);