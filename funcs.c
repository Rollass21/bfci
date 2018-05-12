#include "bfci.h"

static
InsTapePtr initInsTape(){
    InsTapePtr retInsTp = malloc(sizeof(InsTape));
    if(retInsTp == NULL){
        return NULL;
    }

    retInsTp->len = 1;
    retInsTp->tape = malloc(retInsTp->len * sizeof(*retInsTp->tape));
    if(retInsTp->tape == NULL){
        return NULL;
    }
    retInsTp->index = 0;

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
    retDataTp->minWrite = retDataTp->maxWrite = retDataTp->index = retDataTp->len/2;

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
        return NULL;
    }
    
    retTape->data = initDataTape();
    if(retTape->data == NULL){
        return NULL;
    }

    return retTape;
}

/*  initInsSet: Create instruciton set array
 *
 */
InsSetPtr initInsSet(){
    InsSetPtr retSet = malloc(INSSLEN * sizeof(retSet));
    if(retSet == NULL){
        return NULL;
    }
retSet[0] =  MV_R;
    retSet[1] =  MV_L;
    retSet[2] =   INC;
    retSet[3] =   DEC;
    retSet[4] = STD_O;
    retSet[5] = STD_I;
    retSet[6] = WHILE;
    retSet[7] =   END;

    return retSet;
}

/*  isInstruction: Checks if given input c is valid instruction in InsSet, if is
 *                 then returns encoded value of given instruction
 *
 *  @c: Possible instruction
 *  @InsSet: instruction set array initalized by initInsSet()
 */
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
 *  @tape: Tapes, initialized by initTapes(), to cleanup
 */
void freeTapes(TapesPtr tape){
   free(tape->ins->tape);
   tape->ins->tape = NULL;
   free(tape->data->tape);
   tape->data->tape = NULL;
   free(tape);
   tape = NULL;
}

static
int printData(TapesPtr tape){
    if(tape == NULL){
        return SUCCESS;
    }

    DataTapePtr dataTp = tape->data;
    unsigned int min = dataTp->minWrite;
    unsigned int max = dataTp->maxWrite;
    unsigned int middle = dataTp->len/2;

    for(int i = min; i<=max; i++){
        printf("dataTp->tape[%u] = %d\n", i-middle, dataTp->tape[i]);
    }
    return SUCCESS;
}

static
int printIns(TapesPtr tape, InsSetPtr insset){
    if(tape == NULL){
        return SUCCESS;
    }

    InsTapePtr insTp = tape->ins;
    
    for(int i = 0; i < insTp->len; i++){
        printf("insTp->tape[%u] = %c\n", i, insTp->tape[i]);
    }

    return SUCCESS;
}

/*
 *
 *
 */
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
    printf("Instruction Tape\n");
    printf("ins->len = %u\n", tape->ins->len);
    printf("ins->index = %u\n", tape->ins->index);
    printIns(tape, insset);

    printf("\nData Tape\n");
    printf("data->len = %u\n", tape->data->len);
    printf("data->index = %u\n", tape->data->index);
    printData(tape);
    printf("\n");
}
/*  getsrc: Encodes instructions from Brainfuck source file into instruction tape
 * 
 *  @source: Filename of Brainfuck source file
 *  @tape: Tape structure in which the instructions will be stored
 */
// int getsrc(const char source*){}

/*  changeval: Changes value of data according to given instruction
 *
 *  @tape: Tape structure which data tape cell will be incremented/decremented
 */
//int changeval(Tapes tape){}

/*  IO: Takes care of input/output of data tape
 *
 *  @tape: Tape structure which data tape will be manipulated over STDIO
 */
//int IO(Tapes tape){}

/*  move: Moves data tape according to given instruction(left, right)
 *
 *  @tape: Tape structure containing data tape
 */
//int move(Tapes tape){}


