#include <stdio.h>

#include "comun.h"
#include "pullMQ.h"
#include <string.h>
#include "../dyna/dyna_ll_lifo.c"

typedef struct
{
	DYN_LL_LIFO *array;
	int size;
} DYN_ARR;

DYN_ARR dyn_arr;
bool initialized = false;

void create_DYNAR_LIFO() {
	dyn_arr.array = (DYN_LL_LIFO *)malloc(0);
	dyn_arr.size = 0;
	initialized = true;
}

int createMQ(const char *name)
{
	if(initialized == 0)
		create_DYNAR_LIFO();
	DYN_LL_LIFO dyn_ll_lifo;
	dyn_create(&dyn_ll_lifo);
	return 0;
}
int destroyMQ(const char *name)
{
	return 0;
}
int put(const char *name, const void *msg, size_t tam)
{
	return 0;
}
int get(const char *name, void **msg, size_t *tam, bool blocking)
{
	return 0;
}

void main(){
	createMQ("HOLA");
}
