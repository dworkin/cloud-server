# include "code.h"

inherit Statement;


private Expression caseExpression;	/* case label */
private Statement statement;		/* case code */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize case statement
 /*
static void create(int exp, Statement stmt, varargs int line)
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
