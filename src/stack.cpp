#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "stack.h"
#include "allocation.h"


struct Stack_t
{
    ON_CANARY_PROTECTION(Canary_t        left_canary);

    ON_DEBUG(            const char *    BornFile);
    ON_DEBUG(            int             BornLine);
    ON_DEBUG(            const char *    BornFunc);
    ON_DEBUG(            const char *    name);
    ON_HASH_PROTECTION(  uint64_t        DataHash);
    ON_HASH_PROTECTION(  uint64_t        StructHash);
    ON_THREAD_PROTECTION(pthread_mutex_t mutex);

                         bool            inited;
                         StackId_t       id;
                         StackElem_t*    data;
    ON_CANARY_PROTECTION(Canary_t*       DataLeftCanary);
    ON_CANARY_PROTECTION(Canary_t*       DataRightCanary);
                         uint64_t        MemorySize;
                         uint64_t        size;
                         uint64_t        capacity;

    ON_CANARY_PROTECTION(Canary_t        right_canary);
};

static Stack_t* STACKS[MAX_STACK_AMOUNT] = {nullptr};

static FILE* MemoryLogFile = nullptr;

static FILE* DumpFile = nullptr;

static StackReturnCode   StackIsDamaged      (StackId_t StackId, int line, const char* file, const char* function);

static StackReturnCode   StackIsValid        (StackId_t StackId ON_DEBUG(, int line, const char* file, const char* function));

static void              StackAssert         (StackReturnCode code, int line, const char* file, const char* function);

static StackReturnCode   CountDataHash       (StackId_t StackId);

static StackReturnCode   CountStructHash     (StackId_t StackId);

static StackReturnCode   StackDump           (Stack_t* stack, int line, const char* file, const char* function);

static StackReturnCode   StackResize         (StackId_t StackId, size_t newCapacity);

StackId_t StackCtor(int capacity, int line, const char* file, const char* function)
{
    #ifdef DEBUG

    if (!MemoryLogFile)
    {
        MemoryLogFile = fopen(MEMORY_LOG_FILE, "w");
        ON_HTML(fprintf(MemoryLogFile, "<!DOCTYPE html><html>"));
    }

    if (!DumpFile)
    {
        DumpFile = fopen(DUMP_FILE, "w");
        ON_HTML(fprintf(DumpFile, "<!DOCTYPE html><html>"));
    };

    #endif

    StackId_t id = GetStackId();

    if (id == INVALID_STACK_ID)
    {
        err += INVALID_STACK_ID_ERR;

        return INVALID_STACK_ID_ERR;
    }

    if (capacity < MIN_STACK_SIZE)
    {
        capacity = MIN_STACK_SIZE;
    }

    #if defined(DEBUG) || defined(CANARY_PROTECTION)

    uint64_t MemorySize = ALIGNED_TO(sizeof(uint64_t), sizeof(Stack_t)) + ALIGNED_TO(sizeof(uint64_t), sizeof(Canary_t) + capacity * sizeof(StackElem_t)) + sizeof(Canary_t);

    Stack_t* stack = (Stack_t*) log_calloc(MemoryLogFile, 1, MemorySize);

    #else

    uint64_t MemorySize = sizeof(Stack_t) + capacity * sizeof(StackElem_t);

    Stack_t* stack = (Stack_t*) calloc(1, MemorySize);

    #endif

    *stack = {INIT(stack)};

    stack->BornLine = line;

    stack->BornFile = file;

    stack->BornFunc = function;

    ON_THREAD_PROTECTION(pthread_mutex_init(&(stack->mutex), NULL));

    if (!stack)

    {
        err += INVALID_STACK_POINTER;

        return INVALID_STACK_POINTER;
    }

    #if defined(DEBUG) || defined(CANARY_PROTECTION)

    stack->MemorySize = MemorySize;

    stack->DataLeftCanary = (Canary_t*) (stack + 1);

    *(stack->DataLeftCanary) = CANARY;

    stack->DataRightCanary = (Canary_t*)((char*) stack + stack->MemorySize - sizeof(Canary_t));

    *(stack->DataRightCanary) = CANARY;

    stack->data = (StackElem_t*)((char*) stack->DataLeftCanary + sizeof(Canary_t));

    #else

    stack->data = (StackElem_t*) (stack + 1);

    #endif

    if (!stack->data)
    {
        err += INVALID_DATA_POINTER;

        return INVALID_STACK_ID;
    }

    memset((void*) stack->data, POISON, capacity * sizeof(StackElem_t));

    stack->size = 0;

    stack->capacity = capacity;

    stack->inited = true;

    stack->id = id;

    STACKS[id - 1] = stack;

    ON_HASH_PROTECTION(CountDataHash(  id));

    ON_HASH_PROTECTION(CountStructHash(id));

    ON_DEBUG(StackDump(stack, __LINE__, __FILE__, __PRETTY_FUNCTION__));

    return stack->id;
}

