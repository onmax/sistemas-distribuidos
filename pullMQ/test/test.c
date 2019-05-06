#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pullMQ.h"

int main(int argc, char *argv[])
{
    int n;
    char *op, *cola;
    createMQ("cola1=OTRO");
    do
    {
        printf("\nSeleccione nombre de cola y operación (ctrl-D para terminar)\n\tC:create|D:destroy|P:put\n\tG:get no bloqueante|B:get bloqueante\n");
        op = cola = NULL;
        n = scanf("%ms%ms", &cola, &op);
        if (n == 2)
        {
            switch (op[0])
            {
            case 'C':
                if (createMQ(cola) < 0)
                    printf("error creando la cola %s\n", cola);
                else
                    printf("cola %s creada correctamente\n", cola);
                break;
            case 'D':
                if (destroyMQ(cola) < 0)
                    printf("error eliminando la cola %s\n", cola);
                else
                    printf("cola %s eliminada correctamente\n", cola);
                break;
            case 'G':
            case 'B':;
                size_t tam;
                void *mensaje;
                if (get(cola, &mensaje, &tam, (op[0] == 'B') ? true : false) < 0)
                    printf("error leyendo de la cola %s\n", cola);
                else
                {
                    printf("lectura de la cola %s correcta\n", cola);
                    if (tam)
                    {
                        write(1, mensaje, tam);
                        free(mensaje);
                    }
                    else
                        printf("no hay mensajes\n");
                }
                break;
            case 'P':
                printf("Introduzca un mensaje terminado con 'return': ");
                char *linea = NULL;
                size_t n = 0;
                int leido;
                do
                {
                    leido = getline(&linea, &n, stdin);
                } while (leido < 2);
                if (put(cola, linea, leido) < 0)
                    printf("error escribiendo en la cola %s\n", cola);
                else
                    printf("escritura en la cola %s correcta\n", cola);
                free(linea);
                break;
            default:
                printf("operación no válida\n");
            }
            free(cola);
            free(op);
        }
    } while (n != EOF);
    return 0;
}
