#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "stack.h"

StackReturnCode StackTest();

void  EasyHackExample();    // Ability to switch stack ids

void  MediumHackExample();  // CountDataHash doesn't count last elem of data

void  MediumHackExample2(); // Example of imperfection of CountDataHash

void  HardHackExample();    // When INVALID_HASH error is triggered deadlock occurs

void  HardHackExample2();   // StackPop can go before stack->data and return incorrect value

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
                         int64_t         size;
                         uint64_t        capacity;

    ON_CANARY_PROTECTION(Canary_t        right_canary);
};

extern Stack_t* STACKS[MAX_STACK_AMOUNT];

StackReturnCode StackTest()
{
    EasyHackExample();

    // MediumHackExample();

    // MediumHackExample2();

    // HardHackExample();

    // HardHackExample2();

    return EXECUTED;
}

void EasyHackExample()
{
    StackId_t StackId1 = STACK_CTOR(MIN_STACK_SIZE);
    StackId_t StackId2 = STACK_CTOR(MIN_STACK_SIZE);

    StackPush(StackId1, 1) verified;
    StackPush(StackId1, 2) verified;
    StackPush(StackId1, 3) verified;

    StackPush(StackId2, 4) verified;
    StackPush(StackId2, 3) verified;
    StackPush(StackId2, 2) verified;

    Stack_t* tmp    =      STACKS[StackId1 - 1];
    STACKS[StackId1 - 1] = STACKS[StackId2 - 1];
    STACKS[StackId2 - 1] = tmp;

    StackPush(StackId1, 4) verified;
    StackPush(StackId2, 1) verified;

    StackDtor(StackId1)    verified;
    StackDtor(StackId2)    verified;
}

void MediumHackExample()
{
    StackId_t StackId = STACK_CTOR(MIN_STACK_SIZE);

    StackPush(StackId, 1) verified;

    Stack_t* stack = STACKS[StackId - 1];

    stack->data[stack->capacity - 1]--;

    StackPush(StackId, 1) verified;

    StackDtor(StackId)    verified;
}

void MediumHackExample2()
{
    StackId_t StackId = STACK_CTOR(MIN_STACK_SIZE);

    StackPush(StackId, 1) verified;
    StackPush(StackId, 2) verified;
    StackPush(StackId, 3) verified;
    StackPush(StackId, 4) verified;
    StackPush(StackId, 5) verified;
    StackPush(StackId, 6) verified;
    StackPush(StackId, 7) verified;
    StackPush(StackId, 8) verified;

    Stack_t* stack = STACKS[StackId - 1];

    stack->data[stack->capacity - 2] = 0;

    stack->data[stack->capacity - 1] = 239;

    StackPush(StackId, 9) verified;

    StackDtor(StackId)    verified;
}

void HardHackExample()
{
    StackId_t StackId = STACK_CTOR(MIN_STACK_SIZE);

    StackPush(StackId, 1) verified;

    Stack_t* stack = STACKS[StackId - 1];

    stack->StructHash--;

    StackPush(StackId, 1) verified;

    StackDtor(StackId) verified;
}

void HardHackExample2()
{
    StackId_t StackId = STACK_CTOR(MIN_STACK_SIZE);

    for (int i = 0; i < 64; i++)
    {
        StackPush(StackId, 1) verified;
    }

    for (int i = 0; i < 65; i++)
    {
        printf("%lu\n", StackPop(StackId));
    }

    StackDtor(StackId) verified;
}
