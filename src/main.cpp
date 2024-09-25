#include "error_codes.h"

typedef int StackElem_t;
#include "stack.h"

int main()
{
    Stack_t stack = {};

    StackInit(&stack);

    StackTest(&stack);

    StackDestroy(&stack);
}
