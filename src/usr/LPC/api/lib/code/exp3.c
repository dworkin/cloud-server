# include "code.h"
# include "expression.h"

inherit LPCExpression;


private LPCExpression sub1, sub2, sub3;	/* sub expressions */

/*
 * initialize ternary expression
 */
static void create(int operand, LPCExpression exp1, LPCExpression exp2,
		   LPCExpression exp3, varargs int line)
{
    ::create(operand, line);
    sub1 = exp1;
    sub2 = exp2;
    sub3 = exp3;
}

/*
 * emit code for ternary expression
 */
void code()
{
    emit("(");
    sub1->code();
    switch ((int) value()) {
    case LPC_EXP_QUEST:
	emit("?");
	sub2->code();
	emit(":");
	sub3->code();
	break;

    case LPC_EXP_RANGE:
	emit("[");
	if (sub2) {
	    sub2->code();
	}
	emit("..");
	if (sub3) {
	    sub3->code();
	}
	emit("]");
	break;
    }
    emit(")");
}
