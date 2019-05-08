#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pullMQ.h"

int e = 0;
/*
*   Test 1
*   Unit tests: 6
*   It will create 3 queues and destroy them in the same order
*/
bool test1_1()
{
    int status;
    char *name;

    name = "Queue 1";
    status = createMQ(name);
    return status == 0 ? true : false;
}

bool test1_2()
{
    int status;
    char *name;

    name = "q";
    status = createMQ(name);
    return status == 0 ? true : false;
}

bool test1_3()
{
    int status;
    char *name;

    name = "long name for queue";
    status = createMQ(name);
    return status == 0 ? true : false;
}

bool test1_4()
{
     int status;
    char *name;

    name = "Queue 1";
    status = destroyMQ(name);
    return status == 0 ? true : false;
}

bool test1_5()
{
    int status;
    char *name;

    name = "q";
    status = destroyMQ(name);
    return status == 0 ? true : false;
}

bool test1_6()
{
     int status;
    char *name;

    name = "long name for queue";
    status = destroyMQ(name);
    return status == 0 ? true : false;
}

void test_error(int test_error)
{

    printf("Test %d failed\n", test_error);
}

bool test1()
{
    if(!test1_1()) { test_error(1); e++; }
    if(!test1_2()) { test_error(2); e++; }
    if(!test1_3()) { test_error(3); e++; }
    if(!test1_4()) { test_error(4); e++; }
    if(!test1_5()) { test_error(5); e++; }
    if(!test1_6()) { test_error(6); e++; }
    return true;
}

int main(int argc, char *argv[])
{
    int e = 0;
    int tests = 3;
    test1();
    
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
