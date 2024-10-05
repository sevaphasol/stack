#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stack.h>

#ifndef ALLOCATION_H__
#define ALLOCATION_H__

void* log_calloc(FILE* MemoryLogFile, size_t nMemb, size_t size);

void* log_realloc(FILE* MemoryLogFile, void* ptr, size_t SizeInBytes);

void* log_free(FILE* MemoryLogFile, void* ptr);

#endif // ALLOCATION_H__
