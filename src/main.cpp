#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "stack.h"

uint64_t err = NO_ERROR;

extern StackReturnCode StackTest();

int main()
{
    StackTest() verified;

    if (!err)
    {
        printf("\033[32mSuccess\033[0m\n");
    }

    return 0;
}
