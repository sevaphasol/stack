#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "error_codes.h"

#include "stack.h"

int main()
{
    Stack_t stack = {STACK_INIT_PARAM};

    StackCtor(&stack, 10);

    StackTest(&stack);

    StackDtor(&stack);

//     Stack_t stack2 = {STACK_INIT_PARAM};
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
