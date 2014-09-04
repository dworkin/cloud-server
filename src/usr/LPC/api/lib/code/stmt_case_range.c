# include "code.h"

inherit Statement;


private Expression caseFrom, caseTo;	/* case labels */
private Statement statement;		/* case code */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize case statement
 */
static void create(Expression exp1, Expression exp2, Statement stmt,
		   varargs int line)
{
    ::create(line);
    caseFrom = exp1;
    caseTo = exp2;
    statement = stmt;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for case expression
 */
void code()
{
    emit("case");
    caseFrom->code();
    emit("..");
    caseTo->code();
    emit(":");
    statement->code();
}
