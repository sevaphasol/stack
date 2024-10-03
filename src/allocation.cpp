#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "allocation.h"

void* log_calloc(FILE* MemoryLogFile, size_t nMemb, size_t size)
{
    void* ptr = calloc(nMemb, size);

    fprintf(MemoryLogFile, "----------------------\n"
                           "Called calloc         \n"
                           "Number of members: %ld\n"
                           "Size   of members: %ld\n"
                           "Returned: %p          \n"
                           "----------------------\n", nMemb, size, ptr);

    return ptr;
}

void* log_realloc(FILE* MemoryLogFile, void* ptr, size_t SizeInBytes)
{
    ptr = realloc(ptr, SizeInBytes);

    fprintf(MemoryLogFile, "----------------------\n"
                           "Called realloc        \n"
                           "Size in bytes: %ld    \n"
                           "Returned: %p          \n"
                           "----------------------\n", SizeInBytes, ptr);

    return ptr;
}

void* log_free(FILE* MemoryLogFile, void* ptr)
{
    free(ptr);

    fprintf(MemoryLogFile, "----------------------\n"
                           "Called free           \n"
                           "Pointer: %p           \n"
                           "----------------------\n", ptr);

    return ptr;
}
