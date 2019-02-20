# include "code.h"

inherit LPCStatement;


private LPCStatement statement;		/* statement in loop */
private LPCExpression loopCondition;	/* loop condition */

/*
 * initialize do-while statement
 */
static void create(LPCStatement stmt, LPCExpression exp, varargs int line)
{
    ::create(line);
    statement = stmt;
    loopCondition = exp;
}

/*
 * emit code for do-while statement
 */
void code(void)
{
    emit("do ");
    statement->code();
    emit("while(");
    loopCondition->code();
    emit(");");
}
