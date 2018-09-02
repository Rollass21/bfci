#include "bfci.h"

int main(int argc, const char *argv[]){

    const char* source = "bfSrcs/helloworld.bf";
    if(argc > 1)
        source = argv[1];

    ctxObjT Ctx = initCtx(NULL, DATAMAXLEN, 0);
    char* string = "TEST +-,.[]>< abcdABCD0123456789";
    StrToIns(Ctx, string);

    printCtx(Ctx);
    
    freeCtx(Ctx);

    return SUCCESS;
}
