# include "code.h"

inherit Statement;


private Statement statement;		/* statement in loop */
private Expression loopCondition;	/* loop condition */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize do-while statement
 */
static void create(Statement stmt, Expression exp, varargs int line)
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
}
