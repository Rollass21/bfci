#include "bfci.h"

int main(int argc, const char *argv[]){
    TapesPtr tapes = initTapes();
    InsSetPtr insset = initInsSet();

    if(tapes == NULL || insset == NULL){
        freeTapes(tapes, insset);
        fprintf(stderr, "(initInsSet) Error allocating memory!\n");
        return ALLOCFAIL;
    }

    const char* source = "helloworld.bf";
    if(argc > 1)
        source = argv[1];

    if(getsrc(source, tapes, insset) != SUCCESS){
        fprintf(stderr, "(getsrc) Error obtaining source file '%s' !\n", source);
        return FILEFAIL;
    }

    run(tapes);

    printDiagnostics(tapes, insset);
    
    freeTapes(tapes, insset);
    return SUCCESS;
}
