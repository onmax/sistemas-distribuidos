/*
 * Incluya en este fichero todas las implementaciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */
#include "comun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void print_name(const char *name)
{
    if(strlen(name) >= 30){
        char res[50];
        strncpy(res, name, 30);
        printf("%s...(%lu): \n", res, strlen(name));
    }
    else
    {
        printf("%s(%lu)\n", name, strlen(name));
    }
}

void print_message(const void *message, size_t size)
{
    if(size >= 30){
        void *res;
        res = malloc(50);
        memcpy(res, message, 30);
        printf("\t%s...(%lu)\n", (char *)res, size);
        free(res);
    }
    else
    {
        printf("\t%s(%lu)\n", (char *)message, size);
    }
}