StackId_t GetStackId()
{
    StackId_t ReturnId = INVALID_STACK_ID;

    StackId_t id = 0;

    for (id; id < MAX_STACK_AMOUNT; id++)
    {
        if (STACKS[id] == nullptr)
        {
            ReturnId = id + 1;
            break;
        }
    }

    return ReturnId;
}

StackReturnCode StackPush(StackId_t StackId, StackElem_t value)
{
    Stack_t* stack = STACKS[StackId - 1];

    STACK_ASSERT(STACK_IS_VALID(StackId));

    ON_THREAD_PROTECTION(pthread_mutex_lock(&(stack->mutex)));

    STACK_ASSERT(STACK_IS_DAMAGED(StackId));

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

        if (StackResize(StackId, stack->capacity * 2) == FAILED)
        {
            return FAILED;
        }

        stack = STACKS[StackId - 1];

        stack->data[stack->size] = value;
    }

    stack->size++;

    ON_DEBUG(StackDump(stack, __LINE__, __FILE__, __PRETTY_FUNCTION__));

    ON_HASH_PROTECTION(CountDataHash(  StackId));

    ON_HASH_PROTECTION(CountStructHash(StackId));

    STACK_ASSERT(STACK_IS_VALID(  StackId));

    STACK_ASSERT(STACK_IS_DAMAGED(StackId));

    ON_THREAD_PROTECTION(pthread_mutex_unlock(&(stack->mutex)));

    return EXECUTED;
}

StackElem_t StackPop(StackId_t StackId)
{
    Stack_t* stack = STACKS[StackId - 1];

    STACK_ASSERT(STACK_IS_VALID(StackId));

    ON_THREAD_PROTECTION(pthread_mutex_lock(&(stack->mutex)));

    STACK_ASSERT(STACK_IS_DAMAGED(StackId));

    if (stack->size == 0)
    {
        err += STACK_UNDERFLOW;

        return FAILED;
    }

    stack->size--;

    ON_HASH_PROTECTION(CountStructHash(StackId));

    StackElem_t value = stack->data[stack->size];

    if ((stack->size <= stack->capacity / 4) && (stack->capacity / 2 >= MIN_STACK_SIZE))
    {
        if (StackResize(StackId, stack->capacity / 2) == FAILED)
        {
            return FAILED;
        }
    }

    stack = STACKS[StackId - 1];

    stack->data[stack->size] = POISON;

    ON_DEBUG(StackDump(stack, __LINE__, __FILE__, __PRETTY_FUNCTION__));

    ON_HASH_PROTECTION(CountDataHash(  StackId));

    ON_HASH_PROTECTION(CountStructHash(StackId));

    STACK_ASSERT(STACK_IS_VALID(  StackId));

    STACK_ASSERT(STACK_IS_DAMAGED(StackId));

    ON_THREAD_PROTECTION(pthread_mutex_unlock(&(stack->mutex)));

    return value;
}


