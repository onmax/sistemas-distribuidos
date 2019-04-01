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
#define PORT 80
#define OBJECT_SIZE 99999


struct request
{
   unsigned int   sockfd;
   unsigned int   range[2];
};

struct resource
{
   struct         request request[MAX_REQUESTS];
   unsigned int   nrequest;
   unsigned int   size;
   char           path[MAX_PATH];
};

struct resources {
   unsigned int   n;
   struct         resource resource[MAX_NRESOURCES];
};

struct server {
   int            sockfd;
   char           url[MAX_PATH*2 + 2];
   struct hostent *ip;
};

struct servers {
   unsigned int   n;
   struct         server server[MAX_NSERVERS];
};

typedef struct Data {
   struct         resources resources;
   struct         servers servers;
} data;  


void  parse_arguments(int argc, char* argv[], data *dp);
void  set_requests(data *dp);
int   get_size(unsigned int sockfd, char *path);
void  create_sockets(data *dp);

void parse_arguments(int argc, char* argv[], data *dp)
{
   dp->servers.n = 0;
   dp->resources.n = 0;

   for(int i=1;i<argc;i++)
   {
      char* tmp = argv[i];
      if(tmp[0] == ':') {
         strncpy(dp->servers.server[dp->servers.n].url, &tmp[1], strlen(tmp) - 1);
         dp->servers.n++;
      } else {
         strcpy(dp->resources.resource[dp->resources.n].path, tmp);
         dp->resources.n++;
      }
   }
}

int exec_regex(char *h, char *pattern)
{
   regex_t    preg;
   int        rc;
   size_t     nmatch = 2;
   regmatch_t pmatch[2];
 
   if (0 != (rc = regcomp(&preg, pattern, 0)) || 0 != (rc = regexec(&preg, h, nmatch, pmatch, 0))) {
      return -1;
   }

   char size[OBJECT_SIZE] = "";
   sprintf(size, "%.*s", pmatch[1].rm_eo - pmatch[1].rm_so, &h[pmatch[1].rm_so]);
   return atoi(size);
}

// Return the size of an object in a server which has been connected previously
// Return -1 if something went wrong
int get_size(unsigned int sockfd, char *path)
{
   // Send a request without body (HEAD method). Only receive headers.
   char req[MAX_PATH+22] = "";
   sprintf(req, "HEAD %s HTTP/1.0\r\n\r\n", path);
   send(sockfd, req, sizeof(req), 0);

   // Receive the request
   char res[OBJECT_SIZE] = "";
   recv(sockfd, res, OBJECT_SIZE, 0);

   // Get the object size and return it
   return exec_regex(res, "Content-Length: \\([0-9]*\\)");
}

void set_ips(data *dp)
{
   struct hostent *host;
   for(int i = 0; i < dp->servers.n; i++)
   {
      if ((host = gethostbyname(dp->servers.server[0].url)) == NULL) {
         fprintf(stdout,"ERROR, no such host\n");
         exit(1);
      }
      dp->servers.server[i].ip = host;
   }
}

void create_sockets(data *dp)
{
   int sockfd; /* Socket (de tipo TCP) para transmision de datos */
   struct sockaddr_in  server;  /* Direccion TCP servidor */ 
   struct hostent        *he;

   for(int i = 0; i<dp->resources.n; i++)
   {
      /* Creacion del socket TCP */
      fprintf(stdout,"CLIENTE:  Creacion del socket TCP: ");
      if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
      {
         fprintf(stdout,"ERROR\n");
         exit(1);
      }
      fprintf(stdout,"OK\n");

      /* Nos conectamos con el servidor */
      bzero((char*)&server,sizeof(struct sockaddr_in)); /* Pone a 0 la estructura */
      server.sin_family=AF_INET;
      server.sin_port=htons(PORT);
      if ( (he = gethostbyname(dp->servers.server[i % dp->servers.n].url) ) == NULL ) {
            exit(1); /* error */
      }
      memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);

      fprintf(stdout,"CLIENTE:  Conexion al puerto servidor: ");
      if(connect(sockfd, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) < 0)
      {
         fprintf(stdout,"\n\n--------------------!-----------ERROR %d\n",-13);
         close(sockfd); exit(1);
      }
      fprintf(stdout,"OK\n");
      dp->servers.server[i].sockfd = sockfd;
   }
}

void set_requests(data *dp)
{
   for(int i = 0; i < dp->resources.n; i++)
   {
      unsigned int sockfd = dp->servers.server[i % dp->servers.n].sockfd;
      char *path = dp->resources.resource[i].path;
      int full_size = get_size(sockfd, path);
      dp->resources.resource[i].size = full_size;
      printf("sockfd: %d, path:%s\tsize:%d\n", sockfd, path, dp->resources.resource[i].size);

      float range = ceil(full_size / dp->servers.n);
      for(int j = 0; j < dp->servers.n; j ++)
      {
         float start = j * range;
         float end   = (j+1) * range - 1;
         dp->resources.resource[i].request[j].range[0] = (int)start;
         dp->resources.resource[i].request[j].range[1] = (int)end;

         unsigned int sockfd = dp->servers.server[j%dp->servers.n].sockfd;
         dp->resources.resource[i].request[j].sockfd = sockfd;

         dp->resources.resource[i].nrequest++;
      }
   }
}

void send_requests(data *dp)
{
   for(int i = 0; i < dp->resources.n; i++)
   {
      for(int j = 0; j < dp->resources.resource[i].nrequest; j++)
      {
         char req[MAX_PATH+21] = "";
         sprintf(req, "GET %s HTTP/1.0\r\n\r\n", dp->resources.resource[i].path);
         printf("Request: %s\n", req);
         send(dp->resources.resource[i].request[j].sockfd, req, sizeof(req), 0);
      }      
   }
}

void receive_requests(data *dp)
{
   for(int i = 0; i < dp->resources.n; i++)
   {
      for(int j = 0; j < dp->resources.resource[i].nrequest; j++)
      {
         char res[OBJECT_SIZE];
         if(recv(dp->resources.resource[i].request[j].sockfd, res, OBJECT_SIZE, 0) < 0)
         {
            printf("recv error\n");
            exit(1);
         }
         printf("------------------\nRecibido: \n%s\n\n%s\n", dp->resources.resource[i].path, res);
      }      
   }
}

int main(int argc, char* argv[])
{
   if(argc <= 2)
   {
      printf("Arg inválidos");
   } else {
   }

   struct Data d;
   
   parse_arguments(argc, argv, &d);


   create_sockets(&d);
   printf("SET REQUEST\n");

   set_requests(&d);
   printf("SEND REQUEST\n");

   send_requests(&d);
   printf("RECEIVE REQUEST\n");

   receive_requests(&d);

   return 0;
}