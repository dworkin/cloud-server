# include "code.h"

inherit LPCStatement;


/*
 * emit code for break statement
 */
void code()
{
    emit("break;", line());
}
