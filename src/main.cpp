#include <stdlib.h>
#include <stdio.h>

#include "error_codes.h"
#include "stack.h"

int main()
{
    Stack_t stack = {};

    StackInit(&stack, 10);

    for (int i = 0; i < stack.capacity; i++)
    {
        StackPush(&stack, i);
    }

    StackDump(&stack);

    StackTest(&stack);

    StackDestroy(&stack);
}
