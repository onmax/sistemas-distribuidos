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
	for (int i = 0; i < queues.size; i++)
	{
		if (strcmp(queues.array[i].name, name) == 0)
		{
			printf(" in position %d\n", i + 1);
			return i;
		}
	}
	printf(" which has not been found.\n");
	return -1;
}

int queue_search_node(Queue *q, struct Node *node, struct Node **result)
{
	struct Node *current = q->last;
	do
	{
		if (current->next == node)
		{
			*result = current;
			return 0;
		}
	} while ((current = current->next) != NULL);

	return -1;
}

int queue_create(Queue *q, const char *name)
{
	// TODO check malloc
	Queue *temp;
	temp = (Queue *)malloc(sizeof(Queue));
	char *tempname = (char *)malloc(strlen(name));
	strcpy(tempname, name);
	temp->name = tempname;
	temp->first = NULL;
	temp->last = NULL;
	*q = *temp;
	return 0;
}

int createMQ(const char *name)
{
	if (initialized == false)
	{
		queues.array = (Queue *)malloc(0);
		queues.size = 0;
		initialized = true;
	}

	// Comprobar que no haya colas con ese nombre
	for (int i = 0; i < queues.size; i++)
	{
		if (strcmp(queues.array[i].name, name) == 0)
		{
			printf("There is a queue already with name %s\n", name);
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
	if ((queue_create(&queue, name)) < 0)
	{
		return -1;
	}

	// Meter la cola en el array
	queues.array[queues.size - 1] = queue;

	return 0;
}

int queue_destroy(Queue *q)
{
	struct Node *head = q->first;

	while (head != NULL)
	{
		free(head->msg);
		free(head);
		head = head->next;
	}
	return 0;
}

int destroyMQ(const char *name)
{
	// Obtenemos la cola, devuleve -1 en caso de que no exista
	Queue q;
	int index = -1;
	if ((index = get_index(name)) < 0)
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

int queue_push(Queue *q, const void *msg, size_t size)
{
	struct Node *node;
	node = (struct Node *)malloc(sizeof(struct Node));
	node->msg = (void *)malloc(size);
	memcpy(node->msg, msg, size);
	node->size = size;

	if (q->first == NULL)
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

int put(const char *name, const void *msg, size_t size)
{
	// Obtenemos la cola, devuleve -1 en caso de que no exista
	Queue q;
	int index;
	if ((index = get_index(name)) < 0)
	{
		return -1;
	}
	q = queues.array[index];

	queue_push(&q, msg, size);

	queues.array[index] = q;

	return 0;
}

int queue_pop(Queue *q, void **msg, size_t *tam)
{
	if (q->first == NULL)
	{
		return -1;
	}
	struct Node *first = q->first;

	*msg = (void *)first->msg;
	*tam = first->size;

	struct Node *second;
	if (queue_search_node(q, first, &second) < 0)
	{
		q->last = NULL;
		q->first = NULL;
		return 0;
	}

	second->next = NULL;
	q->first = second;

	return 0;
}

int get(const char *name, void **msg, size_t *tam, bool blocking)
{
	// Obtenemos la cola, devuleve -1 en caso de que no exista
	Queue q;
	int index;

	if ((index = get_index(name)) < 0)
	{
		return -1;
	}
	q = queues.array[index];
	if (queue_pop(&q, msg, tam) < 0)
	{
		return -1;
	}
	queues.array[index] = q;
	return 0;
}

void print_everything()
{
	printf("Array:\n");
	struct Node *node;
	Queue queue;
	for (int i = 0; i < queues.size; i++)
	{
		queue = queues.array[i];
		printf("  %d. %s: ", i, queue.name);
		node = queue.last;
		if (node == NULL)
		{
			printf("\n");
			continue;
		}

		do
		{
			printf(" %s (%lu) ->", (char *)node->msg, node->size);
		} while ((node = node->next) != NULL);
		printf("\n");
	}
}

int send_error(int clientfd)
{
	int i = -1;
	size_t size = sizeof(i);
	if (send(clientfd, &size, sizeof(size), 0) < 0)
	{
		return -1;
	}

	if (send(clientfd, &i, size, 0) < 0)
	{
		return -1;
	}

	return 0;
}

Request  deserialize(char serialized[])
{
	// https://stackoverflow.com/questions/15707933/how-to-serialize-a-struct-in-c

	Request request;

	char *operation = serialized;
	request.operation = *((int *)operation);

	char *queue_name_len = operation + sizeof(int);
	request.queue_name_len = *((size_t *)queue_name_len);

	void *queue_name = queue_name_len + sizeof(size_t);
	request.queue_name = malloc(request.queue_name_len);
	memcpy(request.queue_name, queue_name, request.queue_name_len);
	request.queue_name[request.queue_name_len] = '\0';

	if (request.operation == PUT)
	{
		char *msg_len = queue_name + request.queue_name_len;
		request.msg_len = *((size_t *)msg_len);

		void *msg = msg_len + sizeof(size_t);
		request.msg = malloc(request.msg_len);
		memcpy(request.msg, msg, request.msg_len);
	}
	return request;
}

int process_request(const unsigned int clientfd)
{
	size_t request_len = 0;
	if (recv(clientfd, &request_len, sizeof(size_t), 0) < 0)
	{
		send_error(clientfd);
		return 0;
	}

	char request_serialized[request_len];

	if (recv(clientfd, &request_serialized, request_len, 0) < 0)
	{
		send_error(clientfd);
		return -1;
	}

	Request request = deserialize(request_serialized);

	void *msg;
	size_t msg_len = 0;
	int status;

	switch (request.operation)
	{
	case CREATE:
		printf("Creating new queue with name: %s\n", request.queue_name);
		status = createMQ(request.queue_name);
		break;
	case DESTROY:
		printf("Destroying %s", request.queue_name);
		status = destroyMQ(request.queue_name);
		break;
	case PUT:
		printf("Pushing new item to %s", request.queue_name);
		status = put(request.queue_name, request.msg, request.msg_len);
		break;
	case GET:
		printf("Getting item from %s", request.queue_name);
		status = get(request.queue_name, &msg, &msg_len, false);
		break;
	}


	size_t size = sizeof(status) + (request.operation == GET && status == 0 ? (msg_len + sizeof(msg_len)) : 0);
	size_t offset = 0;
	char *response_serialized = 0;

	response_serialized = malloc(size);
	int i = status < 0 ? status : request.operation;
	memcpy(response_serialized + offset, &i, sizeof(i));
	offset += sizeof(int);

	if (request.operation == GET && status == 0)
	{
		memcpy(response_serialized + offset, &msg_len, sizeof(msg_len));
		offset += sizeof(msg_len);

		memcpy(response_serialized + offset, msg, msg_len);
		offset += msg_len;
	}

	if (send(clientfd, &size, sizeof(size_t), 0) < 0)
	{
		return -1;
	}

	if (send(clientfd, response_serialized, size, 0) < 0)
	{
		return -1;
	}
	free(response_serialized);
	msg = 0;
	if(request.operation == PUT)
	{
		free(request.msg);
	}
	return -1;
}

int create_server(int port)
{
	char *host = getenv("BROKER_HOST");

	int sockfd;
	struct sockaddr_in self;
	struct hostent *he;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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
	if (bind(sockfd, (struct sockaddr *)&self, sizeof(self)) != 0)
	{
		perror("socket--bind");
		exit(errno);
	}

	if (listen(sockfd, 20) != 0)
	{
		perror("socket--listen");
		exit(errno);
	}

	while (1)
	{
		int clientfd;
		struct sockaddr_in client_addr;
		socklen_t addrlen = sizeof(client_addr);

		clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
		printf("\n-----------------------------------------------\n");
		printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		process_request(clientfd);
		print_everything();
		printf("-----------------------------------------------\n");
		close(clientfd);
	}
	close(sockfd);
}

int main(int argc, char *argv[])
{

	if (argc != 2)
	{
		fprintf(stderr, "Uso: %s puerto\n", argv[0]);
		return 1;
	}

	create_server(atoi(argv[1]));

	return 0;
}
