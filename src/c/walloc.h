#ifndef WALLOC_H
#define WALLOC_H

#include "main.h"

typedef __SIZE_TYPE__ size_t;
typedef __UINTPTR_TYPE__ uintptr_t;

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *memset(void *s, int c,  size_t len);
i32 memcmp(const void *s1, const void *s2, size_t len);
void *calloc(size_t nElements, size_t size);

#endif
