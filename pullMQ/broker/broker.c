#include <stdio.h>
#include "comun.h"
#include "../libpullMQ/pullMQ.h"

#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAXBUF 1024

Queues queues;
bool initialized = false;

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


int queue_search_node(Queue *q, struct Node *node, struct Node **result)
{
    struct Node *current = q->last;
    do {
        if(current->next == node) {
            *result = current;
            return 0;
        }
    } while((current = current->next) != NULL);
    return 1;
}





int queue_create(Queue *q, const char *name)
{   
    Queue *temp = malloc(sizeof(Queue));
    temp->name = strdup(name);
	temp->first = NULL;
	temp->last = NULL;
    *q = *temp;
    return 0;
}


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



int queue_destroy(Queue *q)
{
    //free(q);
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



int queue_push(Queue *q, const char *msg)
{
    struct Node *node;
    node = (struct Node *)malloc(sizeof(struct Node));
    node->msg = (char *)malloc(sizeof(char *));
    strcpy(node->msg, msg);

    if(q->first == NULL)
    {
        node->next = NULL;
        q->first = node;
    }
    else
    {
        node->next = q->last;
    }
    q->last = node;
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


int queue_pop(Queue *q, void **msg, size_t *tam)
{

    if(q->last == NULL)
    {
        return -1;
    }
    struct Node *first = q->first;

    *msg = (void *)first->msg;
    *tam = strlen(first->msg);
    
    struct Node *second;
    if(queue_search_node(q, first, &second) < 0)
    {
        return -1;
    }

    second->next = NULL;
    q->first = second;

    free(first);

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




int main(int argc, char *argv[]){

    if(argc!=2) {
        fprintf(stderr, "Uso: %s puerto\n", argv[0]);
        return 1;
    }

    char *host = getenv("BROKER_HOST");
    int port = atoi(argv[1]);

    int sockfd, read_size;
	struct sockaddr_in self;
    struct hostent *he;
	char buffer[MAXBUF];
	char *client_message;

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Socket");
		exit(errno);
	}

	bzero(&self, sizeof(self));
	self.sin_family = AF_INET;
	self.sin_port = htons(port);
    if ((he = gethostbyname(host)) == NULL)
    {
        exit(1);
    }
    memcpy(&self.sin_addr, he->h_addr_list[0], he->h_length);
    if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )
	{
		perror("socket--bind");
		exit(errno);
	}

	if ( listen(sockfd, 20) != 0 )
	{
		perror("socket--listen");
		exit(errno);
	}
	while (1)
	{	
        int clientfd;
		struct sockaddr_in client_addr;
		socklen_t addrlen = sizeof(client_addr);

		clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
		printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		while( (read_size = recv(clientfd , client_message , 2000 , 0)) > 0 )
		{
			printf("vamos a bver %s hijo", client_message);
		}
		send(clientfd, buffer, recv(clientfd, buffer, MAXBUF, 0), 0);

		close(clientfd);
	}

	close(sockfd);
    return 0;
}


