#include <stdio.h>

#include "comun.h"
#include "pullMQ.h"
#include <string.h>


struct Node {
	char 			*msg;
	struct 			Node *next;
};

struct Queue {
	struct 			Node *head;
	struct 			Node *tail;
	unsigned int 	size;
	char			*name;
	bool 			blocking;
};

int createMQ(const char *name) {
	printf("LOLL");
	return 0;
}
int destroyMQ(const char *name){
	return 0;
}
int put(const char *name, const void *msg, size_t tam){
	return 0;
}
int get(const char *name, void **msg, size_t *tam, bool blocking){
	return 0;
}
