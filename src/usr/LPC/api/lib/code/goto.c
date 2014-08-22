# include "code.h"

inherit Statement;


private string label;	/* goto label */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize goto statement
 */
static void create(string str, varargs int line)
{
    ::create(line);
    label = str;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for goto statement
 */
void code()
{
    emit("goto " + label + ";", line());
}
