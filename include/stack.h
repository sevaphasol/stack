#include <stdint.h>

#ifndef STACK_H__
#define STACK_H__

/*
    Made as a macro because of ease of use (you shouldn't write visibly line, file, func in your assert)
*/

#define STACK_ASSERT(statement) StackAssert    (statement, __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define STACK_IS_VALID(stack)   StackIsValid   (stack,     __LINE__, __FILE__, __PRETTY_FUNCTION__)

/*
    Made as a macro because of ease of use (you shouldn't write visibly this parametrs)
*/

#define STACK_INIT_PARAM                                                          \
.BornFile    = __FILE__, .BornLine  = __LINE__, .BornFunc = __PRETTY_FUNCTION__,  \
.initialized =    false, .ErrorCode =        0,                                   \
.data        =  nullptr, .size      =        0, .capacity =                   0,  \

int const MIN_STACK_SIZE = 1024;

int const MAX_STACK_SIZE = 1024*1024;

typedef enum StackReturnCodes
{
    EXECUTED              =  0,
    FAILED                = -1,
    STACK_VALID           = -2,
    STACK_INVALID         = -3,

} StackReturnCode;

typedef enum StackErrorCodes
{
    NO_ERROR              = 0,
    STACK_UNDERFLOW       = 1,
    STACK_OVERFLOW        = 2,
    INVALID_STACK_POINTER = 4,
    INVALID_DATA_POINTER  = 8,
    INVALID_SIZE          = 16,
    REQUESTED_TOO_LITTLE  = 32,
    REQUESTED_TOO_MUCH    = 64,
    INVALID_FILE_POINTER  = 128,

} StackErrorCode;

typedef uint64_t StackElem_t;

struct Stack_t
{
    const char*   BornFile;
    int           BornLine;
    const char*   BornFunc;

    bool          initialized;

    uint64_t      ErrorCode;

    StackElem_t*      data;
    uint64_t          size;
    uint64_t      capacity;
};

extern uint64_t err;

StackReturnCode   StackCtor           (Stack_t* stack, int capacity);

StackReturnCode   StackPush           (Stack_t* stack, StackElem_t value);

StackElem_t       StackPop            (Stack_t* stack);

StackReturnCode   StackResize         (Stack_t* stack, size_t newCapacity);

StackReturnCode   StackDtor           (Stack_t* stack);

StackReturnCode   StackTest           (Stack_t* stack);

StackReturnCode   StackDump           (Stack_t* stack,      int line, const char* file, const char* function);

bool              StackIsValid        (Stack_t* stack,      int line, const char* file, const char* function);

void              StackAssert         (bool statement, int line, const char* file, const char* function);

#endif // STACK_H__
