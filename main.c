#include "bfci.h"

int main(int argc, const char *argv[]){
    const char* source;
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

    if(argc > 1){
        source = argv[1];
    }
    else{
        source = "helloworld.bf";
    }
    getsrc(source, tapes, insset);

    printDiagnostics(tapes, insset);
    
    freeTapes(tapes);
    return SUCCESS;
}
