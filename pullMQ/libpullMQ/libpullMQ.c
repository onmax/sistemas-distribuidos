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
    queues.array = (Queue *)realloc(queues.array, queues.size * sizeof(*queues.array));
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
	/*
	queue_destroy(&q);
	*/
	queues.size--;
	Queue *temp = (Queue *)malloc(queues.size * sizeof(*queues.array));
	memmove(
        temp,
        queues.array,
        (index + 1) * sizeof(*queues.array));

    memmove(
        temp + index,
        queues.array + index + 1,
        (queues.size - index) * sizeof(*queues.array));
    free(queues.array);
    queues.array = temp;
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
	struct Node *node;
	Queue queue;
	for(int i = 0; i < queues.size; i++)
	{
		if(i != 0)
		{
			free(node);
		}
		queue = queues.array[i];
		node = queue.last;
		printf("  %s - %d:\n\t", queue.name, i);
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
	char *msg1 = "Elemento 2";
	char *msg2 = "Elemento 3";
	put(name, msg2, strlen(msg1));
	put(name, msg, strlen(msg));
	put(name, msg1, strlen(msg1));
	const char *name2 = "NOMBRE COLA 2";
	createMQ(name2);
	put(name2, msg, strlen(msg));
	put(name2, msg1, strlen(msg));
	put(name2, msg, strlen(msg));
	put(name2, msg, strlen(msg));

	const char *name3 = "O3";
	createMQ(name3);
	put(name3, "A", strlen("A"));
	put(name3, msg, strlen(msg));
	put(name3, msg, strlen(msg));

	void *msgget = "";
	size_t tam = 0;
	if(get(name, &msgget, &tam, false) < 0){
		return -1;
	}

	if(get(name2, &msgget, &tam, false) < 0){
		return -1;
	}
	print_everything();
	destroyMQ(name2);
	print_everything();

	return 0;
}