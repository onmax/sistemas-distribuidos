#include <stddef.h>

/*
 * Incluya en este fichero todas las definiciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */

#define CREATE 1
#define DESTROY 2
#define PUT 3
#define GET 4

typedef struct
{
	int operation;
	size_t queue_name_len;
	char *queue_name;
	size_t msg_len;
	void *msg;
} Request;


struct Node
{
	size_t size;
	void *msg;
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
int queue_push(Queue *q, const void *msg, size_t tam);
int queue_pop(Queue *q, void **msg, size_t *tam);
int queue_search_node(Queue *q, struct Node *node, struct Node **result);
void print_name(const char *name);
void print_message(const void *message, size_t size);