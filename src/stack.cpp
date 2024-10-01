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

    StackDump(stack, __LINE__, __FILE__, __PRETTY_FUNCTION__);

    stack->DataWithCanary = (StackElem_t*) calloc(capacity + 2, sizeof(StackElem_t));

    *stack->DataWithCanary = CANARY;

    *(stack->DataWithCanary + capacity + 1) = CANARY;

    stack->data = stack->DataWithCanary + 1;

    if (!stack->data)
    {
        err += INVALID_DATA_POINTER;

        return FAILED;
    }

    stack->size = 0;

    stack->capacity = capacity;

    stack->inited = true;

    GetHash(stack);

    StackDump(stack, __LINE__, __FILE__, __PRETTY_FUNCTION__);

    return EXECUTED;
}

StackReturnCode StackPush(Stack_t* stack, StackElem_t value)
{
    STACK_ASSERT(STACK_IS_VALID(stack));
    STACK_ASSERT(STACK_IS_DAMAGED(stack));

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

    GetHash(stack);

    stack->size++;

    STACK_ASSERT(STACK_IS_VALID(stack));
    STACK_ASSERT(STACK_IS_DAMAGED(stack));

    return EXECUTED;
}

StackElem_t StackPop(Stack_t* stack)
{
    STACK_ASSERT(STACK_IS_VALID(stack));
    STACK_ASSERT(STACK_IS_DAMAGED(stack));

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

    GetHash(stack);

    STACK_ASSERT(STACK_IS_VALID(stack));
    STACK_ASSERT(STACK_IS_DAMAGED(stack));


    return value;
}

StackReturnCode StackResize(Stack_t* stack, size_t newCapacity)
{
    STACK_ASSERT(STACK_IS_VALID(stack));
    STACK_ASSERT(STACK_IS_DAMAGED(stack));

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

    stack->DataWithCanary = (StackElem_t*) realloc(stack->DataWithCanary, (newCapacity + 2) * sizeof(StackElem_t));

    stack->data = stack->DataWithCanary + 1;

    if (!stack->data)
    {
        err += INVALID_DATA_POINTER;

        return FAILED;
    }

    stack->capacity = newCapacity;

    *(stack->DataWithCanary + stack->capacity + 1) = CANARY;

    GetHash(stack);

    STACK_ASSERT(STACK_IS_VALID(stack));
    STACK_ASSERT(STACK_IS_DAMAGED(stack));

    return EXECUTED;
}

StackReturnCode StackDtor(Stack_t* stack)
{
    STACK_ASSERT(STACK_IS_VALID(stack));
    STACK_ASSERT(STACK_IS_DAMAGED(stack));

    memset(stack->DataWithCanary, 0, stack->capacity + 2);

    free(stack->DataWithCanary);

    stack->data = nullptr;

    stack->size = 0;

    stack->capacity = 0;

    stack->hash = 0;

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

    fprintf(DumpFile, "Local time and date: %s", asctime(TimeInfo));

    PrintErr(DumpFile, err);

    if (!stack)
    {
        fprintf(DumpFile, "Lost stack pointer\n");

        fclose(DumpFile);

        return EXECUTED;
    }

    fprintf(DumpFile, "Stack_t[%p] %s at %s:%d in function %s\nBorn at %s:%d in function %s\n\n",
            stack, stack->name, file, line, function, stack->BornFile, stack->BornLine, stack->BornFunc);

    fprintf(DumpFile, "LEFT STRUCT CANARY = %ld\n", stack->left_canary);

    fprintf(DumpFile, "RIGHT STRUCT CANARY = %ld\n", stack->right_canary);

    fprintf(DumpFile, "capacity = %ld\n", stack->capacity);

    fprintf(DumpFile, "size = %ld\n", stack->size);

    if (!stack->data)
    {
        fprintf(DumpFile, "Lost stack->data pointer\n");

        fprintf(DumpFile, "\n\n---------------------------------------------------------------------\n\n");

        fclose(DumpFile);

        return EXECUTED;
    }

    fprintf(DumpFile, "\nLEFT DATA CANARY = %ld\n\n", *stack->DataWithCanary);

    for (int i = 0; i < stack->capacity; i++)
    {
        fprintf(DumpFile, "[%d] = %ld\n", i, stack->data[i]);
    }

    fprintf(DumpFile, "\nRIGHT DATA CANARY = %ld\n", *(stack->DataWithCanary + stack->capacity + 1));

    fprintf(DumpFile, "\n\n---------------------------------------------------------------------\n\n");

    fclose(DumpFile);

    return EXECUTED;
}

