# include "code.h"
# include "expression.h"

inherit LPCExpression;


private string name;	/* variable name */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize variable reference
 */
static void create(string str, varargs int line)
{
    ::create(LPC_EXP_VAR, line);
    name = str;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for variable reference
 */
void code()
{
    emit(name, line());
}
