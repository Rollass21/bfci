#ifndef BFCI_H
#define BFCI_H 
#include <stdlib.h>
#include <stdio.h>

/* VALUE DEFINITIONS */
#define SUCCESS 0
#define FAIL -1
#define TRUE 0
#define FALSE -1
#define DATAMAX 255
#define DATAMIN 0
#define ALLOCJMP 10
#define DTMAXLEN 30000

/* ERROR DEFINITONS */
#define OVERFLOW -2
#define UNDERFLOW -3
#define NOLOOPEND -4
#define NOLOOPSTART -5
#define ALLOCFAIL -6
#define FILEFAIL -7
#define NDFINS -8
#define NDFUSAGE -9

/* INSTRUCTION SET DEFINITION*/
#define  MV_R '>'
#define  MV_L '<'
#define   INC '+'
#define   DEC '-'
#define STD_O '.'
#define STD_I ','
#define WHILE '['
#define   END ']'

/* INStruction Set LENgth */
#define  INSSLEN 8

/* DATA TYPES DEFINITION */
/* INSTRUCTION SET ARRAY */
typedef int *InsSetPtr;

/* DATA TAPE */
typedef struct _DataTape DataTape;
typedef DataTape *DataTapePtr;
struct _DataTape{
    unsigned char *tape;
    unsigned int index,
                 len,
                 maxWrite,
                 minWrite;
};

/* INSTRUCTION TAPE */
typedef struct _InsTape InsTape;
typedef InsTape *InsTapePtr;
struct _InsTape{
    int *tape;
    unsigned int index,
                 usedlen,
                 len;
};

/* TAPES */
typedef struct _Tapes Tapes;
typedef Tapes *TapesPtr;
struct _Tapes{
    InsTapePtr ins;
    DataTapePtr data;
};

/* FUNCTION DECLARATION */
TapesPtr initTapes();
InsSetPtr initInsSet();
void freeTapes(TapesPtr tape);
void printDiagnostics(TapesPtr tape, InsSetPtr innset);
int getsrc(const char *source, TapesPtr tape, InsSetPtr insset);
int changeval(TapesPtr tape);

#endif
