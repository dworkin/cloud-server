# include "code.h"
# include "expression.h"

inherit LPCExpression;


private LPCType cast;		/* type to cast to */
private LPCExpression sub;	/* expression to cast */

/*
 * initialize cast expression
 */
static void create(LPCType type, LPCExpression exp, varargs int line)
{
    ::create(LPC_EXP_CAST, line);
    cast = type;
    sub = exp;
}

/*
 * emit code for cast expression
 */
void code(void)
{
    emit("((");
    cast->code();
    emit(")");
    sub->code();
    emit(")");
}
