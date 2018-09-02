#include "bfci.h"
#include <unistd.h>

int main(int argc, char **argv){
    ctxObjT Ctx = NULL;
    uint filecount = 0;
    bool given_option_i = FALSE;
    char* source = "bfSrcs/helloworld.bf";
    char* string = "";
    int option;

    opterr = 0;

    if (argc < 2) {
        printHelp();
    }

    while ((option = getopt(argc, argv, "hf:i:")) != -1) {
        switch (option){
            case 'h':
                printHelp();
                break;

            case 'f':
                filecount++;
                if (given_option_i) {
                    fprintf(stderr, "Using both '%s' and '%s' isn't currently possible!\n", "-i", "-t");
                    break;
                }
                source = optarg;
                break;

            case 'i':
                given_option_i = TRUE;
                if(filecount > 0) {
                    fprintf(stderr, "Using both '%s' and '%s' isn't currently possible!\n", "-i", "-t");
                    break;
                }
                string = optarg;
                break;

            case '?':
                switch(optopt){
                    case 'f':
                        fprintf(stderr, "Option '-%c' requires %s argument!\n", optopt, "filepath");
                        break;
                        
                    case 'i':
                        fprintf(stderr, "Option '-%c' requires %s argument!\n", optopt , "string");
                        break;

                    default:
                        fprintf(stderr, "Unknown option '-%c'!\n", optopt);
                }
        }
    }

    if (given_option_i && filecount > 0){
        return NDFUSAGE;
    }

    if (given_option_i) {
        Ctx = initCtx(NULL, DATAMAXLEN, 0);
        StrToIns(Ctx, string);
    }
    if (filecount > 0) {
        Ctx = initCtx(source, DATAMAXLEN, 0);
    }

   /* BIT_SET_TRUE(Ctx->flags, CTX_RUNNING);
    * BIT_SET_FALSE(Ctx->flags, CTX_COMPLETED);
    * BIT_TOGGLE(Ctx->flags, DATA_ALLOW_LOOPED);

    * char* string = "TEST +-,.[]>< abcdABCD0123456789";
    */

    printCtx(Ctx);
    
    freeCtx(Ctx);

    return SUCCESS;
}
