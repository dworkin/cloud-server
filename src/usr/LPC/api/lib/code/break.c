# include "code.h"

inherit LPCStatement;


/*
 * emit code for break statement
 */
void code(void)
{
    emit("break;", line());
}
