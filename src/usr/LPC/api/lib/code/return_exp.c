# include "code.h"

inherit LPCStatement;


private LPCExpression value;	/* value to return */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize return statement
 */
static void create(LPCExpression exp, varargs int line)
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
    emit("return");
    value->code();
    emit(";");
}
