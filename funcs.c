#include "bfci.h"

static int mvRight(ctxObjT, uint);
static int  mvLeft(ctxObjT, uint);
static int incData(ctxObjT, uint);
static int decData(ctxObjT, uint);
static int outData(ctxObjT, uint);
static int inpData(ctxObjT, uint);
static int loopBeg(ctxObjT, uint);
static int loopEnd(ctxObjT, uint);

/* Instruction set */
static const insSetT insSet[] = {
    {.name    = "Move right",
     .opcode  = MVR,
     .command = mvRight,
    },

    {.name    = "Move left",
     .opcode  = MVL,
     .command = mvLeft,
    },
    
    {.name    = "Data increment",
     .opcode  = INC,
     .command = incData,
    },

    {.name    = "Data decrement",
     .opcode  = DEC,
     .command = decData,
    },

    {.name    = "Standard output",
     .opcode  = STDO,
     .command = outData,
    },

    {.name    = "Standard input",
     .opcode  = STDI,
     .command = inpData,
    },

    {.name    = "While loop begin",
     .opcode  = WHILE,
     .command = loopBeg,
    },

    {.name    = "While loop end",
     .opcode  = END,
     .command = loopEnd,
    },
    {.name = NULL}
};

/* HELPER FUNCTIONS */

/* TODO
 * jmp()
 *
 * @tape - ...
 * @dest - Position/Index on which to set instruciton tape index
 *
 * Used to jump between instructions.
 * On correct jump, returns destination index.
 */
static
unsigned int jmp(insObjT insObj, size_t dest){
    if (insObj){
        return FAIL;
    }
    
    if (dest < insObj->usedlen)
        insObj->index = dest;

    return dest;
} 

static
int pushIns(int c, insObjT insObj){
    if (!insObj){
        return FAIL;
    }

    if (insObj->index + 1 > insObj->len){
        insObj->len += ALLOCJMP;

        size_t memSize = insObj->len * sizeof(*insObj->tape);

        insObjT insObjCopy = insObj;
        insObjCopy->tape = realloc(insObj->tape, memSize);
        if (!insObjCopy->tape){
            return ALLOCFAIL;
        }
        insObj = insObjCopy;
    }
    insObj->tape[insObj->index++] = c;
    insObj->usedlen = insObj->index;

    return SUCCESS;
}

/* 
 * isValidCtx: Check whether given object is properly initialized
 */
static bool
isValidCtx(ctxObjT Ctx){
    if (!Ctx || !Ctx->data || !Ctx->ins || !Ctx->stack)
        return FALSE;

    return TRUE;
}

/*  isInstruction: Checks if given input c is valid opcode in InsSet object.
 *                 If found, returns index of instruction, otherwise -1.
 *
 *  @c: Possible instruction opcode
 */
static
int isInstruction(int c) {
    for(int i = 0; insSet[i].name != NULL; i++){
        if (insSet[i].opcode == c)
            return i;
    }

    return FALSE; 
}

/* INSTRUCTION SET FUNCTIONS */

static
int mvRight(ctxObjT Ctx,
            __attribute__((unused)) uint flag){

    if (isValidCtx(Ctx) != TRUE){
        return FAIL;
    }

    /* Loop around */
    if (Ctx->data->index + 1 > Ctx->data->len){
        Ctx->data->index = 0;
        return SUCCESS;
    }
    /* Casual move right */
    ++Ctx->data->index;

    return SUCCESS;
}

static
int mvLeft(ctxObjT Ctx,
           __attribute__((unused)) uint flag){

    if (isValidCtx(Ctx) != TRUE){
        return FAIL;
    }

    /* Loop around */
    if (Ctx->data->index == 0){
        Ctx->data->index = Ctx->data->len;
        return SUCCESS;
    }
    /* Casual move left */
    --Ctx->data->index;

    return SUCCESS;
}

