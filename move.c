#include "bfci.h"

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

