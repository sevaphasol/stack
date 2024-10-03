#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stack.h"

StackReturnCode StackTest();

StackReturnCode StackTest()
{
    StackId_t StackId = StackCtor(MIN_STACK_SIZE);

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

        StackPush(StackId, (int) r);

        r = rand() % 100;
    }

    for (size_t i = 32; i > 0; i--)
    {
        StackPop(StackId);
    }

    StackDtor(StackId) verified;

    return EXECUTED;
}