static int 
incData(ctxObjT Ctx,
        __attribute__ ((unused)) uint flag){

    if (isValidCtx(Ctx) != TRUE){
        return FAIL;
    }
    /* Value loop around */
    if (Ctx->data->tape[Ctx->data->index] == DATAMAX) {
        Ctx->data->tape[Ctx->data->index] = DATAMIN;
    }
    /* Casual increment */
    Ctx->data->tape[Ctx->data->index]++;

    return SUCCESS;
}

static int 
decData(ctxObjT Ctx,
        __attribute__ ((unused)) uint flag){

    if (isValidCtx(Ctx) != TRUE){
        return FAIL;
    }
    /* Value loop around */
    if (Ctx->data->tape[Ctx->data->index] == DATAMIN) {
        Ctx->data->tape[Ctx->data->index] = DATAMAX;
    }
    /* Casual decrement */
    Ctx->data->tape[Ctx->data->index]--;

    return SUCCESS;
}

static int 
outData(ctxObjT Ctx,
        __attribute__((unused)) uint flag){

    if (isValidCtx(Ctx) != TRUE){
        return FAIL;
    }

    putchar(Ctx->data->tape[Ctx->data->index]);
    return SUCCESS;
}

static int 
inpData(ctxObjT Ctx,
        __attribute__((unused)) uint flag){

    if (isValidCtx(Ctx) != TRUE){
        return FAIL;
    }

    Ctx->data->tape[Ctx->data->index] = getchar();
    return SUCCESS;
}


/* TODO
 * strips
 * getsrc: Encodes instructions from Brainfuck source file into instruction tape
 * 
 *  @source: Filename of Brainfuck source file
 *  @tape: Tape structure in which the instructions will be stored
 *  @insset Instruction set array used to identify instructions
 */
static int
getsrc(const char *srcFileName,
       insObjT insObj){

    FILE *srcFile = fopen(srcFileName, "r");
    if (srcFile == NULL){
        return FILEFAIL;
    }
    insObj->srcdest = strdup(srcFileName);

    for(int c = getc(srcFile); c != EOF; c = getc(srcFile)){
        if (isInstruction(c) > -1){
            pushIns(c, insObj);
        }
    }

    insObj->usedlen = insObj->index;
    insObj->index = 0;

    fclose(srcFile);
    return SUCCESS;
}

//TODO
static
insObjT initIns(const char* srcFileName){
    insObjT newInsObj = calloc(1, sizeof(*newInsObj));
    if (!newInsObj)
        return NULL;

    // source file is not needed, can be added lately with strtoins()
    if (srcFileName){
        if (getsrc(srcFileName, newInsObj) == SUCCESS ){
            return newInsObj;
        }
        else{
            fprintf(stderr,"Error obtaining file \'%s\'!\n", srcFileName);
            free(newInsObj);
            return NULL;
        }
    }

    return newInsObj;
}

static
dataObjT initData(size_t datalen){
    dataObjT newDataObj = malloc(sizeof(*newDataObj));
    if (!newDataObj){
        fprintf(stderr, "Error allocating data object!\n");
        return NULL;
    }
    newDataObj->len = datalen;
    newDataObj->usedlen = 0;
    newDataObj->index = 0;
    newDataObj->tape = calloc(newDataObj->len, sizeof(*newDataObj->tape));
    if (!newDataObj->tape){
        free(newDataObj);
        fprintf(stderr, "Error allocating data tape of length %zu!\n", datalen);
        return NULL;
    }

    return newDataObj;
}

static
stackObjT initStack(){
    stackObjT newStackObj = malloc(sizeof(*newStackObj));
    if (!newStackObj){
        fprintf(stderr, "Error allocating stack object!\n");
        return NULL;
    }
    newStackObj->tape = NULL;
    newStackObj->len = 0;

    return newStackObj;
} 

/*  TODO 
 *  initCtx: Create context object needed for proper interpretation of BF
 *
 */
