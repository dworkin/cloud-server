# include "code.h"

inherit LPCStatement;


private LPCExpression value;	/* value to return */

/*
 * initialize return statement
 */
static void create(LPCExpression exp, varargs int line)
{
    ::create(line);
    value = exp;
}

/*
 * emit code for return statement
 */
void code()
{
    emit("return ");
    value->code();
    emit(";");
}
