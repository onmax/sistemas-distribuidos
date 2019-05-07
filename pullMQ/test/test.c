#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pullMQ.h"

int main(int argc, char *argv[])
{
    createMQ("nombre");
    createMQ("y2");
    void *arr = "HOLA";
    put("nombre", arr, 100);

    void *msg;
    size_t tam = 0;
    get("nombre", &msg, &tam, false);
    printf("LEIDO %s - tam %lu\n", (char *)msg, tam);
    //destroyMQ("soy4");
    return 0;
}
