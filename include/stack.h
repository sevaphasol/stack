#include <stdint.h>

#ifndef STACK_H__
#define STACK_H__

/*
    Made as a macro because of ease of use (you shouldn't write visibly line, file, func in your assert)
*/

#define STACK_ASSERT(statement) StackAssert    (statement, __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define STACK_VALIDATOR(stack)  StackValidator (stack,     __LINE__, __FILE__, __PRETTY_FUNCTION__)

/*
    Made as a macro because of ease of use (you shouldn't write visibly this parametrs)
*/

#define STACK_INIT_PARAM                                                       \
.BornFile  = __FILE__, .BornLine = __LINE__, .BornFunc = __PRETTY_FUNCTION__,  \
.ErrorCode =        0,                                                         \
.data      =  nullptr, .size     =        0, .capacity =                   0,  \

typedef enum StackCodes
{
    EXECUTED              =  0,
    FAILED                = -1,

    STACK_VALID           = -2,
    STACK_INVALID         = -3,
} StackCode;

typedef long long StackElem_t;

struct Stack_t
{
    const char*   BornFile;
    int           BornLine;
    const char*   BornFunc;

    uint64_t     ErrorCode;

    StackElem_t*      data;
    uint64_t          size;
    uint64_t      capacity;
};


StackCode   StackCtor      (Stack_t* stack, int capacity);

StackCode   StackPush      (Stack_t* stack, StackElem_t value);

StackElem_t StackPop       (Stack_t* stack);

StackCode   StackResize    (Stack_t* stack, size_t newCapacity);

StackCode   StackDtor      (Stack_t* stack);

StackCode   StackTest      (Stack_t* stack);

StackCode   StackDump      (Stack_t* stack,      int line, const char* file, const char* function);

StackCode   StackValidator (Stack_t* stack,      int line, const char* file, const char* function);

StackCode   StackAssert    (StackCode statement, int line, const char* file, const char* function);

#endif // STACK_H__