StackReturnCode StackTest(Stack_t* stack)
{
    STACK_ASSERT(STACK_IS_VALID  (stack));
    STACK_ASSERT(STACK_IS_DAMAGED(stack));

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

    GetHash(stack);

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

    PRINT_ERR(code, 4096, "INVALID STRUCT CANARY ");

    PRINT_ERR(code, 2048, "INVALID DATA CANARY ");

    PRINT_ERR(code, 1024, "INVALID HASH ");

    PRINT_ERR(code, 512,  "DAMAGED STACK ERR ");

    PRINT_ERR(code, 256,  "INVALID FILE POINTER ");

    PRINT_ERR(code, 128,  "REQUESTED TOO MUCH ");

    PRINT_ERR(code, 64,   "REQUESTED TOO LITTLE ");

    PRINT_ERR(code, 32,   "INVALID SIZE ");

    PRINT_ERR(code, 16,   "INVALID DATA POINTER ");

    PRINT_ERR(code, 8,    "INVALID STACK POINTER ");

    PRINT_ERR(code, 4,    "STACK OVERFLOW ");

    PRINT_ERR(code, 2,    "STACK UNDERFLOW ");

    fprintf(fp, "\n");

    return EXECUTED;
}

StackReturnCode GetHash(Stack_t* stack)
{
    STACK_ASSERT(STACK_IS_VALID(stack));

    uint64_t hash = 5831;

    for (size_t i = 0; i < stack->capacity; i++)
    {
        hash = (33 * hash + hash) + stack->data[i];
    }

    stack->hash = hash;

    return EXECUTED;
}

StackReturnCode StackIsValid(Stack_t* stack ON_DEBUG(, int line, const char* file, const char* function))
{
    StackDump(stack, line, file, function);

    if (!stack)
    {
        err += INVALID_STACK_POINTER;

        StackDump(stack, line, file, function);

        return STACK_INVALID;
    }

    if (!stack->data)
    {
        err += INVALID_DATA_POINTER;

        StackDump(stack, line, file, function);

        return STACK_INVALID;
    }

    if (stack->size > stack->capacity)
    {
        err += INVALID_SIZE;

        StackDump(stack, line, file, function);

        return STACK_INVALID;
    }

    return STACK_VALID;
}

void StackAssert(StackReturnCode code, int line, const char* file, const char* function)
{
    if (code == STACK_INVALID)
    {
        fprintf(stderr, "%s:%d:%s: Assertion failed. STACK IS INVALID\n", file, line, function);

        abort();
    }

    if (code == STACK_DAMAGED)
    {
        fprintf(stderr, "%s:%d:%s: Assertion failed. STACK IS DAMAGED\n", file, line, function);

        abort();
    }
}

StackReturnCode StackIsDamaged(Stack_t* stack, int line, const char* file, const char* function)
{
    STACK_ASSERT(STACK_IS_VALID(stack));

    StackDump(stack, line, file, function);

    if (stack->left_canary != CANARY || stack->right_canary != CANARY)
    {
        err += INVALID_STRUCT_CANARY;

        StackDump(stack, line, file, function);

        PrintErr(stderr, err);

        return STACK_DAMAGED;
    }

    uint64_t hash = stack->hash;

    GetHash(stack);

    if (hash != stack->hash)
    {
        err += INVALID_HASH;

        StackDump(stack, line, file, function);

        PrintErr(stderr, err);

        return STACK_DAMAGED;
    }

    // printf("%ld\n", *(stack->DataWithCanary + stack->capacity + 1));

    if (*(stack->DataWithCanary) != CANARY || *(stack->DataWithCanary + stack->capacity + 1) != CANARY)
    {
        err += INVALID_DATA_CANARY;

        StackDump(stack, line, file, function);

        PrintErr(stderr, err);

        return STACK_DAMAGED;
    }

    StackDump(stack, line, file, function);

    return STACK_NOT_DAMAGED;
}
