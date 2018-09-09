#include "bfci.h"

int main(int argc, char **argv){
    ctxObjT Ctx = NULL;
    uint filecount = 0;
    bool given_option_i = FALSE;
    char* source = "bfSrcs/helloworld.bf";
    char* string = "";
    uint flags = PRINT_DIAGNOSTICS;
    int option;

    opterr = 0;

    if (argc < 2) {
        printHelp();
    }

    //TODO add a way to alter flags through cli
    while ((option = getopt(argc, argv, "ht:i:")) != -1) {
        switch (option){
            case 'h':
                printHelp();
                break;

            case 't':
                filecount++;
                if (given_option_i) {
                    fprintf(stderr, cERR "Using both '%s' and '%s' isn't currently possible!\n" cNO, "-i", "-t");
                    break;
                }
                source = optarg;
                break;

            case 'i':
                given_option_i = TRUE;
                if(filecount > 0) {
                    fprintf(stderr, cERR "Using both '%s' and '%s' isn't currently possible!\n", "-i" cNO, "-t");
                    break;
                }
                string = optarg;
                break;

            case '?':
                switch(optopt){
                    case 't':
                        fprintf(stderr, cERR "Option '-%c' requires %s argument!\n" cNO, optopt, "filepath");
                        break;
                        
                    case 'i':
                        fprintf(stderr, cERR "Option '-%c' requires %s argument!\n" cNO, optopt , "string");
                        break;

                    default:
                        fprintf(stderr, cERR "Unknown option '-%c'!\n" cNO, optopt);
                }
        }
    }

    if (given_option_i && filecount > 0){
        return NDFUSAGE;
    }

    if (given_option_i) {
        Ctx = initCtx(NULL, DATAMAXLEN, flags);
        StrToIns(Ctx, string);
        interpret(Ctx);
    }
    if (filecount > 0) {
        Ctx = initCtx(source, DATAMAXLEN, flags);
        interpret(Ctx);
    }

    freeCtx(Ctx);

    return SUCCESS;
}
