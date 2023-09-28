#ifndef MEMORY_H
#define MEMORY_H

#include "../main.h"

void *memcpy(void *dst, const void *src, unsigned long size);
void *memset(void *dst, int byte, unsigned long size);

#endif
