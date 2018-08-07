#ifndef BFCI_H
#define BFCI_H 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* VALUE DEFINITIONS */
#define SUCCESS      0
#define FAIL        -2
#define TRUE         0
#define FALSE       -1
#define DATAMAX    255
#define DATAMIN      0
#define ALLOCJMP     5
#define DTMAXLEN 30000

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
#define   END ']'

/* DATA TYPES DECALRATIONS */
typedef int bool;
typedef unsigned int uint;

/* {*}Obj are always pointers and need to be initialized */
typedef struct _ctxObjT     *ctxObjT;
typedef struct _dataObjT   *dataObjT;
typedef struct _insObjT    *insObjT;
typedef struct _stackObjT  *stackObjT;

typedef struct _stackCellT  stackCellT;
typedef struct _insSetT     insSetT;
typedef int    (*cmd)       (ctxObjT Ctx, uint flag);

/*
 * _ctxObjT      -- Object encapsulating source code and data
 */
struct _ctxObjT {
    insObjT ins;            /* instruction object containing source code to execute */
    dataObjT data;          /* data object containing data over which instruction operate */
    stackObjT stack;        /* stack object used to store stacked bracket indexes */
    uint flags;             /*  */
};

/*
 * _DataObjT    -- Object containing data over which InsTape operates
 */
struct _dataObjT{
    unsigned char *tape;    /* data array */                              
    size_t index;           /* index of current data block */
    size_t usedlen;         /* furthest visited array index */                 
    size_t len;             /* length of array */                   
};

/*
 * InsObjT      -- Object containing BrainFuck source code
 */
struct _insObjT{
    unsigned char *tape;    /* instruction array */                              
    size_t index;           /* index of current instruction */
    size_t usedlen;         /* furthest visited array index */                 
    size_t len;             /* length of array */                   
    char* srcdest;          /* filename of inputed BrainFuck source code */
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
    size_t len;             /* number of cells in stack array*/
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
ctxObjT initCtx(const char* srcFileName, size_t datalen, uint flags);
void printCtx(ctxObjT Ctx);
void clearCtx(ctxObjT Ctx);

#endif
