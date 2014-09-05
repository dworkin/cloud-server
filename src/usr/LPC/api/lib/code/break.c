# include "code.h"

inherit LPCStatement;


/*
 * NAME:	code()
 * DESCRIPTION:	emit code for break statement
 */
void code()
{
    emit("break;", line());
}
