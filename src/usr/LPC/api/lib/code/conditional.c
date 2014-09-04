# include "code.h"

inherit Statement;


private Expression condExpression;	/* conditional */
private Statement ifStatement;		/* if conditional is true */
private Statement elseStatement;	/* if conditional is false */

/*
 * NAME:	create()
 * DESCRIPTION:	iniitialize if statement
 */
static void create(Expression exp, Statement stmt1, Statement stmt2,
		   varargs int line)
{
    ::create(line);
    condExpression = exp;
    ifStatement = stmt1;
    elseStatement = stmt2;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit if statement
 */
void code()
{
    emit("if");
    condExpression->code();
    emit("{");
    ifStatement->code();
    emit("}");
    if (elseStatement) {
	emit("else ");
	elseStatement->code();
    }
}
