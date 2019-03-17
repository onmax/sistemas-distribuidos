#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 




#define MAX_NSERVERS 100
#define MAX_NRESOURCES 100
#define MAX_PATH 200

struct resources {
   int   n;
   int   sizes[MAX_NRESOURCES];
   char  paths[MAX_NRESOURCES][MAX_PATH];
   int   ranges[MAX_NRESOURCES][2];
};

struct servers {
   int   n;
   char  urls[MAX_NSERVERS][MAX_PATH];
};

typedef struct Data {
   struct resources  resources;
   struct servers    servers;
} data;

typedef struct Pipeline
{
   int sizes[2*MAX_NRESOURCES];
   int main_size[2];
   /*
   int stage3[MAX_NRESOURCES][2];
   int stage4[MAX_NRESOURCES][MAX_NSERVERS][2];
   int stage5[MAX_NRESOURCES][2];
   int stage6[2];
   */
} pipeline;


void parse_arguments(int argc, char* argv[], data *dp);
void set_resources_sizes(data *dp);
int get_size(char* server, char* path);

void parse_arguments(int argc, char* argv[], data *dp)
{
   dp->servers.n = 0;
   dp->resources.n = 0;

   for(int i=1;i<argc;i++)
   {
      char* tmp = argv[i];
      if(tmp[0] == ':') {
         strncpy(dp->servers.urls[dp->servers.n], &tmp[1], strlen(tmp) - 1);
         dp->servers.n++;
      } else {
         strcpy(dp->resources.paths[dp->resources.n], tmp);
         dp->resources.n++;
      }
   }
}

int get_size(char* server, char* path)
{
   CURL *curl = curl_easy_init();
   char final_path[MAX_PATH];
   int size;
   if(curl) {
      snprintf(final_path, sizeof(final_path), "http://%s/~%s", server, path);
      curl_easy_setopt(curl, CURLOPT_URL, final_path);
      curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); /* get us the resource without a body! */
      if(curl_easy_perform(curl) == CURLE_OK) {
         curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size);
      }
   }
   return size;
}

void set_resources_sizes(data *dp)
{
   struct Pipeline pipeline;
   if(pipe(pipeline.main_size))
   {
      printf("Error pipe");
      exit(1);
   }
   switch (fork())
   {
      case -1:
         printf("Error fork");
         exit(1);
         break;
      default:
         printf("papa\n");
         int psize;

         wait(NULL);
         for(int i=0; i < dp->resources.n;i++)
         {
            while(read(pipeline.sizes[2*i], &psize, sizeof(psize))>0)
               printf("%d", psize);
         }
         break;

      case 0:
         for(int i=0; i < dp->resources.n;i++)
         {
            int hsize;
            if(pipe(&pipeline.sizes[2*i]))
            {
               printf("Error pipe");
               exit(1);
            }
            hsize = get_size(dp->servers.urls[0], dp->resources.paths[i]);
            write(pipeline.sizes[2*i+1], &hsize, sizeof(hsize));
            printf("hijo %d. len: %d\n", i, hsize);
         }
         exit(0);
   } 
   printf("LOL\n");
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


   set_resources_sizes(&d);
   return 0;
}