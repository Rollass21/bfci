#include "bfci.h"

int main(int argc, char **argv){
    ctxObjT Ctx;
    char* srcpath;
    char* srcstring;

    if (handleArgs(argc, argv, &srcpath, &srcstring) != SUCCESS)
        return FAIL;

    Ctx = newCtx(srcpath, srcstring, DATAMAXLEN, NULL);
    interpret(Ctx);

    freeCtx(Ctx);
    return SUCCESS;
}
