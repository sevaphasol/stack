#include "cstddef"

#ifndef STACK_H__
#define STACK_H__

#define STACK_ASSERT(statement) StackAssert(statement, __LINE__, __FILE__, __PRETTY_FUNCTION__)

/*
    Made as a macro because of return FAILED
    Checks if function returned FAILED
*/

#define RETURN_FAILED_IF(statement)  \
{                                    \
    if (statement)                   \
    {                                \
        return FAILED;               \
    }                                \
}

/*
    Made as a macro because of return FAILED
    Checks if function returned FAILED
*/

#define RETURN_STACK_INVALID_IF(statement) \
{                                    \
    if (statement)                   \
    {                                \
        return STACK_INVALID;        \
    }                                \
}

typedef enum StackCodes
{
    EXECUTED              =  0,
    FAILED                = -1,

    STACK_VALID           = -2,
    STACK_INVALID         = -3,

    CUTTEN                = -8,
    WIDEN                 = -9,
} StackCode;

typedef int StackElem_t;

struct Stack_t
{
    StackElem_t *data;
    size_t size;
    size_t capacity;
};


StackCode StackInit    (Stack_t* stack, int capacity);

StackCode StackPush    (Stack_t* stack, StackElem_t value);

StackCode StackPop     (Stack_t* stack);

StackCode StackResize  (Stack_t* stack, int mode);

StackCode StackDestroy (Stack_t* stack);

StackCode StackTest    (Stack_t* stack);

StackCode StackDump    (Stack_t* stack);

StackCode StackValid   (Stack_t* stack);

StackCode StackAssert  (StackCode statement, int line, const char* file, const char* function);

#endif // STACK_H__
