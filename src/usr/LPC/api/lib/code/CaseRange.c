# include "Code.h"

inherit LPCEntry;


private LPCExpression caseFrom, caseTo;	/* case range expressions */

/*
 * initialize case range label
 */
static void create(LPCExpression exp1, LPCExpression exp2, varargs int line)
{
    ::create(line);
    caseFrom = exp1;
    caseTo = exp2;
}

/*
 * emit case range label
 */
void code()
{
    emit("case ");
    caseFrom->code();
    emit("..");
    caseTo->code();
    ::code();
}
