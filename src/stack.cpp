#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "stack.h"

static Stack_t* STACKS[MAX_STACK_AMOUNT] = {nullptr};

StackId_t StackCtor(int capacity)
{
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

    #else

    uint64_t MemorySize = sizeof(Stack_t) + capacity * sizeof(StackElem_t);

    #endif

    Stack_t* stack = (Stack_t*) calloc(1, MemorySize);

    *stack = {INIT(stack)};

    if (!stack)
    {
        err += INVALID_STACK_POINTER;

        return INVALID_STACK_ID;
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

    ON_DEBUG(FILE* DumpFile = fopen(DUMP_FILE, "w"));

    ON_DEBUG(stack->DumpFile = DumpFile);

    stack->size = 0;

    stack->capacity = capacity;

    stack->inited = true;

    stack->id = id;

    STACKS[id - 1] = stack;

    ON_DEBUG(GetHash(stack));

    ON_DEBUG(StackDump(stack, __LINE__, __FILE__, __PRETTY_FUNCTION__));

    return stack->id;
}

StackId_t GetStackId()
{
    StackId_t ReturnId = INVALID_STACK_ID;

    StackId_t id = 0;

    for (id = 0; id < MAX_STACK_AMOUNT; id++)
    {
        if (STACKS[id] == nullptr)
        {
            ReturnId = id;
            break;
        }
    }

    return id + 1;
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

    ON_DEBUG(StackDump(stack, __LINE__, __FILE__, __PRETTY_FUNCTION__));

    ON_DEBUG(GetHash(stack));

    stack->size++;

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

    StackElem_t value = stack->data[stack->size];

    if ((stack->size <= stack->capacity / 4) && (stack->capacity / 2 >= MIN_STACK_SIZE))
    {
        if (StackResize(StackId, stack->capacity / 2) == FAILED)
        {
            return FAILED;
        }
    }

    stack = STACKS[StackId - 1];

    stack->data[stack->size] = 0;

    ON_DEBUG(StackDump(stack, __LINE__, __FILE__, __PRETTY_FUNCTION__));

    ON_DEBUG(GetHash(stack));

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

    stack = (Stack_t*) realloc(stack, NewMemorySize);

    if (!(stack))
    {
        err += INVALID_STACK_POINTER;

        return FAILED;
    }

    stack->DataLeftCanary = (Canary_t*) (stack + 1);

    stack->data = (StackElem_t*)((char*) stack->DataLeftCanary + sizeof(Canary_t));

    *((Canary_t*)((char*) stack + stack->MemorySize - sizeof(Canary_t))) = 0; // null previous r_canary

    stack->MemorySize = NewMemorySize;

    stack->capacity = NewCapacity;

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

    ON_DEBUG(GetHash(stack));

    STACKS[StackId - 1] = stack;

    STACK_ASSERT(STACK_IS_VALID(stack));
    STACK_ASSERT(STACK_IS_DAMAGED(stack));

    return EXECUTED;
}

StackReturnCode StackDtor(StackId_t StackId)
{
    Stack_t* stack = STACKS[StackId - 1];

    STACK_ASSERT(STACK_IS_VALID(stack));
    STACK_ASSERT(STACK_IS_DAMAGED(stack));

    ON_DEBUG(fclose(stack->DumpFile));

    memset(stack, 0, stack->MemorySize);

    free(stack);

    stack = nullptr;

    STACKS[StackId - 1] = nullptr;

    return EXECUTED;
}

StackReturnCode StackDump(Stack_t* stack ON_DEBUG(, int line, const char* file, const char* function))
{
    #ifdef DEBUG

    if (!stack->DumpFile)
    {
        fprintf(stderr, "IFP\n");

        err += INVALID_FILE_POINTER;

        return FAILED;
    }

    time_t RawTime;

    struct tm* TimeInfo;

    time(&RawTime);

    TimeInfo = localtime(&RawTime);

    fprintf(stack->DumpFile, "Local time and date: %s", asctime(TimeInfo));

    PrintErr(stack->DumpFile, err);

    if (!stack)
    {
        fprintf(stack->DumpFile, "Lost stack pointer\n");

        fclose(stack->DumpFile);

        return EXECUTED;
    }

    fprintf(stack->DumpFile, "Stack_t[%p] %s at %s:%d in function %s\nBorn at %s:%d in function %s\n\n",
            stack, stack->name, file, line, function, stack->BornFile, stack->BornLine, stack->BornFunc);

    fprintf(stack->DumpFile, "Stack ID = %d\n", stack->id);

    fprintf(stack->DumpFile, "LEFT STRUCT CANARY = %ld\n", stack->left_canary);

    fprintf(stack->DumpFile, "RIGHT STRUCT CANARY = %ld\n", stack->right_canary);

    fprintf(stack->DumpFile, "capacity = %ld\n", stack->capacity);

    fprintf(stack->DumpFile, "size = %ld\n", stack->size);

    if (!stack->data)
    {
        fprintf(stack->DumpFile, "Lost stack->data pointer\n");

        fprintf(stack->DumpFile, "\n\n---------------------------------------------------------------------\n\n");

        return EXECUTED;
    }

    fprintf(stack->DumpFile, "\nLEFT DATA CANARY = %ld\n\n", *(stack->DataLeftCanary));

    for (int i = 0; i < stack->capacity; i++)
    {
        fprintf(stack->DumpFile, "[%d] = %ld\n", i, stack->data[i]);
    }

    fprintf(stack->DumpFile, "\nRIGHT DATA CANARY = %ld\n", *(stack->DataRightCanary));

    fprintf(stack->DumpFile, "\n\n---------------------------------------------------------------------\n\n");

    fflush(stack->DumpFile);

    #endif

    return EXECUTED;
}

StackReturnCode GetHash(Stack_t* stack)
{
    #ifdef DEBUG

    STACK_ASSERT(STACK_IS_VALID(stack));

    uint64_t hash = 5831;

    for (size_t i = 0; i < stack->capacity; i++)
    {
        hash = (33 * hash + hash) + stack->data[i];
    }

    stack->hash = hash;

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
    if (code == STACK_INVALID)
    {
        fprintf(stderr, "%s:%d:%s: Assertion failed. STACK IS INVALID\n", file, line, function);

        PrintErr(stderr, err);

        ON_DEBUG(abort());
    }

    if (code == STACK_DAMAGED)
    {
        fprintf(stderr, "%s:%d:%s: Assertion failed. STACK IS DAMAGED\n", file, line, function);

        PrintErr(stderr, err);

        ON_DEBUG(abort());
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

    uint64_t hash = stack->hash;

    GetHash(stack);

    if (hash != stack->hash)
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

        fclose(stack->DumpFile);

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

void ParseErr (uint64_t code)
{
    PrintErr(stderr, code);
}
