# include "code.h"

inherit LPCStatement;


/*
 * emit code for continue statement
 */
void code()
{
    emit("continue;", line());
}
