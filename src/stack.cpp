#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "stack.h"
#include "allocation.h"


struct Stack_t
{
    ON_DEBUG(Canary_t      left_canary);

    ON_DEBUG(const char*   BornFile);
    ON_DEBUG(int           BornLine);
    ON_DEBUG(const char*   BornFunc);
    ON_DEBUG(const char*   name);
    ON_DEBUG(uint64_t      DataHash);
    ON_DEBUG(uint64_t      StructHash);

    bool                   inited;
    StackId_t              id;
    StackElem_t*           data;
    ON_DEBUG(Canary_t*     DataLeftCanary);
    ON_DEBUG(Canary_t*     DataRightCanary);
    uint64_t               MemorySize;
    uint64_t               size;
    uint64_t               capacity;

    ON_DEBUG(Canary_t      right_canary);
};

static Stack_t* STACKS[MAX_STACK_AMOUNT] = {nullptr};

static FILE* MemoryLogFile = nullptr;

static FILE* DumpFile = nullptr;

static StackReturnCode   StackIsDamaged      (Stack_t* stack, int line, const char* file, const char* function);

static StackReturnCode   StackIsValid        (Stack_t* stack ON_DEBUG(, int line, const char* file, const char* function));

static void              StackAssert         (StackReturnCode code, int line, const char* file, const char* function);

static StackReturnCode   CountDataHash       (Stack_t* stack);

static StackReturnCode   CountStructHash     (Stack_t* stack);

static StackReturnCode   StackDump           (Stack_t* stack ON_DEBUG(, int line, const char* file, const char* function));

static StackReturnCode   StackResize         (StackId_t StackId, size_t newCapacity);

