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
#include <resolv.h>
#include <arpa/inet.h>
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

int createMQ(const char *queue_name)
{
	int socket_fd;
	
	const char *method = "CREATE\r";
	char *message = malloc(strlen(queue_name) + strlen(method));

	char reply[10];
	if((socket_fd = get_connected_socket(&socket_fd)) < 0)
	{
		return -1;
	}
	strcat(message, method);
	strcat(message, queue_name);
	if( send(socket_fd , message , strlen(message) , 0) < 0)
	{
		return -1;
	}
	
	
	if( recv(socket_fd , reply , 2000 , 0) < 0)
	{
		return -1;
	}

	close(socket_fd);
	if(strcmp(reply, "OK"))
		return 0;
	else
		return -1;
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
