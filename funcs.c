#include "bfci.h"

static int mvRight(ctxObjT);
static int  mvLeft(ctxObjT);
static int incData(ctxObjT);
static int decData(ctxObjT);
static int outData(ctxObjT);
static int inpData(ctxObjT);
static int loopBeg(ctxObjT);
static int loopEnd(ctxObjT);

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
     .opcode  = DO,
     .command = loopEnd,
    },
    {.name = NULL}
};

/* HELPER FUNCTIONS */

/* 
 * jmp()
 *
 * @tape - ...
 * @dest - Position/Index on which to set instruciton tape index
 *
 * Used to jump between instructions.
 * On correct jump, returns destination index.
 */
static
int jmp(insObjT insObj, size_t dest){
    if (!insObj){
        return FAIL;
    }
    
    if (dest >= insObj->usedlen) {
        fprintf(stderr, "Jump destination to unused instruction tape index!\n");
        return NDFUSAGE;
    }

    insObj->index = dest;

    return SUCCESS;
} 

static
int pushIns(int c, insObjT insObj){
    if (!insObj){
        return FAIL;
    }

    // potential resize
    if (insObj->usedlen + 1 > insObj->len){
        insObj->len += ALLOCJMP;

        size_t memSize = insObj->len * sizeof(*insObj->tape);

        insObjT insObjCopy = insObj;
        insObjCopy->tape = realloc(insObj->tape, memSize);
        if (!insObjCopy->tape){
            return ALLOCFAIL;
        }
        insObj = insObjCopy;
    }

    // adding instruction
    insObj->tape[insObj->usedlen++] = c;

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
    for (int i = 0; insSet[i].name != NULL; i++){
        if (insSet[i].opcode == c)
            return i;
    }

    return -1; 
}

/* INSTRUCTION SET FUNCTIONS */

//TODO
static
int mvRight(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return FAIL;
    }

    /* Loop around */
    if (Ctx->data->index + 1 == Ctx->data->len){
        Ctx->data->index = 0;
        return SUCCESS;
    }
    
    /* Trace furthest index */
    if (Ctx->data->index + 1 >= Ctx->data->usedlen){
        Ctx->data->usedlen = Ctx->data->index + 1;
    }

    /* Casual move right once everything is figured out */
    Ctx->data->index++;

    return SUCCESS;
}

//TODO
static
int mvLeft(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return FAIL;
    }

    /* Loop around */
    if (Ctx->data->index == 0){
        Ctx->data->index = Ctx->data->len;
        return SUCCESS;
    }
    
    /* Casual move left once everything is figured out */
    Ctx->data->index--;

    return SUCCESS;
}

static int 
incData(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return FAIL;
    }

    /* Value loop around */
    if (Ctx->data->tape[Ctx->data->index] == DATAMAX) {
        Ctx->data->tape[Ctx->data->index] = DATAMIN;
        return SUCCESS;
    }

    /* Casual increment */
    Ctx->data->tape[Ctx->data->index]++;

    return SUCCESS;
}

static int 
decData(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return FAIL;
    }

    /* Value loop around */
    if (Ctx->data->tape[Ctx->data->index] == DATAMIN) {
        Ctx->data->tape[Ctx->data->index] = DATAMAX;
        return SUCCESS;
    }

    /* Casual decrement */
    Ctx->data->tape[Ctx->data->index]--;

    return SUCCESS;
}

static int 
outData(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return FAIL;
    }

    putchar(Ctx->data->tape[Ctx->data->index]);
    return SUCCESS;
}

static int 
inpData(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return FAIL;
    }

    Ctx->data->tape[Ctx->data->index] = getchar();
    return SUCCESS;
}


/* TODO 
 * strips
 * getsrc: Encodes instructions from Brainfuck source file into instruction tape
 *         Used to extract instructions from file.
 *  @source: Filename of Brainfuck source file
 *  @tape: Tape structure in which the instructions will be stored
 *  @insset Instruction set array used to identify instructions
 */
