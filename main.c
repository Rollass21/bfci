#include "bfci.h"

int main(int argc, const char *argv[]){
    TapesPtr tapes = initTapes();
    if(tapes == NULL){
        fprintf(stderr, "Error allocating memory! (initTapes)\n");
        return ALLOCFAIL;
    } 
    InsSetPtr insset = initInsSet();
    if(insset == NULL){
        fprintf(stderr, "Error allocating memory! (initInsSet)\n");
        return ALLOCFAIL;
    }

    int c = MV_L;
    if(isInstruction(c, insset) != FALSE){
        saveIns(c, tapes);
    }

    c = MV_R;
    if(isInstruction(c, insset) != FALSE){
        saveIns(c, tapes);
    }

    printDiagnostics(tapes, insset);
    
    freeTapes(tapes);
    if(tapes == NULL){
        printf("cleanup done!\n");
    }

    return SUCCESS;
}