StackReturnCode StackResize(StackId_t StackId, size_t NewCapacity)
{
    Stack_t* stack = STACKS[StackId - 1];

    STACK_ASSERT(STACK_IS_VALID(StackId));

    STACK_ASSERT(STACK_IS_DAMAGED(StackId));

    if (NewCapacity < MIN_STACK_SIZE)
    {
        err += REQUESTED_TOO_LITTLE;

        return FAILED;
    }

    if (NewCapacity > MAX_STACK_SIZE)
    {
        err += REQUESTED_TOO_MUCH;

        return FAILED;
    }

    #if defined(DEBUG) || defined(CANARY_PROTECTION)

    uint64_t NewMemorySize = ALIGNED_TO(sizeof(uint64_t), sizeof(Stack_t)) + \
                             ALIGNED_TO(sizeof(uint64_t), sizeof(Canary_t) + \
                             NewCapacity * sizeof(StackElem_t)) + sizeof(Canary_t);

    stack = (Stack_t*) log_realloc(MemoryLogFile, stack, NewMemorySize);

    if (!(stack))
    {
        err += INVALID_STACK_POINTER;

        return FAILED;
    }

    stack->DataLeftCanary = (Canary_t*) (stack + 1);

    stack->data = (StackElem_t*)((char*) stack->DataLeftCanary + sizeof(Canary_t));

    *((Canary_t*)((char*) stack + stack->MemorySize - sizeof(Canary_t))) = 0; // null previous r_canary

    stack->MemorySize = NewMemorySize;

    stack->capacity   = NewCapacity;

    stack->DataRightCanary = (Canary_t*) ((char*) stack + NewMemorySize - sizeof(Canary_t));

    *(stack->DataRightCanary) = CANARY;

    ON_DEBUG(StackDump(stack, __LINE__, __FILE__, __PRETTY_FUNCTION__));

    #else

    uint64_t NewMemorySize = sizeof(Stack_t) + NewCapacity * sizeof(StackElem_t);

    stack = (Stack_t*) realloc(stack, NewMemorySize);

    if (!(stack))
    {
        err += INVALID_STACK_POINTER;

        return FAILED;
    }

    stack->data = (StackElem_t*) (stack + 1);

    stack->MemorySize = NewMemorySize;

    stack->capacity = NewCapacity;

    #endif

    STACKS[StackId - 1] = stack;

    ON_HASH_PROTECTION(CountDataHash(  StackId));

    ON_HASH_PROTECTION(CountStructHash(StackId));

    STACK_ASSERT(STACK_IS_VALID(  StackId));

    STACK_ASSERT(STACK_IS_DAMAGED(StackId));

    return EXECUTED;
}

StackReturnCode StackDtor(StackId_t StackId)
{
    Stack_t* stack = STACKS[StackId - 1];

    if (!stack)
    {
        return EXECUTED;
    }

    memset(stack, 0, stack->MemorySize);

    #ifdef DEBUG

    log_free(MemoryLogFile, stack);

    if (MemoryLogFile)
    {
        ON_HTML(fprintf(MemoryLogFile, "</html>\n"));

        fclose(MemoryLogFile);
    }

    if (DumpFile)
    {
        ON_HTML(fprintf(DumpFile, "</html>\n"));

        fclose(DumpFile);
    }

    #else

    free(stack);

    #endif

    #ifdef THREAD_PROTECTION

    if (&(stack->mutex))
    {
        pthread_mutex_destroy(&(stack->mutex));
    }

    #endif

    stack = nullptr;

    STACKS[StackId - 1] = nullptr;

    return EXECUTED;
}

