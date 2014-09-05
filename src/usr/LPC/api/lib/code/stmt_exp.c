# include "code.h"

inherit LPCStatement;


private LPCExpression expression;	/* expression */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize expression statement
 */
static void create(LPCExpression exp, varargs int line)
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
