# include "code.h"

inherit LPCStatement;


/*
 * emit code for continue statement
 */
void code(void)
{
    emit("continue;", line());
}
