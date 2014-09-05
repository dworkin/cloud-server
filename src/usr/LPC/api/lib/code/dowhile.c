# include "code.h"

inherit LPCStatement;


private LPCStatement statement;		/* statement in loop */
private LPCExpression loopCondition;	/* loop condition */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize do-while statement
 */
static void create(LPCStatement stmt, LPCExpression exp, varargs int line)
{
    ::create(line);
    statement = stmt;
    loopCondition = exp;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for do-while statement
 */
void code()
{
    emit("do ");
    statement->code();
    emit("while");
    loopCondition->code();
    emit(";");
}