StackId_t StackCtor(int capacity)
{
    #ifdef DEBUG

    if (!MemoryLogFile)
    {
        MemoryLogFile = fopen(MEMORY_LOG_FILE, "w");
    }

    if (!DumpFile)
    {
        DumpFile = fopen(DUMP_FILE, "w");
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

    #ifdef DEBUG

    uint64_t MemorySize = ALIGNED_TO(sizeof(uint64_t), sizeof(Stack_t)) + ALIGNED_TO(sizeof(uint64_t), sizeof(Canary_t) + capacity * sizeof(StackElem_t)) + sizeof(Canary_t);

    Stack_t* stack = (Stack_t*) log_calloc(MemoryLogFile, 1, MemorySize);

    #else

    uint64_t MemorySize = sizeof(Stack_t) + capacity * sizeof(StackElem_t);

    Stack_t* stack = (Stack_t*) calloc(1, MemorySize);

    #endif


    *stack = {INIT(stack)};

    if (!stack)
    {
        err += INVALID_STACK_POINTER;

        return INVALID_STACK_POINTER;
    }

    #ifdef DEBUG

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

    ON_DEBUG(CountDataHash(stack));

    ON_DEBUG(CountStructHash(stack));

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

    STACK_ASSERT(STACK_IS_VALID(stack));

    STACK_ASSERT(STACK_IS_DAMAGED(stack));

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

    ON_DEBUG(CountDataHash(stack));

    ON_DEBUG(CountStructHash(stack));

    STACK_ASSERT(STACK_IS_VALID(stack));

    STACK_ASSERT(STACK_IS_DAMAGED(stack));

    return EXECUTED;
}

StackElem_t StackPop(StackId_t StackId)
{
    Stack_t*stack = STACKS[StackId - 1];

    STACK_ASSERT(STACK_IS_VALID(stack));

    STACK_ASSERT(STACK_IS_DAMAGED(stack));

    if (stack->size == 0)
    {
        err += STACK_UNDERFLOW;

        return FAILED;
    }

    stack->size--;

    ON_DEBUG(CountStructHash(stack));

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

    ON_DEBUG(CountDataHash(stack));

    ON_DEBUG(CountStructHash(stack));

    STACK_ASSERT(STACK_IS_VALID(stack));

    STACK_ASSERT(STACK_IS_DAMAGED(stack));

    return value;
}


StackReturnCode StackResize(StackId_t StackId, size_t NewCapacity)
{
    Stack_t* stack = STACKS[StackId - 1];

    STACK_ASSERT(STACK_IS_VALID(stack));

    STACK_ASSERT(STACK_IS_DAMAGED(stack));

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

    #ifdef DEBUG

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

    StackDump(stack, __LINE__, __FILE__, __PRETTY_FUNCTION__);

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

    ON_DEBUG(CountDataHash(stack));

    ON_DEBUG(CountStructHash(stack));

    STACK_ASSERT(STACK_IS_VALID(stack));

    STACK_ASSERT(STACK_IS_DAMAGED(stack));

    return EXECUTED;
}

StackReturnCode StackDtor(StackId_t StackId)
{
    Stack_t* stack = STACKS[StackId - 1];

    STACK_ASSERT(STACK_IS_VALID(stack));
    STACK_ASSERT(STACK_IS_DAMAGED(stack));

    memset(stack, 0, stack->MemorySize);

    log_free(MemoryLogFile, stack);

    ON_DEBUG(fclose(MemoryLogFile));

    ON_DEBUG(fclose(DumpFile));

    stack = nullptr;

    STACKS[StackId - 1] = nullptr;

    return EXECUTED;
}

StackReturnCode StackDump(Stack_t* stack ON_DEBUG(, int line, const char* file, const char* function))
{
    #ifdef DEBUG

    if (!DumpFile)
    {
        fprintf(stderr, "IFP\n");

        err += INVALID_FILE_POINTER;

        return FAILED;
    }

    time_t RawTime;

    struct tm* TimeInfo;

    time(&RawTime);

    TimeInfo = localtime(&RawTime);

    fprintf(DumpFile, "Local time and date: %s", asctime(TimeInfo));

    PrintErr(DumpFile, err);

    if (!stack)
    {
        fprintf(DumpFile, "Lost stack pointer\n");

        fclose(DumpFile);

        return EXECUTED;
    }

    fprintf(DumpFile, "Stack_t[%p] %s at %s:%d in function %s\nBorn at %s:%d in function %s\n\n",
            stack, stack->name, file, line, function, stack->BornFile, stack->BornLine, stack->BornFunc);

    fprintf(DumpFile, "Stack ID             = %d\n\n",    stack->id);

    fprintf(DumpFile, "LEFT  STRUCT CANARY  = %lu\n",     stack->left_canary);

    fprintf(DumpFile, "RIGHT STRUCT CANARY  = %lu\n\n",   stack->right_canary);

    fprintf(DumpFile, "LEFT  DATA   CANARY  = %lu\n",   *(stack->DataLeftCanary));

    fprintf(DumpFile, "RIGHT DATA   CANARY  = %lu\n\n", *(stack->DataRightCanary));

    fprintf(DumpFile, "STRUCT HASH          = %lu\n",     stack->StructHash);

    fprintf(DumpFile, "DATA   HASH          = %lu\n\n",   stack->DataHash);

    fprintf(DumpFile, "capacity             = %lu\n",     stack->capacity);

    fprintf(DumpFile, "size                 = %lu\n\n",   stack->size);

    if (!stack->data)
    {
        fprintf(DumpFile, "Lost stack->data pointer\n");

        fprintf(DumpFile, "\n\n---------------------------------------------------------------------\n\n");

        return EXECUTED;
    }

    for (int i = 0; i < stack->capacity; i++)
    {
        if (i < stack->size)
        {
            fprintf(DumpFile, "[%d] = %ld\n", i, stack->data[i]);
        }
        else
        {
            fprintf(DumpFile, "[%d] = (POISON)\n", i);
        }
    }

    fprintf(DumpFile, "\n\n---------------------------------------------------------------------\n\n");

    #endif

    return EXECUTED;
}

StackReturnCode CountDataHash(Stack_t* stack)
{
    #ifdef DEBUG

    STACK_ASSERT(STACK_IS_VALID(stack));

    uint64_t DataHash = 5831;

    for (size_t i = 0; i < stack->capacity; i++)
    {
        DataHash = 33 * DataHash + stack->data[i];
    }

    stack->DataHash = DataHash;

    #endif

    return EXECUTED;
}

StackReturnCode CountStructHash(Stack_t* stack)
{
    #ifdef DEBUG

    STACK_ASSERT(STACK_IS_VALID(stack));

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

    return EXECUTED;
}

StackReturnCode StackIsValid(Stack_t* stack ON_DEBUG(, int line, const char* file, const char* function))
{
    ON_DEBUG(StackDump(stack, line, file, function));

    if (!stack)
    {
        err += INVALID_STACK_POINTER;

        ON_DEBUG(StackDump(stack, line, file, function));

        return STACK_INVALID;
    }

    if (stack->id == INVALID_STACK_ID)
    {
        err += INVALID_STACK_ID_ERR;

        ON_DEBUG(StackDump(stack, line, file, function));

        return INVALID_STACK_ID;
    }

    if (!stack->data)
    {
        err += INVALID_DATA_POINTER;

        ON_DEBUG(StackDump(stack, line, file, function));

        return STACK_INVALID;
    }

    if (stack->size > stack->capacity)
    {
        err += INVALID_SIZE;

        ON_DEBUG(StackDump(stack, line, file, function));

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

StackReturnCode StackIsDamaged(Stack_t* stack, int line, const char* file, const char* function)
{
    #ifdef DEBUG

    STACK_ASSERT(STACK_IS_VALID(stack));

    StackDump(stack, line, file, function);

    if (stack->left_canary != CANARY || stack->right_canary != CANARY)
    {
        err += INVALID_STRUCT_CANARY;

        StackDump(stack, line, file, function);

        PrintErr(stderr, err);

        return STACK_DAMAGED;
    }

    uint64_t DataHash = stack->DataHash;

    uint64_t StructHash = stack->StructHash;

    CountDataHash(stack);

    CountStructHash(stack);

    if (StructHash != stack->StructHash)
    {
        err += INVALID_HASH;

        StackDump(stack, line, file, function);

        PrintErr(stderr, err);

        return STACK_DAMAGED;
    }

    if (DataHash != stack->DataHash)
    {
        err += INVALID_HASH;

        StackDump(stack, line, file, function);

        PrintErr(stderr, err);

        return STACK_DAMAGED;
    }

    if (*(stack->DataLeftCanary) != CANARY || *(stack->DataRightCanary) != CANARY)
    {
        err += INVALID_DATA_CANARY;

        StackDump(stack, line, file, function);

        PrintErr(stderr, err);

        return STACK_DAMAGED;
    }

    StackDump(stack, line, file, function);

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
        fprintf(fp, "NO ERROR\n");

        return EXECUTED;
    }

    fprintf(fp, "ERRORS: ");

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

    fprintf(fp, "\n");

    return EXECUTED;
}

StackReturnCode ParseErr(FILE* fp, uint64_t code, int line, const char* file, const char* function)
{
    uint64_t nextPow = code;

    if (!fp)
    {
        return FAILED;
    }

    fprintf(fp, "Called in %s:%d:%s\n", file, line, function);

    if (code == 0)
    {
        fprintf(fp, "NO ERROR\n");

        return EXECUTED;
    }

    fprintf(fp, "ERRORS: ");

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

    fprintf(fp, "\n");

    return EXECUTED;
}
