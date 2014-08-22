# include "code.h"

inherit Statement;


private Expression value;	/* value to return */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize return statement
 */
static void create(Expression exp, varargs int line)
{
    ::create(line);
    value = exp;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for return statement
 */
void code()
{
    emit("return ");
    value->code();
    emit(";");
}
