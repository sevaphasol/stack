#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "stack.h"
#include "error_codes.h"
#include "stdlib.h"

StackCode StackCtor(Stack_t* stack, int capacity)
{
    if (!stack)
    {
        return FAILED;
    }

    stack->data = (StackElem_t*) calloc(capacity, sizeof(StackElem_t));

    if (!stack->data)
    {
        return FAILED;
    }

    stack->size = 0;

    stack->capacity = capacity;

    return EXECUTED;
}

StackCode StackPush(Stack_t* stack, StackElem_t value)
{
    STACK_ASSERT(STACK_VALIDATOR(stack));

    if (stack->size < stack->capacity)
    {
        stack->data[stack->size] = value;
    }
    else
    {
        if (StackResize(stack, stack->capacity * 2) == FAILED)
        {
            return FAILED;
        }

        stack->data[stack->size] = value;
    }

    STACK_ASSERT(STACK_VALIDATOR(stack));

    #ifdef DEBUG

    printf("StackPush: [%ld] = %lld\n", stack->size, stack->data[stack->size]);

    #endif

    stack->size++;

    return EXECUTED;
}

StackElem_t StackPop(Stack_t* stack)
{
    STACK_ASSERT(STACK_VALIDATOR(stack));

    if (stack->size == 0)
    {
        return FAILED;
    }

    stack->size--;

    StackElem_t value = stack->data[stack->size];

    if (stack->size <= stack->capacity / 4)
    {
        StackResize(stack, stack->capacity / 2) == FAILED;
    }

    #ifdef DEBUG

    printf("StackPop:  [%ld] = %lld\n", stack->size, stack->data[stack->size]);

    #endif

    stack->data[stack->size] = 0;

    STACK_ASSERT(STACK_VALIDATOR(stack));

    return value;
}

StackCode StackResize(Stack_t* stack, size_t newCapacity)
{
    STACK_ASSERT(STACK_VALIDATOR(stack));

    stack->data = (StackElem_t*) realloc(stack->data, newCapacity * sizeof(StackElem_t));

    if (!stack->data)
    {
        return FAILED;
    }

    stack->capacity = newCapacity;

    STACK_ASSERT(STACK_VALIDATOR(stack));

    return EXECUTED;
}

StackCode StackDtor(Stack_t* stack)
{
    STACK_ASSERT(STACK_VALIDATOR(stack));

    free(stack->data);

    stack->data = nullptr;

    stack->size = 0;

    stack->capacity = 0;

    return EXECUTED;
}

StackCode StackDump(Stack_t* stack, int line, const char* file, const char* function)
{
    FILE* fp = fopen("dump.txt", "wb");

    if (!fp)
    {
        return FAILED;
    }

    time_t RawTime;

    struct tm* TimeInfo;

    time(&RawTime);

    TimeInfo = localtime(&RawTime);

    fprintf(fp, "dump.txt created at local time and date: %s\n", asctime(TimeInfo));

    if (!stack)
    {
        fprintf(fp, "Lost stack pointer\n");

        fclose(fp);

        return EXECUTED;
    }

    fprintf(fp, "Stack_t[%p] failed at %s:%d in function %s\nBorn at %s:%d in function %s\n\n",
            stack, file, line, function, stack->BornFile, stack->BornLine, stack->BornFunc);

    fprintf(fp, "capacity = %ld\n", stack->capacity);

    fprintf(fp, "size = %ld\n", stack->size);

    if (!stack->data)
    {
        fprintf(fp, "Lost stack->data pointer\n");

        fclose(fp);

        return EXECUTED;
    }

    for (int i = 0; i < stack->capacity; i++)
    {
        fprintf(fp, "[%d] = %lld\n", i, stack->data[i]);
    }

    fclose(fp);

    return EXECUTED;
}

StackCode StackTest(Stack_t* stack)
{
    STACK_ASSERT(STACK_VALIDATOR(stack));

    for (size_t i = 0; i < stack->capacity; i++)
    {
        StackPush(stack, i);
    }

    #ifdef DEBUG

    printf("\n");

    for (size_t i = 0; i < stack->size; i++)
    {
        printf("RealValue: [%ld] = %lld\n", i, stack->data[i]);
    }

    printf("\n");

    #endif

    for (size_t i = stack->size; i > 0; i--)
    {
        StackPop(stack);
    }

    return EXECUTED;
}

StackCode StackValidator(Stack_t* stack, int line, const char* file, const char* function)
{
    if (stack == nullptr || stack->data == nullptr || stack->size > stack->capacity)
    {
        StackDump(stack, line, file, function);

        return STACK_INVALID;
    }

    return STACK_VALID;
}

StackCode StackAssert(StackCode statement, int line, const char* file, const char* function)
{
    if (statement == STACK_INVALID)
    {
        fprintf(stderr, "%s:%d:%s: Assertion failed.\n", file, line, function);

        abort();
    }

    return EXECUTED;
}

