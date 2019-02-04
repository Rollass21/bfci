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

    *srcpath = *srcstring = NULL;

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
addInsCell(insObjT insObj, uchar opIndex, uint times, size_t matching){
    if (insObj->usedlen + 1 > insObj->len){
        insObj->len += INSALLOCJMP;

        size_t memSize = insObj->len * sizeof(*insObj->tape);

        insObjT insObjCopy = insObj;
        insObjCopy->tape = realloc(insObj->tape, memSize);
        if (!insObjCopy->tape){
            return ALLOCFAIL;
        }
        insObj = insObjCopy;
    }

    /* adding instruction */
    insObj->tape[insObj->usedlen].opIndex = opIndex;
    insObj->tape[insObj->usedlen].times = times;
    insObj->tape[insObj->usedlen].matching = matching;
    insObj->tape[insObj->usedlen].ismatched = false;
    insObj->usedlen++;

    return SUCCESS;
}

/* pushIns()
 * Pushes instruction @ins to instruction object's @insObj tape
 *
 * @ins:    exact instruction to be pushed into tape
 * @insObj: instruciton object to which @ins is pushed
 *
 * Returns SUCCESS on correct push, otherwise error specific definiton of
 * value <0.
 */
static int
pushIns(uchar opIndex, insObjT insObj){
    if (!insObj){
        return FAIL;
    }

    /* can do RLE only after initial push  */
    if (insObj->usedlen > 0) {
        insCellT* lastcell = &insObj->tape[insObj->usedlen - 1];

        /* explicitly comparing to NULL because lastcell might be misleading */
        if (lastcell != NULL) {
            bool timesoverflow = (lastcell->times + 1 < lastcell->times);
            bool same = (lastcell->opIndex == opIndex);
            bool RLEable = (same && !timesoverflow && insSet[opIndex].RLEable);

            if (RLEable) {
                lastcell->times++;
                return SUCCESS;
            }
        }
    }

    return addInsCell(insObj, opIndex, 1, 0);
}

/* isValidCtx()
 * Checks whether given object is properly initialized and possible to be dereferenced
 *
 * @Ctx: object to be checked
 *
 * Returns true/false respectively on whether Ctx is valid.
 */
bool
isValidCtx(ctxObjT Ctx) {
    if (!Ctx || !Ctx->data || !Ctx->ins)
        return false;

    return true;
}

/* hasSource()
 * Checks whether given object contains any sort of source
 *
 * @Ctx: object to be checked
 *
 * Returns true/false respectively on whether Ctx contains source.
 */
bool
hasSource(ctxObjT Ctx){

    if (!isValidCtx(Ctx)) return false;
    if (!(Ctx->ins->flags & (INS_FROM_FILE | INS_FROM_STRING))) return false;
    if (Ctx->ins->usedlen == 0) return false;

    return true;
}

/* hasSource()
 * Checks whether given object is ready to be interpreted
 *
 * @Ctx: object to be checked
 *
 * Returns true/false respectively on whether Ctx is ready to be executed.
 */
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
/* isInstruction()
 * Checks if given char @c is valid opcode in InsSet object.
 *
 * @c: Possible instruction opcode to be checked
 *
 * If @c is valid instruction opcode, returns index of instruction, otherwise -1.
 */
static int
isInstruction(char c) {
    for (int i = 0; insSet[i].name != NULL; i++){
        if (insSet[i].opcode == c)
            return i;
    }

    return -1; 
}

/* isBalanced()
 * Checks whether given instruction object's tape opening an closing brackets
 * are balanced, used for syntax checking.
 *
 * @insObj: instruction object containing source to be checked for balanced
 *          brackets
 *
 * Returns true/false respectively on whether @insObj's DO and WHILE instrucitons
 * are balanced.
 */
static bool
isBalanced(insObjT insObj){
    if (!insObj || !insObj->tape) {
        return TRUE;
    }

    size_t currindex = insObj->index;
    long int count = 0;

    /* if count gets below zero, closing bracket was used without pair opening bracket */
    while (count >= 0 && currindex < insObj->usedlen){
        switch (insSet[insObj->tape[currindex].opIndex].opcode){
            case OPC_WHILE: 
                count++;
                break;

            case OPC_DO:
                count--;
                break;

            default:
                break;
        }

        currindex++;
    }

    /* balanced brackets will set count to 0, more opening brackets would leave count>0 */
    return (count == 0) ? TRUE : FALSE;
}

/*  fromFile()
 *  One of methods used in getSrc() to obtain source code into @insObj.
 *  Expects 2 arguments, the first is instruction object to which to enter the
 *  source and the second should be a string containing path to BF source file.
 *
 *  @insObj:  Instruction object to which tape we want the source to be written
 *  @...:     String (Char*) containing BF source file path
 *
 *  Returns SUCCESS on succesfull execution, otherwise specific error definition
 *  of value <0.
 */
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

