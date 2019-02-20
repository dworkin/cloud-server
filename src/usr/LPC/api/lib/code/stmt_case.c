# include "code.h"

inherit LPCStatement;


private LPCExpression caseExpression;	/* case label */
private LPCStatement statement;		/* case code */

/*
 * initialize case statement
 */
static void create(LPCExpression exp, LPCStatement stmt, varargs int line)
{
    ::create(line);
    caseExpression = exp;
    statement = stmt;
}

/*
 * emit code for case statement
 */
void code(void)
{
    emit("case ");
    caseExpression->code();
    emit(":");
    statement->code();
}
