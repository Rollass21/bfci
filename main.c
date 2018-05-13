#include "bfci.h"

int main(int argc, const char *argv[]){
    const char* source;
    TapesPtr tapes = initTapes();
    if(tapes == NULL){
        fprintf(stderr, "(initTapes) Error allocating memory !\n");
        return ALLOCFAIL;
    } 
    InsSetPtr insset = initInsSet();
    if(insset == NULL){
        fprintf(stderr, "(initInsSet) Error allocating memory!\n");
        return ALLOCFAIL;
    }

    if(argc > 1){
        source = argv[1];
    }
    else{
        source = "helloworld.bf";
    }
    if(getsrc(source, tapes, insset) != SUCCESS){
        fprintf(stderr, "(getsrc) Error obtaining source file '%s' !\n", source);
        return FILEFAIL;
    }

    run(tapes);

    printDiagnostics(tapes, insset);
    
    freeTapes(tapes);
    return SUCCESS;
}
