# include "Code.h"

inherit LPCStatement;


private LPCExpression expression;	/* switch expression */
private LPCStmtBlock statement;		/* switch code */

/*
 * initialize switch statement
 */
static void create(LPCExpression exp, LPCStmtBlock stmt, varargs int line)
{
    ::create(line);
    expression = exp;
    statement = stmt;
}

/*
 * emit code for switch statement
 */
void code()
{
    emit("switch(");
    expression->code();
    emit(")");
    statement->code();
}
