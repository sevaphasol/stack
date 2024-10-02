#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stack.h"

StackReturnCode StackTest (Stack_t* stack);

StackReturnCode StackTest(Stack_t* stack)
{
    FILE* UnitTestFile = fopen("unit_test", "wb");

    if (!UnitTestFile)
    {
        err += INVALID_FILE_POINTER;

        return FAILED;
    }

    srand((unsigned int) time(NULL));

    int r = rand() % 100;

    for (size_t i = 0; i < 32; i++)
    {
        fprintf(UnitTestFile, "%d\n", r);

        StackPush(stack, (int) r);

        r = rand() % 100;
    }

    for (size_t i = stack->size; i > 0; i--)
    {
        StackPop(stack);
    }

    printf("\033[32mExecuted\033[0m\n");

    return EXECUTED;
}
