#include <stdio.h>
#include "comun.h"

#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct
{
	int operation;
	size_t queue_name_len;
	char *queue_name;
	size_t msg_len;
	void *msg;
	bool blocking;
} Request;

struct Node
{
	size_t size;
	void *msg;
	struct Node *next;
};

typedef struct
{
	char *name;
	struct Node *first;
	struct Node *last;
	int *awaiting;
	int n_awaiting;
} Queue;

typedef struct
{
	Queue *array;
	int size;
} Queues;
//TODO reorganize functions
int queue_create(Queue *q, char *name);
int createMQ(char *name);
int queue_destroy(Queue *q);
int destroyMQ(const char *name);
int queue_push(Queue *q, const void *msg, size_t tam);
int put(const char *name, const void *msg, size_t size);
int queue_pop(Queue *q, void **msg, size_t *tam, bool blocking, int client_fd);
int get(const char *name, void **msg, size_t *tam, bool blocking, int client_fd);
int queue_search_node(Queue *q, struct Node *node, struct Node **result);
void print_name(const char *name);
void print_message(const void *message, size_t size);
void print_everything();
int send_error(int clientfd);

Queues queues;
// TODO: initialized var could be removed
bool initialized = false;
void print_name(const char *name)
{
    if(strlen(name) >= 30){
        char res[50];
        strncpy(res, name, 30);
        printf("%s...(%lu): ", res, strlen(name));
    }
    else
    {
        printf("%s(%lu)", name, strlen(name));
    }
}

void print_message(const void *message, size_t size)
{
    if(size >= 30){
        void *res;
        res = malloc(50);
        memcpy(res, message, 30);
        printf("\t%s...(%lu)\n", (char *)res, size);
        free(res);
    }
    else
    {
        printf("\t%s(%lu)\n", (char *)message, size);
    }
}


int get_index(const char *name)
{
	// Buscamos una cola cuyo nombre sea igual a name, devuelve -1 en caso de que no exista
	for (int i = 0; i < queues.size; i++)
	{
		if (strcmp(queues.array[i].name, name) == 0)
		{
			printf(" in position %d. ", i + 1);
			return i;
		}
	}
	printf(" which has not been found. ");
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

int queue_create(Queue *q, char *name)
{
	// TODO check malloc
	Queue *temp;
	temp = (Queue *)malloc(sizeof(Queue));
	//temp->name = (char *)malloc(strlen(name));
	//strcpy(temp->name, name);
	temp->name = name;
	temp->first = NULL;
	temp->last = NULL;
	temp->awaiting = malloc(0);
	temp->n_awaiting = 0;
	*q = *temp;
	return 0;
}

int createMQ(char *name)
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
			printf("There is a queue already with name \n");
			print_name(name);
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

	for(int i = 0; i < q->n_awaiting; i++)
	{
		printf("\nSending error to %d\n", q->awaiting[i]);
		send_error(q->awaiting[i]);
	}
	free(q->awaiting);

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
	// Obtenemos la cola, devuelve -1 en caso de que no exista
	Queue q;
	int index = -1;
	if ((index = get_index(name)) < 0)
	{
		return -1;
	}
	q = queues.array[index];
	free(queues.array[index].name);
	queue_destroy(&q);

	queues.size--;

	// allocate an array with a size 1 less than the current one
	Queue *temp = malloc(queues.size * sizeof(Queue));

	// copy everything BEFORE the index
	memcpy(temp, queues.array, index * sizeof(Queue));

	if (index != queues.size)
		// copy everything AFTER the index
		memcpy(
			temp + index,
			queues.array + index + 1,
			(queues.size - index) * sizeof(Queue));

	free(queues.array);
	queues.array = temp;
	return 0;
}

int awaiting_arr_pop(Queue *q, const void *msg, size_t size)
{
	int client_fd = q->awaiting[0];
	q->n_awaiting--;
	int *temp = malloc(q->n_awaiting * sizeof(int));

	memcpy(
		temp,
		q->awaiting + 1,
		q->n_awaiting * sizeof(int));

	free(q->awaiting);
	q->awaiting = temp;

	size_t serialized_size = GET + size + sizeof(size);
	size_t offset = 0;
	char *response_serialized = 0;

	response_serialized = malloc(serialized_size);
	int operation = GET;

	memcpy(response_serialized + offset, &operation, sizeof(operation));
	offset += sizeof(int);

	memcpy(response_serialized + offset, &size, sizeof(size));
	offset += sizeof(size);

	memcpy(response_serialized + offset, msg, size);
	offset += size;

	uint32_t size_net = htonl(serialized_size);
	if (send(client_fd, &size_net, sizeof(size_t), 0) < 0)
	{
		return -1;
	}

	if (send(client_fd, response_serialized, serialized_size, 0) < 0)
	{
		return -1;
	}

	return 0;
}