ctxObjT
initCtx(const char* srcFileName,
        size_t datalen,
        uint flags){
    /* creating context object */
    ctxObjT newCtx = malloc(sizeof(*newCtx));
    if (!newCtx){ return NULL; }
    /* creating instruction object */
    newCtx->ins = initIns(srcFileName);
    if (!newCtx->ins){ goto insCleanup; } 
    /* creating data object */
    newCtx->data = initData(datalen);
    if (!newCtx->data){ goto dataCleanup; }
    /* creating stack object */
    newCtx->stack = initStack();
    if (!newCtx->stack){ goto stackCleanup; }

    newCtx->flags = flags;

    return newCtx;

 stackCleanup:
    free(newCtx->data);
 dataCleanup:
    free(newCtx->ins);
 insCleanup:
    free(newCtx);
    return NULL;
}


/*TODO
 * clearCtx: Frees memory allocated for tapes
 *
 * @tape: Tapes, initialized by initTapes() & initInsSet() to cleanup
 */
void clearCtx(ctxObjT Ctx){
    /* if already initialized or not cleared */
    if (Ctx){
        /* clean instruction object */
        free(Ctx->ins->tape);
        Ctx->ins->tape = NULL;
        free(Ctx->ins->srcdest);
        Ctx->ins->srcdest = NULL;
        free(Ctx->ins);
        Ctx->ins = NULL;

        /* clean data object */
        free(Ctx->data->tape);
        Ctx->data->tape = NULL;
        free(Ctx->data);
        Ctx->data = NULL;

        /* clean stack object */
        free(Ctx->stack->tape);
        Ctx->stack->tape = NULL;
        free(Ctx->stack);
        Ctx->stack = NULL;

        /* clean context object */
        free(Ctx);
        Ctx = NULL;
    }
}

/* DIAGNOSTICS TOOLS */
static
int printIns(ctxObjT Ctx){
    if (!Ctx || !Ctx->ins){
        return FAIL;
    }

    for(size_t i = 0; i < Ctx->ins->usedlen; i++){
        printf("%c", Ctx->ins->tape[i]);
    }
    printf("\n");

    return SUCCESS;
}

static
int printData(ctxObjT Ctx){
    if (!Ctx || !Ctx->data){
        return SUCCESS;
    }

    for(size_t i = 0; i < Ctx->data->usedlen; i++){
        printf("[%3d] ", Ctx->data->tape[i]);
    }

    return SUCCESS;
}

static
int printStack(ctxObjT Ctx) {
    if (!Ctx || !Ctx->stack){
        return SUCCESS;
    }

    for(size_t i = 0; i < Ctx->stack->len; i++){
        printf("start[%zu] end[%zu]\n",Ctx->stack->tape[i].start, Ctx->stack->tape[i].end);
    }

    return SUCCESS;
}

void printCtx(ctxObjT Ctx){
    if(Ctx){
        printf("\n\n");
        printf("--Instruction Tape--\n");
        printf("ins->len = %zu\n", Ctx->ins->len);
        printf("ins->index = %zu\n", Ctx->ins->index);
        printf("ins->usedlen = %zu\n", Ctx->ins->usedlen);
        printf("ins->srcdest = \'%s\'\n", Ctx->ins->srcdest);
        printIns(Ctx);

        printf("\n");
        printf("--Data Tape--\n");
        printf("data->len = %zu\n", Ctx->data->len);
        printf("data->index = %zu\n", Ctx->data->index);
        printData(Ctx);

        printf("\n");
        printf("--Stack tape--\n");
        printf("stack->len = %zu\n", Ctx->stack->len);
        printStack(Ctx);
    }
}



/* TODO
 * getMatchingEnd()
 *
 *  @tape - Pointer to structure containing initialized data & instruction tapes
 *
 *  Returns index of closing bracket paired with the one currently under
 *  instruction tape index.
 *  If pairing bracket not found, returns 0.
 */
static size_t
getMatchingEnd(insObjT insObj){
    if (insObj || insObj->tape) {
        return FAIL;
    }

    int count = 1;
    unsigned int currindex = insObj->index;
    
    /* Search for matching brace until end of instruction tape */
    while (count != 0 && ++currindex < insObj->usedlen ){
        switch (insObj->tape[currindex]){
            case WHILE: count++;
                        break;

            case   END: count--;
                        break;
        }
    }

    if (count) {
        return 0;
    }

    return currindex;
}