/*  fromString()
 *  One of methods used in getSrc() to obtain source code into @insObj.
 *  Expects 2 arguments, the first is instruction object to which to enter the
 *  source and the second should be a string containing BF source code.
 *
 *  @insObj:  Instruction object to which tape we want the source to be written
 *  @...:     String (Char*) containing BF source code 
 *
 *  Returns SUCCESS on succesfull execution, otherwise specific error definition
 *  of value <0.
 */
static int
fromString(void* insObj, ...){
    int index;            
    int retval;
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
            if ((retval = pushIns(index, ins)) != SUCCESS) return retval;
        }

        /* next char */
        srcstring++;
    }

    va_end(arglist);
    return SUCCESS;
}

/* getsrc()
 * Encodes instructions from Brainfuck source {file/code string} into
 * instruction tape.
 * Used to extract instructions from file or string.
 *
 * @insObj     Instruction object to which will be instructions saved
 * @srcpath:   Filepath of Brainfuck source file, set to NULL if not known or 
 *             will use @srcstring as a source.
 * @srcstring: String containing Brainfuck source code, set to NULL if not
 *             known or will use @srcpath as a source.
 *
 * Its unsupported to use both @srcpath and @srcstring to obtain the source, will
 * result in not using either one of those, i.e. not fully initializing
 * instruction object (In which case user needs to add the source by invoking
 * addsrc() on context object after succesfull newCtx()).
 *
 *  Returns SUCCESS on proper initialization of @insObj (not entering any source
 *  is aslo considered a SUCCESS), otherwise specific error definition of value
 *  <0. If both @srcpath and @srcstring are !NULL, returns NDFUSAGE.
 */
static int
getsrc(insObjT insObj,
       const char* srcpath,
       const char* srcstring){

    if (!insObj) return FAIL;

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

/* addsrc()
 * Encodes instructions from Brainfuck source {file/code string} into
 * instruction tape.
 * Used to extract instructions from file or string to Ctx after newCtx initialization.
 *
 * @insObj     Context object contaiing insObj to which will be instructions saved
 * @srcpath:   Filepath of Brainfuck source file, set to NULL if not known or 
 *             will use @srcstring as a source.
 * @srcstring: String containing Brainfuck source code, set to NULL if not
 *             known or will use @srcpath as a source.
 *
 * Its unsupported to use both @srcpath and @srcstring to obtain the source, will
 * result in not using either one of those, i.e. not fully initializing
 * instruction object.
 *
 *  Returns SUCCESS on proper initialization of @Ctx's insObj (not entering any source
 *  is aslo considered a SUCCESS), otherwise specific error definition of value
 *  <0. If both @srcpath and @srcstring are !NULL, returns NDFUSAGE.
 *  In case @Ctx already contains any source, FAIL is returned.
 */
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

/* initIns()
 * Initializes Instruction Object
 *
 * @srcpath:   Filepath of Brainfuck source file, set to NULL if not known or 
 *             will use @srcstring as a source.
 * @srcstring: String containing Brainfuck source code, set to NULL if not
 *             known or will use @srcpath as a source.
 * @settings:  Original settings that are written into new insObj, later used to
 *             restore original state of insObj on repeated interpretation
 *
 * On succesfull execution returns newly allocated & initialized insObj,
 * otherwise returns NULL
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

/* initData()
 * Initializes Data Object
 *
 * @datalen:  Starting size of new dataObj's data tape, on integer type value >0
 *            allocates that length to tape, on ==0 allocates nothing and relies
 *            on dynamic allocation (also sets respective flags).
 * @settings: Original settings that are written into new dataObj, later used to
 *            restore original state of dataObj on repeated interpretation
 *
 * On succesfull execution returns newly allocated & initialized dataObj,
 * otherwise returns NULL
 */
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

/* initCtx()
 * Initializes Context Object
 *
 * @settings: Original settings that are written into new dataObj, later used to
 *            restore original state of ctxObj on repeated interpretation
 *
 * On succesfull execution returns newly allocated & initialized ctxObj,
 * otherwise returns NULL
 */
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


/* TODO 
 * newCtx()
 * Creates new context object needed for proper interpretation of BF source
 *
 * @srcpath:   Filepath of Brainfuck source file, set to NULL if not known or 
 *             will use @srcstring as a source.
 * @srcstring: String containing Brainfuck source code, set to NULL if not
 *             known or will use @srcpath as a source.
 * @datalen:   Starting size of new dataObj's data tape, on integer type value >0
 *             allocates that length to tape, on ==0 allocates nothing and relies
 *             on dynamic allocation (also sets respective flags).
 * @settings:  Original settings that are written in each subobject and stored
 *             for later restore to original state on repeated interpretation
 *
 * On succesfull execution returns newly allocated & initialized ctxObj with all
 * of its subobjects (later needs to be freed by freeCtx()), otherwise returns 
 * NULL and frees the resources.
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

    return newCtxObj;

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
        /* Erases memory accesed through interpretation */
        memset(dataObj->tape, DATAMIN, dataObj->usedlen * sizeof(*dataObj->tape));
        dataObj->usedlen = 0;
        //TODO need to find a way to not overwrite DATA_ALLOW_DYNAMIC_GROW bit
        dataObj->flags = Ctx->settings.flags.data;
    }

    return;
}

