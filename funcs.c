#include "bfci.h"

static int mvRight(ctxObjT);
static int mvLeft (ctxObjT);
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
void
handleArgs(int argc,
           char** argv,
           att(unused) uint* flags,
           uint* filecount,
           bool* givenstring,
           char* source,
           char* string){

    int option;
    *givenstring = FALSE;
    *filecount = 0;

    if (argc < 2) {
        printHelp();
        return;
    }

    while ((option = getopt(argc, argv, "ht:i:")) != -1) {
        switch (option){
        case 'h':
            printHelp();
            return;

        case 't':
            if (*givenstring) {
                fprintf(stderr, "Using both '%s' and '%s' isn't currently possible!\n", "-i", "-t");
            }
            *filecount++;
            source = optarg;
            break;

        case 'i':
            if(*filecount > 0) {
                fprintf(stderr, "Using both '%s' and '%s' isn't currently possible!\n", "-i", "-t");
            }
            *givenstring = TRUE;
            string = optarg;
            break;

        case '?':
            switch(optopt){
            case 't':
                fprintf(stderr, "Option '-%c' requires %s argument!\n", optopt, "filepath");

            case 'i':
                fprintf(stderr, "Option '-%c' requires %s argument!\n", optopt , "string");

            default:
                fprintf(stderr, "Unknown option '-%c'!\n", optopt);
            }
        }
    }

    return;
}
*/

/* 
 * jmp()
 *
 * @tape - ...
 * @dest - Position/Index on which to set instruciton tape index
 *
 * Used to jump between instructions.
 * On correct jump, returns destination index and sets flag jumped.
 */
static
int jmp(insObjT insObj, size_t dest){
    if (!insObj){
        return FAIL;
    }
    
    if (dest >= insObj->usedlen) {
        return PASTBOUNDS;
    }

    insObj->index = dest;
    insObj->jumped = true;

    return SUCCESS;
} 

static
int pushIns(uchar ins, insObjT insObj){
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
    insObj->tape[insObj->usedlen++] = ins;

    return SUCCESS;
}

/* 
 * isValidCtx: Check whether given object is properly initialized
 */
static bool
isValidCtx(ctxObjT Ctx){
    if (!Ctx || !Ctx->data || !Ctx->ins || !Ctx->stack)
        return false;

    return true;
}

/*  isInstruction: Checks if given input c is valid opcode in InsSet object.
 *                 If found, returns index of instruction, otherwise -1.
 *
 *  @c: Possible instruction opcode
 */
