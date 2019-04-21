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
    int size;
} List;

void create_list(List *list)
{
    list->array = (char **)malloc(0);
    list->size = 0;
}

void print_list(List *list)
{
    printf("Size: %d -> [", list->size);
    for (int i = 0; i < list->size; ++i)
        printf("'%s', ", list->array[i]);
    printf("]\n");

}

int push(List *list, char *element)
{
    list->size++;
    list->array = (char **)realloc(list->array, list->size * sizeof(*list->array));
    list->array[list->size - 1] = (char *)malloc(strlen(element) * sizeof(*list->array[list->size - 1]));
    if (!list->array[list->size - 1])
        return -1;
    strcpy(list->array[list->size - 1], element);
    return 0;
}

int remove_element_at(List *list, int index_to_remove)
{
    list->size--;
    char **temp = (char **)malloc(list->size * sizeof(*list->array));
    
    memmove(
        temp,
        list->array,
        (index_to_remove + 1) * sizeof(*list->array));
    memmove(
        temp + index_to_remove,
        list->array + index_to_remove + 1,
        (list->size - index_to_remove) * sizeof(*list->array));

    free(list->array[index_to_remove]);
    list->array = temp;
    return 0;
}

int index_of(List *list, char *element)
{
    int i = 0;

    for(; i < list->size; i++)
    {
        printf("INDEZ: %d %s\n", i, list->array[i]);
        if(strcmp(list->array[i], element) == 0) {
            return i;
        }
    }

    return -1;
}

int remove_element(List *list, char *element)
{
    int index;
    if((index = index_of(list, element)) < 0) {
        printf("El elemento no se encuentra en la lista\n");
        return -1;
    }
    printf("%d\n", index);
    remove_element_at(list, index);
}

int main()
{
    List list;
    int i;
    char a[4];

    create_list(&list);
    for (i = 0; i < 10; i++)
    {
        sprintf(a, "%d", i * 10 + 10);
        push(&list, a);
    }
    print_list(&list);
    remove_element(&list, "50");
    print_list(&list);
}
