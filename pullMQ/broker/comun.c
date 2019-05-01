/*
 * Incluya en este fichero todas las implementaciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */
#include "comun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


int queue_create(Queue *q, const char *name)
{
    q->name = (char *)malloc(strlen(name));
    strcpy(q->name, name);
	q->first = NULL;
	q->last = NULL;
    return 0;
}

int queue_destroy(Queue *q)
{
    free(q);
    return 0;
}

int queue_push(Queue *q, const char *msg)
{
    struct Node *node;
    node = (struct Node *)malloc(sizeof(struct Node));
    node->msg = (char *)malloc(strlen(msg));
    strcpy(node->msg, msg);

    if(q->first == NULL)
    {
        node->next = NULL;
        q->first = node;
    }
    else
    {
        node->next = q->last;
    }
    q->last = node;
    return 0;
}

int queue_pop(Queue *q, void **msg, size_t *tam)
{

    if(q->last == NULL)
    {
        return -1;
    }
    struct Node *first = q->first;

    *msg = (void *)first->msg;
    *tam = strlen(first->msg);
    
    struct Node *second;
    if(queue_search_node(q, first, &second) < 0)
    {
        return -1;
    }

    second->next = NULL;
    q->first = second;

    free(first);

    return 0;
}

int queue_search_node(Queue *q, struct Node *node, struct Node **result)
{
    struct Node *current = q->last;
    do {
        if(current->next == node) {
            *result = current;
            return 0;
        }
    } while((current = current->next) != NULL);
    return 1;
} 