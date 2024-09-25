#include <stdlib.h>
#include <stdio.h>

#include "error_codes.h"
#include "stack.h"

int main()
{
    Stack_t stack = {};

    StackInit(&stack, 10);

    StackTest(&stack);

    StackPrint(&stack);

    StackDestroy(&stack);
}