StackReturnCode StackDump(Stack_t* stack ON_DEBUG(, int line, const char* file, const char* function))
{
    #ifdef DEBUG

    if (!DumpFile)
    {
        fprintf(stderr, "INVALID FILE POINTER\n");

        err += INVALID_FILE_POINTER;

        return FAILED;
    }

    time_t RawTime;

    struct tm* TimeInfo;

    time(&RawTime);

    TimeInfo = localtime(&RawTime);

    ON_HTML(fprintf(DumpFile, "<p style=\"color:LightGrey;\">"
                              "Local time and date: </p><p style=\"color:LightBlue;\">%s<bp>"
                              "</p>", asctime(TimeInfo)));

    ON_LOG( fprintf(DumpFile, "Local time and date: %s\n", asctime(TimeInfo)));

    PrintErr(DumpFile, err);

    if (!stack)
    {
        ON_HTML(fprintf(DumpFile, "<p style=\"color:Red\">"
                                  "Lost stack pointer</br>"
                                  "</p>"));

        ON_LOG( fprintf(DumpFile, "Lost stack pointer\n"));

        fclose(DumpFile);

        return EXECUTED;
    }

    ON_LOG(fprintf(DumpFile,  "Stack_t[%p] %s at %s:%d in function %s\nBorn at %s:%d in function %s\n\n"
                              "Stack ID             = %d\n\n"
                              "LEFT  STRUCT CANARY  = %lu\n"
                              "RIGHT STRUCT CANARY  = %lu\n\n"
                              "LEFT  DATA   CANARY  = %lu\n"
                              "RIGHT DATA   CANARY  = %lu\n\n"
                              "STRUCT HASH          = %lu\n"
                              "DATA   HASH          = %lu\n\n"
                              "capacity             = %lu\n"
                              "size                 = %lu\n\n",
                              stack, stack->name, file, line, function, stack->BornFile, stack->BornLine, stack->BornFunc,
                              stack->id,
                              stack->left_canary,
                              stack->right_canary,
                              *(stack->DataLeftCanary),
                              *(stack->DataRightCanary),
                              stack->StructHash,
                              stack->DataHash,
                              stack->capacity,
                              stack->size));

    ON_HTML(fprintf(DumpFile, "<h3>Stack_t[<em style=\"color:Red;\">%p</em>] %s"
                              " at <em style=\"color:Red;\">%s</em>:"
                              "<em style=\"color:Red;\">%d</em> in function"
                              " <em style=\"color:Red;\">%s</em><br>Born"
                              " at <em style=\"color:Red;\">%s</em>:"
                              "<em style=\"color:Red;\">%d</em> in function "
                              "<em style=\"color:Red;\">%s</em></h1><br>"
                              "Stack ID              = <em style=\"color:Red;\">%d</em><br><br>"
                              "LEFT   STRUCT CANARY  = <em style=\"color:Red;\">%lu</em><br>"
                              "RIGHT  STRUCT CANARY  = <em style=\"color:Red;\">%lu</em><br><br>"
                              "LEFT   DATA   CANARY  = <em style=\"color:Red;\">%lu</em><br>"
                              "RIGHT  DATA   CANARY  = <em style=\"color:Red;\">%lu</em><br><br>"
                              "STRUCT HASH           = <em style=\"color:Red;\">%lu</em><br>"
                              "DATA   HASH           = <em style=\"color:Red;\">%lu</em><br><br>"
                              "capacity              = <em style=\"color:Red;\">%lu</em><br>"
                              "size                  = <em style=\"color:Red;\">%lu</em><br><br>"
                              "</em>",
                              stack, stack->name, file, line, function, stack->BornFile, stack->BornLine, stack->BornFunc,
                              stack->id,
                              stack->left_canary,
                              stack->right_canary,
                              *(stack->DataLeftCanary),
                              *(stack->DataRightCanary),
                              stack->StructHash,
                              stack->DataHash,
                              stack->capacity,
                              stack->size));

    if (!stack->data)
    {
        ON_HTML(fprintf(DumpFile, "<p style=\"color:LightRed\">"
                                  "Lost stack->data pointer<br>"
                                  "<br><br>---------------------------------------------------------------------<br><br></p>"));

        ON_LOG(fprintf(DumpFile,  "Lost stack->data pointer\n"
                                  "\n\n---------------------------------------------------------------------\n\n"));

        return EXECUTED;
    }

    for (int i = 0; i < stack->capacity; i++)
    {
        if (i < stack->size)
        {
            ON_HTML(fprintf(DumpFile, "<em style=\"color:LightGrey;\">"
                                      "[%d] = </em><em style=\"color:LightBlue;\">%ld</em><br>", i, stack->data[i]));

            ON_LOG( fprintf(DumpFile, "[%d] = %ld\n", i, stack->data[i]));
        }
        else
        {
            ON_HTML(fprintf(DumpFile, "<em style=\"color:LightGrey;\">"
                                      "[%d] = </em><em style=\"color:LightBlue;\">%ld (POISON)</em><br>", i, stack->data[i]));

            ON_LOG( fprintf(DumpFile, "[%d] = %ld (POISON) \n", i, stack->data[i]));
        }
    }

    ON_HTML(fprintf(DumpFile, "<p><br><br>---------------------------------------------------------------------<br><br></p>"));

    ON_LOG( fprintf(DumpFile, "\n\n---------------------------------------------------------------------\n\n"));

    #endif

    return EXECUTED;
}

