# include "code.h"

inherit LPCStatement;


/*
 * NAME:	code()
 * DESCRIPTION:	emit code for continue statement
 */
void code()
{
    emit("continue;", line());
}
