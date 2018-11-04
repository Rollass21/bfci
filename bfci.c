#include "bfci.h"

extern const insSetT insSet[];

int
handleArgs(int argc,
           char** argv,
           char** srcpath,
           char** srcstring){

    int option;
    bool given_valid_option = false;
    bool given_srcpath = false;
    bool given_srcstring = false;

    if (argc < 2) {
        printHelp();
        return QUIT;
    }

    while ((option = getopt(argc, argv, "ht:i:")) != -1) {
        given_valid_option = true;
        switch (option){
            case 'h':
                printHelp();
                return QUIT;

            case 't':
                given_srcpath = true;
                if (given_srcstring) {
                    fprintf(stderr, "Using both '%s' and '%s' isn't currently possible!\n" , "-i", "-t");
                    return QUIT;
                }
                *srcpath = optarg;
                break;

            case 'i':
                given_srcstring = true;
                if(given_srcpath) {
                    fprintf(stderr, "Using both '%s' and '%s' isn't currently possible!\n", "-i", "-t");
                    return QUIT;
                }
                *srcstring = optarg;
                break;

            case '?':
                return QUIT;

            default:
                return QUIT;
        }
    }

    return (given_valid_option) ? SUCCESS : FAIL;
}

static int
pushIns(uchar ins, insObjT insObj){
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
 * isValidCtx()
 * Checks whether given object is properly initialized and ready to be executed
 *
 */
bool
isValidCtx(ctxObjT Ctx) {
    if (!Ctx || !Ctx->data || !Ctx->ins || !Ctx->stack)
        return false;

    return true;
}

bool
hasSource(ctxObjT Ctx){
    if (!isValidCtx(Ctx)) return false;

    return (Ctx->ins->flags & (INS_FROM_FILE | INS_FROM_STRING));
}

bool
canBeExecutedCtx(ctxObjT Ctx) {
    /* check if every object is allocated */
    if (!isValidCtx(Ctx)) return false;
    /* check for current ctx state */
    if (Ctx->flags & (CTX_STOPPED | CTX_RUNNING)) return false;
    /* check if source to run is present */
    if (!hasSource(Ctx)){
        return false;
    }

    return true;
};

// TODO too slow, maybe a hash table in future?
/*  isInstruction()
 *  Checks if given input c is valid opcode in InsSet object.
 *  If found, returns index of instruction, otherwise -1.
 *
 *  @c: Possible instruction opcode to be checked
 */
static int
isInstruction(char c) {
    for (int i = 0; insSet[i].name != NULL; i++){
        if (insSet[i].opcode == c)
            return i;
    }

    return -1; 
}

static int
fromFile(void* insObj, ...){
    int index;            
    va_list arglist;
    insObjT ins = insObj;
    va_start(arglist, insObj);

    char* srcpath = va_arg(arglist, char*); 

    if (!ins | !srcpath)
        return FAIL;

    FILE *srcfile = fopen(srcpath, "r");    
    if (!srcfile) {
        fprintf(stderr, cERR "ERROR: File \'%s\' cannot be opened!\n" cNO, srcpath);
        return FILEFAIL;
    }

    ins->srcpath = strdup(srcpath);
    FLAG_SET_FALSE(ins->flags, INS_FROM_STRING);
    FLAG_SET_TRUE(ins->flags, INS_FROM_FILE);

    /* filter out just legit opcodes */
    for (int c = getc(srcfile); c != EOF; c = getc(srcfile)){
        if ((index = isInstruction(c)) > -1){
            pushIns(index, ins);
        }
    }

    fclose(srcfile);
    va_end(arglist);
    return SUCCESS;
}

static int
fromString(void* insObj, ...){
    int index;            
    va_list arglist;
    insObjT ins = insObj;
    va_start(arglist, insObj);

    char* srcstring = va_arg(arglist, char*); 

    if (!ins | !srcstring)
        return FAIL;

    FLAG_SET_FALSE(ins->flags, INS_FROM_FILE);
    FLAG_SET_TRUE(ins->flags, INS_FROM_STRING);

    while(*srcstring != '\0'){
        if ((index = isInstruction(*srcstring)) > -1){
            pushIns(index, ins);
        }

        /* next char */
        srcstring++;
    }

    va_end(arglist);
    return SUCCESS;
}

/* getsrc: Encodes instructions from Brainfuck source file into instruction tape
 *         Used to extract instructions from file.
 *  @insObj     Instruction object to which will be instructions saved
 *  @srcpath:   Filename of Brainfuck source file, set to NULL if not known or 
 *              will use @srcstring as a source.
 *  @srcstring: String containing Brainfuck source code, set to NULL if not
 *              known or will use @srcpath as a source.
 *
 * Its illegal to use both @srcpath and @srcpath to get the source,
 *
 * Returns SUCCESS if properly initialized the insObj.
 */
static int
getsrc(insObjT insObj,
       const char* srcpath,
       const char* srcstring){

    const char* src = NULL;
    methodT fromWhat;
    /* anding, because we cant obtain source from both methods atst and we can not-get the source */
    if (srcpath && srcstring) {
        FLAG_SET_FALSE(insObj->flags, INS_FROM_STRING | INS_FROM_FILE);
        printf(cERR "ERROR: More than one source is prohibited!\n" cNO);
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

    return (!src) ? SUCCESS : fromWhat((void*) insObj, (void*) src);
}

int
addsrc(ctxObjT Ctx,
       const char* srcpath,
       const char* srcstring){

    if (!isValidCtx(Ctx)){
        return FAIL;
    }

    if (hasSource(Ctx)){
        fprintf(stderr, cERR "ERROR: Cannot add another source to Context!\n" cNO);
        return FAIL; 
    }
    
    return getsrc(Ctx->ins, srcpath, srcstring);
}

/* 
 * insObjT: initiates Instruction Object
 *
 * @srcpath: file containing BF source, if NULL then instructions need to
 *               be set with StrToIns() before execution
 */
static insObjT
initIns(const char* srcpath,
        const char* srcstring,
        settingsT settings){
    insObjT newInsObj = calloc(1, sizeof(*newInsObj));
    if (!newInsObj) {
        fprintf(stderr, cERR "Error allocating instruction object! \n" cNO);
        return NULL;
    }

    newInsObj->srcpath = NULL;
    newInsObj->flags = settings.flags.ins;
   
    // source file is not needed, can be later added with StrToIns()
    if (getsrc(newInsObj, srcpath, srcstring) != SUCCESS){
        return NULL; 
    }

    return newInsObj;
}

static dataObjT
initData(size_t datalen,
         settingsT settings){
    dataObjT newDataObj = calloc(1, sizeof(*newDataObj));
    if (!newDataObj){
        fprintf(stderr, cERR "Error allocating data object!\n" cNO);
        return NULL;
    }
    newDataObj->len = datalen;
    newDataObj->usedlen = 0;
    newDataObj->flags = settings.flags.data;

    if (datalen > 0) {
        newDataObj->tape = calloc(newDataObj->len, sizeof(*newDataObj->tape));
        if (!newDataObj->tape) {
            free(newDataObj);
            fprintf(stderr, cERR "Error allocating data tape of length %zu!\n" cNO, datalen);
            return NULL;
        }
        newDataObj->usedlen = 1;
    }

    return newDataObj;
}

static stackObjT
initStack(settingsT settings){
    stackObjT newStackObj = calloc(1, sizeof(*newStackObj));
    if (!newStackObj) {
        fprintf(stderr, cERR "Error allocating stack object!\n" cNO);
        return NULL;
    }
    newStackObj->tape = NULL;
    newStackObj->flags = settings.flags.stack;

    return newStackObj;
} 


static ctxObjT
initCtx(settingsT settings) {
    ctxObjT newCtxObj = malloc(sizeof(*newCtxObj));
    if (!newCtxObj) {
        fprintf(stderr, cERR "Error allocating ctx object!\n" cNO);
        return NULL;
    }
    newCtxObj->flags = settings.flags.ctx;
    newCtxObj->settings = settings;

    return newCtxObj;
}


/*  TODO 
 *  newCtx: Create context object needed for proper interpretation of BF
 *
 */
ctxObjT
newCtx(const char* srcpath,
       const char* srcstring,
       size_t datalen,
       settingsT* settings){

    /* when settings are not specified, use defaults */
    if (!settings){
        settings = &defaults;
    };

    /* creating context object */
    ctxObjT newCtxObj = initCtx(*settings);
    if (!newCtxObj) { goto ctxCleanup; }

    /* creating instruction object */
    newCtxObj->ins = initIns(srcpath, srcstring, *settings);
    if (!newCtxObj->ins) { goto insCleanup; } 
    /* checking if file was loaded correctly */
    if (srcpath && !newCtxObj->ins->srcpath) { goto insCleanup; }

    /* creating data object */
    newCtxObj->data = initData(datalen, *settings);
    if (!newCtxObj->data && datalen > 0) { goto dataCleanup; }

    /* creating stack object */
    newCtxObj->stack = initStack(*settings);
    if (!newCtxObj->stack) { goto stackCleanup; }

    return newCtxObj;

 stackCleanup:
    free(newCtxObj->data);
 dataCleanup:
    free(newCtxObj->ins);
 insCleanup:
    free(newCtxObj);
 ctxCleanup:
    return NULL;
}

static void
restoreIns(ctxObjT Ctx){
    if (!isValidCtx(Ctx))
        return;
    insObjT insObj = Ctx->ins;

    if (insObj) {
        insObj->index = 0;
        //TODO need to find a way to not overwrite INS_FROM_{FILE,STRING} bits
        insObj->flags = Ctx->settings.flags.ins;
    }

    return;
}

static void
restoreData(ctxObjT Ctx){
    if (!isValidCtx(Ctx))
        return;
    dataObjT dataObj = Ctx->data;

    if (dataObj) {
        dataObj->index = 0; 
        memset(dataObj->tape, DATAMIN, dataObj->usedlen * sizeof(*dataObj->tape));
        dataObj->usedlen = 0;
        dataObj->flags = Ctx->settings.flags.data;
    }

    return;
}

static void
restoreStack(ctxObjT Ctx){
    if (!isValidCtx(Ctx))
        return;
    stackObjT stackObj = Ctx->stack;

    if (stackObj) {
        memset(stackObj->tape, 0, stackObj->len * sizeof(*stackObj->tape));
        stackObj->len = 0;
        stackObj->flags = Ctx->settings.flags.stack;
    }

    return;
}

static void
restoreCtx(ctxObjT Ctx){
    if (Ctx) {
        restoreIns(Ctx);
        restoreData(Ctx);
        restoreStack(Ctx);

       //BIT_SET_FALSE(Ctx->flags, CTX_COMPLETED | CTX_RUNNING | CTX_STOPPED);
       Ctx->flags = Ctx->settings.flags.ctx;
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
Usage:     bfci [-h][-t <filepath>][-i <string>]\n\
Options:\n\
           -i '<string>'   interpret given string as if it was a file\n\
           -h              display this screen\n\
           -t <filepath>   interpret given BF source file target\n";

    printf("%s", helptext);
}

/* DIAGNOSTICS TOOLS */
static
void printIns(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
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
    if (!isValidCtx(Ctx)){
        return;
    }

    for(size_t i = 0; i < Ctx->data->usedlen; i++){
        printf("[%3d] ", Ctx->data->tape[i]);
    }

    return;
}

static
void printStack(ctxObjT Ctx) {
    if (!isValidCtx(Ctx)){
        return;
    }

    for(size_t i = 0; i < Ctx->stack->len; i++){
        printf("start[%zu] end[%zu]\n", Ctx->stack->tape[i].start, Ctx->stack->tape[i].end);
    }

    return;
}

// TODO somehow make these into list of all flags with their str rep and value to just for loop through them
static
void printFlags(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return;
    }

    ctxFlagsT   ctxf = Ctx->flags;
    insFlagsT   insf = Ctx->ins->flags;
    dataFlagsT  dataf = Ctx->data->flags;
    //stackFlagsT stackf = Ctx->stack->flags;

    printf("ctx flags: %d\n", ctxf);
    printf("%d CTX_RUNNING\n",                 (ctxf & CTX_RUNNING)              ? 1 : 0);
    printf("%d CTX_COMPLETED\n",               (ctxf & CTX_COMPLETED)            ? 1 : 0);
    printf("%d CTX_STOPPED\n",                 (ctxf & CTX_STOPPED)              ? 1 : 0);
    printf("%d DEBUG_PRINT_DIAGNOSTICS\n",     (ctxf & DEBUG_PRINT_DIAGNOSTICS)  ? 1 : 0);
    printf("%d DEBUG_TEST\n",                  (ctxf & DEBUG_TEST)               ? 1 : 0);
    printf("%d DEBUG_TEST_STRICT\n",           (ctxf & DEBUG_TEST_STRICT)        ? 1 : 0);

    printf("data flags: %d\n", dataf);
    printf("%d DATA_PENDING_OUT\n",            (dataf & DATA_PENDING_OUT)        ? 1 : 0);
    printf("%d DATA_PENDING_IN\n",             (dataf & DATA_PENDING_IN)         ? 1 : 0);
    printf("%d DATA_ALLOW_LOOPAROUND\n",       (dataf & DATA_ALLOW_LOOPAROUND)   ? 1 : 0);
    printf("%d DATA_ALLOW_OVERFLOW\n",         (dataf & DATA_ALLOW_OVERFLOW)     ? 1 : 0);
    printf("%d DATA_ALLOW_UNDERFLOW\n",        (dataf & DATA_ALLOW_UNDERFLOW)    ? 1 : 0);
    printf("%d DATA_DYNAMIC_GROW\n",           (dataf & DATA_ALLOW_DYNAMIC_GROW) ? 1 : 0);
    
    printf("ins flags: %d\n", insf);
    printf("%d INS_FROM_FILE \n",              (insf & INS_FROM_FILE)            ? 1 : 0);
    printf("%d INS_FROM_STRING \n",            (insf & INS_FROM_STRING)          ? 1 : 0);

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
            case OPC_WHILE: 
                count++;
                break;

            case OPC_DO:
                count--;
                break;
        }

        currindex++;
    }

    /* balanced brackets will set count to 0, more opening brackets would leave count>0 */
    return (count == 0) ? TRUE : FALSE;
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
    cmdT currCMD = insSet[currIns].command;

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
    if (!isValidCtx(Ctx))
        return false;

    /* dont increment if we previously jumped on current instruction */
    if (Ctx->ins->flags & INS_JUMPED) {
        FLAG_SET_FALSE(Ctx->ins->flags, INS_JUMPED);
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
    if (!isValidCtx(Ctx)){
        return FAIL; 
    }

    int retval = SUCCESS;

    do {
        retval = execIns(Ctx);
        if (retval == FAIL) return FAIL;
    } while (incPC(Ctx));

    return SUCCESS;
}

int
interpret(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return FAIL; 
    }
    
    // before interpretation
    if (Ctx->flags & (CTX_COMPLETED | CTX_RUNNING | CTX_STOPPED)) {
        restoreCtx(Ctx);
    }

    /*
    if (Ctx->flags & (DEBUG_TEST | DEBUG_TEST_STRICT)) {
        if (!test() && Ctx->flags & DEBUG_TEST_STRICT) {
            fprintf(stderr, "Didn't pass all tests! Aborting...\n");
            return -1;
        }
    }
    */
    if (!canBeExecutedCtx(Ctx)){
        fprintf(stderr, cERR "ERROR: Context is not ready for execution!\n" cNO);
        return FAIL; 
    }

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

    if (Ctx->flags & DEBUG_PRINT_DIAGNOSTICS)
        printCtx(Ctx);

    return SUCCESS;
}

