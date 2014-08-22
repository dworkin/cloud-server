# include "code.h"

inherit Statement;


private Expression initial, condition, increment;	/* expressions */
private Statement statement;				/* statement */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize for loop
 */
static void create(Expression exp1, Expression exp2, Expression exp3,
		   Statement stmt, varargs int line)
{
    ::create(line);
    initial = exp1;
    condition = exp2;
    increment = exp3;
    statement = stmt;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for for loop
 */
void code()
{
    emit("for(");
    if (initial) {
	initial->code();
    }
    emit(";");
    if (condition) {
	condition->code();
    }
    emit(";");
    if (increment) {
	increment->code();
    }
    emit(")");
    statement->code();
}
