#ifndef BFCI_H
#define BFCI_H 
#include <assert.h>
#include <stdlib.h>
#include  <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <limits.h>

#include "tcolors.h"

enum {
    false = 0,
    true = 1
};

/* VALUE DEFINITIONS */
#define SUCCESS          2
#define FAIL            -2
#define TRUE             1
#define FALSE            0 
#define EXIT             0
#define DATAMAX        255
#define DATAMIN          0
#define DATAALLOCJMP    10
#define DATAMAXLEN   30000
#define INSALLOCJMP     15

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
#define QUIT        -12

/* INSTRUCTION SET OPCODES*/
#define OPC_MVRD  '>'
#define OPC_MVLD  '<'
#define OPC_INCD  '+'
#define OPC_DECD  '-'
#define OPC_STDO  '.'
#define OPC_STDI  ','
#define OPC_WHILE '['
#define OPC_DO    ']'

typedef enum {
    opc_mvrd  = OPC_MVRD,
    opc_mvld  = OPC_MVLD,  
    opc_incd  = OPC_INCD,  
    opc_decd  = OPC_DECD,  
    opc_stdo  = OPC_STDO, 
    opc_stdi  = OPC_STDI, 
    opc_while = OPC_WHILE,
    opc_do    = OPC_DO,   
} opcodeT;

/* MACROS */
#define BIT_TOGGLE(VAR, N)     (VAR ^= (N))
#define BIT_SET_TRUE(VAR, N)   (VAR |= (N))
#define BIT_SET_FALSE(VAR, N)  (VAR &= ~(N))

#define FLAG_TOGGLE(VAR, N)    BIT_TOGGLE(VAR, N);
#define FLAG_SET(VAR, N, VAL)  (VAL) ? BIT_SET_TRUE(VAR, N) : BIT_SET_FALSE(VAR, N);
#define FLAG_SET_TRUE(VAR, N)  FLAG_SET(VAR, N, true)
#define FLAG_SET_FALSE(VAR, N) FLAG_SET(VAR, N, false)

#define att(type)             __attribute__((type))

/*  FLAGS DEFINITIONS */
/* defaults to int with minimum of 2 bytes, for anything bigger use: 
 *  
 * states will be used for runtime checking to for ex.
 *      error out on DATA_OVERFLOW
 */
/* if you are using vim, theres a cute lil way of doing these "lists" with g Ctrl+a ;-) */
/* states of the machine itself */
typedef enum {
    CTX_RUNNING               = 1 <<  1,
    CTX_STOPPED               = 1 <<  2,
    CTX_COMPLETED             = 1 <<  3,
    DEBUG_PRINT_DIAGNOSTICS   = 1 <<  4,
    DEBUG_TEST                = 1 <<  5,
    DEBUG_TEST_STRICT         = 1 <<  6,
} ctxFlagsT;

/* states of the data object */
typedef enum {
    DATA_PENDING_OUT          = 1 <<  1,
    DATA_PENDING_IN           = 1 <<  2,
    DATA_ALLOW_LOOPAROUND     = 1 <<  3,  
    DATA_LOOPED               = 1 <<  4,
    DATA_ALLOW_OVERFLOW       = 1 <<  5,  
    DATA_OVERFLOW             = 1 <<  6,
    DATA_ALLOW_UNDERFLOW      = 1 <<  7,  
    DATA_UNDERFLOW            = 1 <<  8,
    DATA_ALLOW_DYNAMIC_GROW   = 1 <<  9, 
} dataFlagsT;

/* states of the ins object */
typedef enum {
    INS_JUMPED                = 1 <<  0,
    INS_FROM_STRING           = 1 <<  1,
    INS_FROM_FILE             = 1 <<  2,
} insFlagsT;

/* GENERAL DATA TYPEDEFS */
typedef unsigned char uchar;
typedef unsigned int uint;
typedef char     bool;

/* {*}ObjT are always pointers and need to be initialized */
typedef struct _ctxObjT     *ctxObjT;
typedef struct _dataObjT    *dataObjT;
typedef struct _insObjT     *insObjT;

/* Element structures */
typedef struct _insSetT     insSetT;
typedef struct _insCellT    insCellT;

/* Drivers/Helpers */
typedef int     (*cmdT)       (ctxObjT Ctx);
typedef bool    (testfuncT)   (void);
typedef int     (*methodT)    (void*, ...);

typedef struct _flagsettingsT flagsettingsT;
typedef struct _flagsettingsT {
    ctxFlagsT    ctx;
    dataFlagsT   data;
    insFlagsT    ins;
} flagsettingsT;

typedef struct _settingsT settingsT;
typedef struct _settingsT {
    flagsettingsT flags;
} settingsT;

extern settingsT defaults;
extern settingsT debug;

/*
 * _ctxObjT      -- Object encapsulating source code and data
 */
struct _ctxObjT {
    insObjT ins;            /* instruction object containing source code to execute */
    dataObjT data;          /* data object containing data over which instruction operate */
    ctxFlagsT flags;        /* flags containing states of machine */
    settingsT settings;     /* copy of initial settings used to restore ctxObj */
};

/* TODO - add min and max and figure out how to do it when min and max overlaps
 * _DataObjT    -- Object containing data over which InsTape operates
 */
struct _dataObjT{
    uchar *tape;            /* data tape array */                              
    size_t index;           /* index of current data block */
    size_t usedlen;         /* furthest visited array index + 1*/                 
    size_t len;             /* length of array */                   
    dataFlagsT flags;       /* informatinsa/on about the state of dataObj */
};

struct _insCellT {
    uchar opIndex;          /* index of operation in instruction set */
    uint times;             /* how many times is the operation repeated in source */
    size_t matching;        /* index of matching operation (used only for opc_while and opc_do) */
    bool ismatched;
};

/*
 * InsObjT      -- Object containing BrainFuck source code
 */
struct _insObjT{
    insCellT* tape;         /* instruction tape array */                              
    size_t index;           /* index of current instruction */
    size_t usedlen;         /* furthest visited array index */                 
    size_t len;             /* length of array */                   
    char* srcpath;          /* filepath of inputed BrainFuck source code */
    insFlagsT flags;        /* information about the state of insObj */
};

/* 
 * _insSetT     -- Instruction set command specifics
 */
struct _insSetT {
    const char *name;       /* name of operation or NULL at end of instructions list */
    opcodeT opcode;         /* identifier of specific operation */
    cmdT command;           /* operation specific function to call */
    bool RLEable;           /* whether this instruction can be RLEncoded */
};

/* API FUNCTION DECLARATIONS */
ctxObjT newCtx(const char* srcpath, const char* srcstring, size_t datalen, settingsT* settings);
bool isValidCtx(ctxObjT Ctx);
void printCtx(ctxObjT Ctx);
void freeCtx(ctxObjT Ctx);
void printHelp();
int interpret(ctxObjT Ctx);
int addsrc(ctxObjT Ctx, const char* srcpath, const char* srcstring);
int handleArgs(int argc, char** argv, char** srcpath, char** srcstring);

#endif
