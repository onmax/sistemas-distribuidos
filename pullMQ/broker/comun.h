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
	struct Node *first;
	struct Node *last;
} Queue;

typedef struct 
{
	Queue *array;
	int size;
} Queues;

int queue_create(Queue *q, const char *name);
int queue_destroy(Queue *q);
int queue_push(Queue *q, const char *msg);
int queue_pop(Queue *q, void **msg, size_t *tam);
int queue_search_node(Queue *q, struct Node *node, struct Node **result);