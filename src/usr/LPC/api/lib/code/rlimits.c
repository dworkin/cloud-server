# include "code.h"

inherit Statement;


private Expression stack, ticks;	/* resource limits */
private StmtBlock statement;		/* limited code */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize rlimits statement
 */
static void create(Expression exp1, Expression exp2, StmtBlock stmt,
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
