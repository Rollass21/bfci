#include "bfci.h"

extern size_t getMatchingClosing(insObjT insObj);
extern const insSetT insSet[];

int jmp(insObjT insObj, size_t dest);
int LIFOPop(ctxObjT Ctx);
int LIFOPush(ctxObjT Ctx, size_t startindex, size_t endindex);
int LIFOPushNew(ctxObjT Ctx, size_t startindex, size_t endindex);

/* 
 * getMatchingEnd()
 *
 * @insObj - Pointer to structure containing instruction tape
 *
 * @currindex: index of opening bracket in @string in which we want to find a closing bracket
 * @lastindex: last index to be checked, i.e. @string[@lastindex + 1] wont be checked
 * @string: string itself to be checked
 *
 * Returns index of closing bracket paired with the one currently under
 * currindex.
 * If pairing or starting bracket not found, or unvalid arguments, returns 0.
 */
size_t
getMatchingClosing(insObjT insObj){
    if (!insObj){
        return 0; 
    }

    size_t currindex = insObj->index;
    size_t lastindex = insObj->usedlen;
    /* hopefully long int is big enough */
    long int seen = 0;

    /* currindex should point to starting bracket to which we want to find closing */
    if (insSet[insObj->tape[currindex]].opcode != opc_while){
        return 0;
    }

    /* Search for matching brace until end of string */
    do {
        switch (insSet[insObj->tape[currindex]].opcode){
            case opc_while:
                        seen++;
                        break;

            case opc_do:
                        seen--;
                        break;
            default:
                        break;
        }
    } while (seen > 0 && ++currindex <= lastindex);
    
    return (seen != 0) ? 0 : currindex;
}

/* 
 * jmp()
 *
 * @tape - ...
 * @dest - Position/Index on which to set instruciton tape index
 *
 * Used to jump between instructions.
 * On correct jump, returns destination index and sets flag jumped.
 */
int
jmp(insObjT insObj, size_t dest){
    if (!insObj){
        return FAIL;
    }
    
    if (dest >= insObj->usedlen) {
        return PASTBOUNDS;
    }

    insObj->index = dest;
    FLAG_SET_TRUE(insObj->flags, INS_JUMPED);

    return SUCCESS;
} 

int
LIFOPop(ctxObjT Ctx){
    //figure out if you are able to pop something, if not return;
    if (!Ctx->stack->len) {
        return FAIL;
    }
        
    // realloc stack-tape to sizeof(bracket pair) * --stack->len
    size_t memSize = --Ctx->stack->len * sizeof(*Ctx->stack->tape);
    stackCellT* tapeCopy = realloc(Ctx->stack->tape, memSize);

    // error check for null
    if (!tapeCopy && memSize > 0) {
        Ctx->stack->len++;
        return ALLOCFAIL;
    }

    Ctx->stack->tape = tapeCopy;

    return SUCCESS;
}

int
LIFOPush(ctxObjT Ctx,
         size_t startindex,
         size_t endindex){

    if (!isValidCtx(Ctx)) {
        return FAIL;
    }
    
    //TODO add overflow protection, either check defined MAX, or if len > len+1 
    // inc stack->len
    // reallocate stack->tape to sizeof(bracket pair) * stack->len
    size_t memSize = ++Ctx->stack->len * sizeof(*Ctx->stack->tape);
    stackCellT* tapeCopy = realloc(Ctx->stack->tape, memSize);
    
    // error check for null
    if (!tapeCopy) {
        Ctx->stack->len--;
        return ALLOCFAIL;
    }
    // succesfull alloc
    Ctx->stack->tape = tapeCopy;

    // set new pair with provided values
    size_t stackIndex = Ctx->stack->len - 1;
    Ctx->stack->tape[stackIndex].start = startindex;
    Ctx->stack->tape[stackIndex].end = endindex;

    return SUCCESS;
}

int 
LIFOPushNew(ctxObjT Ctx,
            size_t startindex,
            size_t endindex){

    if (!isValidCtx(Ctx)) {
        return FAIL;
    }

    // if there already something is, check if that something == args
    if (Ctx->stack->len) {
        size_t stackIndex = Ctx->stack->len - 1;

        /* if the same bracket pair is already written in stack, do nothing */
        if (startindex == Ctx->stack->tape[stackIndex].start ||
            endindex   == Ctx->stack->tape[stackIndex].end) {
            return SUCCESS;
        }
    }

    return LIFOPush(Ctx, startindex, endindex);
}
