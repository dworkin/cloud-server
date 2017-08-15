# include "code.h"

inherit LPCStatement;


private LPCExpression caseFrom, caseTo;	/* case labels */
private LPCStatement statement;		/* case code */

/*
 * initialize case statement
 */
static void create(LPCExpression exp1, LPCExpression exp2, LPCStatement stmt,
		   varargs int line)
{
    ::create(line);
    caseFrom = exp1;
    caseTo = exp2;
    statement = stmt;
}

/*
 * emit code for case expression
 */
void code()
{
    emit("case ");
    caseFrom->code();
    emit("..");
    caseTo->code();
    emit(":");
    statement->code();
}
