# include "code.h"

inherit Statement;


private Expression expression;	/* expression */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize expression statement
 */
static void create(Expression exp, varargs int line)
{
    ::create(line);
    expression = exp;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for expression statement
 */
void code()
{
    expression->code();
    emit(";");
}
