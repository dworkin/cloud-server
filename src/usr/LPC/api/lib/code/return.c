# include "code.h"

inherit Statement;


/*
 * NAME:	code()
 * DESCRIPTION:	emit code for return statement
 */
void code()
{
    emit("return;", line());
}
