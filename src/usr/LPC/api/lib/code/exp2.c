# include "code.h"
# include "expression.h"

inherit LPCExpression;


private LPCExpression sub1, sub2;	/* sub expressions */

/*
 * initialize binary expression
 */
static void create(int operand, LPCExpression exp1, LPCExpression exp2,
		   varargs int line)
{
    ::create(operand, line);
    sub1 = exp1;
    sub2 = exp2;
}

/*
 * emit code for binary expression
 */
void code()
{
    emit("(");
    sub1->code();
    switch ((int) value()) {
    case LPC_EXP_ADD:
	emit("+");
	break;

    case LPC_EXP_AND:
	emit("&");
	break;

    case LPC_EXP_ASSIGN:
	emit("=");
	break;

    case LPC_EXP_ASSIGN_ADD:
	emit("+=");
	break;

    case LPC_EXP_ASSIGN_AND:
	emit("&=");
	break;

    case LPC_EXP_ASSIGN_DIV:
	emit("/=");
	break;

    case LPC_EXP_ASSIGN_LSHIFT:
	emit("<<=");
	break;

    case LPC_EXP_ASSIGN_MOD:
	emit("%=");
	break;

    case LPC_EXP_ASSIGN_MULT:
	emit("*=");
	break;

    case LPC_EXP_ASSIGN_OR:
	emit("|=");
	break;

    case LPC_EXP_ASSIGN_RSHIFT:
	emit(">>=");
	break;

    case LPC_EXP_ASSIGN_SUB:
	emit("-=");
	break;

    case LPC_EXP_ASSIGN_XOR:
	emit("^=");
	break;

    case LPC_EXP_COMMA:
	emit(",");
	break;

    case LPC_EXP_DIV:
	emit("/");
	break;

    case LPC_EXP_EQ:
	emit("==");
	break;

    case LPC_EXP_GE:
	emit(">=");
	break;

    case LPC_EXP_GT:
	emit(">");
	break;

    case LPC_EXP_INDEX:
	emit("[");
	sub2->code();
	emit("])");
	return;

    case LPC_EXP_INSTANCEOF:
	emit("<-");
	break;

    case LPC_EXP_LAND:
	emit("&&");
	break;

    case LPC_EXP_LE:
	emit("<=");
	break;

    case LPC_EXP_LOR:
	emit("||");
	break;

    case LPC_EXP_LSHIFT:
	emit(">>");
	break;

    case LPC_EXP_LT:
	emit("<");
	break;

    case LPC_EXP_MOD:
	emit("%");
	break;

    case LPC_EXP_MULT:
	emit("*");
	break;

    case LPC_EXP_NE:
	emit("!=");
	break;

    case LPC_EXP_OR:
	emit("|");
	break;

    case LPC_EXP_RSHIFT:
	emit(">>");
	break;

    case LPC_EXP_SUB:
	emit("-");
	break;

    case LPC_EXP_XOR:
	emit("^");
	break;
    }
    sub2->code();
    emit(")");
}
