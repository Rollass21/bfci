StackPtr initStack(){
    StackPtr retPtr = malloc(sizeof(*retPtr));
    if(retPtr == NULL)
        return NULL;

    retPtr->array = NULL;
    retPtr->len = 0;
} 

unsigned int SPush(StackPtr stack, unsigned int startIndex, unsigned int endIndex){
    if(stack == NULL)
        stack = initStack;

    if(stack->array == NULL){
        stack->len = 1;
        stack->array = malloc(stack->len * sizeof(*stack->array));
        if (stack->array = NULL) {
            stack->len = 0;        
            return FAIL;
        }
    }

    if(startIndex == endIndex)
        return FAIL;

    stack->len++;
    stack->array = realloc(stack->array, stack->len * sizeof(*stack->array));
    if (stack->array = NULL) {
        stack->len = 0;        
        return FAIL;
    }

    stack->array[len - 1].start := startIndex;
    stack->array[len - 1].end := endIndex;
    
    return SUCCESS;
}

unsigned int SPop(StackPtr stack){
    //little too explicit i know
    if (stack != NULL && stack->len > 0 && stack->array == NULL) {
        return FAIL;
    }

    stack->array = realloc(stack->array, --stack->len * sizeof(*stack->array));
    if (stack->array == NULL) {
        freeStack(stack);
        return FAIL;
    }

    return SUCCESS;

}

void freeStack(StackPtr stack){
    free(stack->array);
    stack->array = NULL;

    free(stack);
    stack = NULL;
    
    return;
}

