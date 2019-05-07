/*
 *
 * NO MODIFICAR
 *
 */
#include <stdbool.h>
#include <stddef.h>

int createMQ(const char *queue_name);
int destroyMQ(const char *queue_name);

int put(const char *queue_name, const void *msg, size_t size);
int get(const char *queue_name, void **msg, size_t *size, bool blocking);
