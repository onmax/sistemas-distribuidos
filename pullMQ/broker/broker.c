#include <stdio.h>
#include "comun.h"
#include "../libpullMQ/pullMQ.h"

#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h> 
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
    temp->name = name;
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
		if(strcmp(queues.array[i].name, name) == 0)
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
	printf("ESTOY CREANDO Y MI NOMBRE ES %s\n", name);
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



int queue_push(Queue *q, const void *msg)
{
    struct Node *node;
    node = (struct Node *)malloc(sizeof(struct Node));
    node->msg = (void *)malloc(sizeof(void *));
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
		printf("  %s - %d:\n\t", queue.name, i);
		queue = queues.array[i];
		node = queue.last;
		if(node == NULL){
			printf("vacío\n");
			continue;
		}

		do {
			printf(" %s ->", (char *)node->msg);
		} while((node = node->next) != NULL);
		printf("\n");
	}
}

int get_container(const unsigned int clientfd)
{
	size_t container_len = 0;
	if(recv(clientfd, &container_len, sizeof(size_t), 0) < 0) {
		return -1;
	}
	printf("LENGTH:%d\n", (int)container_len);

	char serialized[container_len];
	if(recv(clientfd, serialized, container_len, 0) < 0) {
		return -1;
	}

	// Deserialization
	// https://stackoverflow.com/questions/15707933/how-to-serialize-a-struct-in-c

	Container container;
	
	char *operation = serialized;
	container.operation = *((int *) operation);

	char *queue_name_len = operation + sizeof(int);
	container.queue_name_len = *((int *) queue_name_len);
   	
	char *queue_name = queue_name_len + sizeof(int);
	container.queue_name = (char *)malloc(container.queue_name_len + 1);
	memcpy(container.queue_name, queue_name, container.queue_name_len * 4);
	container.queue_name[container.queue_name_len] = '\0';

	printf("LENGTH:%d\n", (int)strlen(container.queue_name));

	printf("seri: %d LONGITUD NOMBRE->%d\nNombre->|%s|\n", 
		container.operation, container.queue_name_len, container.queue_name);

	/*
	switch (container.operation)
	{
	case CREATE:
		return createMQ(container.queue_name);
		break;
	
	default:
		break;
	}	
	*/
	return -1;
}

int create_server(int port)
{
	char *host = getenv("BROKER_HOST");

    int sockfd;
	struct sockaddr_in self;
    struct hostent *he;
	
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

		get_container(clientfd);
		/*
		while( (read_size = recv(clientfd, client_message_len, sizeof(size_t) , 0)) > 0 ){
			break;
		}
		char client_message[atoi(client_message_len)];

		while( (read_size = recv(clientfd, client_message, atoi(client_message_len) , 0)) > 0 )
		{
			printf("Message received: \n%s\n", client_message);
			break;
		}
		result = detect_method(client_message, msg_from_queue, &size_msg_from_queue) == 0 ? "OK" : "ERROR";
		print_everything();
		send(clientfd, result, strlen(result), 0);
		memset(client_message, '\0', sizeof(client_message));
		msg_from_queue[0] = '\0';
		size_msg_from_queue = 0;
		*/
		close(clientfd);
	}

	close(sockfd);
}

int main(int argc, char *argv[]){

    if(argc!=2) {
        fprintf(stderr, "Uso: %s puerto\n", argv[0]);
        return 1;
    }

	create_server(atoi(argv[1]));
    
    return 0;
}


