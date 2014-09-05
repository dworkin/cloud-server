# include "code.h"

inherit LPCCode;


private int priv;		/* private flag */
private string label;		/* inherit label */
private LPCExpression class;	/* string constant */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize inherit statement
 */
static void create(int flag, string l, LPCExpression exp, varargs int line)
{
    ::create(line);
    priv = flag;
    label = l;
    class = exp;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for inherit statement
 */
void code()
{
    if (priv) {
	emit("private ", line());
    }
    emit("inherit", line());
    if (label) {
	emit(" " + label);
    }
    class->code();
    emit(";");
}
