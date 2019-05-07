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
	char *serialized  = 0;
	char reply[10];

	if((socket_fd = get_connected_socket(&socket_fd)) < 0)
	{
		return -1;
	}
	size_t size = sizeof(operation) +
					strlen(queue_name) + sizeof(strlen(queue_name));
			
	if(msg != NULL)
	{
		size += msg_len + sizeof(msg_len);
	}

	// Serialization
	// https://stackoverflow.com/questions/15707933/how-to-serialize-a-struct-in-c

	size_t offset = 0;
	serialized = malloc(size + sizeof(offset));

	serialized[offset] = operation;
	offset += sizeof(operation);

	serialized[offset] = (int)strlen(queue_name);
	offset += sizeof(int);

	strncpy(serialized + offset, queue_name, strlen(queue_name));
	offset += strlen(queue_name);
	
	// TODO msg with the SIZE
	size_t serialized_len = size + sizeof(offset);
	printf("Size sent: %lu\n", serialized_len);
	
	if(send(socket_fd, &serialized_len, sizeof(size_t), 0) < 0)
	{
		return -1;
	}
	
	if(send(socket_fd, serialized, offset, 0) < 0)
	{
		return -1;
	}	
	printf("Puntero: %p\n", serialized);

	Container container;
	
	char *oo = serialized;
	container.operation = *((int *) oo);

	char *ll = oo + sizeof(int);
	container.queue_name_len = *((int *) ll);
   	
	char *nn = ll + sizeof(int);
	container.queue_name = (char *)malloc(container.queue_name_len + 1);
	memcpy(container.queue_name, nn, container.queue_name_len);
	container.queue_name[container.queue_name_len] = '\0';

	printf("seri: %d LONGITUD NOMBRE->%d\nNombre->|%s|\n", 
		container.operation, container.queue_name_len, container.queue_name);
	return 0;
	//return strcmp(reply, "OK");
}

int createMQ(const char *queue_name)
{
	if(send_request(CREATE, queue_name, NULL, 0) < 0)
		return -1;
	
	printf("ENVAIDO\n");
	/*
	if( send(socket_fd , 'X' , 1 , 0) < 0)
	{
		return -1;
	}
	
	
	if( recv(socket_fd , reply , 2000 , 0) < 0)
	{
		return -1;
	} 

	close(socket_fd);
	printf("RES: %s\n", reply);
	if(strcmp(reply, "OK"))
		return 0;
	else
		return -1;
		*/
	return 0;
}

int destroyMQ(const char *queue_name)
{
	return 0;
}

int put(const char *queue_name, const void *mensaje, size_t tam)
{
	return 0;

}

int get(const char *queue_name, void **mensaje, size_t *tam, bool blocking)
{
	return 0;

}
