# include "code.h"

inherit LPCStatement;


private LPCExpression caseExpression;	/* case label */
private LPCStatement statement;		/* case code */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize case statement
 */
static void create(LPCExpression exp, LPCStatement stmt, varargs int line)
{
    ::create(line);
    caseExpression = exp;
    statement = stmt;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for case statement
 */
void code()
{
    emit("case ");
    caseExpression->code();
    emit(":");
    statement->code();
}
