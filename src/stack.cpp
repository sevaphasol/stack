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

    stack->initialized = true;

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
        if (StackResize(stack, stack->capacity * 2) == FAILED)
        {
            err += STACK_OVERFLOW;

            return FAILED;
        }

        stack->data[stack->size] = value;
    }

    STACK_ASSERT(STACK_IS_VALID(stack));

    #ifdef DEBUG

    printf("StackPush: [%ld] = %ld\n", stack->size, stack->data[stack->size]);

    #endif

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

    if (stack->size <= stack->capacity / 4)
    {
        StackResize(stack, stack->capacity / 2) == FAILED;
    }

    #ifdef DEBUG

    printf("StackPop:  [%ld] = %ld\n", stack->size, stack->data[stack->size]);

    #endif

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

StackReturnCode StackDump(Stack_t* stack, int line, const char* file, const char* function)
{
    FILE* DumpFile = fopen("dump.txt", "wb");

    if (!DumpFile)
    {
        err += INVALID_FILE_POINTER;

        return FAILED;
    }

    time_t RawTime;

    struct tm* TimeInfo;

    time(&RawTime);

    TimeInfo = localtime(&RawTime);

    fprintf(DumpFile, "dump.txt created at local time and date: %s\n", asctime(TimeInfo));

    if (!stack)
    {
        fprintf(DumpFile, "Lost stack pointer\n");

        fclose(DumpFile);

        return EXECUTED;
    }

    fprintf(DumpFile, "Stack_t[%p] failed at %s:%d in function %s\nBorn at %s:%d in function %s\n\n",
            stack, file, line, function, stack->BornFile, stack->BornLine, stack->BornFunc);

    fprintf(DumpFile, "capacity = %ld\n", stack->capacity);

    fprintf(DumpFile, "size = %ld\n", stack->size);

    if (!stack->data)
    {
        fprintf(DumpFile, "Lost stack->data pointer\n");

        fclose(DumpFile);

        return EXECUTED;
    }

    for (int i = 0; i < stack->capacity; i++)
    {
        fprintf(DumpFile, "[%d] = %ld\n", i, stack->data[i]);
    }

    fclose(DumpFile);

    return EXECUTED;
}

StackReturnCode StackTest(Stack_t* stack)
{
    STACK_ASSERT(STACK_IS_VALID(stack));

    FILE* UnitTestFile = fopen("unit_test", "wb");

    if (!UnitTestFile)
    {
        err = INVALID_FILE_POINTER;

        return FAILED;
    }

    srand((unsigned int) time(NULL));

    int r = rand() % 100;

    for (size_t i = 0; i < 2049; i++)
    {
        fprintf(UnitTestFile, "%d\n", r);

        StackPush(stack, (int) r);

        r = rand() % 100;

        printf("Capacity = %ld\n", stack->capacity);
    }

    printf("\n\n");

    for (size_t i = stack->size; i > 0; i--)
    {
        StackPop(stack);
        printf("Capacity = %ld\n", stack->capacity);
    }

    return EXECUTED;
}

bool StackIsValid(Stack_t* stack, int line, const char* file, const char* function)
{
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
