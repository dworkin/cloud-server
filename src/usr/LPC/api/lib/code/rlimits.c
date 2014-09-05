# include "code.h"

inherit LPCStatement;


private LPCExpression stack, ticks;	/* resource limits */
private LPCStmtBlock statement;		/* limited code */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize rlimits statement
 */
static void create(LPCExpression exp1, LPCExpression exp2, LPCStmtBlock stmt,
		   varargs int line)
{
    ::create(line);
    stack = exp1;
    ticks = exp2;
    statement = stmt;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for rlimits statement
 */
void code()
{
    emit("rlimits(");
    stack->code();
    emit(";");
    ticks->code();
    emit(")");
    statement->code();
}
