/*
 *
 * NO MODIFICAR
 *
 */
#include <stdbool.h>
#include <stddef.h>

int createMQ(const char *cola);
int destroyMQ(const char *cola);

int put(const char *cola, const void *mensaje, size_t tam);
int get(const char *cola, void **mensaje, size_t *tam, bool blocking);

