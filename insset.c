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
     .RLEable = true,
    },

    {.name    = "Move left",
     .opcode  = opc_mvld,
     .command = OP_mvLeft,
     .RLEable = true,
    },

    {.name    = "Data increment",
     .opcode  = opc_incd,               
     .command = OP_incData,
     .RLEable = true,
    },

    {.name    = "Data decrement",
     .opcode  = opc_decd,
     .command = OP_decData,
     .RLEable = true,
    },

    {.name    = "Standard output",
     .opcode  = opc_stdo,
     .command = OP_outData,
     .RLEable = true,
    },

    {.name    = "Standard input",
     .opcode  = opc_stdi,
     .command = OP_inpData,
     .RLEable = true,
    },

    {.name    = "While loop begin",
     .opcode  = opc_while,
     .command = OP_loopBeg,
     .RLEable = false,
    },

    {.name    = "While loop end",
     .opcode  = opc_do,
     .command = OP_loopEnd,
     .RLEable = false,
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
    size_t dest;

    if (Ctx->ins->tape[Ctx->ins->index].ismatched) {
        dest = Ctx->ins->tape[Ctx->ins->index].matching;
    }
    else {
        dest = getMatchingClosing(Ctx->ins);
        if (!dest)
            return NOLOOPEND;
        Ctx->ins->tape[Ctx->ins->index].matching = dest;
        Ctx->ins->tape[Ctx->ins->index].ismatched = true;

        Ctx->ins->tape[dest].matching = Ctx->ins->index;
        Ctx->ins->tape[dest].ismatched = true;
    }
            
    /* JUMP RULE */
    /* Jump to closing bracket when data==0 */
    if (Ctx->data->tape[Ctx->data->index] == 0) {
        jmp(Ctx->ins, dest); 
    }

    /* if jump rule is FALSE, just let programm continue */
    return SUCCESS;
}

int 
OP_loopEnd(ctxObjT Ctx){
    /* JUMP RULE */
    /* Jump back to stored opening bracket when data!=0 */
    if (Ctx->data->tape[Ctx->data->index] != 0) {
        if (!Ctx->ins->tape[Ctx->ins->index].ismatched) {
            return NOLOOPSTART;
        }
        /* if there is openings bracket to jump to, do jmp() */
        jmp(Ctx->ins, Ctx->ins->tape[Ctx->ins->index].matching);         
        return SUCCESS;
    }

    /* if jump rule is FALSE, just let programm continue */
    return SUCCESS;
}
