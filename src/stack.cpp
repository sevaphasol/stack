#include "stack.cpp"
#include "error_codes.h"

int StackInit(Stack_t* stack, int capacity)
{
    stack = StackElem_t* (calloc(capacity, sizeof(StackElem_t));

    if (!stack)
    {
        return FAILED;
    }

    return EXECUTED;
}

int StackPush(Stack_t* stack, StackElem_t value)
{
    StackAssert(StackValid(stack));

    stack->data[stack->size++] = value;

    StackAssert(StackValid(stack));

    return EXECUTED;
}

int StackPop(Stack_t* stack)
{
    return EXECUTED;
}

int StackResize(Stack_t* stack)
{
    return EXECUTED;
}

int StackDestroy(Stack_t* stack)
{
    return EXECUTED;
}

int StackDump(Stack_t* stack)
{
    return EXECUTED;
}

int StackValid(Stack_t* stack)
{
    return EXECUTED;
}

int StackAssert(bool statement)
{
    if (!statement)
    {

    }
}
