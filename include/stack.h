#include  <stdint.h>
#include  <pthread.h>

#ifndef STACK_H__
#define STACK_H__

#ifdef  DEBUG

#define ON_DEBUG(...) __VA_ARGS__

#define INIT(name) CANARY, __FILE__, __LINE__, __PRETTY_FUNCTION__, #name, 0, 0, PTHREAD_MUTEX_INITIALIZER,   \
                   false,  INVALID_STACK_ID, nullptr, nullptr, nullptr, 0, 0, 0, CANARY                       \

#define STACK_ASSERT(     code) StackAssert    (code, __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define STACK_IS_VALID(  stack) StackIsValid   (stack,     __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define STACK_IS_DAMAGED(stack) StackIsDamaged (stack,     __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define verified && ParseErr(stderr, err, __LINE__, __FILE__, __PRETTY_FUNCTION__)

#else

#define ON_DEBUG(...)

#define INIT(name) false, INVALID_STACK_ID, nullptr, 0, 0, 0

#define STACK_ASSERT(statement)

#define STACK_IS_VALID(stack)

#define STACK_IS_DAMAGED(stack)

#define verified && ParseErr(stderr, err, __LINE__, __FILE__, __PRETTY_FUNCTION__)

#endif

#define DEDHYPEBEAST  0xCEBA1488BADEDA

#define STRUCT_HASH_OFFSET (uint64_t) &(((Stack_t*) nullptr)->StructHash)

#define MUTEX_OFFSET       (uint64_t) &(((Stack_t*) nullptr)->mutex)

#define PRINT_ERR(code, pow, str)      \
if ((nextPow = code % pow) >= pow / 2) \
{                                      \
    fprintf(fp, str);                  \
}                                      \

#define ALIGNED_TO(val, bytes) bytes + (val - bytes % val) % val

typedef uint64_t StackElem_t;

typedef uint64_t Canary_t;

typedef int StackId_t;

const int         MIN_STACK_SIZE   = 8;

const int         MAX_STACK_SIZE   = 1024*1024;

const int         MAX_STACK_AMOUNT = 16;

const char* const DUMP_FILE       = "dump.html";

const char* const MEMORY_LOG_FILE = "memory.html";

const Canary_t    CANARY = DEDHYPEBEAST;

const int         POISON = 0;

typedef enum StackReturnCodes
{
    EXECUTED              =  0,
    FAILED                = -1,
    STACK_VALID           = -2,
    STACK_INVALID         = -3,
    STACK_DAMAGED         = -4,
    STACK_NOT_DAMAGED     = -5,
    INVALID_STACK_ID      = -6,
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
    INVALID_STACK_ID_ERR  = 4096,
} StackErrorCode;

extern uint64_t err;

StackId_t                StackCtor           (int capacity);

StackId_t                GetStackId          ();

StackReturnCode          StackPush           (StackId_t StackId, StackElem_t value);

StackElem_t              StackPop            (StackId_t StackId);

StackReturnCode          StackDtor           (StackId_t StackId);

StackReturnCode          PrintErr            (FILE* fp, uint64_t code);

StackReturnCode          ParseErr            (FILE* fp, uint64_t code, int line, const char* file, const char* function);

#endif // STACK_H__
