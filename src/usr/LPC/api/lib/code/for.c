# include "code.h"

inherit LPCStatement;


private LPCExpression initial, condition, increment;	/* expressions */
private LPCStatement statement;				/* statement */

/*
 * initialize for loop
 */
static void create(LPCExpression exp1, LPCExpression exp2, LPCExpression exp3,
		   LPCStatement stmt, varargs int line)
{
    ::create(line);
    initial = exp1;
    condition = exp2;
    increment = exp3;
    statement = stmt;
}

/*
 * emit code for for loop
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