static int
getsrc(const char *srcFilePath,
       insObjT insObj){

    FILE *srcFile = fopen(srcFilePath, "r");
    if (srcFile == NULL){
        return FILEFAIL;
    }
    insObj->srcpath = strdup(srcFilePath);

    /* filter out just legit opcodes */
    for (int c = getc(srcFile); c != EOF; c = getc(srcFile)){
        if (isInstruction(c) > -1){
            pushIns(c, insObj);
        }
    }

    fclose(srcFile);
    return SUCCESS;
}

/*  
 *  StrToIns:
 *
 *  @Ctx:
 *  @string: string containing instructions, needs to be NULL terminated!
 */
uchar*
StrToIns(ctxObjT Ctx, const char* string){
    //if is valid and if there are already some instructions saved...
    if (!Ctx || !Ctx->ins || Ctx->ins->tape || !string ){
        return NULL;
    }
    
    /* filter out just legit opcodes */
    for (; *string != '\0'; string++) {
        if (isInstruction((int) *string) > -1){
            pushIns((int) *string, Ctx->ins);
        }
    }

    return Ctx->ins->tape;
}

//TODO
/* 
 * insObjT: initiates Instruction Object
 *
 * @srcFilePath: file containing BF source, if NULL then instructions need to
 *               be set with StrToIns() before execution
 */
static
insObjT initIns(const char* srcFilePath){
    insObjT newInsObj = calloc(1, sizeof(*newInsObj));
    if (!newInsObj) {
        fprintf(stderr, "Error allocating instruction object!\n");
        return NULL;
    }

    newInsObj->srcpath = NULL;
    // source file is not needed, can be later added with StrToIns()
    if (srcFilePath){
        if (getsrc(srcFilePath, newInsObj) == SUCCESS ){
            return newInsObj;
        }
        else{
            fprintf(stderr,"Error obtaining file \'%s\'!\n", srcFilePath);
            return newInsObj;
        }
    }

    return newInsObj;
}

static
dataObjT initData(size_t datalen){
    dataObjT newDataObj = calloc(1, sizeof(*newDataObj));
    if (!newDataObj){
        fprintf(stderr, "Error allocating data object!\n");
        return NULL;
    }
    newDataObj->len = datalen;

    newDataObj->tape = calloc(newDataObj->len, sizeof(*newDataObj->tape));
    if (!newDataObj->tape){
        free(newDataObj);
        fprintf(stderr, "Error allocating data tape of length %zu!\n", datalen);
        return NULL;
    }
    newDataObj->usedlen = 1;

    return newDataObj;
}

static
stackObjT initStack(){
    stackObjT newStackObj = calloc(1, sizeof(*newStackObj));
    if (!newStackObj){
        fprintf(stderr, "Error allocating stack object!\n");
        return NULL;
    }
    newStackObj->tape = NULL;

    return newStackObj;
} 

/*  TODO 
 *  initCtx: Create context object needed for proper interpretation of BF
 *
 */
