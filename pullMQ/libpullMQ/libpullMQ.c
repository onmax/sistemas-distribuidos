#include <stdio.h>

#include "comun.h"
#include "pullMQ.h"
#include <string.h>
#include <stdlib.h>
#include<stdio.h>	//printf
#include<string.h>	//strlen
#include<sys/socket.h>	//socket
#include<arpa/inet.h>	//inet_addr
#include <netdb.h>
#include <stdio.h>
#include "../libpullMQ/pullMQ.h"

#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h> 

int get_connected_socket()
{
	int socket_fd;
	struct sockaddr_in server;

	socket_fd = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_fd == -1)
	{
		return -1;
	}
    struct hostent *he;
    char *host = getenv("BROKER_HOST");
    int port = atoi(getenv("BROKER_PORT"));
    
	server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if ((he = gethostbyname(host)) == NULL)
    {
        return -1;
    }

    memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);

	//Connect to remote server
	if (connect(socket_fd , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		return -1;
	}
	return socket_fd;
}

int send_request(const unsigned int operation, const char *queue_name, void *msg, size_t msg_len)
{
	int socket_fd;
	char *serialized = 0;
	char reply[10];

	if((socket_fd = get_connected_socket(&socket_fd)) < 0)
	{
		return -1;
	}

	size_t size = sizeof(operation) +
					strlen(queue_name) + sizeof(strlen(queue_name)) * 2;
					
	// Serialization
	// https://stackoverflow.com/questions/15707933/how-to-serialize-a-struct-in-c

	size_t offset = 0;
	serialized = calloc(1, size);

	serialized[offset] = operation;
	offset += sizeof(operation);

	serialized[offset] = strlen(queue_name);
	offset += sizeof(size_t);

	memcpy(serialized + offset, queue_name, strlen(queue_name));
	offset += strlen(queue_name);
	
	if((msg != NULL))
	{
	// TODO msg with the SIZE. Not fininished!!!
		size += msg_len + sizeof(msg_len);
		serialized = realloc(serialized, size);

		serialized[offset] = (int)strlen(queue_name);
		offset += sizeof(int);
	}

	size_t serialized_len = size + sizeof(offset) * 2;
	if(send(socket_fd, &serialized_len, sizeof(size_t), 0) < 0)
	{
		return -1;
	}
	
	if(send(socket_fd, serialized, offset, 0) < 0)
	{
		return -1;
	}	

	// TODO check response str
	return 0;
}

int createMQ(const char *queue_name)
{
	return send_request(CREATE, queue_name, NULL, 0);
}

int destroyMQ(const char *queue_name)
{
	return send_request(DESTROY, queue_name, NULL, 0);
}

int put(const char *queue_name, const void *mensaje, size_t tam)
{
	return send_request(PUT, queue_name, NULL, 0);
}

int get(const char *queue_name, void **mensaje, size_t *tam, bool blocking)
{
	return send_request(GET, queue_name, NULL, 0);
}
