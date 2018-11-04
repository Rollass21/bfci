#include "bfci.h"                  

extern size_t getMatchingClosing(insObjT insObj);
extern const insSetT insSet[];

extern int jmp(insObjT insObj, size_t dest);
extern int LIFOPop(ctxObjT Ctx);
extern int LIFOPush(ctxObjT Ctx, size_t startindex, size_t endindex);
extern int LIFOPushNew(ctxObjT Ctx, size_t startindex, size_t endindex);
                                   
int OP_mvRight(ctxObjT);       
int OP_mvLeft (ctxObjT);       
int OP_incData(ctxObjT);       
int OP_decData(ctxObjT);       
int OP_outData(ctxObjT);       
int OP_inpData(ctxObjT);       
int OP_loopBeg(ctxObjT);       
int OP_loopEnd(ctxObjT);       
                                   
/* Instruction set */              
const insSetT insSet[] = {  
    {.name    = "Move right",      
     .opcode  = opc_mvrd,
     .command = OP_mvRight,
    },

    {.name    = "Move left",
     .opcode  = opc_mvld,
     .command = OP_mvLeft,
    },

    {.name    = "Data increment",
     .opcode  = opc_incd,               
     .command = OP_incData,
    },

    {.name    = "Data decrement",
     .opcode  = opc_decd,
     .command = OP_decData,
    },

    {.name    = "Standard output",
     .opcode  = opc_stdo,
     .command = OP_outData,
    },

    {.name    = "Standard input",
     .opcode  = opc_stdi,
     .command = OP_inpData,
    },

    {.name    = "While loop begin",
     .opcode  = opc_while,
     .command = OP_loopBeg,
    },

    {.name    = "While loop end",
     .opcode  = opc_do,
     .command = OP_loopEnd,
    },

    {.name = NULL}
};                                 

/* INSTRUCTION SET FUNCTIONS */

int
OP_mvRight(ctxObjT Ctx){
    /* Loop around */
    if (Ctx->data->index + 1 == Ctx->data->len){
        FLAG_SET_TRUE(Ctx->data->flags, DATA_LOOPED);
        Ctx->data->index = 0;
        return SUCCESS;
    }
    
    // TODO needs to be redone for each tape cell individually
    /* Trace furthest index */
    if (Ctx->data->index + 1 == Ctx->data->usedlen){
        Ctx->data->usedlen++;
    }

    /* Casual move right once everything is figured out */
    FLAG_SET_FALSE(Ctx->data->flags, DATA_LOOPED);
    Ctx->data->index++;

    return SUCCESS;
}

int
OP_mvLeft(ctxObjT Ctx){
    /* Loop around */
    if (Ctx->data->index == 0){
        FLAG_SET_TRUE(Ctx->data->flags, DATA_LOOPED);
        Ctx->data->index = Ctx->data->len - 1;
        Ctx->data->usedlen = Ctx->data->len;
        return SUCCESS;
    }
    
    /* Casual move left once everything is figured out */
    FLAG_SET_FALSE(Ctx->data->flags, DATA_LOOPED);
    Ctx->data->index--;

    return SUCCESS;
}

int 
OP_incData(ctxObjT Ctx){
    /* Value loop around */
    if (Ctx->data->tape[Ctx->data->index] == DATAMAX) {
        FLAG_SET_FALSE(Ctx->data->flags, DATA_UNDERFLOW);
        FLAG_SET_TRUE(Ctx->data->flags, DATA_OVERFLOW);
        Ctx->data->tape[Ctx->data->index] = DATAMIN;
        return SUCCESS;
    }

    /* Casual increment */
    FLAG_SET_FALSE(Ctx->data->flags, DATA_UNDERFLOW & DATA_OVERFLOW);
    Ctx->data->tape[Ctx->data->index]++;

    return SUCCESS;
}

int 
OP_decData(ctxObjT Ctx){
    /* Value loop around */
    if (Ctx->data->tape[Ctx->data->index] == DATAMIN) {
        FLAG_SET_TRUE(Ctx->data->flags, DATA_UNDERFLOW);
        FLAG_SET_FALSE(Ctx->data->flags, DATA_OVERFLOW);
        Ctx->data->tape[Ctx->data->index] = DATAMAX;
        return SUCCESS;
    }

    /* Casual decrement */
    FLAG_SET_FALSE(Ctx->data->flags, DATA_UNDERFLOW & DATA_OVERFLOW);
    Ctx->data->tape[Ctx->data->index]--;

    return SUCCESS;
}

int 
OP_outData(ctxObjT Ctx){
    putchar(Ctx->data->tape[Ctx->data->index]);
    return SUCCESS;
}

int 
OP_inpData(ctxObjT Ctx){
    Ctx->data->tape[Ctx->data->index] = getchar();
    return SUCCESS;
}

int 
OP_loopBeg(ctxObjT Ctx){
    size_t startindex = Ctx->ins->index;
    size_t endindex   = getMatchingClosing(Ctx->ins);
    if (!endindex) {
        return NOLOOPEND;
    }

    LIFOPushNew(Ctx, startindex, endindex);

    /* JUMP RULE */
    /* Jump to closing bracket when data==0 */
    if (Ctx->data->tape[Ctx->data->index] == 0) {
        jmp(Ctx->ins, Ctx->stack->tape[Ctx->stack->len - 1].end); 
    }

    /* if jump rule is FALSE, just let programm continue */
    return SUCCESS;
}

int 
OP_loopEnd(ctxObjT Ctx){
    /* JUMP RULE */
    /* Jump back to stored opening bracket when data!=0 */
    if (Ctx->data->tape[Ctx->data->index] != 0) {
        if (!Ctx->stack->len) {
            return NOLOOPSTART;
        }
        /* if there is openings bracket to jump to, do jmp() */
        jmp(Ctx->ins, Ctx->stack->tape[Ctx->stack->len - 1].start);         
        return SUCCESS;
    }

    LIFOPop(Ctx);
    /* if jump rule is FALSE, just let programm continue */
    return SUCCESS;
}
