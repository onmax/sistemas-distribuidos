#include <stdio.h>

#include "comun.h"
#include "pullMQ.h"
#include <string.h>
#include <stdlib.h>


QUEUES queues;
bool initialized = false;

int createMQ(const char *name)
{
	printf("1");
	if(initialized == false)
	{
		queues.array = (QUEUE *)malloc(0);
		queues.size = 0;
		initialized = true;
	}
	printf("2");

	// Comprobar que no haya colas con ese nombre
	for(int i = 0; i < queues.size; i++)
	{
		if(strcmp(queues.array[i].name,name) == 0)
		{
			return -1;
		}
	}
	printf("3");


	// Reservar espacio para el nuevo elemento
	queues.size++;
    queues.array = (QUEUE *)realloc(queues.array, queues.size * sizeof(queues.array));
	if (queues.array == NULL)
	{
        return -1;
	}
	printf("5");
	
	// Crear la cola
	QUEUE queue;
	if((queue_create(&queue, name)) < 0)
	{
		return -1;
	}
	
	// Meter la cola en el array
    queues.array[queues.size - 1] = queue;
	printf("6");

	return 0;
}

int search_queue(QUEUE *q, const char *name)
{
	// Buscamos una cola cuyo nombre sea igual a name, devuleve -1 en caso de que no exista	
	for(int i = 0; i < queues.size; i++)
	{
		if(strcmp(queues.array[i].name,name) == 0)
		{
			q = &queues.array[i];
			return 0;
		}
	}
	return -1;
}

int destroyMQ(const char *name)
{
	// Obtenemos la cola, devuleve -1 en caso de que no exista
	QUEUE q;
	if(search_queue(&q, name) < 0)
	{
		return -1;
	}
	queue_destroy(&q);
	return 0;
}
int put(const char *name, const void *msg, size_t tam)
{
	// Obtenemos la cola, devuleve -1 en caso de que no exista
	QUEUE q;
	if(search_queue(&q, name) < 0)
	{
		return -1;
	}
	queue_push(&q, msg);
	return 0;
}
int get(const char *name, void **msg, size_t *tam, bool blocking)
{
	// Obtenemos la cola, devuleve -1 en caso de que no exista
	QUEUE q;
	if(search_queue(&q, name) < 0)
	{
		return -1;
	}
	queue_pop(&q, msg, tam);
	return 0;
}

void print_everything(){
	printf("TAMAÑO ARRAY: %d\n", queues.size);
	for(int i = 0; queues.size; i++)
	{
		printf("Cola %d - elementos\n", i);
		QUEUE q = queues.array[i];
		struct Node *node = q.head;
		if(node != NULL){
			printf("\t%s ", node->msg);
			node = node->next;
			while((node = node->next) != NULL)
			{
				printf(" %s ", node->msg);
			}
			printf("\n\n");
		}
		
	}
}