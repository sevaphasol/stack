#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <cstring>

#include "stack.h"
#include "stdlib.h"

StackReturnCode StackCtor(Stack_t* stack, int capacity)
{
    if (!stack)
    {
        err += INVALID_STACK_POINTER;

        return FAILED;
    }

    stack->data = (StackElem_t*) calloc(capacity, sizeof(StackElem_t));

    if (!stack->data)
    {
        err += INVALID_DATA_POINTER;

        return FAILED;
    }

    stack->size = 0;

    stack->capacity = capacity;

    stack->inited = true;

    return EXECUTED;
}

StackReturnCode StackPush(Stack_t* stack, StackElem_t value)
{
    STACK_ASSERT(STACK_IS_VALID(stack));

    if (stack->size < stack->capacity)
    {
        stack->data[stack->size] = value;
    }
    else
    {
        if (stack->capacity > MAX_STACK_SIZE)
        {
            err += STACK_OVERFLOW;

            return FAILED;
        }

        if (StackResize(stack, stack->capacity * 2) == FAILED)
        {
            return FAILED;
        }

        stack->data[stack->size] = value;
    }

    STACK_ASSERT(STACK_IS_VALID(stack));

    stack->size++;

    return EXECUTED;
}

StackElem_t StackPop(Stack_t* stack)
{
    STACK_ASSERT(STACK_IS_VALID(stack));

    if (stack->size == 0)
    {
        err += STACK_UNDERFLOW;

        return FAILED;
    }

    stack->size--;

    StackElem_t value = stack->data[stack->size];

    if ((stack->size <= stack->capacity / 4) && (stack->capacity / 2 >= MIN_STACK_SIZE))
    {
        if (StackResize(stack, stack->capacity / 2) == FAILED)
        {
            return FAILED;
        }
    }

    stack->data[stack->size] = 0;

    STACK_ASSERT(STACK_IS_VALID(stack));

    return value;
}

StackReturnCode StackResize(Stack_t* stack, size_t newCapacity)
{
    STACK_ASSERT(STACK_IS_VALID(stack));

    if (newCapacity < MIN_STACK_SIZE)
    {
        err += REQUESTED_TOO_LITTLE;

        return FAILED;
    }

    if (newCapacity > MAX_STACK_SIZE)
    {
        err += REQUESTED_TOO_MUCH;

        return FAILED;
    }

    stack->data = (StackElem_t*) realloc(stack->data, newCapacity * sizeof(StackElem_t));

    if (!stack->data)
    {
        err += INVALID_DATA_POINTER;

        return FAILED;
    }

    stack->capacity = newCapacity;

    STACK_ASSERT(STACK_IS_VALID(stack));

    return EXECUTED;
}

StackReturnCode StackDtor(Stack_t* stack)
{
    STACK_ASSERT(STACK_IS_VALID(stack));

    memset(stack->data, 0, stack->capacity);

    free(stack->data);

    stack->data = nullptr;

    stack->size = 0;

    stack->capacity = 0;

    return EXECUTED;
}

StackReturnCode StackDump(Stack_t* stack ON_DEBUG(, int line, const char* file, const char* function))
{
    FILE* DumpFile = fopen("dump.txt", "a");

    if (!DumpFile)
    {
        err += INVALID_FILE_POINTER;

        return FAILED;
    }

    time_t RawTime;

    struct tm* TimeInfo;

    time(&RawTime);

    TimeInfo = localtime(&RawTime);

    fprintf(DumpFile, "dump.txt created at local time and date: %s", asctime(TimeInfo));

    PrintErr(DumpFile, err);

    if (!stack)
    {
        fprintf(DumpFile, "Lost stack pointer\n");

        fclose(DumpFile);

        return EXECUTED;
    }

    fprintf(DumpFile, "Stack_t[%p] %s at %s:%d in function %s\nBorn at %s:%d in function %s\n\n",
            stack, stack->name, file, line, function, stack->BornFile, stack->BornLine, stack->BornFunc);

    fprintf(DumpFile, "capacity = %ld\n", stack->capacity);

    fprintf(DumpFile, "size = %ld\n", stack->size);

    if (!stack->data)
    {
        fprintf(DumpFile, "Lost stack->data pointer\n");

        fprintf(DumpFile, "\n\n---------------------------------------------------------------------\n\n");

        fclose(DumpFile);

        return EXECUTED;
    }

    for (int i = 0; i < stack->capacity; i++)
    {
        fprintf(DumpFile, "[%d] = %ld\n", i, stack->data[i]);
    }

    fprintf(DumpFile, "\n\n---------------------------------------------------------------------\n\n");

    fclose(DumpFile);

    return EXECUTED;
}

StackReturnCode StackTest(Stack_t* stack)
{
    STACK_ASSERT(STACK_IS_VALID(stack));

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

StackReturnCode PrintErr(FILE* fp, uint64_t code)
{
    uint64_t nextPow = code;

    if (!fp)
    {
        return FAILED;
    }

    if (code == 0)
    {
        fprintf(fp, "NO ERROR\n");

        return EXECUTED;
    }

    fprintf(fp, "ERRORS: ");

    if (code >= 128)
    {
        fprintf(fp, "INVALID FILE POINTER ");
    }

    if ((nextPow = code % 128) >= 64)
    {
        fprintf(fp, "REQUESTED TOO MUCH ");
    }

    if ((nextPow = code % 64) >= 32)
    {
        fprintf(fp, "REQUESTED TOO LITTLE ");
    }

    if ((nextPow = code % 32) >= 16)
    {
        fprintf(fp, "INVALID SIZE ");
    }

    if ((nextPow = code % 16) >= 8)
    {
        fprintf(fp, "INVALID DATA POINTER ");
    }

    if ((nextPow = code % 8) >= 4)
    {
        fprintf(fp, "INVALID STACK POINTER ");
    }

    if ((nextPow = code % 4) >= 2)
    {
        fprintf(fp, "STACK OVERFLOW ");
    }

    if ((nextPow = code % 2) >= 1)
    {
        fprintf(fp, "STACK UNDERFLOW ");
    }

    fprintf(fp, "\n");

    return EXECUTED;
}

bool StackIsValid(Stack_t* stack ON_DEBUG(, int line, const char* file, const char* function))
{
    StackDump(stack, line, file, function);

    if (!stack)
    {
        err += INVALID_STACK_POINTER;

        StackDump(stack, line, file, function);

        return false;
    }

    if (!stack->data)
    {
        err += INVALID_DATA_POINTER;

        StackDump(stack, line, file, function);

        return false;
    }

    if (stack->size > stack->capacity)
    {
        err += INVALID_SIZE;

        StackDump(stack, line, file, function);

        return false;
    }

    return true;
}

void StackAssert(bool statement, int line, const char* file, const char* function)
{
    if (!statement)
    {
        fprintf(stderr, "%s:%d:%s: Assertion failed.\n", file, line, function);

        abort();
    }
}
