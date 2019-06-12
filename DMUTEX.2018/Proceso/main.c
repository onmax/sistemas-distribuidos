/* DMUTEX (2009) Sistemas Operativos Distribuidos
 * C�digo de Apoyo
 *
 * ESTE C�DIGO DEBE COMPLETARLO EL ALUMNO:
 *    - Para desarrollar las funciones de mensajes, reloj y
 *      gesti�n del bucle de tareas se recomienda la implementaci�n
 *      de las mismas en diferentes ficheros.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MSG_SIZE 100
#define PROCESS_NAME_SIZE 100
#define TASK_NAME_SIZE 15

#define CLOCK_UPDATE 1
#define LOCK_MSG 2
#define MSG 3

const char *SERVER_NAME = "127.1";

typedef struct
{
  char proc[PROCESS_NAME_SIZE];
  unsigned int port;
} Process;

typedef struct
{
  unsigned int msg_type;
  unsigned int index;
  int *clocks;
  unsigned int n_clocks;
} clock_data;

clock_data *get_clock(int *clocks, int msg_type, int process_index, int nprocesses)
{
  clock_data *clock;

  clock = malloc(sizeof(clock_data));
  clock->msg_type = msg_type;
  clock->index = process_index;
  clock->n_clocks = nprocesses;
  clock->clocks = malloc(sizeof(int) * clock->n_clocks);
  for (int i = 0; i < clock->n_clocks; i++)
    clock->clocks[i] = clocks[i];
  return clock;
}

int send_msg(clock_data *clock, int port)
{
  int sockfd;
  struct sockaddr_in servaddr;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(SERVER_NAME);
  servaddr.sin_port = htons(port);

  sendto(sockfd, &clock, sizeof(clock), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

  return 1;
}

int get_index(Process *processes, int nprocesses, char *target)
{
  for (int i = 0; i < nprocesses; i++)
    if (!strcmp(processes[i].proc, target))
      return i;
  return -1;
}

void tick(int *clocks, int i, const char *iam)
{
  clocks[i]++;
  fprintf(stdout, "%s: TICK\n", iam);
}

int main(int argc, char *argv[])
{
  int port;
  char line[PROCESS_NAME_SIZE], proc[PROCESS_NAME_SIZE];
  struct sockaddr_in address;
  int socket_fd;
  Process *processes;
  int nprocesses;
  int nclocks;
  int *clocks;
  int current_clock = 0;

  if (argc < 2)
  {
    fprintf(stderr, "Uso: proceso <ID>\n");
    return 1;
  }
  const char *iam = argv[1];

  /* Establece el modo buffer de entrada/salida a linea */
  setvbuf(stdout, (char *)malloc(sizeof(char) * PROCESS_NAME_SIZE), _IOLBF, PROCESS_NAME_SIZE);
  setvbuf(stdin, (char *)malloc(sizeof(char) * PROCESS_NAME_SIZE), _IOLBF, PROCESS_NAME_SIZE);

  if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    fprintf(stderr, "Error: Creando socket\n");
    exit(-1);
  }

  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    fprintf(stderr, "Error: Asignación del puerto servidor\n");
    exit(-1);
    close(socket_fd);
  }
  socklen_t len = sizeof(address);
  getsockname(socket_fd, (struct sockaddr *)&address, &len);

  port = ntohs(address.sin_port);

  fprintf(stdout, "%s: %d\n", iam, port);

  processes = malloc(0);
  clocks = malloc(0);
  nprocesses = 0;
  nclocks = 0;

  for (; fgets(line, 80, stdin);)
  {
    if (!strcmp(line, "START\n"))
      break;

    sscanf(line, "%[^:]: %d", proc, &port);

    Process p;
    memcpy(p.proc, proc, PROCESS_NAME_SIZE);
    p.port = port;

    nprocesses++;
    processes = (Process *)realloc(processes, nprocesses * sizeof(Process));
    processes[nprocesses - 1] = p;

    if (!strcmp(proc, iam))
    {
      current_clock = nclocks;
      clocks = (int *)realloc(clocks, nprocesses * sizeof(int));
      clocks[nclocks++ - 1] = 0;
    }
  }

  /* Procesar Acciones */
  char msg[MSG_SIZE];
  char task[PROCESS_NAME_SIZE];
  char target_process[TASK_NAME_SIZE];

  for (; fgets(msg, MSG_SIZE, stdin);)
  {
    sscanf(msg, "%s %s", task, target_process);

    if (!strcmp(task, "EVENT"))
      tick(clocks, current_clock, iam);
    else if (!strcmp(task, "GETCLOCK"))
    {
      fprintf(stdout, "%s: LC[", iam);
      for (int i = 0; i < nclocks - 1; i++)
        fprintf(stdout, "%d,", clocks[i]);
      fprintf(stdout, "%d]\n", clocks[nclocks - 1]);
    }
    else if (!strcmp(task, "RECEIVE"))
    {

      /* Recibe un mensaje */
      struct sockaddr_in clock_address;
      clock_data *clock_received;
      clock_received = malloc(sizeof(clock_data));
      socklen_t size = sizeof(struct sockaddr_in);

      recvfrom(socket_fd, &clock_received, sizeof(clock_data), 0, (struct sockaddr *)&clock_address, &size);

      // Obtiene el máximo de cada reloj
      if (clock_received->msg_type == CLOCK_UPDATE)
      {
        for (int i = 0; i < clock_received->n_clocks; i++)
          if (clock_received->clocks[i] > clocks[i])
            clocks[i] = clock_received->clocks[i];
      }
      else if (clock_received->msg_type == MSG)
        fprintf(stdout, "%s: RECEIVE(MSG,%s)\n", iam, processes[clock_received->index].proc);
      else if (clock_received->msg_type == LOCK_MSG)
        fprintf(stdout, "%s: RECEIVE(LOCK,%s)\n", iam, processes[clock_received->index].proc);
      tick(clocks, current_clock, iam);
    }
    else if (!strcmp(task, "LOCK"))
    {
      tick(clocks, current_clock, iam);

      for (int i = 0; i < nprocesses; i++)
      {
        if (i != current_clock)
        {
          clock_data *clock;
          clock = get_clock(clocks, LOCK_MSG, i, nprocesses);
          send_msg(clock, processes[i].port);
          fprintf(stdout, "%s: SEND(LOCK,%s)\n", iam, processes[i].proc);
        }
      }
    }
    else if (!strcmp(task, "UNLOCK"))
    {
    }
    else if (!strcmp(task, "MESSAGETO"))
    {
      tick(clocks, current_clock, iam);

      clock_data *clock;
      int i;
      if ((i = get_index(processes, nprocesses, target_process)) == -1)
      {
        fprintf(stderr, "No encontrado\n");
        exit(-1);
      }

      clock = get_clock(clocks, MSG, current_clock, nprocesses);
      send_msg(clock, processes[i].port);
      fprintf(stdout, "%s: SEND(MSG,%s)\n", iam, target_process);
    }
    else if (!strcmp(task, "FINISH"))
    {
      exit(0);
    }
    else
    {
      fprintf(stderr, "Error: Tarea no encontrada\n");
    }
  }
  return 0;
}
