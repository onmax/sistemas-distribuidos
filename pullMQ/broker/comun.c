/*
 * Incluya en este fichero todas las implementaciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */
#include "comun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comun.h"
#include <stdbool.h>
#include "linkedList"
typedef struct
{
    char **array;
    int size;
} List;

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
} LIFO;

typedef struct
{
    LIFO*    array;
    int             size;
} LinkedLists;




int main()
{
    LinkedLists lists;
    int i;
    char a[4];

    create_queues(&lists);
    /*
    for (i = 0; i < 10; i++)
    {
        sprintf(a, "%d", i * 10 + 10);
        push(&list, a);
    }
    print_list(&list);
    remove_element(&list, "55");
    print_list(&list);
    */
}
