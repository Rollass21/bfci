#include "bfci.h"

int main(int argc, const char *argv[]){

    int err;
    cell *ins_tape,
         *d_tape;
    unsigned long int dt_len,
                      inst_len,
                      i,
                      dt_index = 0;
     
    inst_len = dt_len = ARRJMP;
    ins_tape = (cell*) calloc(inst_len, sizeof(cell));
    d_tape = (cell*) calloc(dt_len, sizeof(cell));


    if(argc>1){
        if( (err = getsrc(argv[1], ins_tape, &inst_len)) != SUCCESS){
            return err;
        }
    }
    else{
        return NDFUSAGE;
    }
    
    
    for(i = 0; i < inst_len; i++){
        switch(*ins_tape){
            /* Move right */
            case '>' :
                if( (err = move(*ins_tape, d_tape, &dt_index, &dt_len)) != SUCCESS){}
                break;
            /* Move left */
            case '<' :
                if( (err = move(*ins_tape, d_tape, &dt_index, &dt_len)) != SUCCESS){}
                break;
            /* Increment */
            case '+' :
                if( (err = changeval(*ins_tape, d_tape)) != SUCCESS ){}
                break;
            /* Decrement */
            case '-' :
                if( (err = changeval(*ins_tape, d_tape)) != SUCCESS ){}
                break;
            /* Output */
            case '.' :
                if( (err = IO(*ins_tape, d_tape)) != SUCCESS ){}
                break;
            /* Input */
            case ',' :
                if( (err = IO(*ins_tape, d_tape)) != SUCCESS ){}
                break;
            /* While begin */
            case '[' : // idk yet 
                break;
            /* While end */
            case ']' : // idk yet
                break;
        }
    }
    
    /* Cleaning up pointers */
    free(ins_tape);
    free(d_tape);
    ins_tape = d_tape = NULL;

    return 0;
}
