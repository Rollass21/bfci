#include "stack.h"

StackPtr
initStack(){
    StackPtr retPtr = malloc(sizeof(*retPtr));
    if(retPtr == NULL)
        return NULL;

    retPtr->array = NULL;
    retPtr->len = 0;

    return retPtr;
} 

int
SPush(StackPtr stack, unsigned int startindex, unsigned int endindex){
    if(stack == NULL)
        stack = initStack();

    if(stack->array == NULL){
        stack->len = 1;
        stack->array = malloc(stack->len * sizeof(*stack->array));
        if (stack->array == NULL) {
            stack->len = 0;        
            return FAIL;
        }
    }

    if (startindex == stack->array[stack->len - 1].start
        || endindex == stack->array[stack->len - 1].end)
        return TRUE;

    stack->array = realloc(stack->array, ++stack->len * sizeof(*stack->array));
    if (stack->array == NULL) {
        freeStack(stack);
        return FAIL;
    }

    stack->array[stack->len - 1].start = startindex;
    stack->array[stack->len - 1].end = endindex;
    
    return SUCCESS;
}

int
SPop(StackPtr stack){
    //little too explicit i know
    if (stack == NULL && stack->len == 0 && stack->array == NULL) {
        return FAIL;
    }
    
    stack->array = realloc(stack->array, --stack->len * sizeof(*stack->array));
    if (stack->array == NULL) {
        // when at 0 byte size, realloc already frees the array
        freeStack(stack);
        return FAIL;
    }

    return SUCCESS;

}

void
freeStack(StackPtr stack){
    free(stack->array);
    stack->array = NULL;

    free(stack);
    stack = NULL;
    
    return;
}

