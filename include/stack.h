#include <stdint.h>

#ifndef STACK_H__
#define STACK_H__

#ifdef DEBUG

#define ON_DEBUG(...) __VA_ARGS__

#define INIT(name) CANARY, __FILE__, __LINE__, __PRETTY_FUNCTION__, #name, 0, false, nullptr, nullptr, 0, 0, CANARY

#define STACK_ASSERT(     code) StackAssert    (code, __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define STACK_IS_VALID(  stack) StackIsValid   (stack,     __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define STACK_IS_DAMAGED(stack) StackIsDamaged (stack,     __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define verified && fprintf(stderr, "\033[31mFailed\033[0m\n")

#else

#define ON_DEBUG(...)

#define INIT(name) CANARY, false, nullptr, 0, 0, CANARY

#define STACK_ASSERT(statement)

#define STACK_IS_VALID(stack)

#define STACK_IS_DAMAGED(stack)

#endif

#define PRINT_ERR(code, pow, str)      \
if ((nextPow = code % pow) >= pow / 2) \
{                                      \
    fprintf(fp, str);                  \
}                                      \

int const MIN_STACK_SIZE = 8;

int const MAX_STACK_SIZE = 1024*1024;

const uint64_t CANARY  = 0xCEBA;

typedef enum StackReturnCodes
{
    EXECUTED              =  0,
    FAILED                = -1,
    STACK_VALID           = -2,
    STACK_INVALID         = -3,
    STACK_DAMAGED         = -4,
    STACK_NOT_DAMAGED     = -5,
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
    DAMAGED_STACK_ERR     = 256,
    INVALID_HASH          = 512,
    INVALID_DATA_CANARY   = 1024,
    INVALID_STRUCT_CANARY = 2048,
} StackErrorCode;

typedef uint64_t StackElem_t;

struct Stack_t
{
    uint64_t left_canary;

    ON_DEBUG(const char*   BornFile);
    ON_DEBUG(int           BornLine);
    ON_DEBUG(const char*   BornFunc);
    ON_DEBUG(const char*   name);
    ON_DEBUG(uint64_t      hash);

    bool              inited;
    StackElem_t*      data;
    StackElem_t*      DataWithCanary;
    uint64_t          size;
    uint64_t          capacity;

    uint64_t right_canary;
};

extern uint64_t err;

StackReturnCode   StackCtor           (Stack_t* stack, int capacity);

StackReturnCode   StackPush           (Stack_t* stack, StackElem_t value);

StackElem_t       StackPop            (Stack_t* stack);

StackReturnCode   StackResize         (Stack_t* stack, size_t newCapacity);

StackReturnCode   StackDtor           (Stack_t* stack);

StackReturnCode   StackTest           (Stack_t* stack);

StackReturnCode   StackDump           (Stack_t* stack ON_DEBUG(, int line, const char* file, const char* function));

StackReturnCode   PrintErr            (FILE* fp, uint64_t code);

StackReturnCode   StackIsDamaged      (Stack_t* stack, int line, const char* file, const char* function);

StackReturnCode   StackIsValid        (Stack_t* stack ON_DEBUG(, int line, const char* file, const char* function));

void              StackAssert         (StackReturnCode code, int line, const char* file, const char* function);

static StackReturnCode GetHash        (Stack_t* stack);

#endif // STACK_H__
