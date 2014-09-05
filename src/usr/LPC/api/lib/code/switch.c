# include "code.h"

inherit LPCStatement;


private LPCExpression expression;	/* switch expression */
private LPCStmtBlock statement;		/* switch code */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize switch statement
 */
static void create(LPCExpression exp, LPCStmtBlock stmt, varargs int line)
{
    ::create(line);
    expression = exp;
    statement = stmt;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for switch statement
 */
void code()
{
    emit("switch(");
    expression->code();
    emit(")");
    statement->code();
}
