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

/* ENCODED ISNTRUCTION SET */
#define  E_MV_R 0
#define  E_MV_L 1
#define   E_INC 2
#define   E_DEC 3
#define E_STD_O 4
#define E_STD_I 5
#define E_WHILE 6
#define   E_END 7

#define NoIS 8 // Number of InstructionS avaiable

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
                 len;
};

/* TAPES */
typedef struct _Tapes Tapes;
typedef Tapes *TapesPtr;
struct _Tapes{
    InsTape ins;
    DataTape data;
};

/* FUNCTION DECLARATION */
TapesPtr initTapes();
InsSetPtr initInsSet();
int isInstruction(int c, InsSetPtr InsSet);
void freeTapes(TapesPtr tape){
int printData(TapesPtr tape){

#endif
