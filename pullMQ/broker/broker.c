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
#include <sys/socket.h>

#define bool int
#define false 0
#define true 1

// If DEBUG is true, the program prints information for every request
// and also prints the status of the queue after every operation
// requested by the client
const bool DEBUG = true;

// Client(libpullMQ) should send the following structure serialized.
// Attributes that should have the request for every operation:
// 		CREATE: operation, queue_name_len, queue_name
// 		DESTROY: operation, queue_name_len, queue_name
// 		PUT: operation, queue_name_len, queue_name, msg_len, msg
// 		DESTROY: operation, queue_name_len, queue_name, blocking
typedef struct
{
	int operation;
	size_t queue_name_len;
	char *queue_name;
	size_t msg_len;
	void *msg;
	bool blocking;
} Request;

// Node of the queue. If the node is the last should be pointing to NULL
struct Node
{
	size_t size;
	void *msg;
	struct Node *next;
};

// Information about a queue.
// TODO: Remove last node and only work with the head
// awaiting: It is an array with ints which are the client sockets that
// have request a blocking request
// If there are no nodes in the queue, head and last should be pointing
// to NULL
typedef struct
{
	char *name;
	struct Node *first;
	struct Node *last;
	int *awaiting;
	int n_awaiting;
} Queue;

// Array of queues
typedef struct
{
	Queue *array;
	int size;
} Queues;

// Variable that contains all the queues.
// It is initialize in main()
Queues queues;

/***********************  DEBUG  ***********************/
// Prints the string given in the argument. If the string is too long
// (length greater than 30 chars), then it will only print the first
// 30 chars
void print_name(const char *name);

// Prints the void given in the argument (it is converted to char *).
// If the size is greater than 30, then it will only print the first
// 30 chars
void print_message(const void *message, size_t size);

// Prints the content of the array of queues (with the messages that
// contains every queue and the array of clients sockets that request
// a blocking get)
void print_everything();
/*********************  END DEBUG  *********************/

/***********************  QUEUE  ***********************/
// Create a new queue with everything set to empty or 0 except for the
// name
int queue_create(Queue *q, char *name);

// Removes a queue: remove all the nodes and free awaiting array
int queue_destroy(Queue *q);

// Creates a new node and append it to the queue
int queue_push(Queue *q, void *msg, size_t size);

// Gets head node from the queue q, reads data, set msg and size and
// free the node
int queue_pop(Queue *q, void **msg, size_t *size, bool blocking, int client_fd);

// Looks for a node with next attribute equals to second argument
// and set it in the memory position given by the third argument
int queue_search_node(Queue *q, struct Node *node, struct Node **result);

// Push a client socket to awaiting array
int awaiting_arr_push(Queue *q, int client_socket);

// Get the first client socket from awaiting array
// If it return -10, the array is empty
int awaiting_arr_pop(Queue *q);

// It will send data to awaiting sockets (if they don't went down)
// Remove the first element of the array of waiting
int send_data_to_awaiting_socket(Queue *q, void *msg, size_t size);
/*********************  END QUEUE  *********************/

/******************  ARRAY OF QUEUES  ******************/
// Returns the index of a queue in the array. If it doesn't exists returns
// -1
int get_index(const char *name);

// Creates a new queue and pushed to the array of queues
int createMQ(char *name);

// Destroy queue and removed from the array of queues.
// If the queue doesn't exists return -1
int destroyMQ(const char *name);

// Push a new message to a queue and update the array of queues.
// If the queue doesn't exists return -1
int put(const char *name, void *msg, size_t size);

// Get a new message from a queue, remove the message from the queue
// and update the array of queues.
// If the queue doesn't exists return -1
// If blocking is set to true, the client socket will be left open
// until a put request is made to the queue. In that case the broker
// will send the data to the client who made the get blocking
// request
int get(const char *name, void **msg, size_t *size, bool blocking, int client_fd);
/****************  END ARRAY OF QUEUES  ****************/

/******************  SERVER FUNCTIONS ******************/
// Returns a response to client_fd. It will send two responses
// The first one with the size. The second one with the data
int send_response(int client_fd, void *data, size_t size);

// Given a array of chars it will convert the data to a Request
// struct. At least the request struct will have operation,
// name and name_len. See Request struct for further info
Request deserialize(char *serialized);

// TODO
int serialize(int operation, int status, void *msg, size_t msg_len, char **serialized, size_t *serialized_len);

// For every connection from a client it will receive data,
// convert it to a struct, made the operation requested
// by client (CREATE, DESTROY, PUT, GET) and send back a response
int process_request(const unsigned int client_fd);

// It will create a server socket listening to port number
// given in the argument. For every request it will call to
// process_request
int create_server(int port);
/****************  END SERVER FUNCTIONS ****************/

