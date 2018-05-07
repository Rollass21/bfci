#include "bfci.h"

int changeval(cell ins_cell, cell *d_tape){
    
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
