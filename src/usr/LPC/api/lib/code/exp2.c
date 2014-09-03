# include "code.h"
# include "expression.h"

inherit Expression;


private Expression sub1, sub2;	/* sub expressions */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize binary expression
 */
static void create(int operand, Expression exp1, Expression exp2,
		   varargs int line)
{
    ::create(operand, line);
    sub1 = exp1;
    sub2 = exp2;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for binary expression
 */
void code()
{
    emit("(");
    sub1->code();
    switch ((int) value()) {
    case EXP_ADD:
	emit("+");
	break;

    case EXP_AND:
	emit("&");
	break;

    case EXP_ASSIGN:
	emit("=");
	break;

    case EXP_ASSIGN_ADD:
	emit("+=");
	break;

    case EXP_ASSIGN_AND:
	emit("&=");
	break;

    case EXP_ASSIGN_DIV:
	emit("/=");
	break;

    case EXP_ASSIGN_LSHIFT:
	emit("<<=");
	break;

    case EXP_ASSIGN_MOD:
	emit("%=");
	break;

    case EXP_ASSIGN_MULT:
	emit("*=");
	break;

    case EXP_ASSIGN_OR:
	emit("|=");
	break;

    case EXP_ASSIGN_RSHIFT:
	emit(">>=");
	break;

    case EXP_ASSIGN_SUB:
	emit("-=");
	break;

    case EXP_ASSIGN_XOR:
	emit("^=");
	break;

    case EXP_COMMA:
	emit(",");
	break;

    case EXP_DIV:
	emit("/");
	break;

    case EXP_EQ:
	emit("==");
	break;

    case EXP_GE:
	emit(">=");
	break;

    case EXP_GT:
	emit(">");
	break;

    case EXP_INDEX:
	emit("[");
	sub2->code();
	emit("])");
	return;

    case EXP_INSTANCEOF:
	emit("<-");
	break;

    case EXP_LAND:
	emit("&&");
	break;

    case EXP_LE:
	emit("<=");
	break;

    case EXP_LOR:
	emit("||");
	break;

    case EXP_LSHIFT:
	emit(">>");
	break;

    case EXP_LT:
	emit("<");
	break;

    case EXP_MOD:
	emit("%");
	break;

    case EXP_MULT:
	emit("*");
	break;

    case EXP_NE:
	emit("!=");
	break;

    case EXP_OR:
	emit("|");
	break;

    case EXP_RSHIFT:
	emit(">>");
	break;

    case EXP_SUB:
	emit("-");
	break;

    case EXP_XOR:
	emit("^");
	break;
    }
    sub2->code();
    emit(")");
}
