#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "stack.h"

uint64_t err = NO_ERROR;

int main()
{
    remove("dump.txt");

    Stack_t stack = {INIT(stack)};

    StackCtor(&stack, MIN_STACK_SIZE) verified;

    StackTest(&stack) verified;

    StackDtor(&stack) verified;

    return 0;
}
