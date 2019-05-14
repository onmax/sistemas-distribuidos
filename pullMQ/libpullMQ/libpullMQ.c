#include <stdio.h>

#include "comun.h"
#include "pullMQ.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>		//printf
#include <string.h>		//strlen
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
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

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
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
	if (connect(socket_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		return -1;
	}
	return socket_fd;
}

//mayve operation type short?
int send_request(const unsigned int operation, const char *queue_name,
				 const void *put_msg, size_t put_msg_len,
				 void **get_msg, size_t *get_msg_len)
{
	int socket_fd;
	char *serialized = 0;
	if ((socket_fd = get_connected_socket(&socket_fd)) < 0)
	{
		return -1;
	}

	size_t size = sizeof(operation) +
				  strlen(queue_name) + sizeof(strlen(queue_name)) + (operation == PUT ? put_msg_len + sizeof(put_msg_len) : 0);
	// Serialization
	// https://stackoverflow.com/questions/15707933/how-to-serialize-a-struct-in-c
	size_t offset = 0;

	serialized = calloc(1, size);

	memcpy(serialized + offset, &operation, sizeof(operation));
	offset += sizeof(operation);

	size_t name_len = strlen(queue_name);
	memcpy(serialized + offset, &name_len, sizeof(name_len));
	offset += sizeof(size_t);

	memcpy(serialized + offset, queue_name, strlen(queue_name));
	offset += strlen(queue_name);

	if (operation == PUT)
	{
		memcpy(serialized + offset, &put_msg_len, sizeof(put_msg));
		offset += sizeof(put_msg_len);

		memcpy(serialized + offset, put_msg, put_msg_len);
		offset += put_msg_len;
	}

	uint32_t size_net = htonl(size);
	if (send(socket_fd, &size_net, sizeof(size_t), 0) < 0)
	{
		return -1;
	}
	if (send(socket_fd, serialized, size, 0) < 0)
	{
		return -1;
	}
	free(serialized);
	size_t reply_len = 0;
	uint32_t reply_len32 = 0;
	if (recv(socket_fd, &reply_len32, sizeof(size_t), MSG_WAITALL) < 0)
	{
		return -1;
	}

	if ((reply_len = ntohl(reply_len32)) < 0)
	{
		return 0;
	}

	char reply[reply_len];
	if (recv(socket_fd, &reply, reply_len, MSG_WAITALL) < 0)
	{
		return -1;
	}
	close(socket_fd);

	// Deserialize
	int status = *((int *)reply);
	if (status != operation)
	{
		return -1;
	}

	if (status == GET)
	{
		*get_msg_len = 0;
		*get_msg = 0;

		char *msg_len = reply + sizeof(int);
		*get_msg_len = *((size_t *)msg_len);
		char *msg = msg_len + sizeof(size_t);
		*get_msg = malloc(*get_msg_len);
		memcpy(*get_msg, msg, *get_msg_len);
	}

	return 0;
}

int createMQ(const char *queue_name)
{
	return send_request(CREATE, queue_name, NULL, 0, NULL, 0);
}

int destroyMQ(const char *queue_name)
{
	return send_request(DESTROY, queue_name, NULL, 0, NULL, 0);
}

int put(const char *queue_name, const void *msg, size_t size)
{
	return send_request(PUT, queue_name, msg, size, NULL, 0);
}

int get(const char *queue_name, void **msg, size_t *size, bool blocking)
{
	return send_request(GET, queue_name, NULL, 0, msg, size);
}
