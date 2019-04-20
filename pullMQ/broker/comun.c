/*
 * Incluya en este fichero todas las implementaciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */
#include "comun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char **array;
    size_t index;
    size_t size;
} List;

void new_list(List *list, size_t length)
{
    list->array = (char **)malloc(0);
    list->index = 0;
    list->size = 0;
}

void push(List *list, char *element)
{
    printf("SOY %d \n", (int)list->size);
    list->size += strlen(element);
    printf("CURRENT SIZE %ld\n", list->size);
    list->array = (char **)realloc(list->array, list->size);
    list->array[list->index++] = element;
    printf("str %s\n", list->array[0]);
}

int main()
{
    List a;
    int i;

    new_list(&a, strlen("hola")); // initially 5 elements
    push(&a, "hola");             // automatically resizes as necessary
    for (i = 0; i < 100; i++) {
        printf("\nit %d\n", i);
        push(&a, "HOLA");         // automatically resizes as necessary
    }
    printf("%s\n", a.array[0]);   // print 10th element
    printf("%d\n", (int)a.index); // print number of elements
}
