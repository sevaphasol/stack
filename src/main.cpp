#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "stack.h"

uint64_t err = NO_ERROR;

const uint64_t CANARY  = 0xCEBA;

int main()
{
    remove("dump.txt");

    Stack_t stack = {INIT(stack)};

    StackCtor(&stack, MIN_STACK_SIZE);

    StackTest(&stack);

    StackDtor(&stack);

    return 0;
}
