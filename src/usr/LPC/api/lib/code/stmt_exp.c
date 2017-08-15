# include "code.h"

inherit LPCStatement;


private LPCExpression expression;	/* expression */

/*
 * initialize expression statement
 */
static void create(LPCExpression exp, varargs int line)
{
    ::create(line);
    expression = exp;
}

/*
 * emit code for expression statement
 */
void code()
{
    expression->code();
    emit(";");
}
