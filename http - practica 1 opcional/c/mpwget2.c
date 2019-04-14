#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <math.h>

#include <netdb.h>

#include <regex.h>

#define MAX_NSERVERS 100
#define MAX_NRESOURCES 100
#define MAX_REQUESTS 100
#define MAX_PATH 200
#define MAX_DOMAIN 200
#define PORT 80
#define OBJECT_SIZE 99999

struct resource
{
    unsigned int body_size;
    char path[MAX_PATH];
    unsigned int header_size;
};

struct resources
{
    unsigned int n;
    struct resource resource[MAX_NRESOURCES];
};

struct request
{
    unsigned int index;
    unsigned int range[2];
};

struct requests
{
    struct request request[MAX_NSERVERS * MAX_NRESOURCES];
    unsigned int nrequests;
};

struct server
{
    int sockfd;
    char url[MAX_DOMAIN];
    struct hostent *ip;
    struct requests requests;
};

struct servers
{
    unsigned int n;
    struct server server[MAX_NSERVERS];
};

typedef struct Data
{
    struct resources resources;
    struct servers servers;
} data;

void parse_arguments(int argc, char *argv[], data *dp)
{
    dp->servers.n = 0;
    dp->resources.n = 0;

    for (int i = 1; i < argc; i++)
    {
        char *tmp = argv[i];
        if (tmp[0] == ':')
        {
            // regex url
            strncpy(dp->servers.server[dp->servers.n].url, &tmp[1], strlen(tmp) - 1);
            dp->servers.n++;
        }
        else
        {
            strcpy(dp->resources.resource[dp->resources.n].path, tmp);
            dp->resources.n++;
        }
    }
}

void create_sockets(data *dp)
{
    int sockfd;                /* Socket (de tipo TCP) para transmision de datos */
    struct sockaddr_in server; /* Direccion TCP servidor */
    struct hostent *he;

    for (int i = 0; i < dp->servers.n; i++)
    {
        /* Creacion del socket TCP */
        fprintf(stdout, "CLIENTE:  Creacion del socket TCP: ");
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            fprintf(stdout, "ERROR\n");
            exit(1);
        }
        fprintf(stdout, "OK\n");

        /* Nos conectamos con el servidor */
        bzero((char *)&server, sizeof(struct sockaddr_in)); /* Pone a 0 la estructura */
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);
        if ((he = gethostbyname(dp->servers.server[i % dp->servers.n].url)) == NULL)
        {
            exit(1);
        }
        memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);

        fprintf(stdout, "CLIENTE:  Conexion al puerto servidor: ");
        if (connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0)
        {
            fprintf(stdout, "\n\nERROR %d\n", -13);
            close(sockfd);
            exit(1);
        }
        fprintf(stdout, "OK\n");

        // Maybe remove next line in future
        dp->servers.server[i].ip = he;
        dp->servers.server[i].sockfd = sockfd;
    }
}

char* exec_regex(char *h, char *pattern)
{
    //https://stackoverflow.com/questions/17312336/warning-function-returns-address-of-local-variable-enabled-by-default
    regex_t preg;
    int rc;
    size_t nmatch = 2;
    regmatch_t pmatch[2];

    if (0 != (rc = regcomp(&preg, pattern, 0)) || 0 != (rc = regexec(&preg, h, nmatch, pmatch, 0)))
    {
        return "";
    }

    char size[OBJECT_SIZE] = "";
    sprintf(size, "%.*s", pmatch[1].rm_eo - pmatch[1].rm_so, &h[pmatch[1].rm_so]);
    return size;
}

// Return the size of an object in a server which has been connected previously
// Return -1 if something went wrong
void set_sizes(data *dp, unsigned int sockfd, char *path, int i)
{
    // Send a request without body (HEAD method). Only receive headers.
    char *method = "HEAD %s HTTP/1.0\r\n\r\n";
    char req[MAX_PATH + sizeof(method)] = "";
    sprintf(req, method, path);
    send(sockfd, req, sizeof(req), 0);

    // Receive the request
    char res[OBJECT_SIZE] = "";
    recv(sockfd, res, OBJECT_SIZE, 0);

    // Plus 4 because we need the start of the body in response
    dp->resources.resource[i].header_size = strlen(res) + 4;
    dp->resources.resource[i].body_size = atoi(exec_regex(res, "Content-Length: \\([0-9]*\\)"));
}

void handle_requests(data *dp)
{
    for (int i = 0; i < dp->resources.n; i++)
    {
        set_sizes(dp, dp->servers.server[dp->servers.n % i].sockfd, dp->resources.resource[i].path, i);
        int range = ceil(dp->resources.resource[i].body_size / dp->servers.n);
        for (int j = 0; j < dp->servers.n; j++)
        {
            int start = j * range;
            int end = (j + 1) * range;

            char req[MAX_PATH + 21] = "";
            sprintf(req, "GET %s HTTP/1.0\r\n\r\n", dp->resources.resource[i].path);
            send(dp->servers.server[j].sockfd, req, strlen(req), 0);
        }
    }

    for (int i = 0; i < dp->resources.n; i++)
    {
        for (int j = 0; j < dp->servers.n; j++)
        {
            int object_size = dp->resources.resource[i].header_size + dp->resources.resource[i].body_size;
            char res[dp->resources.resource[i].header_size + dp->resources.resource[i].body_size];
            recv(dp->servers.server[j].sockfd, res, /*object_size*/99999,MSG_WAITALL);
            printf("Reponse: %s\n", res);
        }
    }
}

int main(int argc, char *argv[])
{
    struct Data d;

    parse_arguments(argc, argv, &d);
    create_sockets(&d);
    handle_requests(&d);
}