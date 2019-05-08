#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pullMQ.h"


bool test1()
{
    int status;
    char *name;

    name = "Queue 1";
    status = createMQ(name);
    return status == 0 ? true : false;
}

bool test2()
{
    int status;
    char *name;

    name = "q";
    status = createMQ(name);
    return status == 0 ? true : false;
}

bool test3()
{
    int status;
    char *name;

    name = "long name for queue";
    status = createMQ(name);
    return status == 0 ? true : false;
}

void test_error(int e)
{
    printf("Test %d failed\n", e);
}

int main(int argc, char *argv[])
{
    int e = 0;
    int tests = 3;
    
    if(!test1()) { test_error(1); e++; }
    if(!test2()) { test_error(2); e++; }
    if(!test3()) { test_error(3); e++; }

    double p = (tests - e) * 100 / tests;
    printf("\nTEST: %.2f %% tests passed\nRemember to check server side\n", p);
    /*
    createMQ("nombre");
    createMQ("y2");
    void *arr = "HOLA";
    put("nombre", arr, 100);

    void *msg;
    size_t tam = 0;
    get("nombre", &msg, &tam, false);
    printf("LEIDO %s - tam %lu\n", (char *)msg, tam);
    //destroyMQ("soy4");
    */
    return 0;
}