static
int isInstruction(char c) {
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
    if (Ctx->data->index + 1 == Ctx->data->usedlen){
        Ctx->data->usedlen++;
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
        Ctx->data->index = Ctx->data->len - 1;
        Ctx->data->usedlen = Ctx->data->len;
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

static int
fromFile(void* insObj, void* arg){
    int index;
    insObjT ins = insObj;
    if (!ins || !arg)
        return FAIL;

    char* srcpath = (char*) arg;
    FILE *srcfile = fopen(srcpath, "r");    
    if (!srcfile)
        return FILEFAIL;

    ins->srcpath = strdup(srcpath);
    FLAG_SET(ins->flags, INS_FROM_FILE, false);

    /* filter out just legit opcodes */
    // add RLE support
    for (int c = getc(srcfile); c != EOF; c = getc(srcfile)){
        if ((index = isInstruction(c)) > -1){
            pushIns(index, ins);
        }
    }

    fclose(srcfile);
    return SUCCESS;
}

static int
fromString(void* insObj, void* arg){
    int index;            
    insObjT ins = insObj;
    if (!ins || !arg )
        return FAIL;

    if (ins->flags & INS_FROM_FILE)
        return NDFUSAGE;

    FLAG_SET(ins->flags, INS_FROM_STRING, true);

    char* string = (char*) arg;
    while(*string != '\0'){
        if ((index = isInstruction(*string)) > -1){
            pushIns(index, ins);
        }

        /* next char */
        string++;
    }

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
getsrc(insObjT insObj,
       const char* srcpath,
       const char* srcstring){

    const char* src;
    method fromWhat;
    /* and, because we cant obtain source from both methods atst and we dont have to really get the source */
    if (srcpath && srcstring) {
        FLAG_SET(insObj->flags, INS_FROM_STRING | INS_FROM_FILE, false)
        return NDFUSAGE;
    }

    /* for when the file contains bf source*/
    if (srcpath){
        fromWhat = fromFile;
        src = srcpath;
    }

    /* for when the string contains bf source*/
    if (srcstring){
        fromWhat = fromString;
        src = srcstring;
    }

    return fromWhat((void*) insObj, (void*) src);
}


/*  
 *  StrToIns:
 *
 *  @Ctx:
 *  @string: string containing instructions
 */
uchar*
StrToIns(ctxObjT Ctx, const char* string){
    //if is valid and if there are already some instructions saved...
    if (!Ctx || !Ctx->ins || Ctx->ins->tape || !string ){
        return NULL;
    }
    
    /* filter out just legit opcodes */
    for (; *string != '\0'; string++) {
        if (isInstruction(*string) > -1){
            pushIns(*string, Ctx->ins);
        }
    }

    return Ctx->ins->tape;
}

/* 
 * insObjT: initiates Instruction Object
 *
 * @srcFilePath: file containing BF source, if NULL then instructions need to
 *               be set with StrToIns() before execution
 */
static
insObjT initIns(const char* srcpath,
                const char* srcstring){
    insObjT newInsObj = calloc(1, sizeof(*newInsObj));
    if (!newInsObj) {
        fprintf(stderr, cERR "Error allocating instruction object! \n" cNO);
        return NULL;
    }

    newInsObj->srcpath = NULL;
    newInsObj->flags = 0;
   
    // source file is not needed, can be later added with StrToIns()
    if (getsrc(newInsObj, srcpath, srcstring) != SUCCESS){
        return NULL; 
    }

    return newInsObj;
}

static
dataObjT initData(size_t datalen){
    dataObjT newDataObj = calloc(1, sizeof(*newDataObj));
    if (!newDataObj){
        fprintf(stderr, cERR "Error allocating data object!\n" cNO);
        return NULL;
    }
    newDataObj->len = datalen;
    newDataObj->usedlen = 0;

    if (datalen > 0) {
        newDataObj->tape = calloc(newDataObj->len, sizeof(*newDataObj->tape));
        if (!newDataObj->tape){
            free(newDataObj);
            fprintf(stderr, cERR "Error allocating data tape of length %zu!\n" cNO, datalen);
            return NULL;
        }
        newDataObj->usedlen = 1;
    }

    return newDataObj;
}

static
stackObjT initStack(){
    stackObjT newStackObj = calloc(1, sizeof(*newStackObj));
    if (!newStackObj){
        fprintf(stderr, cERR "Error allocating stack object!\n" cNO);
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
initCtx(const char* srcpath,
        const char* srcstring,
        size_t datalen,
        uint flags){

    /* creating context object */
    ctxObjT newCtx = malloc(sizeof(*newCtx));
    if (!newCtx){ return NULL; }

    /* creating instruction object */
    newCtx->ins = initIns(srcpath, srcstring);
    if (!newCtx->ins){ goto insCleanup; } 
    /* checking if file was loaded correctly */
    if (srcpath && !newCtx->ins->srcpath){ goto insCleanup; }

    /* creating data object */
    newCtx->data = initData(datalen);
    if (!newCtx->data && datalen > 0){ goto dataCleanup; }

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

static void
resetIns(insObjT insObj){
    if (insObj) {
        insObj->index = 0;
    }

    return;
}

static void
resetData(dataObjT dataObj){
    if (dataObj) {
        dataObj->index = 0; 
        memset(dataObj->tape, DATAMIN, dataObj->usedlen * sizeof(*dataObj->tape));
        dataObj->usedlen = 0;
    }

    return;
}

static void
resetStack(stackObjT stackObj){
    if (stackObj) {
        memset(stackObj->tape, 0, stackObj->len * sizeof(*stackObj->tape));
        stackObj->len = 0;
    }

    return;
}

static void
resetCtx(ctxObjT Ctx){
    if (Ctx) {
        resetIns(Ctx->ins);
        resetData(Ctx->data);
        resetStack(Ctx->stack);

       BIT_SET_FALSE(Ctx->flags, CTX_COMPLETED | CTX_RUNNING);
    }

    return;
}

static void
freeIns(insObjT insObj){
    if (insObj) {
        free(insObj->tape);
        insObj->tape = NULL;
        free(insObj->srcpath);
        insObj->srcpath = NULL;
        free(insObj);
        insObj = NULL;
    }

    return;
}

static void
freeData(dataObjT dataObj){
    if (dataObj) {
        free(dataObj->tape);
        dataObj->tape = NULL;
        free(dataObj);
    }
    
    return;
}

static void
freeStack(stackObjT stackObj){
    if (stackObj) {
        stackObj->tape = NULL;
        free(stackObj);
    }

    return;
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
        if (Ctx->ins){
            freeIns(Ctx->ins);
            Ctx->ins = NULL;
        }

        /* clean data object */
        if (Ctx->data) {
            freeData(Ctx->data);
            Ctx->data = NULL;
        }

        /* clean stack object */
        if (Ctx->stack) {
            freeStack(Ctx->stack);
            Ctx->stack = NULL;
        }
        /* clean context object */
        free(Ctx);
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
        uchar cmdIndex = Ctx->ins->tape[i];
        printf("%c", insSet[cmdIndex].opcode);
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
        printf("start[%zu] end[%zu]\n", Ctx->stack->tape[i].start, Ctx->stack->tape[i].end);
    }

    return;
}

// TODO make these into list of all flags with their str rep and value to just for loop through them
static
void printFlags(ctxObjT Ctx){
    if (!Ctx || !Ctx->stack){
        return;
    }
    uint flags = Ctx->flags;

    printf("value: %u\n", flags);

    printf("%u CTX_RUNNING\n",           (flags & CTX_RUNNING)              ? 1 : 0); 
    printf("%u CTX_COMPLETED\n",         (flags & CTX_COMPLETED)            ? 1 : 0);
    printf("%u DATA_PENDING_OUT\n",      (flags & DATA_PENDING_OUT)         ? 1 : 0);
    printf("%u DATA_PENDING_IN\n",       (flags & DATA_PENDING_IN)          ? 1 : 0);
    printf("%u DATA_ALLOW_LOOPAROUND\n", (flags & DATA_ALLOW_LOOPAROUND)    ? 1 : 0);
    printf("%u DATA_ALLOW_OVERFLOW\n",   (flags & DATA_ALLOW_OVERFLOW)      ? 1 : 0);
    printf("%u DATA_ALLOW_UNDERFLOW\n",  (flags & DATA_ALLOW_UNDERFLOW)     ? 1 : 0);
    printf("%u DATA_DYNAMIC_GROW\n",     (flags & DATA_ALLOW_DYNAMIC_GROW)  ? 1 : 0);
    printf("%u PRINT_DIAGNOSTICS\n",     (flags & PRINT_DIAGNOSTICS)        ? 1 : 0);
    printf("%u TEST\n",                  (flags & TEST)                     ? 1 : 0);
    printf("%u TEST_STRICT\n",           (flags & TEST_STRICT)              ? 1 : 0);
    
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


static
bool isBalanced(insObjT insObj){
    if (!insObj || !insObj->tape) {
        return TRUE;
    }

    size_t currindex = insObj->index;
    long int count = 0;

    /* if count gets below zero, closing bracket was used without pair opening bracket */
    while (count >= 0 && currindex < insObj->usedlen){
        switch (insObj->tape[currindex]){
            case WHILE: count++;
                        break;

            case    DO: count--;
                        break;
        }

        currindex++;
    }

    /* balanced brackets will set count to 0, more opening brackets would leave count>0 */
    return (count == 0) ? TRUE : FALSE;
}
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
static size_t
getMatchingClosing(insObjT insObj){
    if (!insObj){
        return 0; 
    }

    size_t currindex = insObj->index;
    size_t lastindex = insObj->usedlen;
    /* hopefully long int is big enough */
    long int seen = 0;

    /* currindex should point to starting bracket to which we want to find closing */
    if (insSet[insObj->tape[currindex]].opcode != WHILE){
        return 0;
    }

    /* Search for matching brace until end of string */
    do {
        switch (insSet[insObj->tape[currindex]].opcode){
            case WHILE: seen++;
                        break;

            case    DO: seen--;
                        break;
        }
    } while (seen > 0 && ++currindex <= lastindex);
    
    return (seen != 0) ? 0 : currindex;
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
    if (!tapeCopy && memSize > 0) {
        Ctx->stack->len++;
        return ALLOCFAIL;
    }

    Ctx->stack->tape = tapeCopy;

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
 * execIns: Takes care of executing the right instruction
 *          Executes one instruction at Ctx->ins->index
 *
 * @tape: Tape structure of which one instruction tape will be executed
 */
static int
execIns(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return FAIL;
    }

    uchar currIns = Ctx->ins->tape[Ctx->ins->index];
    cmd currCMD = insSet[currIns].command;

    return currCMD(Ctx);
}

/* incPC()
 *
 * Increment programm counter, in other words Ctx->ins->index
 *
 * Return True if there are possibly more instructions to execute, False if
 * there are no more
 */
static bool
incPC(ctxObjT Ctx){
    if (!Ctx) return false;

    /* dont increment if we previously jumped on current instruction */
    if (Ctx->ins->jumped) {
        Ctx->ins->jumped = false;
        return true;
    }

    /* programm counter incrementation */
    if (Ctx->ins->index + 1 < Ctx->ins->usedlen) {
        Ctx->ins->index++;
        return true;
    }

    return false;
}

static int
execCtx(ctxObjT Ctx){
    int retval = SUCCESS;

    do {
        retval = execIns(Ctx);

        switch (retval) {
         default:
                continue;
        }
    } while (incPC(Ctx));

    return retval;
}

int
interpret(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return FAIL; 
    }
    
    // before interpretation
    if (Ctx->flags & CTX_COMPLETED) {
        resetCtx(Ctx);
    }
    
    /*
    if (Ctx->flags & (TEST | TEST_STRICT)) {
        if (!test() && Ctx->flags & TEST_STRICT) {
            fprintf(stderr, "Didn't passed all tests! Aborting...\n");
            return -1;
        }
    }
    */

    /* syntax checks */
    if(!isBalanced(Ctx->ins)){
        fprintf(stderr, cERR "ERROR: Unbalanced brackets!\n" cNO);
        return FAIL;
    }
    BIT_SET_TRUE(Ctx->flags, CTX_RUNNING);

    // interpretation
    execCtx(Ctx);
    
    // after interpretation
    BIT_SET_TRUE(Ctx->flags, CTX_COMPLETED);
    BIT_SET_FALSE(Ctx->flags, CTX_RUNNING);

    if (Ctx->flags & PRINT_DIAGNOSTICS)
        printCtx(Ctx);

    return SUCCESS;
}

/* test functions */

static bool
test_mvRight(void){
    /* 5 is just some random positive number for testing */
    size_t datalen = 5;
    bool passed = true;
    ctxObjT Ctx = initCtx(NULL, NULL, datalen, 0);

    /* casual move test */
    ASSERT(mvRight(Ctx) == SUCCESS, "casual move: retval check");
    ASSERT(Ctx->data->index == 1, "casual move: data index update");
    ASSERT(Ctx->data->usedlen == 2, "casual move: data usedlen update");

    /* index loop around test */
    /* simulate highest possible index to induce looparound*/
    Ctx->data->usedlen = Ctx->data->len;
    Ctx->data->index = Ctx->data->len - 1;   

    ASSERT(mvRight(Ctx) == SUCCESS, "loop around: retval check");
    ASSERT(Ctx->data->index != Ctx->data->len - 1, "loop around: data index update");
    ASSERT(Ctx->data->index == 0, "loop around: data index looped");

    /* NULL tests */
    ASSERT(mvLeft(NULL) == FAIL, "NULL as Ctx retval check");
    freeData(Ctx->data);
    Ctx->data = NULL;
    ASSERT(mvLeft(Ctx) == FAIL, "NULL as Ctx->data retval check");

    freeCtx(Ctx);
    return passed;
}

static bool
test_mvLeft(void){
    /* 5 is just some random positive number for testing */
    size_t datalen = 5;
    bool passed = true;
    ctxObjT Ctx = initCtx(NULL, NULL, datalen, 0);

    /* casual move test */
    /* simulate move right to avoid looparound */
    mvRight(Ctx);

    ASSERT(mvLeft(Ctx) == SUCCESS, "casual move: retval check");
    ASSERT(Ctx->data->index == 0, "casual move: data index update");
    ASSERT(Ctx->data->usedlen == 2, "casual move: data usedlen update");

    /* index loop around test */
    /* simulate lowest possible index to induce looparound*/
    Ctx->data->index = 0;

    ASSERT(mvLeft(Ctx) == SUCCESS, "index loop around: retval check");
    ASSERT(Ctx->data->index != 0, "index loop around: data index update");
    ASSERT(Ctx->data->index == Ctx->data->len - 1, "index loop around: data index looped");
    ASSERT(Ctx->data->usedlen == Ctx->data->len, "index loop around: data usedlen update")

    /* NULL tests */
    ASSERT(mvLeft(NULL) == FAIL, "NULL as Ctx retval check");
    freeData(Ctx->data);
    Ctx->data = NULL;
    ASSERT(mvLeft(Ctx) == FAIL, "NULL as Ctx->data retval check");

    freeCtx(Ctx);
    return passed;
}

static bool
test_incData(void){
    /* 5 is just some random positive number for testing */
    size_t datalen = 5;
    bool passed = true;
    ctxObjT Ctx = initCtx(NULL, NULL, datalen, 0);

    /* zero indexes will also test initCtx which must set start index to 0 */

    /* casual increment test */
    ASSERT(incData(Ctx) == SUCCESS, "casual inc: retval check");
    ASSERT(Ctx->data->tape[0] != 0, "casual inc: data val update");
    ASSERT(Ctx->data->tape[0] == 1, "casual inc: data val inced");

    /* value loop around test */
    /* simulate highest possible value to induce looparound*/
    Ctx->data->tape[0] = DATAMAX;

    ASSERT(incData(Ctx) == SUCCESS, "val loop around inc: retval check");
    ASSERT(Ctx->data->tape[0] != DATAMAX, "val loop around inc: data val update");
    ASSERT(Ctx->data->tape[0] == DATAMIN, "val loop around inc: data val looped");

    /* NULL tests */
    ASSERT(incData(NULL) == FAIL, "NULL as Ctx as check");
    freeData(Ctx->data);
    Ctx->data = NULL;
    ASSERT(incData(Ctx) == FAIL, "NULL as Ctx->data retval check");

    freeCtx(Ctx);
    return passed;
}

static bool
test_decData(void){
    /* 5 is just some random positive number for testing */
    size_t datalen = 5;
    bool passed = true;
    ctxObjT Ctx = initCtx(NULL, NULL, datalen, 0);

    /* zero indexes will also test initCtx which must set start index to 0 */

    /* casual increment test */
    /* simulate number > 0 to avoid looparound */
    Ctx->data->tape[0] = 1; 

    ASSERT(decData(Ctx) == SUCCESS, "casual dec: retval check");
    ASSERT(Ctx->data->tape[0] != 1, "casual dec: data val update");
    ASSERT(Ctx->data->tape[0] == 0, "casual dec: data val decd");

    /* value loop around test */
    /* simulate lowest possible value to induce looparound*/
    Ctx->data->tape[0] = DATAMIN;

    ASSERT(decData(Ctx) == SUCCESS, "val loop around dec: retval check");
    ASSERT(Ctx->data->tape[0] != DATAMIN, "val loop around dec: data val update");
    ASSERT(Ctx->data->tape[0] == DATAMAX, "val loop around dec: data val looped");

    /* NULL tests */
    ASSERT(decData(NULL) != SUCCESS, "NULL as Ctx as check");
    freeData(Ctx->data);
    Ctx->data = NULL;
    ASSERT(decData(Ctx) != SUCCESS, "NULL as Ctx->data retval check");

    freeCtx(Ctx);
    return passed;
}


static bool
test_jmp(){
    bool passed = true;
    int retval;

    ctxObjT Ctx = initCtx(NULL,"+[+--]+", 0, 0);
    //StrToIns(Ctx, "+[+--]+"); 
    /* test jump to higher index */
    retval = jmp(Ctx->ins, 5);
    ASSERT(Ctx->ins->index == 5, "set index: higher index");
    ASSERT(retval != FAIL, "retval: not failed");

    /* test jump to lower index */
    retval = jmp(Ctx->ins, 1);
    ASSERT(Ctx->ins->index == 1, "set index: lower index");
    ASSERT(retval != FAIL, "retval: not failed");

    /* test jump to same index */
    retval = jmp(Ctx->ins, 1);
    ASSERT(Ctx->ins->index == 1, "set index: same index");
    ASSERT(retval != FAIL, "retval: not failed");

    /* test jump to unused index */
    retval = jmp(Ctx->ins, Ctx->ins->len + 1);
    ASSERT(Ctx->ins->index == 1, "set index: out of bounds index");
    ASSERT(retval == PASTBOUNDS, "retval: PASTBOUNDS");

    /* test null insObject */
    retval = jmp(NULL, 1);
    ASSERT(Ctx->ins->index == 1, "arg check: null as insObj");
    ASSERT(retval == FAIL, "retval: failed");

    freeCtx(Ctx);
    return passed;
}

bool
test(void){
    testfunc *tests[] = {
        test_mvRight,
        test_mvLeft,
        test_incData,
        test_decData,
        test_jmp,
    };

    size_t ntests = sizeof(tests) / sizeof(*tests);
    size_t npassed = 0;

    for (size_t i = 0; i < ntests; i++){
        if (tests[i]())
            npassed++; 
    }
        
    printf(cBLUE "passed: %zu/%zu tests" cNO "\n" , npassed, ntests);
    return (npassed == ntests);
}

