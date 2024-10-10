#include  <stdint.h>
#include  <pthread.h>

#ifndef STACK_H__
#define STACK_H__

#if defined(DEBUG) || defined(HASH_PROTECTION) || defined(CANARY_PROTECTION) || defined(THREAD_PROTECTION)

#ifdef  DEBUG

#define INIT(name) CANARY, __FILE__, __LINE__, __PRETTY_FUNCTION__, \
                   #name, 0, 0,                                     \
                   false,  INVALID_STACK_ID, nullptr, nullptr,      \
                   nullptr, 0, 0, 0, CANARY                         \

#define ON_DEBUG(...)             __VA_ARGS__

#define ON_THREAD_PROTECTION(...)

// #define THREAD_PROTECTION

#define ON_CANARY_PROTECTION(...) __VA_ARGS__

#define CANARY_PROTECTION

#define ON_HASH_PROTECTION(...)   __VA_ARGS__

#define HASH_PROTECTION

#else

#define ON_DEBUG(...)

#ifdef  THREAD_PROTECTION

#define INIT(name) PTHREAD_MUTEX_INITIALIZER, false, INVALID_STACK_ID, nullptr, 0, 0, 0

#define ON_THREAD_PROTECTION(...) __VA_ARGS__

#define ON_CANARY_PROTECTION(...)

#define ON_HASH_PROTECTION(  ...)

#endif

#ifdef  CANARY_PROTECTION

#define INIT(name) CANARY, false, INVALID_STACK_ID, nullptr, nullptr, nullptr, 0, 0, 0, CANARY

#define ON_CANARY_PROTECTION(...) __VA_ARGS__

#define ON_THREAD_PROTECTION(...)

#define ON_HASH_PROTECTION(  ...)

#endif

#ifdef  HASH_PROTECTION

#define INIT(  name) 0, 0, false, INVALID_STACK_ID, nullptr, 0, 0, 0

#define ON_HASH_PROTECTION(  ...) __VA_ARGS__

#define ON_THREAD_PROTECTION(...)

#define ON_CANARY_PROTECTION(...)

#endif

#endif

#else

#define INIT(          name) false, INVALID_STACK_ID, nullptr, 0, 0, 0

#define ON_DEBUG(            ...)

#define ON_THREAD_PROTECTION(...)

#define ON_CANARY_PROTECTION(...)

#define ON_HASH_PROTECTION(  ...)

#endif

#define STACK_ASSERT(     code)    StackAssert    (code,            __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define STACK_IS_VALID(  stack)    StackIsValid   (stack ON_DEBUG(, __LINE__, __FILE__, __PRETTY_FUNCTION__))

#define STACK_IS_DAMAGED(stack)    StackIsDamaged (stack,           __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define STACK_CTOR(      capacity) StackCtor      (capacity,        __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define verified && ParseErr(stderr, err, __LINE__, __FILE__, __PRETTY_FUNCTION__)

#define DEDHYPEBEAST  0xCEBA1488BADEDA

#define STRUCT_HASH_OFFSET (uint64_t) &(((Stack_t*) nullptr)->StructHash)

#define MUTEX_OFFSET       (uint64_t) &(((Stack_t*) nullptr)->mutex)

#define PRINT_ERR(code, pow, str)      \
if ((nextPow = code % pow) >= pow / 2) \
{                                      \
    fprintf(fp, str);                  \
}                                      \

#define ALIGNED_TO(val, bytes) bytes + (val - bytes % val) % val

typedef int StackElem_t;

typedef uint64_t Canary_t;

typedef int      StackId_t;

const   int      MinStackSize   = 8;

const   int      MaxStackSize   = 1024*1024;

const   int      MaxStackAmount = 16;

const   Canary_t CANARY = DEDHYPEBEAST;

const   int      POISON = 0;

extern  uint64_t err;

#ifdef FILE_HTML

const char* const DumpFileName    = "dump.html";

const char* const MemoryLogFileName  = "memory.html";

#define ON_HTML(...) __VA_ARGS__

#define ON_LOG( ...)

#else

const char* const DumpFileName   = "dump.log";

const char* const MemoryLogFileName = "memory.log";

#define ON_HTML(...)

#define ON_LOG( ...)  __VA_ARGS__

#endif

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

StackId_t                StackCtor           (int capacity, int line, const char* file, const char* function);

StackId_t                GetStackId          ();

StackReturnCode          StackPush           (StackId_t StackId, StackElem_t value);

StackElem_t              StackPop            (StackId_t StackId);

StackReturnCode          StackDtor           (StackId_t StackId);

StackReturnCode          PrintErr            (FILE* fp, uint64_t code);

StackReturnCode          ParseErr            (FILE* fp, uint64_t code, int line, const char* file, const char* function);

#endif // STACK_H__