//TODO
static int 
loopBeg(ctxObjT Ctx,
        uint flag){

    return SUCCESS;
}

//TODO
static int 
loopEnd(ctxObjT Ctx,
        uint flag){

    return SUCCESS;
}

/* 
 * loop()
 *
 * @tape - ...
 * @stack - Pointer to stack that corresponds to given @tape
 *
 */
/*
static
int loop(TapesPtr tape, stackObjT stack){
    if (tape == NULL || stack == NULL){
        return FAIL;
    }
    
    int ret = SUCCESS;

    InsTapePtr insTp = tape->ins;
    DataTapePtr dataTp = tape->data;
    
    switch(insTp->tape[insTp->index]){
        case WHILE:;
            unsigned int start = insTp->index;
            unsigned int end = getMatchingEnd(tape);
            if (end == 0) {
                //freeStack(stack);
                return NOLOOPEND;
            }
            SPush(stack, start, end);
            
            // If current data == 0, jump to END
            if (dataTp->tape[dataTp->index] == 0) {
                jmp(tape, stack->tape[stack->len - 1].end);
                SPop(stack);
            }

            break;

        case END:
            // If stack is empty
            if (stack->len == 0) {
                return NOLOOPSTART;    
            }
            // If current data != 0, jump to WHILE start
            if (dataTp->tape[dataTp->index] != 0) {
                jmp(tape, stack->tape[stack->len - 1].start);
            }

            break;

        default:
            ret = NDFINS;
    }

    return ret;
}

*/
/* TODO
 * execute: Takes care of executing the right instruction
 *          - for now the values are hardcodec, in future there will be function
 *            array containing each one of the functions
 *
 * @tape: Tape structure of which one instruction tape will be executed
 */
static
int execute(ctxObjT Ctx){
    /*
    if (tape == NULL){
        return FAIL; 
    }
    int ins = tape->ins->tape[tape->ins->index];

    if (ins==   INC || ins==   DEC) return changeval(tape);
    if (ins== STD_O || ins== STD_I) return        IO(tape);
    if (ins==  MV_R || ins==  MV_L) return      move(tape);
    if (ins== WHILE || ins==   END) return      loop(tape, stack);
    */

    return NDFINS;
}

//TODO
/*int run(TapesPtr tape){
    if (tape == NULL){
        return FAIL; }

    stackObjT stack = initStack();
    if (stack == NULL)
        return ALLOCFAIL;

    InsTapePtr insTp = tape->ins;
    int ret = SUCCESS;

    for(insTp->index = 0; insTp->index < insTp->usedlen; insTp->index++){
        if ((ret = execute(tape, stack)) != SUCCESS)
            return ret;
    }
    return ret;
}
*/

static int
SPush(stackObjT stack, size_t startindex, size_t endindex){
    if (stack == NULL)
        stack = initStack();

    if (stack->tape == NULL){
        stack->len = 1;
        stack->tape = malloc(stack->len * sizeof(*stack->tape));
        if (stack->tape == NULL) {
            stack->len = 0;        
            return FAIL;
        }
    }

    if (startindex == stack->tape[stack->len - 1].start
        || endindex == stack->tape[stack->len - 1].end)
        return TRUE;

    stack->tape = realloc(stack->tape, ++stack->len * sizeof(*stack->tape));
    if (stack->tape == NULL) {
        //freeStack(stack);
        return FAIL;
    }

    stack->tape[stack->len - 1].start = startindex;
    stack->tape[stack->len - 1].end = endindex;
    
    return SUCCESS;
}

static int
SPop(stackObjT stack){
    //little too explicit i know
    if (stack == NULL && stack->len == 0 && stack->tape == NULL) {
        return FAIL;
    }
    
    stack->tape = realloc(stack->tape, --stack->len * sizeof(*stack->tape));
    if (stack->tape == NULL) {
        // when at 0 byte size, realloc already frees the array
        //freeStack(stack);
        return FAIL;
    }

    return SUCCESS;

}
