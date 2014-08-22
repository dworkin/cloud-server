# include "code.h"
# include "expression.h"

inherit Expression;


private Expression sub1, sub2, sub3;	/* sub expressions */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize ternary expression
 */
static void create(int operand, Expression exp1, Expression exp2,
		   Expression exp3, varargs int line)
{
    ::create(operand, line);
    sub1 = exp1;
    sub2 = exp2;
    sub3 = exp3;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for ternary expression
 */
void code()
{
    emit("(");
    sub1->code();
    switch ((int) value()) {
    case EXP_QUEST:
	emit("?");
	sub2->code();
	emit(":");
	sub3->code();
	break;

    case EXP_RANGE:
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