int queue_push(Queue *q, const void *msg, size_t size)
{
	struct Node *node;
	node = (struct Node *)malloc(sizeof(struct Node));
	node->msg = malloc(size);
	//TODO
	memcpy(node->msg, msg, size);
	node->size = size;

	if (q->first == NULL)
	{
		if (q->n_awaiting > 0)
		{
			awaiting_arr_pop(q, msg, size);
			return 0;
		}
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
	// Obtenemos la cola, devueleve -1 en caso de que no exista
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

int awaiting_arr_push(Queue *q, int client_socket)
{
	q->n_awaiting++;
	q->awaiting = (int *)realloc(q->awaiting, q->n_awaiting * sizeof(int));
	q->awaiting[q->n_awaiting - 1] = client_socket;
	return 0;
}

int queue_pop(Queue *q, void **msg, size_t *tam, bool blocking, int client_fd)
{
	if (q->first == NULL)
	{
		if (blocking)
		{
			printf(":blocked:");
			awaiting_arr_push(q, client_fd);
			return 1;
		}
		else
		{
			return -1;
		}
	}
	struct Node *first = q->first;

	*msg = first->msg;
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

int get(const char *queue_name, void **msg, size_t *size, bool blocking, int client_fd)
{
	// Obtenemos la cola, devuelve -1 en caso de que no exista
	Queue q;
	int index;

	if ((index = get_index(queue_name)) < 0)
	{
		return -1;
	}
	q = queues.array[index];
	int status;
	if ((status = queue_pop(&q, msg, size, blocking, client_fd)) != 0)
	{
		if (status == 1)
		{
			queues.array[index] = q;
		}

		return status;
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
		printf("  %d. ", i);
		print_name(queue.name);
		printf("\tsockets awaiting: [");
		for (int j = 0; j < queue.n_awaiting; j++)
		{
			printf("%d, ", queue.awaiting[j]);
		}
		printf("]\n");
		node = queue.last;
		if (node == NULL)
		{
			printf("\n");
			continue;
		}

		do
		{
			print_message(node->msg, node->size);
		} while ((node = node->next) != NULL);
		printf("\n");
	}
}

int send_error(int clientfd)
{
	int i = 10;
	uint32_t size = htonl(sizeof(i));
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

Request deserialize(char *serialized)
{
	// https://stackoverflow.com/questions/15707933/how-to-serialize-a-struct-in-c

	Request request;

	request.operation = *((int *)serialized);
	serialized += sizeof(request.operation);

	request.queue_name_len = *((size_t *)serialized);
	serialized += sizeof(request.queue_name_len);

	request.queue_name = malloc(request.queue_name_len + 1);
	// TODO: maybe chatn to strcat??
	memcpy(request.queue_name, serialized, request.queue_name_len);
	request.queue_name[request.queue_name_len] = '\0';
	serialized += request.queue_name_len;

	if (request.operation == PUT)
	{
		request.msg_len = *((size_t *)serialized);
		serialized += sizeof(request.msg_len);

		request.msg = malloc(request.msg_len);
		memcpy(request.msg, serialized, request.msg_len);
	}
	else if (request.operation == GET)
	{
		request.blocking = *((char *)serialized) == '1';
		printf(" IZ block %d\n", request.blocking);
	}
	return request;
}

int process_request(const unsigned int clientfd)
{
	size_t request_len = 0;
	uint32_t request_len32 = 0;
	if (recv(clientfd, &request_len32, sizeof(size_t), 0) < 0)
	{
		send_error(clientfd);
		return 0;
	}
	if ((request_len = ntohl(request_len32)) < 0)
	{
		return 0;
	}

	char *request_serialized = malloc(request_len);
	if (request_serialized == NULL)
	{
		// TODO free serialized
		send_error(clientfd);
		return -1;
	}

	if (recv(clientfd, request_serialized, request_len, MSG_WAITALL) < 0)
	{
		// TODO free serialized
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
		printf("Creating new queue with name: ");
		print_name(request.queue_name);
		status = createMQ(request.queue_name);
		break;
	case DESTROY:
		printf("Destroying ");
		print_name(request.queue_name);
		status = destroyMQ(request.queue_name);
		break;
	case PUT:
		printf("Pushing new item to ");
		print_name(request.queue_name);
		status = put(request.queue_name, request.msg, request.msg_len);
		break;
	case GET:
		printf("Getting item from ");
		print_name(request.queue_name);
		status = get(request.queue_name, &msg, &msg_len, request.blocking, clientfd);
		break;
	}
	printf("\n");
	if (status == 1)
	{
		return 1;
	}

	free(request_serialized);
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

	uint32_t size_net = htonl(size);
	if (send(clientfd, &size_net, sizeof(size_t), 0) < 0)
	{
		return -1;
	}

	if (send(clientfd, response_serialized, size, 0) < 0)
	{
		return -1;
	}
	free(response_serialized);
	msg = 0;
	if (request.operation == PUT)
	{
		free(request.msg);
	}

	return 0;
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
		int status = process_request(clientfd);
		if (queues.size != 1)
			print_everything();
		printf("-----------------------------------------------\n");
		if (status != 1)
		{
			close(clientfd);
		}
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
