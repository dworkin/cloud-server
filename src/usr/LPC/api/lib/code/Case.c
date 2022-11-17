# include "Code.h"

inherit LPCEntry;


private LPCExpression caseExpression;	/* case expression */

/*
 * initialize case label
 */
static void create(LPCExpression exp, varargs int line)
{
    ::create(line);
    caseExpression = exp;
}

/*
 * emit case label
 */
void code()
{
    emit("case ");
    caseExpression->code();
    ::code();
}
