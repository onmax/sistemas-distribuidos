#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node
{
    char *msg;
    struct Node *next;
};

typedef struct
{
    struct Node *head;
    struct Node *tail;
    unsigned int size;
} DYNA_LL_LIFO;

void dyna_print(DYNA_LL_LIFO *lifo)
{
    if(lifo->head == NULL)
        return;
    struct Node *n = lifo->head;
        
    while (n->next != NULL)
    {
        printf(" %s ", n->msg);
        n = n->next;
    }
    printf(" %s \n", n->msg);
}

void dyn_create(DYNA_LL_LIFO *lifo)
{
    lifo->head = NULL;
    lifo->tail = NULL;
    lifo->size = 0;
}

void push(DYNA_LL_LIFO *list, char *msg)
{
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    node->msg = msg;
    if (list->tail != NULL)
        list->tail->next = node;
    list->tail = node;
    if (list->head == NULL)
        list->head = node;
    list->size++;
}

char* pop(DYNA_LL_LIFO *list)
{
    char *msg = list->head->msg;
    list->head = list->head->next != NULL ? list->head->next : NULL;
    list->size--;
    return msg;
}

void dyna_ll_lifo_test()
{
    DYNA_LL_LIFO list;
    int i;
    char a[4];
    dyn_create(&list);
    push(&list, "elem1");
    push(&list, "elem2");
    push(&list, "elem4");
    push(&list, "elem5");
    print_LIFO(&list);
    char* msg = pop(&list);
    msg = pop(&list);
    msg = pop(&list);
    msg = pop(&list);
    printf("Eliminado %s\n", msg);
    print_LIFO(&list);
}