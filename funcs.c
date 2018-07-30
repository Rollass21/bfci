#include "bfci.h"
#include "stack.h"

/* TODO
 * jmp()
 *
 * @tape - ...
 * @dest - Position/Index on which to set instruciton tape index
 *
 * Used to between instructions
 * On correct jump, returns destination index, doesnt error out in case of
 * overflow
 */
static unsigned int
jmp(TapesPtr tape, unsigned int dest){
    if(tape == NULL){
        return NDFUSAGE;
    }

    InsTapePtr insTp = tape->ins;
    
    if (dest < insTp->usedlen)
        insTp->index = dest;

    return dest;
}

static
InsTapePtr initInsTape(){
    InsTapePtr retInsTp = malloc(sizeof(InsTape));
    if(retInsTp == NULL){
        return NULL;
    }

    retInsTp->len = ALLOCJMP;
    retInsTp->tape = malloc(retInsTp->len * sizeof(*retInsTp->tape));
    if(retInsTp->tape == NULL){
        return NULL;
    }
    retInsTp->index = 0;
    retInsTp->usedlen = 0;

    return retInsTp;
}

static
DataTapePtr initDataTape(){
    DataTapePtr retDataTp = malloc(sizeof(DataTape));
    if(retDataTp == NULL){
        return NULL;
    }

    retDataTp->len = DTMAXLEN;
    retDataTp->tape = calloc(retDataTp->len, sizeof(*retDataTp->tape));
    if(retDataTp == NULL){
        return NULL;
    }
    retDataTp->minWrite = retDataTp->maxWrite = retDataTp->index = 0;

    return retDataTp;
}

/*  initTapes: Create instruction and data tapes
 *
 */
TapesPtr initTapes(){
    TapesPtr retTape = malloc(sizeof(Tapes));
    if(retTape == NULL){
        return NULL;
    }
    retTape->ins = initInsTape();
    if(retTape->ins == NULL){
        return retTape = NULL;
 
    } 
    retTape->data = initDataTape();
    if(retTape->data == NULL){
        retTape->ins = NULL;
        retTape = NULL;
        return NULL;
    }

    return retTape;
}

/*  initInsSet: Create instruciton set array
 *
 */
InsSetPtr initInsSet(){
    InsSetPtr retSet = malloc(INSSLEN * sizeof(*retSet));
    if(retSet == NULL){
        return NULL;
    }
    
    int i = 0;
    retSet[i++] =  MV_R;
    retSet[i++] =  MV_L;
    retSet[i++] =   INC;
    retSet[i++] =   DEC;
    retSet[i++] = STD_O;
    retSet[i++] = STD_I;
    retSet[i++] = WHILE;
    retSet[i++] =   END;

    return retSet;
}

/*  isInstruction: Checks if given input c is valid instruction in InsSet, if is
 *                 then returns encoded value of given instruction
 *
 *  @c: Possible instruction
 *  @insset: instruction set array initalized by initInsSet()
 */
static
int isInstruction(int c, InsSetPtr insset){
    for(int i = 0; i < INSSLEN; i++){
        if(c == insset[i]){
            return i;
        }
    }
    return FALSE;
}

/*  freeTapes: Frees memory allocated for tapes
 *
 *  @tape: Tapes, initialized by initTapes() & initInsSet() to cleanup
 */
void freeTapes(TapesPtr tape, InsSetPtr insset){
    if(tape){
        /* cleanup instruction tape */
        free(tape->ins->tape);
        tape->ins->tape = NULL;
        free(tape->ins);
        tape->ins = NULL;

        /* cleanup data tape */
        free(tape->data->tape);
        tape->data->tape = NULL;
        free(tape->data);
        tape->data = NULL;

        free(tape);
        tape = NULL;
    }
           
    if(insset){
        /* cleanup instruction set array */
        free(insset);
        insset = NULL;
    }

    return;
}

static
int printData(TapesPtr tape){
    if(tape == NULL){
        return SUCCESS;
    }

    DataTapePtr dataTp = tape->data;
    unsigned int min = dataTp->minWrite;
    unsigned int max = dataTp->maxWrite;

    for(int i = min; i<=max; i++){
        printf("dataTp->tape[%u] = %d\n", i, dataTp->tape[i]);
    }
    return SUCCESS;
}

static
int printIns(TapesPtr tape, InsSetPtr insset){
    if(tape == NULL){
        return SUCCESS;
    }
    InsTapePtr insTp = tape->ins;
    
    for(int i = 0; i < insTp->usedlen; i++){
        printf("%c", insTp->tape[i]);
    }
    printf("\n");

    return SUCCESS;
}

static
int saveIns(int c, TapesPtr tape){
    if(tape == NULL){
        return NDFUSAGE;
    }

    InsTapePtr insTp = tape->ins;

    if(insTp->index + 1 > insTp->len){
        insTp->len += ALLOCJMP;
        size_t insSize = insTp->len * sizeof(*insTp->tape);

        insTp->tape = realloc(insTp->tape, insSize);
        if(insTp->tape == NULL){
            return ALLOCFAIL;
        }
    }

    insTp->tape[insTp->index++] = c;
    return SUCCESS;
}