/***********************  DEBUG  ***********************/
void print_name(const char *name)
{
	if (strlen(name) > 30)
	{
		char res[50];
		strncpy(res, name, 30);
		printf("%s...(%lu): ", res, strlen(name));
	}
	else
		printf("%s(%lu)", name, strlen(name));
}

void print_message(const void *message, size_t size)
{
	if (size > 30)
	{
		void *res;
		res = malloc(50);
		memcpy(res, message, 30);
		printf("\t%s...(%lu)\n", (char *)res, size);
		free(res);
	}
	else
		printf("\t%s(%lu)\n", (char *)message, size);
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
			continue;

		do
		{
			print_message(node->msg, node->size);
		} while ((node = node->next) != NULL);
		printf("\n");
	}
}
/*********************  END DEBUG  *********************/

/***********************  QUEUE  ***********************/
int queue_create(Queue *q, char *name)
{
	// TODO check malloc
	Queue *temp;
	temp = (Queue *)malloc(sizeof(Queue));
	temp->name = name;
	temp->first = NULL;
	temp->last = NULL;
	temp->awaiting = malloc(0);
	temp->n_awaiting = 0;
	*q = *temp;
	return 0;
}

int queue_destroy(Queue *q)
{
	struct Node *head = q->first;

	for (int i = 0; i < q->n_awaiting; i++)
	{
		if (DEBUG)
			printf("\nSending error to %d\n", q->awaiting[i]);
		int err = 100;
		send_response(q->awaiting[i], &err, sizeof(int));
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

int queue_push(Queue *q, void *msg, size_t size)
{
	struct Node *node;
	node = (struct Node *)malloc(sizeof(struct Node));
	node->msg = malloc(size);
	memcpy(node->msg, msg, size);
	node->size = size;

	if (q->first == NULL)
	{
		if (send_data_to_awaiting_socket(q, msg, size) >= 0)
			return 0;
		node->next = NULL;
		q->first = node;
	}
	else
		node->next = q->last;
	q->last = node;
	return 0;
}

int queue_pop(Queue *q, void **msg, size_t *size, bool blocking, int client_fd)
{
	if (q->first == NULL)
	{
		if (blocking)
		{
			if (DEBUG)
				printf(":blocked:");
			awaiting_arr_push(q, client_fd);
			return 1;
		}
		else
		{
			*size = 0;
			return 2;
		}
	}
	struct Node *first = q->first;

	*msg = first->msg;
	*size = first->size;

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

int awaiting_arr_push(Queue *q, int client_socket)
{
	q->n_awaiting++;
	q->awaiting = (int *)realloc(q->awaiting, q->n_awaiting * sizeof(int));
	q->awaiting[q->n_awaiting - 1] = client_socket;
	return 0;
}

int awaiting_arr_pop(Queue *q)
{
	if (q->n_awaiting <= 0)
		return -1;

	int client_fd = q->awaiting[0];
	q->n_awaiting--;
	int *temp = malloc(q->n_awaiting * sizeof(int));
	memcpy(
		temp,
		q->awaiting + 1,
		q->n_awaiting * sizeof(int));

	free(q->awaiting);
	q->awaiting = temp;
	return client_fd;
}

int send_data_to_awaiting_socket(Queue *q, void *msg, size_t size)
{
	int client_fd;
	if ((client_fd = awaiting_arr_pop(q)) < 0)
	{
		return client_fd;
	}
	char *serialized;
	size_t serialized_len;
	if (serialize(GET, 0, msg, size, &serialized, &serialized_len) < 0)
	{
		return -1;
	}
	if (send_response(client_fd, serialized, serialized_len))
	{
		queue_push(q, msg, size);
		return -1;
	}
	return 0;
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
/*********************  END QUEUE  *********************/

/******************  ARRAY OF QUEUES  ******************/
int get_index(const char *name)
{
	for (int i = 0; i < queues.size; i++)
	{
		if (strcmp(queues.array[i].name, name) == 0)
		{
			if (DEBUG)
				printf(" in position %d. ", i + 1);
			return i;
		}
	}
	if (DEBUG)
		printf(" which has not been found. ");
	return -1;
}

int createMQ(char *name)
{
	// Checks that there is no queue with that name
	if (get_index(name) >= 0)
		return -1;

	// Allocate memory
	queues.size++;
	queues.array = (Queue *)realloc(queues.array, queues.size * sizeof(*queues.array));
	if (queues.array == NULL)
		return -1;

	// Create queue
	Queue queue;
	if ((queue_create(&queue, name)) < 0)
		return -1;

	// Push the queue to array
	queues.array[queues.size - 1] = queue;
	return 0;
}

int destroyMQ(const char *name)
{
	Queue q;
	int index;

	// Checks that the queue exists
	if ((index = get_index(name)) < 0)
		return -1;

	q = queues.array[index];
	free(queues.array[index].name);
	queue_destroy(&q);

	queues.size--;

	// Allocate an array with a size 1 less than the current one
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

int put(const char *name, void *msg, size_t size)
{
	Queue q;
	int index;

	// Checks that the queue exists
	if ((index = get_index(name)) < 0)
		return -1;

	q = queues.array[index];

	queue_push(&q, msg, size);

	queues.array[index] = q;

	return 0;
}

int get(const char *queue_name, void **msg, size_t *size, bool blocking, int client_fd)
{
	Queue q;
	int index;

	// Checks that the queue exists
	if ((index = get_index(queue_name)) < 0)
		return -1;

	q = queues.array[index];
	int status;
	if ((status = queue_pop(&q, msg, size, blocking, client_fd)) < 0)
		return -1;

	queues.array[index] = q;
	return status;
}
/****************  END ARRAY OF QUEUES  ****************/

/******************  SERVER FUNCTIONS ******************/
int send_response(int client_fd, void *data, size_t size)
{
	uint32_t size_net = htonl(size);
	if (send(client_fd, &size_net, sizeof(size), MSG_NOSIGNAL) < 0)
	{
		return -1;
	}
	if (send(client_fd, data, size, MSG_NOSIGNAL) < 0)
	{
		return -1;
	}

	if (EPIPE == errno)
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
		request.blocking = *((char *)serialized) == '1';
	return request;
}

int serialize(int operation, int status, void *msg, size_t msg_len, char **serialized, size_t *serialized_len)
{
	size_t size = sizeof(status) + (operation == GET && status == 0 ? (msg_len + sizeof(msg_len)) : 0);
	size_t offset = 0;
	char *response_serialized = 0;

	response_serialized = malloc(size);
	int i = status < 0 ? status : operation;
	memcpy(response_serialized + offset, &i, sizeof(i));
	offset += sizeof(int);

	if (operation == GET && (status == 0 || status == 2))
	{
		memcpy(response_serialized + offset, &msg_len, sizeof(msg_len));
		offset += sizeof(msg_len);

		memcpy(response_serialized + offset, msg, msg_len);
		offset += msg_len;
	}

	*serialized_len = size;
	*serialized = response_serialized;
	return 0;
}

int process_request(const unsigned int client_fd)
{
	size_t request_len = 0;
	uint32_t request_len32 = 0;

	if (recv(client_fd, &request_len32, sizeof(size_t), 0) < 0 ||
		(request_len = ntohl(request_len32)) < 0)
	{
		int err = 100;
		send_response(client_fd, &err, sizeof(int));
		return 0;
	}

	char *request_serialized = malloc(request_len);
	if (request_serialized == NULL ||
		recv(client_fd, request_serialized, request_len, MSG_WAITALL) < 0)
	{
		free(request_serialized);
		int err = 100;
		send_response(client_fd, &err, sizeof(int));
		return -1;
	}

	Request request = deserialize(request_serialized);

	void *msg;
	size_t msg_len = 0;
	int status;
	switch (request.operation)
	{
	case CREATE:
		if (DEBUG)
		{
			printf("Creating new queue with name ");
			print_name(request.queue_name);
		}
		status = createMQ(request.queue_name);
		break;
	case DESTROY:
		if (DEBUG)
		{
			printf("Destroying ");
			print_name(request.queue_name);
		}
		status = destroyMQ(request.queue_name);
		break;
	case PUT:
		if (DEBUG)
		{
			printf("Pushing new item to ");
			print_name(request.queue_name);
		}
		status = put(request.queue_name, request.msg, request.msg_len);
		break;
	case GET:
		if (DEBUG)
		{
			printf("Getting item from ");
			print_name(request.queue_name);
		}
		status = get(request.queue_name, &msg, &msg_len, request.blocking, client_fd);
		break;
	}
	if (DEBUG)
		printf("\n");

	if (status == 1)
		return 1;

	free(request_serialized);

	char *serialized;
	size_t serialized_len;
	if (serialize(request.operation, status, msg, msg_len, &serialized, &serialized_len) < 0)
	{
		return -1;
	}
	return send_response(client_fd, serialized, serialized_len);
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
		exit(1);

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
		int client_fd;
		struct sockaddr_in client_addr;
		socklen_t addrlen = sizeof(client_addr);

		client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
		if (DEBUG)
		{
			printf("\n-----------------------------------------------\n");
			printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		}
		int status = process_request(client_fd);
		if (DEBUG)
		{
			queues.size > 0
				? print_everything()
				: printf("No queues\n");
			printf("-----------------------------------------------\n");
		}
		if (status != 1)
			close(client_fd);
	}
	close(sockfd);
}
/****************  END SERVER FUNCTIONS ****************/

int main(int argc, char *argv[])
{

	if (argc != 2)
	{
		fprintf(stderr, "Uso: %s puerto\n", argv[0]);
		return 1;
	}

	queues.array = (Queue *)malloc(0);
	queues.size = 0;

	create_server(atoi(argv[1]));

	return 0;
}
