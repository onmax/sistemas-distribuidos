/*
 * Incluya en este fichero todas las implementaciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */
#include "comun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


int queue_create(QUEUE *q, const char *name)
{
    strcpy(q->name, name);
	q->head = NULL;
	q->tail = NULL;
    return 0;
}

int queue_destroy(QUEUE *q)
{
    free(q);
    return 0;
}

int queue_push(QUEUE *q, const char *msg)
{
    struct Node *node;
    node = (struct Node *)malloc(sizeof(struct Node));
    strcpy(node->msg, msg);
    node->next = NULL;

    if(q->head == NULL)
    {
        q->head = node;
    }

    q->tail = node;
    return 0;
}

int queue_pop(QUEUE *q, void **msg, size_t *tam)
{
    if(q->head == NULL)
    {
        return -1;
    }
    struct Node *head = q->head;

    *msg = (void *)head->msg;
    *tam = strlen(head->msg);
    free(head);
    return 0;
}