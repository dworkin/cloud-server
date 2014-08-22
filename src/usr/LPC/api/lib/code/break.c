# include "code.h"

inherit Statement;


/*
 * NAME:	code()
 * DESCRIPTION:	emit code for break statement
 */
void code()
{
    emit("break;", line());
}
