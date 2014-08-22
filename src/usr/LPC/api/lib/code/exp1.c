# include "code.h"
# include "expression.h"

inherit Expression;


private Expression sub;		/* sub-expression */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize monadic expression
 */
static void create(int operand, Expression exp, varargs int line)
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
    case EXP_CATCH:
	emit("catch");
	sub->code();
	break;

    case EXP_NEG:
	emit("~");
	sub->code();
	break;

    case EXP_NOT:
	emit("!");
	sub->code();
	break;

    case EXP_UMIN:
	emit("-");
	sub->code();
	break;

    case EXP_UPLUS:
	emit("+");
	sub->code();
	break;

    case EXP_PRE_INCR:
	emit("++");
	sub->code();
	break;

    case EXP_PRE_DECR:
	emit("--");
	sub->code();
	break;

    case EXP_POST_INCR:
	sub->code();
	emit("++", line());
	break;

    case EXP_POST_DECR:
	sub->code();
	emit("--", line());
	break;
    }
    emit(")");
}
