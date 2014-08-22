# include "code.h"

inherit Statement;


/*
 * NAME:	code()
 * DESCRIPTION:	emit code for continue statement
 */
void code()
{
    emit("continue;", line());
}
