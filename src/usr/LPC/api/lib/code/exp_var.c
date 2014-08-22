# include "code.h"
# include "expression.h"

inherit Expression;


private string name;	/* variable name */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize variable reference
 */
static void create(string str, varargs int line)
{
    ::create(EXP_VAR, line);
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
