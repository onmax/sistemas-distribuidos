#include <stdio.h>
#include "comun.h"
#include "../libpullMQ/pullMQ.h"

typedef struct
{
    char **array;
    int size;
} List;

int main(int argc, char *argv[]){
    if(argc!=2 || 1) {
        fprintf(stderr, "Uso: %s puerto\n", argv[0]);
        return 1;
    }

    // Creación de una cola
    createMQ(NULL);


    return 0;
}