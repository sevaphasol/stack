#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "stack.h"

StackReturnCode StackTest();

extern Stack_t* STACKS[MAX_STACK_AMOUNT];

void* PthrPush(void* args);

void* PthrDel(void* args);

StackReturnCode StackTest()
{
    StackId_t StackId = StackCtor(MIN_STACK_SIZE);

    pthread_t threads[2];

    pthread_create(&threads[0], NULL, PthrPush,  &StackId);

    pthread_create(&threads[1], NULL, PthrDel,   &StackId);

    pthread_join(threads[0], NULL);

    pthread_join(threads[1], NULL);


//     FILE* UnitTestFile = fopen("unit_test", "wb");
//
//     if (!UnitTestFile)
//     {
//         err += INVALID_FILE_POINTER;
//
//         return FAILED;
//     }
//
//     srand((unsigned int) time(NULL));
//
//     int r = rand() % 100;
//
//     for (size_t i = 0; i < 32; i++)
//     {
//         fprintf(UnitTestFile, "%d\n", r);
//
//         StackPush(StackId, (int) r);
//
//         r = rand() % 100;
//     }
//
//     for (size_t i = 32; i > 0; i--)
//     {
//         StackPop(StackId);
//     }
//
//     StackDtor(StackId) verified;
//
//     return EXECUTED;
    return EXECUTED;
}

void* PthrPush(void* args)
{
    StackId_t id = *((StackId_t*) args);

    StackPush(id, 1);
    StackPush(id, 1);

    StackPush(id, 1);

    StackPush(id, 1);

    StackPush(id, 1);

    StackPush(id, 1);

    StackPush(id, 1);

    pthread_exit(NULL);
}

void* PthrDel(void* args)
{
    StackId_t id = *((StackId_t*) args);

    printf("%d\n", id);

    // *((StackElem_t*)((char*) STACKS[id - 1]->data + 3*sizeof(StackElem_t))) = 0;;

    StackPush(id, 12);

    StackPush(id, 1488);

    StackPush(id, 1123);

    StackPush(id, 14);

    StackPush(id, 1223);

    StackPush(id, 16);

    StackPush(id, 188);

    pthread_exit(NULL);
}
