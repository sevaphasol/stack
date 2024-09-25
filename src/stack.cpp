#include <stdio.h>
#include <assert.h>

#include "stack.h"
#include "error_codes.h"
#include "stdlib.h"

StackCode StackInit(Stack_t* stack, int capacity)
{
    RETURN_FAILED_IF(stack == nullptr);

    stack->data = (StackElem_t*) calloc(capacity, sizeof(StackElem_t));

    RETURN_FAILED_IF(stack->data == nullptr);

    stack->size = 0;

    stack->capacity = capacity;

    return EXECUTED;
}

StackCode StackPush(Stack_t* stack, StackElem_t value)
{
    STACK_ASSERT(StackValid(stack));

    if (stack->size < stack->capacity)
    {
        stack->data[stack->size++] = value;
    }
    else
    {
        RETURN_FAILED_IF(StackResize(stack, WIDEN) == FAILED);

        stack->data[stack->size++] = value;
    }

    STACK_ASSERT(StackValid(stack));

    return EXECUTED;
}

StackCode StackPop(Stack_t* stack)
{
    STACK_ASSERT(StackValid(stack));

    if (stack->size < 1)
    {
        return FAILED;
    }

    stack->size -= 1;

    if (stack->size <= stack->capacity / 4)
    {
        RETURN_FAILED_IF(StackResize(stack, CUTTEN) == FAILED);
    }

    return EXECUTED;
}

StackCode StackResize(Stack_t* stack, int mode)
{
    STACK_ASSERT(StackValid(stack));

    if (mode == WIDEN)
    {
        stack->data = (StackElem_t*) realloc(stack->data, stack->capacity * 2 * sizeof(StackElem_t));

        RETURN_FAILED_IF(stack->data == nullptr);

        stack->capacity = stack->capacity * 2;
    }
    else if (mode == CUTTEN)
    {
        stack->data = (StackElem_t*) realloc(stack->data, stack->capacity / 2 * sizeof(StackElem_t));

        RETURN_FAILED_IF(stack->data == nullptr);

        stack->capacity = stack->capacity / 2;
    }

    STACK_ASSERT(StackValid(stack));

    return EXECUTED;
}

StackCode StackDestroy(Stack_t* stack)
{
    STACK_ASSERT(StackValid(stack));

    free(stack->data);

    stack->data = nullptr;

    stack->size = 0;

    stack->capacity = 0;

    return EXECUTED;
}

StackCode StackTest(Stack_t* stack)
{
    return EXECUTED;
}

StackCode StackDump(Stack_t* stack)
{
    STACK_ASSERT(StackValid(stack));

    for (int i = 0; i < stack->capacity; i++)
    {
        printf("[%d] = %d\n", i, stack->data[i]);
    }

    return EXECUTED;
}

StackCode StackValid(Stack_t* stack)
{
    RETURN_STACK_INVALID_IF(stack == nullptr || stack->data == nullptr || stack->size > stack->capacity);

    return STACK_VALID;
}

StackCode StackAssert(StackCode statement, int line, const char* file, const char* function)
{
    if (statement == STACK_INVALID)
    {
        printf("%s:%d:%s: Assertion failed.\n", file, line, function);

        // abort();
    }

    return EXECUTED;
}