void printDiagnostics(TapesPtr tape, InsSetPtr insset){
    printf("\n\n##########################\n");
    printf("# Instruction Tape\n");
    printf("ins->len = %u\n", tape->ins->len);
    printf("ins->index = %u\n", tape->ins->index);
    printf("ins->usedlen = %u\n", tape->ins->usedlen);
    printIns(tape, insset);

    printf("\n");
    printf("# Data Tape\n");
    printf("data->len = %u\n", tape->data->len);
    printf("data->index = %u\n", tape->data->index);
    printData(tape);
    printf("##########################\n");
}
/*  getsrc: Encodes instructions from Brainfuck source file into instruction tape
 * 
 *  @source: Filename of Brainfuck source file
 *  @tape: Tape structure in which the instructions will be stored
 *  @insset Instruction set array used to identify instructions
 */
int getsrc(const char *source, TapesPtr tape, InsSetPtr insset){
    int c;
    FILE *src_file = fopen(source, "r");
    if(src_file == NULL){
        return FILEFAIL;
    }

    while((c = getc(src_file)) != EOF){
        if(isInstruction(c, insset) != FALSE){
            saveIns(c, tape);
        }
    }
    tape->ins->usedlen = tape->ins->index;
    tape->ins->index = 0;

    fclose(src_file);
    return SUCCESS;
}

/*  changeval: Changes value of data according to given instruction
 *
 *  @tape: Tape structure which data tape cell will be incremented/decremented
 */
static
int changeval(TapesPtr tape){
    if(tape == NULL){
        return FAIL;
    }

    int ret = SUCCESS;

    InsTapePtr insTp = tape->ins;
    DataTapePtr dataTp = tape->data;
    
    switch(insTp->tape[insTp->index]){
        case INC:
            if(dataTp->tape[dataTp->index]+1 > DATAMAX){
                ret = OVERFLOW;
            }
            dataTp->tape[dataTp->index]++;
            break;

        case DEC:
            if(dataTp->tape[dataTp->index]-1 < DATAMIN){
                ret = UNDERFLOW;
            }
            dataTp->tape[dataTp->index]--;
            break;

        default:
            ret = NDFINS;
    }

    return ret;
}

/*  IO: Takes care of input/output of data tape
 *
 *  @tape: Tape structure which data tape will be manipulated over STDI/O
 */
static
int IO(TapesPtr tape){
    if(tape == NULL){
        return FAIL;
    }
    
    int ret = SUCCESS;

    InsTapePtr insTp = tape->ins;
    DataTapePtr dataTp = tape->data;
    
    switch(insTp->tape[insTp->index]){
        case STD_O:
            putc(dataTp->tape[dataTp->index], stdout);
            break;

        case STD_I:
            getc(stdin);
            break;

        default:
            ret = NDFINS;
    }

    return ret;
}

/*
 * move()
 *
 *  @tape: Tape structure containing data tape
 *
 *  Moves data tape according to given instruction under instruction tape
 *  index(left, right)
*/
static 
int move(TapesPtr tape){
    if(tape == NULL){
        return FAIL;
    }
    
    int ret = SUCCESS;

    InsTapePtr insTp = tape->ins;
    DataTapePtr dataTp = tape->data;
    
    switch(insTp->tape[insTp->index]){
        case MV_L:
            if(dataTp->index == 0){
                ret = PASTBOUNDS;
                dataTp->index = dataTp->len;
            }
            if(--dataTp->index < dataTp->minWrite){
                dataTp->minWrite = dataTp->index;
            }
            break;

        case MV_R:
            if(++dataTp->index > dataTp->len){
                ret = PASTBOUNDS;
                dataTp->index = 0;
            }
            if(dataTp->index > dataTp->maxWrite){
                dataTp->maxWrite = dataTp->index;
            }
            break;

        default:
            ret = NDFINS;
    }

    return ret;
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
unsigned int
getMatchingEnd(TapesPtr tape){
    if(tape == NULL) {
        return FAIL;
    }

    InsTapePtr insTp = tape->ins;
    int count = 1;
    unsigned int currindex = insTp->index;
    
    /* Search for matching brace until end of instruction tape */
    while (count != 0 && ++currindex < insTp->usedlen ){
        switch (insTp->tape[currindex]){
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


/* TODO
 * loop()
 *
 * @tape - ...
 * @stack - Pointer to stack that corresponds to given @tape
 *
 */
static
int loop(TapesPtr tape, StackPtr stack){
    if(tape == NULL || stack == NULL){
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
                freeStack(stack);
                return NOLOOPEND;
            }
            SPush(stack, start, end);
            
            // If current data == 0, jump to END
            if (dataTp->tape[dataTp->index] == 0) {
                jmp(tape, stack->array[stack->len - 1].end);
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
                jmp(tape, stack->array[stack->len - 1].start);
            }

            break;

        default:
            ret = NDFINS;
    }

    return ret;
}

/* execute: Takes care of executing the right instruction
 *          - for now the values are hardcodec, in future there will be function
 *            array containing each one of the functions
 *
 * @tape: Tape structure of which one instruction tape will be executed
 */
static
int execute(TapesPtr tape, StackPtr stack){
    if(tape == NULL){
        return FAIL; 
    }
    int ins = tape->ins->tape[tape->ins->index];

    if(ins==   INC || ins==   DEC) return changeval(tape);
    if(ins== STD_O || ins== STD_I) return        IO(tape);
    if(ins==  MV_R || ins==  MV_L) return      move(tape);
    if(ins== WHILE || ins==   END) return      loop(tape, stack);

    return NDFINS;
}

int run(TapesPtr tape){
    if(tape == NULL){
        return FAIL;
    }

    StackPtr stack = initStack();
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





