#include "bfci.h"

extern const insSetT insSet[];

size_t getMatchingClosing(insObjT insObj);
int jmp(insObjT insObj, size_t dest);

/* 
 * getMatchingEnd()
 *
 * @insObj - Pointer to structure containing instruction tape
 *
 * @currindex: index of opening bracket in @string in which we want to find a closing bracket
 * @lastindex: last index to be checked, i.e. @string[@lastindex + 1] wont be checked
 * @string: string itself to be checked
 *
 * Returns index of closing bracket paired with the one currently under
 * currindex.
 * If pairing or starting bracket not found, or unvalid arguments, returns 0.
 */
size_t
getMatchingClosing(insObjT insObj){
    if (!insObj){
        return 0; 
    }

    size_t currindex = insObj->index;
    size_t lastindex = insObj->usedlen;
    /* hopefully long int is big enough */
    long int seen = 0;

    /* currindex should point to starting bracket to which we want to find closing */
    if (insSet[insObj->tape[currindex].opIndex].opcode != opc_while){
        return 0;
    }

    /* Search for matching brace until end of string */
    do {
        switch (insSet[insObj->tape[currindex].opIndex].opcode){
            case opc_while:
                        seen++;
                        break;

            case opc_do:
                        seen--;
                        break;
            default:
                        break;
        }
    } while (seen > 0 && ++currindex <= lastindex);
    
    return (seen != 0) ? 0 : currindex;
}

/* 
 * jmp()
 *
 * @tape - ...
 * @dest - Position/Index on which to set instruciton tape index
 *
 * Used to jump between instructions.
 * On correct jump, returns destination index and sets flag jumped.
 */
int
jmp(insObjT insObj, size_t dest){
    if (!insObj){
        return FAIL;
    }
    
    if (dest >= insObj->usedlen) {
        return PASTBOUNDS;
    }

    insObj->index = dest;
    FLAG_SET_TRUE(insObj->flags, INS_JUMPED);

    return SUCCESS;
} 
