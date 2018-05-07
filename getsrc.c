#include "bfci.h"

/*
 * getsrc: Gets all instructions from Brainfuck source file and saves them into ins_tape array
 * 
 * @source: Input file containing Brainfuck source code
 * @ins_tape: Output array which will be set to contain instructions
 * @length: Output number of elements in ins_ptr
 */

int getsrc(const char *source, cell *ins_tape, unsigned long int *inst_len){
    FILE *src_file;
    int c;
    unsigned long int i = 0;

    src_file = fopen(source, "r");
    if(src_file == NULL){
        return FILEFAIL;
    }

    while((c = getc(src_file)) != EOF){
        
        /* if input char is in instruction set */
        if( (c == '>') || (c == '<') || (c == '+') || (c == '-') ||
            (c == '.') || (c == ',') || (c == '[') || (c == ']') ){
        
            if(i > *inst_len){
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
