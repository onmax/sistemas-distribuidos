#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pullMQ.h"
#include <string.h>

#define r return
#define c createMQ
#define d destroyMQ
#define p put
#define g get
#define ERR_QUEUE_NOT_EXIST "Queue doesnt exists"
#define ERR_CREATING "Queue could not be created"
#define ERR_DESTROYING "Queue could not be destroyed"
#define ERR_PUSHING "Message could not be pushed"
#define ERR_GETTING "Unable to get message"
#define MSGS_NOT_EQUAL "Messages are not equal"
#define MSGS_LEN_NOT_EQUAL "Messages length are not equal"

int e = 0;
int tests = 0;

void * randomstr(size_t length) {
    //https://codereview.stackexchange.com/questions/29198/random-string-generator-in-c
    char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";        
    char *random = NULL;
    random = malloc(sizeof(char) * (length));
    if (length && random) {            
        for (int n = 0; n < length; n++) {            
            random[n] = charset[rand() % (int)(sizeof(charset) -1)];
        }
    }
    return (void *)random;
}

void test_error()
{

    printf("Test %d failed\n", tests);
}

bool panic(char *err)
{
    printf("Error in test %i: %s\n", tests, err);
    return false;
}


/*
*   Test 1
*   It will create 3 queues and destroy them in the same order
*/
bool test1()
{
    tests++;
    if(c("queue 1") < 0) { r panic(ERR_CREATING); }
    if(c("q") < 0) { r panic(ERR_CREATING); }
    if(c("long name for queue") < 0) { r panic(ERR_CREATING); }
    if(d("queue 1") < 0) { r panic(ERR_DESTROYING); }
    if(d("q") < 0) { r panic(ERR_DESTROYING); }
    if(d("long name for queue") < 0) { r panic(ERR_DESTROYING); }
    return true;
}

/*
 * Test 2
 * It will create a queue, push a element and get the element back
 */
bool test2()
{
    tests++;
    char *queue = "queue 1";
    void *msg = "This is a normal message";
    size_t msg_len = 24; 
    void *msg_get = 0;
    size_t msg_get_len = 0;

    if(c(queue) < 0) { r panic(ERR_CREATING); }
    if(p(queue, msg, msg_len) < 0) { r panic(ERR_PUSHING); }
    if(g(queue, &msg_get, &msg_get_len, false) < 0) { r panic(ERR_GETTING); }

    if(memcmp(msg_get, msg, msg_len) != 0) { r panic(MSGS_NOT_EQUAL); }
    if(msg_len != msg_get_len) { r panic(MSGS_LEN_NOT_EQUAL); }

    if(d(queue)) { r panic(ERR_DESTROYING); }

    return true;
}

/*
* Test 3
* It will try to create two queues with the same name
* Then it will be removed and added again
*/
bool test3()
{
    tests++;
    char *queue = "queue";
    if(c(queue) < 0) { r panic(ERR_CREATING); }
    if(c(queue) == 0) { r panic(ERR_CREATING); }
    if(d(queue) < 0) { r panic(ERR_DESTROYING); }
    if(c(queue) < 0) { r panic(ERR_CREATING); }
    if(d(queue) < 0) { r panic(ERR_DESTROYING); }
    return true;
}

/**
 * Test 4
 * It will try to destroy a queue that doesnt exists
 */
bool test4()
{
    tests++;
    if(d("queue") >= 0) { r panic(ERR_QUEUE_NOT_EXIST); }
    return true;
}

/**
 * Test 5
 * It will try to put a message in a queue that doesnt exists
 */
bool test5()
{
    tests++;
    void *msg = "message";
    size_t size = 7;
    if(p("queue", msg, size) >= 0) { r panic(ERR_QUEUE_NOT_EXIST); }
    return true;
}

/**
 * Test 6
 * It will try to get a message in a queue that doesnt exists
 */
bool test6()
{
    tests++;
    void *msg = 0;
    size_t size = 0;
    if(g("queue", &msg, &size, false) >= 0) { r panic(ERR_QUEUE_NOT_EXIST); }
    return true;
}

/**
 * Test 7
 * It will crete a really long array to send and push it in the queue
 */
bool test7()
{
    tests++;
    size_t size = 10000;
    void *msg = randomstr(size);
    
    void *msg_get = 0;
    size_t get_msg_len = 0;
    
    if(c("queue") < 0) { r panic(ERR_CREATING); }
    if(p("queue", msg, size) < 0) { r panic(ERR_PUSHING); }
    if(g("queue", &msg_get, &get_msg_len, false) == 0) { r panic(ERR_GETTING); }
    if(d("queue") < 0) { r panic(ERR_DESTROYING); }
    return true;
}

int main(int argc, char *argv[])
{
    printf("\n\nTests:\n");

    /*
    if(!test1()) { test_error(); e++; };
    if(!test2()) { test_error(); e++; };
    if(!test3()) { test_error(); e++; };
    if(!test4()) { test_error(); e++; };
    if(!test5()) { test_error(); e++; };
    if(!test6()) { test_error(); e++; };
    */
    if(!test7()) { test_error(); e++; };
    double percentage = (tests - e) * 100 / tests;
    printf("%.2f %% tests passed\nRemember to check server side\n", percentage);
    return 0;
}
