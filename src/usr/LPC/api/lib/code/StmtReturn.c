# include "Code.h"

inherit LPCStatement;


/*
 * emit code for return statement
 */
void code()
{
    emit("return;", line());
}
