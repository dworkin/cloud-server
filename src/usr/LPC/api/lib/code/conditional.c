# include "code.h"

inherit LPCStatement;


private LPCExpression condExpression;	/* conditional */
private LPCStatement ifStatement;	/* if conditional is true */
private LPCStatement elseStatement;	/* if conditional is false */

/*
 * NAME:	create()
 * DESCRIPTION:	iniitialize if statement
 */
static void create(LPCExpression exp, LPCStatement stmt1, LPCStatement stmt2,
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
    emit("if(");
    condExpression->code();
    emit("){");
    ifStatement->code();
    emit("}");
    if (elseStatement) {
	emit("else ");
	elseStatement->code();
    }
}
