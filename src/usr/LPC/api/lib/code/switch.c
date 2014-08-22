# include "code.h"

inherit Statement;


private Expression expression;	/* switch expression */
private StmtBlock statement;	/* switch code */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize switch statement
 */
static void create(Expression exp, StmtBlock stmt, varargs int line)
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
    emit("switch");
    expression->code();
    statement->code();
}
