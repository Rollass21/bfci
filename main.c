#include "bfci.h"

int main(int argc, const char *argv[]){
    TapesPtr tapes = initTapes();
    if(tapes == NULL){
        fprintf(stderr, "Error allocating memory! (initTapes)\n");
        return 0;
    } 
    InsSetPtr insset = initInsSet();
    if(insset == NULL){
        fprintf(stderr, "Error allocating memory! (initInsSet)\n");
    }

    printf("tapes->ins.len = %u\n", tapes->ins.len);
    printf("tapes->ins.index = %u\n", tapes->ins.index);
    printf("tapes->data.len = %u\n", tapes->ins.len);
    printf("tapes->data.index = %u\n", tapes->ins.index);

    printData(tapes);

    if(isInstruction(MV_R, insset) != FALSE){
        printf("isInstruction works!\n");        
    }

    freeTapes(tapes);
    if(tapes == NULL){
        printf("cleanup done!\n");
    }

    return 0;
}
