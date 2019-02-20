# include "code.h"

inherit LPCStatement;


private string label;	/* goto label */

/*
 * initialize goto statement
 */
static void create(string str, varargs int line)
{
    ::create(line);
    label = str;
}

/*
 * emit code for goto statement
 */
void code(void)
{
    emit("goto " + label + ";", line());
}
