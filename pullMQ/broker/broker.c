#include <stdio.h>
#include "comun.h"
#include "../libpullMQ/pullMQ.h"

int main(int argc, char *argv[]){
    if(argc!=2 || 1) {
        fprintf(stderr, "Uso: %s puerto\n", argv[0]);
        return 1;
    }
    printf("\nYo soy broker: %s\n\n", argv[0]);
    // Creación de una cola: añadiría una nueva entrada a la estructura de datos de las colas.
    createMQ(NULL);


    return 0;
}