StackReturnCode CountDataHash(StackId_t StackId)
{
    #ifdef DEBUG

    Stack_t* stack = STACKS[StackId - 1];

    STACK_ASSERT(STACK_IS_VALID(StackId));

    uint64_t DataHash = 5831;

    for (size_t i = 0; i < stack->capacity; i++)
    {
        DataHash = 33 * DataHash + stack->data[i];
    }

    stack->DataHash = DataHash;

    #endif

    return EXECUTED;
}

StackReturnCode CountStructHash(StackId_t StackId)
{
    #ifdef HASH_PROTECTION

    Stack_t* stack = STACKS[StackId - 1];

    STACK_ASSERT(STACK_IS_VALID(StackId));

    #ifdef THREAD_PROTECTION

    uint64_t FirstOffset, FirstSize, SecondOffset, SecondSize = 0;

    if (STRUCT_HASH_OFFSET < MUTEX_OFFSET)
    {
        FirstOffset  = STRUCT_HASH_OFFSET;
        FirstSize    = sizeof(stack->StructHash);
        SecondOffset = MUTEX_OFFSET;
        SecondSize   = sizeof(stack->mutex);
    }
    else
    {
        FirstOffset  = STRUCT_HASH_OFFSET;
        FirstSize    = sizeof(stack->StructHash);
        SecondOffset = MUTEX_OFFSET;
        SecondSize   = sizeof(stack->mutex);
    }

    uint64_t StructHash = 5831;

    for (size_t i = 0; i < FirstOffset; i++)
    {
        StructHash = 33 * StructHash + *((char*) stack + i);
    }

    for (size_t i = FirstOffset + FirstSize; i < SecondOffset; i++)
    {
        StructHash = 33 * StructHash + *((char*) stack + i);
    }

    for (size_t i = SecondOffset + SecondSize; i < sizeof(Stack_t); i++)
    {
        StructHash = 33 * StructHash + *((char*) stack + i);
    }

    stack->StructHash = StructHash;

    #else

    uint64_t StructHash = 5831;

    for (size_t i = 0; i < STRUCT_HASH_OFFSET; i++)
    {
        StructHash = 33 * StructHash + *((char*) stack + i);
    }

    for (size_t i = STRUCT_HASH_OFFSET + sizeof(stack->StructHash); i < sizeof(Stack_t); i++)
    {
        StructHash = 33 * StructHash + *((char*) stack + i);
    }

    stack->StructHash = StructHash;

    #endif

    #endif

    return EXECUTED;
}

StackReturnCode StackIsValid(StackId_t StackId ON_DEBUG(, int line, const char* file, const char* function))
{
    Stack_t* stack = STACKS[StackId - 1];

    ON_DEBUG(StackDump(stack, line, file, function));

    if (!stack)
    {
        err += INVALID_STACK_POINTER;

        ON_DEBUG(StackDump(stack, line, file, function));

        StackDtor(StackId);

        return STACK_INVALID;
    }

    if (stack->id == INVALID_STACK_ID)
    {
        err += INVALID_STACK_ID_ERR;

        ON_DEBUG(StackDump(stack, line, file, function));

        StackDtor(StackId);

        return INVALID_STACK_ID;
    }

    if (!stack->data)
    {
        err += INVALID_DATA_POINTER;

        ON_DEBUG(StackDump(stack, line, file, function));

        StackDtor(StackId);

        return STACK_INVALID;
    }

    if (stack->size > stack->capacity)
    {
        err += INVALID_SIZE;

        ON_DEBUG(StackDump(stack, line, file, function));

        StackDtor(StackId);

        return STACK_INVALID;
    }

    return STACK_VALID;
}

void StackAssert(StackReturnCode code, int line, const char* file, const char* function)
{
    if (code == STACK_DAMAGED || code == STACK_INVALID)
    {
        fprintf(stderr, "%s:%d:%s: Assertion failed. STACK IS INVALID\n", file, line, function);

        PrintErr(stderr, err);

        #ifdef DEBUG

        if (MemoryLogFile)
        {
            fflush(MemoryLogFile);

            fclose(MemoryLogFile);
        }

        if (DumpFile)
        {
            fflush(DumpFile);

            fclose(DumpFile);
        }

        abort();

        #endif
    }
}