ctxObjT
initCtx(const char* srcFilePath,
        size_t datalen,
        uint flags){

    /* creating context object */
    ctxObjT newCtx = malloc(sizeof(*newCtx));
    if (!newCtx){ return NULL; }
    /* creating instruction object */
    newCtx->ins = initIns(srcFilePath);
    if (!newCtx->ins){ goto insCleanup; } 
    /* checking if file was loaded correctly */
    if (srcFilePath && !newCtx->ins->srcpath){ goto insCleanup; }
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
 * freeCtx: Frees memory allocated for tapes
 *
 * @tape: Tapes, initialized by initTapes() & initInsSet() to cleanup
 */
void freeCtx(ctxObjT Ctx){
    /* if already initialized or not cleared */
    if (Ctx){
        /* clean instruction object */
        free(Ctx->ins->tape);
        Ctx->ins->tape = NULL;
        free(Ctx->ins->srcpath);
        Ctx->ins->srcpath = NULL;
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


void printHelp(){
    const char* helptext = "\
Name:      BrainFuck C Interpreter\n\
Usage:     bfci [-options ...]\n\
Options:\n\
           -i <string>     interpret given string as if it was a file\n\
           -h              display this screen\n\
           -t <filepath>   interpret given BF source file target\n";
           //-f <flags>      set flags based on decimal <flags> value\n";

    printf("%s", helptext);
}

/* DIAGNOSTICS TOOLS */
static
void printIns(ctxObjT Ctx){
    if (!Ctx || !Ctx->ins){
        return;
    }

    for(size_t i = 0; i < Ctx->ins->usedlen; i++){
        printf("%c", Ctx->ins->tape[i]);
    }
    printf("\n");

    return;
}

static
void printData(ctxObjT Ctx){
    if (!Ctx || !Ctx->data){
        return;
    }

    for(size_t i = 0; i < Ctx->data->usedlen; i++){
        printf("[%3d] ", Ctx->data->tape[i]);
    }

    return;
}

static
void printStack(ctxObjT Ctx) {
    if (!Ctx || !Ctx->stack){
        return;
    }

    for(size_t i = 0; i < Ctx->stack->len; i++){
        printf("start[%zu] end[%zu]\n",Ctx->stack->tape[i].start, Ctx->stack->tape[i].end);
    }

    return;
}

static
void printFlags(ctxObjT Ctx){
    if (!Ctx || !Ctx->stack){
        return;
    }
    uint flags = Ctx->flags;

/*  if (Ctx->flags | ) printf("\n");*/
    printf("value: %u\n", flags);

    printf("%u CTX_RUNNING\n",          (flags & CTX_RUNNING)          ? 1 : 0); 
    printf("%u CTX_COMPLETED\n",        (flags & CTX_COMPLETED)        ? 1 : 0);
    printf("%u DATA_PENDING_OUT\n",     (flags & DATA_PENDING_OUT)     ? 1 : 0);
    printf("%u DATA_PENDING_IN\n",      (flags & DATA_PENDING_IN)      ? 1 : 0);
    printf("%u DATA_ALLOW_LOOPED\n",    (flags & DATA_ALLOW_LOOPED)    ? 1 : 0);
    printf("%u DATA_ALLOW_OVERFLOW\n",  (flags & DATA_ALLOW_OVERFLOW)  ? 1 : 0);
    printf("%u DATA_ALLOW_UNDERFLOW\n", (flags & DATA_ALLOW_UNDERFLOW) ? 1 : 0);
    printf("%u DATA_DYNAMIC_GROW\n",    (flags & DATA_DYNAMIC_GROW)    ? 1 : 0);
    printf("%u PRINT_DIAGNOSTICS\n",    (flags & PRINT_DIAGNOSTICS)    ? 1 : 0);
    
    return;
}

void printCtx(ctxObjT Ctx){
    if(Ctx){
        printf("\n");
        printf("--Instruction Tape--\n");
        printf("ins->len = %zu\n", Ctx->ins->len);
        printf("ins->index = %zu\n", Ctx->ins->index);
        printf("ins->usedlen = %zu\n", Ctx->ins->usedlen);
        printf("ins->srcpath = \'%s\'\n", Ctx->ins->srcpath);
        printIns(Ctx);

        printf("\n");
        printf("--Data Tape--\n");
        printf("data->len = %zu\n", Ctx->data->len);
        printf("data->index = %zu\n", Ctx->data->index);
        printf("data->usedlen = %zu\n", Ctx->data->usedlen);
        printData(Ctx);
        printf("\n");

        printf("\n");
        printf("--Stack Tape--\n");
        printf("stack->len = %zu\n", Ctx->stack->len);
        printStack(Ctx);

        printf("\n");
        printf("--Flags--\n");
        printFlags(Ctx);
    }
}


// XXX ins->usedlen is not really the described 'last used index number'
// TODO fix ins->usedlen incrementing
static
bool isBalanced(insObjT insObj){
    if (!insObj || !insObj->tape || !insObj->usedlen) {
        return 0;
    }

    size_t currindex = insObj->index;
    long int count = 0;

    /* if count gets below zero, closing bracket was used without pair opening bracket */
    while (count >= 0 && ++currindex < insObj->usedlen){
        switch (insObj->tape[currindex]){
            case WHILE: count++;
                        break;

            case    DO: count--;
                        break;
        }
    }

    /* balanced brackets will set count to 0, more opening brackets would leave count>0 */
    return (count) ? FALSE : TRUE;
}
/* 
 * getMatchingEnd()
 *
 * @insObj - Pointer to structure containing instruction tape
 *
 * Returns index of closing bracket paired with the one currently under
 * instruction tape index.
 * If pairing or starting bracket not found, or unvalid insObject, returns 0.
 */
static size_t
getMatchingClosing(insObjT insObj){
    if (!insObj || !insObj->tape || !insObj->usedlen) {
        return 0;
    }

    size_t currindex = insObj->index;
    /* maybe too small? time will tell */
    size_t seen = 0;

    /* Search for matching brace until end of instruction tape */
    do {
        switch (insObj->tape[currindex]){
            case WHILE: seen++;
                        break;

            case    DO: seen--;
                        break;
        }
    } while (seen && ++currindex < insObj->usedlen);
    
    return (!seen) ? currindex : 0;
}

static int
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

/*
 * TODO - do it so there doesnt have to be and if() whether 
 * Doesnt push if the arg values are equal to last stored values in stack
 * Doesn push when..
 */
static int 
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

static int
LIFOPop(ctxObjT Ctx){

    if (!isValidCtx(Ctx)) {
        return FAIL;
    }

    //figure out if you are able to pop something, if not return;
    if (!Ctx->stack->len) {
        return FAIL;
    }
        
    // realloc stack-tape to sizeof(bracket pair) * --stack->len
    size_t memSize = --Ctx->stack->len * sizeof(*Ctx->stack->tape);
    stackCellT* tapeCopy = realloc(Ctx->stack->tape, memSize);

    // error check for null
    if (!tapeCopy) {
        Ctx->stack->len++;
        return ALLOCFAIL;             
    }

    return SUCCESS;
}

//TODO
static int 
loopBeg(ctxObjT Ctx){
    if(!isValidCtx(Ctx)){
        return FAIL;
    }
    size_t startindex = Ctx->ins->index;
    size_t endindex   = getMatchingClosing(Ctx->ins);
    if (!endindex) {
        return NOLOOPEND;
    }

    /* TODO push NEW indexes */
    LIFOPushNew(Ctx, startindex, endindex);

    /* JUMP RULE */
    /* Jump to closing bracket when data==0 */
    if (Ctx->data->tape[Ctx->data->index] == 0) {
        jmp(Ctx->ins, Ctx->stack->tape[Ctx->stack->len - 1].end); 
    }

    /* if jump rule is FALSE, just let programm continue */
    return SUCCESS;
}

//TODO
static int 
loopEnd(ctxObjT Ctx){
    if(!isValidCtx(Ctx)){
        return FAIL;
    }

    /* JUMP RULE */
    /* Jump back to stored opening bracket when data!=0 */
    if (Ctx->data->tape[Ctx->data->index] != 0) {
        if (!Ctx->stack->len) {
            return NOLOOPSTART;
        }
        /* if there is openings bracket to jump to, do jmp() */
        jmp(Ctx->ins, Ctx->stack->tape[Ctx->stack->len - 1].start);         
        return SUCCESS;
    }

    LIFOPop(Ctx);
    /* if jump rule is FALSE, just let programm continue */
    return SUCCESS;
}

/* TODO
 * execute: Takes care of executing the right instruction
 *          Executes one instruction at Ctx->ins->index
 *
 * @tape: Tape structure of which one instruction tape will be executed
 */
static
int execute(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return FAIL;
    }

    uchar currIns = Ctx->ins->tape[Ctx->ins->index];
    int currCMDIndex = isInstruction(currIns);
    cmd currCMD = insSet[currCMDIndex].command;

    return currCMD(Ctx);
}

int
interpret(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return FAIL; 
    }
    
    // before interpretation
    /* syntax checks */
    if(!isBalanced(Ctx->ins)){
        fprintf(stderr, "Unbalanced brackets!\n");    
        return FAIL;
    }
    BIT_SET_TRUE(Ctx->flags, CTX_RUNNING);

    // interpretation
    for(Ctx->ins->index = 0; Ctx->ins->index < Ctx->ins->usedlen; Ctx->ins->index++){
        execute(Ctx);
    }
    
    // after interpretation
    BIT_SET_TRUE(Ctx->flags, CTX_COMPLETED);
    BIT_SET_FALSE(Ctx->flags, CTX_RUNNING);

    if (Ctx->flags & PRINT_DIAGNOSTICS)
        printCtx(Ctx);

    return SUCCESS;
}

