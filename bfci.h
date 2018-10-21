#ifndef BFCI_H
#define BFCI_H 
#include <assert.h>
#include <stdlib.h>
#include  <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tcolors.h"

enum {
    false = 0,
    true = 1
};

/* VALUE DEFINITIONS */
#define SUCCESS      2
#define FAIL        -2
#define TRUE         1
#define FALSE        0 
#define EXIT         0
#define DATAMAX    255
#define DATAMIN      0
#define ALLOCJMP     5
#define DATAMAXLEN 30000

/* ERROR DEFINITONS */
#define OVERFLOW    -3
#define UNDERFLOW   -4
#define NOLOOPEND   -5
#define NOLOOPSTART -6
#define ALLOCFAIL   -7
#define FILEFAIL    -8
#define NDFINS      -9
#define NDFUSAGE    -10
#define PASTBOUNDS  -11

/* INSTRUCTION SET OPCODES*/
#define   MVR '>'
#define   MVL '<'
#define   INC '+'
#define   DEC '-'
#define  STDO '.'
#define  STDI ','
#define WHILE '['
#define    DO ']'

/* MACROS */
#define BIT_TOGGLE(NUM, N)    (NUM ^= (N))
#define BIT_SET_TRUE(NUM, N)  (NUM |= (N))
#define BIT_SET_FALSE(NUM, N) (NUM &= ~(N))

#define att(type)             __attribute__((type))

#define CASE_PASS(name) fprintf(stderr, cGREEN "PASSED: %s: %s" cNO "\n", __func__, name)
#define CASE_FAIL(name) fprintf(stderr,   cERR "FAILED: %s: %s at %s:%d" cNO "\n", __func__, name, __FILE__, __LINE__)
/* little hack to do two things atst in false branch, fragile and not really robust
 * - when using ASSERT() there needs to be bool 'passed' variable as this macro relies on that
 */
#define ASSERT(x, name)   (x) ? CASE_PASS(name) : CASE_FAIL(name) | (passed = false);
#define NASSERT(x, name)  ASSERT(!(x), name)

/*  FLAGS DEFINITIONS */
/* int size is at least 16 bits */
typedef enum {
    /* states of machine itself */
    /* if you are using vim, theres a cute lil way of doing these "lists" with g Ctrl+a ;-) */
    /* TODO Will be used in future for async execution */
    CTX_RUNNING              =       1,
    CTX_COMPLETED            = 1 <<  2,
    DATA_PENDING_OUT         = 1 <<  3,
    DATA_PENDING_IN          = 1 <<  4,

    /* Allowed to be set by user */
    DATA_ALLOW_LOOPED        = 1 <<  5,  
    DATA_ALLOW_OVERFLOW      = 1 <<  6,  
    DATA_ALLOW_UNDERFLOW     = 1 <<  7,  
    DATA_DYNAMIC_GROW        = 1 <<  8, 
    PRINT_DIAGNOSTICS        = 1 <<  9,
    TEST                     = 1 << 10,
    TEST_STRICT              = 1 << 11,

} ctxFlags;

/* DATA TYPES DECALRATIONS */
typedef unsigned char uchar;
typedef unsigned int uint;
typedef char bool;

/* {*}Obj are always pointers and need to be initialized */
typedef struct _ctxObjT     *ctxObjT;
typedef struct _dataObjT    *dataObjT;
typedef struct _insObjT     *insObjT;
typedef struct _stackObjT   *stackObjT;

typedef struct _stackCellT  stackCellT;
typedef struct _insSetT     insSetT;

typedef int    (*cmd)       (ctxObjT Ctx);
typedef bool   (testfunc)   (void);

/*
 * _ctxObjT      -- Object encapsulating source code and data
 */
struct _ctxObjT {
    insObjT ins;            /* instruction object containing source code to execute */
    dataObjT data;          /* data object containing data over which instruction operate */
    stackObjT stack;        /* stack object used to store stacked bracket indexes */
    uint flags;             /* flags containing states of machine */
};

/* TODO - add min and max and figure out how to do it when min and max overlaps
 * _DataObjT    -- Object containing data over which InsTape operates
 */
struct _dataObjT{
    uchar *tape;    /* data array */                              
    size_t index;           /* index of current data block */
    size_t usedlen;         /* furthest visited array index + 1*/                 
    size_t len;             /* length of array */                   
};

/*
 * InsObjT      -- Object containing BrainFuck source code
 */
struct _insObjT{
    char *tape;             /* instruction array */                              
    size_t index;           /* index of current instruction */
    size_t usedlen;         /* furthest visited array index + 1*/                 
    size_t len;             /* length of array */                   
    char* srcpath;          /* filepath of inputed BrainFuck source code */
    bool jumped;            /* whether current instruction was jumped on by previous one */
};

/*
 * _stackCell   -- Indexable starting and ending brackets in stackObjT
 */
struct _stackCellT {
    size_t start,           /* index of opening bracket, as WHILE in insSet */
           end;             /* index of closing bracket, as END in insSet */
};

/*
 * _stackObjT   -- Object containing 
 */
struct _stackObjT {
    size_t len;             /* number of cells in stack array */
    stackCellT *tape;       /* stack array containing bracket pairs */
};

/* 
 * _InsSetT     -- Information about BrainFuck instruction set 
 */
struct _insSetT {
    const char *name;       /* name of operation or NULL at end of object */
    char opcode;            /* identifier of specific operation */
    cmd command;            /* operation specific function to call */
};


/* FUNCTION DECLARATIONS */
ctxObjT initCtx(const char* srcFilePath, size_t datalen, uint flags);
void printCtx(ctxObjT Ctx);
void freeCtx(ctxObjT Ctx);
char* StrToIns(ctxObjT Ctx, const char* string);
void printHelp();
int interpret(ctxObjT Ctx);
bool test();

/*void handleArgs(int argc, char** argv,
                att(unused) uint* flags,
                uint* filecount,
                bool* givenstring,
                char* source,
                char* string);
*/


#endif
