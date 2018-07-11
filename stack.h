#include <stdio.h>
#include <stdlib.h>

#define SUCCESS   0
#define FAIL     -1
#define TRUE      0
#define FALSE    -1

#define SAMEADDR -2

#define S_last(s) (s != NULL) ? s->array[s->len-1] : NULL;

typedef struct _bracketPair BracketPair;
typedef BracketPair* BracketPairPtr;
struct _bracketPair {
    unsigned int start,
                 end;
};

typedef struct _stack Stack;
typedef Stack* StackPtr;

struct _stack {
    unsigned int len;
    BracketPairPtr array;
};

StackPtr initStack();
unsigned int SPush(StackPtr stack, unsigned int startIndex, unsigned int endIndex);
unsigned int SPop(StackPtr stack);
void freeStack(StackPtr stack);

