#ifndef STACK_H__
#define STACK_H__

struct Stack_t
{
    StackElem_t *data;
    size_t size;
    size_t capacity;
}

int StackInit(Stack_t* stack, int capacity);

int StackPush(Stack_t* stack, StackElem_t value);

int StackPop(Stack_t* stack);

int StackResize(Stack_t* stack);

int StackDestroy(Stack_t* stack);

int StackDump(Stack_t* stack);

int StackValid(Stack_t* stack);

int StackAssert(bool statement);

#endif // STACK_H__
