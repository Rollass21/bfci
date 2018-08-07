#include "bfci.h"

int main(int argc, const char *argv[]){


    const char* source = "bfSrcs/helloworld.bf";
    if(argc > 1)
        source = argv[1];

    ctxObjT Ctx = initCtx(source, DATAMAX, 0);

    printCtx(Ctx);
    
    clearCtx(Ctx);

    return SUCCESS;
}
