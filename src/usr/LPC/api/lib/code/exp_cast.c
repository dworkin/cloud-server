# include "code.h"
# include "expression.h"

inherit Expression;


private string type;		/* type to cast to */
private Expression sub;		/* expression to cast */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize cast expression
 */
static void create(string t, Expression exp, varargs int line)
{
    ::create(EXP_CAST, line);
    type = t;
    sub = exp;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for cast expression
 */
void code()
{
    emit("((" + type + ")");
    sub->code();
    emit(")");
}