StackReturnCode StackIsDamaged(StackId_t StackId, int line, const char* file, const char* function)
{
    #if defined(DEBUG) || defined(HASH_PROTECTION) || defined(CANARY_PROTECTION)

    Stack_t* stack = STACKS[StackId - 1];

    STACK_ASSERT(STACK_IS_VALID(StackId));

    ON_DEBUG(StackDump(stack, line, file, function));

    #ifdef DEBUG

    if (stack->left_canary != CANARY || stack->right_canary != CANARY)
    {
        err += INVALID_STRUCT_CANARY;

        ON_DEBUG(StackDump(stack, line, file, function));

        StackDtor(StackId);

        return STACK_DAMAGED;
    }

    if (*(stack->DataLeftCanary) != CANARY || *(stack->DataRightCanary) != CANARY)
    {
        err += INVALID_DATA_CANARY;

        StackDump(stack, line, file, function);

        StackDtor(StackId);

        return STACK_DAMAGED;
    }

    uint64_t DataHash = stack->DataHash;

    uint64_t StructHash = stack->StructHash;

    CountDataHash(StackId);

    CountStructHash(StackId);

    if (StructHash != stack->StructHash)
    {
        err += INVALID_HASH;

        ON_DEBUG(StackDump(stack, line, file, function));

        StackDtor(StackId);

        return STACK_DAMAGED;
    }

    if (DataHash != stack->DataHash)
    {
        err += INVALID_HASH;

        ON_DEBUG(StackDump(stack, line, file, function));

        StackDtor(StackId);

        return STACK_DAMAGED;
    }

    #else

    #ifdef CANARY_PROTECTION

    if (stack->left_canary != CANARY || stack->right_canary != CANARY)
    {
        err += INVALID_STRUCT_CANARY;

        ON_DEBUG(StackDump(stack, line, file, function));

        StackDtor(StackId);

        return STACK_DAMAGED;
    }

    if (*(stack->DataLeftCanary) != CANARY || *(stack->DataRightCanary) != CANARY)
    {
        err += INVALID_DATA_CANARY;

        ON_DEBUG(StackDump(stack, line, file, function));

        StackDtor(StackId);

        return STACK_DAMAGED;
    }

    #endif

    #ifdef HASH_PROTECTION

    uint64_t DataHash = stack->DataHash;

    uint64_t StructHash = stack->StructHash;

    CountDataHash(StackId);

    CountStructHash(StackId);

    if (StructHash != stack->StructHash)
    {
        err += INVALID_HASH;

        ON_DEBUG(StackDump(stack, line, file, function));

        StackDtor(StackId);

        return STACK_DAMAGED;
    }

    if (DataHash != stack->DataHash)
    {
        err += INVALID_HASH;

        ON_DEBUG(StackDump(stack, line, file, function));

        StackDtor(StackId);

        return STACK_DAMAGED;
    }

    #endif

    ON_DEBUG(StackDump(stack, line, file, function));

    #endif

    #endif

    return STACK_NOT_DAMAGED;
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
        ON_HTML(fprintf(fp, "<p style=\"color: LightGreen\";>NO ERROR<br></p>"));

        ON_LOG(fprintf(fp, "OK\n"));

        return EXECUTED;
    }

    ON_HTML(fprintf(fp, "<p style=\"color: Red\";>ERRORS: "));

    ON_LOG(fprintf(fp, "ERRORS: "));

    PRINT_ERR(code, 8192, "INVALID STACK ID ");

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

    ON_HTML(fprintf(fp, "</p><br>"));

    ON_LOG(fprintf(fp, "\n"));

    return EXECUTED;
}

StackReturnCode ParseErr(FILE* fp, uint64_t code, int line, const char* file, const char* function)
{
    uint64_t nextPow = code;

    if (!fp)
    {
        return FAILED;
    }

    ON_HTML(fprintf(fp, "<p style=\"color: LightBlue\";>Called in %s:%d:%s</p><br>", file, line, function));

    ON_LOG( fprintf(fp, "Called in %s:%d:%s\n", file, line, function));

    PrintErr(fp, code);

    return EXECUTED;
}
