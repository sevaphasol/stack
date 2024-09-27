#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "stack.h"

uint64_t err = NO_ERROR;

int main()
{
    Stack_t stack = {INIT(stack)};

    StackCtor(&stack, MIN_STACK_SIZE);

    StackTest(&stack);

    StackDtor(&stack);

//     Stack_t stack2 = {INIT(stack)};
//
//     StackCtor(&stack2, 10);
//
//     stack2.data = nullptr;
//
//     StackTest(&stack2);
//
//     StackDtor(&stack2);

    return 0;
}
