#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "stack.h"

uint64_t err = NO_ERROR;

extern StackReturnCode StackTest (Stack_t* stack);

int main()
{
    Stack_t* stack = StackCtor(MIN_STACK_SIZE);

    StackTest(stack) verified;

    StackDtor(&stack) verified;

    return 0;
}
