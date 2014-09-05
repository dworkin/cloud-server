# include "code.h"
# include "expression.h"

inherit LPCExpression;


private LPCExpression sub;	/* sub-expression */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize monadic expression
 */
static void create(int operand, LPCExpression exp, varargs int line)
{
    ::create(operand, line);
    sub = exp;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for monadic expression
 */
void code()
{
    emit("(");
    switch ((int) value()) {
    case LPC_EXP_CATCH:
	emit("catch");
	sub->code();
	break;

    case LPC_EXP_NEG:
	emit("~");
	sub->code();
	break;

    case LPC_EXP_NOT:
	emit("!");
	sub->code();
	break;

    case LPC_EXP_UMIN:
	emit("-");
	sub->code();
	break;

    case LPC_EXP_UPLUS:
	emit("+");
	sub->code();
	break;

    case LPC_EXP_PRE_INCR:
	emit("++");
	sub->code();
	break;

    case LPC_EXP_PRE_DECR:
	emit("--");
	sub->code();
	break;

    case LPC_EXP_POST_INCR:
	sub->code();
	emit("++", line());
	break;

    case LPC_EXP_POST_DECR:
	sub->code();
	emit("--", line());
	break;
    }
    emit(")");
}
