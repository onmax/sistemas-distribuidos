/*
 *
 * NO MODIFICAR
 *
 */
#include <stdbool.h>
#include <stddef.h>

int createMQ(const char *queue_name);
int destroyMQ(const char *queue_name);

int put(const char *queue_name, const void *mensaje, size_t tam);
int get(const char *queue_name, void **mensaje, size_t *tam, bool blocking);
