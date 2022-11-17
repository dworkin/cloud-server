# include "Code.h"

inherit LPCStatement;


/*
 * emit code for continue statement
 */
void code()
{
    emit("continue;", line());
}
