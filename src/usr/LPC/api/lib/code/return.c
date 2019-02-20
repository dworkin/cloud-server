# include "code.h"

inherit LPCStatement;


/*
 * emit code for return statement
 */
void code(void)
{
    emit("return;", line());
}
