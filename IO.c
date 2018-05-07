#include "bfci.h"

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
