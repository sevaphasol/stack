#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "allocation.h"

void* log_calloc(FILE* MemoryLogFile, size_t nMemb, size_t size)
{
    void* ptr = calloc(nMemb, size);

    if (MemoryLogFile)
    {
        ON_HTML(fprintf(MemoryLogFile, "<p>"
                                       "Called calloc                                      <br>"
                                       "Number of members: <em style=\"color:Red\">%ld</em><br>"
                                       "Size   of members: <em style=\"color:Red\">%ld</em><br>"
                                       "Returned: <em style=\"color:Red\">%p</em>          <br>"
                                       "----------------------<br>"
                                       "</p>",
                                       nMemb,
                                       size,
                                       ptr));

        ON_LOG( fprintf(MemoryLogFile, "Called calloc         \n"
                                       "Number of members: %ld\n"
                                       "Size   of members: %ld\n"
                                       "Returned: %p          \n"
                                       "----------------------\n",
                                       nMemb,
                                       size,
                                       ptr));
    }

    return ptr;
}

void* log_realloc(FILE* MemoryLogFile, void* ptr, size_t SizeInBytes)
{
    ptr = realloc(ptr, SizeInBytes);

    if (MemoryLogFile)
    {
        ON_HTML(fprintf(MemoryLogFile, "<p>"
                                       "Called realloc                                 <br>"
                                       "Size in bytes: <em style=\"color:Red\">%ld</em><br>"
                                       "Returned: <em style=\"color:Red\">%p</em>      <br>"
                                       "----------------------<br>"
                                       "</p>",
                                       SizeInBytes,
                                       ptr));

        ON_LOG(fprintf(MemoryLogFile, "Called realloc        \n"
                                      "Size in bytes: %ld    \n"
                                      "Returned: %p          \n"
                                      "----------------------\n", SizeInBytes, ptr));
    }

    return ptr;
}

void* log_free(FILE* MemoryLogFile, void* ptr)
{
    free(ptr);

    if (MemoryLogFile)
    {
        ON_HTML(fprintf(MemoryLogFile, "<p>"
                                       "Called free                             <br>"
                                       "Pointer: <em style=\"color:Red\">%p</em><br>"
                                       "----------------------<br>"
                                       "</p>",
                                       ptr));

        ON_LOG(fprintf(MemoryLogFile, "Called free           \n"
                                      "Pointer: %p           \n"
                                      "----------------------\n", ptr));

    }

    return ptr;
}