/* restoreCtx()
 * Used  to restore context object to its original settings (leaving some
 * settings to be left unchanged - mostly settings that implies from runtime or
 * arguments passed upon creating the now restored context)
 *
 * @Ctx: Context object to be restored
 */
static void
restoreCtx(ctxObjT Ctx){
    if (Ctx) {
        restoreIns(Ctx);
        restoreData(Ctx);

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

/* bitsetwmask()
 * Sets bits of @x to @v with exception of true bits in @mask
 *
 * @x
 */
static int
bitsetwmask(uint* x, uint v, uint mask){
    *x = (*x & mask) | (~mask & v);

    return *x;
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

/* freeCtx()
 * Frees memory allocated to Ctx and sets subobject pointers to NULL.
 *
 * @Ctx: context object allocated by newCtx() to be freed.
 */
void
freeCtx(ctxObjT Ctx){
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

        /* clean context object */
        free(Ctx);
    }
}

/* printHelp()
 * Prints help text with possible options to stdout for user to see.
 */
void
printHelp(){
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
static void
printIns(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return;
    }

    for(size_t i = 0; i < Ctx->ins->usedlen; i++){
        uchar cmdIndex = Ctx->ins->tape[i].opIndex;
        uchar times = Ctx->ins->tape[i].times;
        if (times > 1)
            printf("%u%c", times, insSet[cmdIndex].opcode);
        else
            printf("%c", insSet[cmdIndex].opcode);
    }
    printf("\n");

    return;
}

static void
printData(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return;
    }

    for(size_t i = 0; i < Ctx->data->usedlen; i++){
        printf("[%3d] ", Ctx->data->tape[i]);
    }

    return;
}

// TODO somehow make these into list of all flags with their strrep and value to just for loop through them
/* printFlags()
 * Prints the contents of each subobjects flags with respective name of flag.
 */
static void
printFlags(ctxObjT Ctx){
    if (!isValidCtx(Ctx)){
        return;
    }

    ctxFlagsT   ctxf = Ctx->flags;
    insFlagsT   insf = Ctx->ins->flags;
    dataFlagsT  dataf = Ctx->data->flags;

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

/* printCtx()
 * Prints all inside values of context object and its subobjects to stdout
 *
 * @Ctx: Object to be exposed
 */
void
printCtx(ctxObjT Ctx){
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
        printf("--Flags--\n");
        printFlags(Ctx);
    }
}

/* INTERPRETATION TIME */

/* execIns()
 * Takes care of executing the right operation for given instruction at
 * instruction index.
 * Executes one instruction at Ctx->ins->index
 *
 * @Ctx: VALID! context object from which will be executed current instruction.
 *
 * Returns retrun value of a command associated with instruction.
 */
static int
execIns(ctxObjT Ctx){

    uchar currOPIndex = Ctx->ins->tape[Ctx->ins->index].opIndex;
    cmdT currCMD = insSet[currOPIndex].command;

    return currCMD(Ctx);
}

// TODO add handling of overflows, underflows (flags in general) before incrementing
/* incPC()
 * Increment programm counter, in other words: fancier Ctx->ins->index++
 *
 * @Ctx: VALID! context object which programm counter will be incremented.
 *
 * Returns true if there are possibly more instructions to execute, false if
 * there are no more.
 */
static bool
incPC(ctxObjT Ctx){
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

/* execCtx()
 * Executes all instructins on @Ctx's instruction tape
 *
 * @Ctx: Context to be executed
 *
 * Returns SUCCESS if every instruction goes fine and dandy, otherwise stops
 * and returns FAIL.
 */
static int
execCtx(ctxObjT Ctx){
    int retval = SUCCESS;
    insObjT insObj = Ctx->ins;

    do {
        uint times = insObj->tape[insObj->index].times;
        for(; times > 0; times--) {
            retval = execIns(Ctx);
            if (retval == FAIL) return FAIL;
        }
    } while (incPC(Ctx));

    return SUCCESS;
}

/* interpret()
 * Takes care of correct interpretation of initialized context ready to run
 *
 * @Ctx: context object to interpret
 *
 * Returns SUCCESS on FAIL-less execution, otherwise returns specific error
 * definition of value <0
 */
int
interpret(ctxObjT Ctx){
    int retval;

    if (!isValidCtx(Ctx)){
        return FAIL; 
    }
    
    // before execution
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

    // execution cycle
    retval = execCtx(Ctx);
    
    // after execution
    BIT_SET_TRUE(Ctx->flags, CTX_COMPLETED);
    BIT_SET_FALSE(Ctx->flags, CTX_RUNNING);

    if (Ctx->flags & DEBUG_PRINT_DIAGNOSTICS)
        printCtx(Ctx);

    return retval;
}

