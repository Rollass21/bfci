#include "bfci.h"

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
    dataTp->minWrite = dataTp->maxWrite = dataTp->index = floor(dataTp->len/2);

    return retTape;
}

InsSetPtr initInsSet(){
    InsSetPtr retSet = malloc(NoIS * sizeof(retSet));

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

int isInstruction(int c, InsSetPtr InsSet){
    for(int i = 0; i < NoIS-1; i++){
        if(c == InsSet[i]) return i;
    }

    return FALSE;
}

void freeTapes(TapesPtr tape){
   free(tape->ins.tape);
   tape->ins.tape = NULL;
   free(tape->data.tape);
   tape->data.tape = NULL;
   free(tape);
   tape = NULL;
}

int printData(TapesPtr tape){
    if(tape == NULL){
        return FAIL;
    }

    DataTapePtr dataTp = &tape->data;
    unsigned int min = dataTp->minWrite;
    unsigned int max = dataTp->maxWrite;
    unsigned int middle = floor(dataTp->len/2);

    for(int i = min; i<=max; i++){
        printf("dataTp->tape[%u] = %d\n", i-middle, dataTp->tape[i]);
    }
    return SUCCESS;
}

int changeval(Tapes tape){
    switch(ins_cell){
        /* Increment value */
        case '+':
            if(*d_tape + 1 > CELLMAX){
                return OVERFLOW;
            }

            *d_tape++;
            break;
        /* Decrement value */
        case '-':
            if(*d_tape - 1 < 0){
                return UNDERFLOW;
            }
            *d_tape--;
            break;
        /* Undefined instruction */
        default :
            return NDFINS;    
    }
    return SUCCESS;
}

/*
 * getsrc: Gets all instructions from Brainfuck source file and saves them into ins_tape array
 * 
 * @source: Input file containing Brainfuck source code
 * @ins_tape: Output array which will be set to contain instructions
 * @length: Output number of elements in ins_ptr
 */

int getsrc(const char *source, Tapes tape ){
    FILE *src_file;
    int c;
    unsigned long int i = 0;

    src_file = fopen(source, "r");
    if(src_file == NULL){
        return FILEFAIL;
    }

    while((c = getc(src_file)) != EOF){
        if(isINSTRUCTION(c)){ 
            if(i > tape.ins){
                *inst_len = i;
                ins_tape = (cell*) realloc(ins_tape, *inst_len);
                if(ins_tape == NULL){
                    return ALLOCFAIL;
                }
            
            }
            ins_tape[i] = (cell) c;
            i++;
        }
    }
    
    
    
    fclose(src_file);
    return SUCCESS; 
}

/*
 * IO: Takes care of input/output of data tape d_tape array
 *
 * @ins_cell: Input instruction that specifies either input or output of cell d_tape points on
 * @d_tape: Input data tape pointer pointing to cell which value will be either outputed to stdout or overwritten
 * 
 */


int IO(cell ins_cell, cell *d_tape){
    
    switch(ins_cell){
        
        /* Output */
        case '.':
            putc(*d_tape, stdout);
            break;

        /* Input */
        case ',':
            *d_tape = getc(stdin);
            break;
        default :
            return NDFINS;    
    }
    
    return SUCCESS;
}

/*
 * move: Moves data tape pointer d_tape according to given instruction ins_cell
 *
 * @ins_cell: Input instruction that specifies either increment or decrement operation
 * @d_tape: Input/Output data tape which contains data over which instructions operate
 * @dt_index: Input index of current cell in data tape used to error check if we surpassed the limit of data tape length
 * @dt_len: Input/Output number of elements that data tape array d_tape contains
 */

int move(cell ins_cell, cell *d_tape, unsigned long int *dt_index, unsigned long int *dt_len){

    switch(ins_cell){
        
        /* Move right */ 
        case '>':
            if(*dt_index + 1 > *dt_len){
                if(*dt_index + 1 > DTMAXLEN){
                    return OVERFLOW;
                }
        
                *dt_len++;
                d_tape = (cell*) realloc(d_tape, *dt_len);
                if(d_tape == NULL){
                    return ALLOCFAIL;
                 }
            }
            *dt_index++;
            d_tape++;
            *d_tape = 0;
            break;

        /* Move left */ 
        case '<':
            if(dt_index - 1 < 0){
                return UNDERFLOW;
            }
            *dt_index--;
            d_tape--;
            break;

        /* Undefined instruction */         
        default :
            return NDFINS;
    }
    
    return SUCCESS;
}

