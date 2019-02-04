#include "bfci.h"

/* If you are adding new ones, dont forget to 'extern' refrence them into bfci.h */

/* DONT DELETE! is referenced when newCtx settings argument is NULL!
 * Default (blank) preset of settings used as fallback. Is chosen automatically on NULL as settings argument in newCtx() 
 */
settingsT
defaults = {
    .flags = {
        .ins   = 0,
        .data  = 0,
        .ctx   = 0,
    }
};

/* Used for debugging, outputs contents of internal objects*/
settingsT
debug = {
    .flags = {
        .ins   = 0,
        .data  = 0,
        .ctx   = DEBUG_TEST | DEBUG_TEST_STRICT | DEBUG_PRINT_DIAGNOSTICS,
    }
};

