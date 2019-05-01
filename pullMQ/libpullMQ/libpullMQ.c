#include <stdio.h>

#include "comun.h"
#include "pullMQ.h"
#include <string.h>
#include <stdlib.h>

Queues queues;
bool initialized = false;

int createMQ(const char *name)
{
	if(initialized == false)
	{
		queues.array = (Queue *)malloc(0);
		queues.size = 0;
		initialized = true;
	}
	// Comprobar que no haya colas con ese nombre
	for(int i = 0; i < queues.size; i++)
	{
		if(strcmp(queues.array[i].name,name) == 0)
		{
			return -1;
		}
	}


	// Reservar espacio para el nuevo elemento
	queues.size++;
    queues.array = (Queue *)realloc(queues.array, queues.size * sizeof(queues.array));
	if (queues.array == NULL)
	{
        return -1;
	}
	
	// Crear la cola
	Queue queue;
	if((queue_create(&queue, name)) < 0)
	{
		return -1;
	}
	
	// Meter la cola en el array
    queues.array[queues.size - 1] = queue;
	return 0;
}


int destroyMQ(const char *name)
{
	// Obtenemos la cola, devuleve -1 en caso de que no exista
	Queue q;
	int index = -1;
	if((index = get_index(name)) < 0)
	{
		return -1;
	}
	q = queues.array[index];
	queue_destroy(&q);
	// SHOULD DESTROY THE ITEM IN THE ARRAY AS WELL
	return 0;
}

int put(const char *name, const void *msg, size_t tam)
{
	// Obtenemos la cola, devuleve -1 en caso de que no exista
	Queue q;
	int index;
	if((index = get_index(name)) < 0)
	{
		return -1;
	}
	q = queues.array[index];
	queue_push(&q, msg);
	queues.array[index] = q;
	return 0;
}

int get(const char *name, void **msg, size_t *tam, bool blocking)
{
	// Obtenemos la cola, devuleve -1 en caso de que no exista
	Queue q;
	int index;

	if((index = get_index(name)) < 0)
	{
		return -1;
	}
	q = queues.array[index];
	if(queue_pop(&q, msg, tam) < 0)
	{
		return -1;
	}
	queues.array[index] = q;
	return 0;
}

void print_everything(){
	printf("TAMAÑO ARRAY: %d\n", queues.size);
	for(int i = 0; i < queues.size; i++)
	{
		printf("  %s:\n\t", queues.array[i].name);
		struct Node *node = queues.array[i].last;
		do {
			printf(" %s ->", node->msg);
		} while((node = node->next) != NULL);
		printf("\n");
	}
}

int get_index(const char *name)
{
	// Buscamos una cola cuyo nombre sea igual a name, devuelve -1 en caso de que no exista	
	for(int i = 0; i < queues.size; i++)
	{	
		if(strcmp(queues.array[i].name,name) == 0)
		{
			return i;
		}
	}

	return -1;
}



int main()
{
	const char *name = "NOMBRE COLA 1";
	createMQ(name);
	char *msg = "Elemento 1";
	put(name, msg, strlen(msg));
	char *msg1 = "Elemento 2";
	put(name, msg1, strlen(msg1));
	put(name, msg1, strlen(msg1));
	print_everything();
	void *msgget = "";
	size_t tam = 0;
	if(get(name, &msgget, &tam, false) < 0){
		return -1;
	}
	print_everything();

	/*
	if(get(name, &msgget, &tam, false) < 0){
		return -1;
	}*/
	return 0;
}