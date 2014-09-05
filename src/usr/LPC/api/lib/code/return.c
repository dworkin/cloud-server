# include "code.h"

inherit LPCStatement;


/*
 * NAME:	code()
 * DESCRIPTION:	emit code for return statement
 */
void code()
{
    emit("return;", line());
}
