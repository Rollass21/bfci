#include "bfci.h"

/*  initTapes: Create instruction and data tapes
 *
 */
TapesPtr initTapes(){
    TapesPtr retTape = malloc(sizeof(Tapes));
    if(retTape == NULL){
        return NULL;
    }
    
    InsTapePtr insTp = &retTape->ins;
    DataTapePtr dataTp = &retTape->data;

    /* Instruction tape initialization */
    insTp->len = 1;
    insTp->tape = malloc(insTp->len * sizeof(*insTp->tape));
    if(insTp->tape == NULL){
        return NULL;
    }
    insTp->index = 0;
    
    /* Data tape initialization */
    dataTp->len = DTMAXLEN;


    //q:Would it be faster to just malloc and zero the tape cell only after reaching it?
    //a:Yes. I guess.
    dataTp->tape = calloc(dataTp->len, sizeof(*dataTp->tape));
    if(dataTp == NULL){
        return NULL;
    }
    dataTp->minWrite = dataTp->maxWrite = dataTp->index = dataTp->len/2;

    return retTape;
}

/*  initInsSet: Create instruciton set array
 *
 */
InsSetPtr initInsSet(){
    InsSetPtr retSet = malloc(NoIS * sizeof(retSet));
    if(retSet == NULL){
        return NULL;
    }

    retSet[E_MV_R]  =  MV_R;
    retSet[E_MV_L]  =  MV_L;
    retSet[E_INC]   =   INC;
    retSet[E_DEC]   =   DEC;
    retSet[E_STD_O] = STD_O;
    retSet[E_STD_I] = STD_I;
    retSet[E_WHILE] = WHILE;
    retSet[E_END]   =   END;

    return retSet;
}

/*  isInstruction: Checks if given input c is valid instruction in InsSet
 *
 *  @c: Possible instruction
 *  @InsSet: instruction set array initalized by initInsSet()
 */
int isInstruction(int c, InsSetPtr InsSet){
    for(int i = 0; i < NoIS; i++){
        if(c == InsSet[i]){
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
   free(tape->ins.tape);
   tape->ins.tape = NULL;
   free(tape->data.tape);
   tape->data.tape = NULL;
   free(tape);
   tape = NULL;
}

/*  printData: Shows the data in the range over which instructions operated 
 *
 *  @tape: Tape structure which data to print
 */
int printData(TapesPtr tape){
    if(tape == NULL){
        return FAIL;
    }

    DataTapePtr dataTp = &tape->data;
    unsigned int min = dataTp->minWrite;
    unsigned int max = dataTp->maxWrite;
    unsigned int middle = dataTp->len/2;

    for(int i = min; i<=max; i++){
        printf("dataTp->tape[%u] = %d\n", i-middle, dataTp->tape[i]);
    }
    return SUCCESS;
}

/*
 *
 *
 */
//int saveIns(TapesPtr tape){}

/*  changeval: Changes value of data according to given instruction
 *
 *  @tape: Tape structure which data tape cell will be incremented/decremented
 */
//int changeval(Tapes tape){}

/*  getsrc: Encodes instructions from Brainfuck source file into instruction tape
 * 
 *  @source: Filename of Brainfuck source file
 *  @tape: Tape structure in which the encoded instructions will be stored
 */
//int getsrc(const char *source, Tapes tape ){}

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
