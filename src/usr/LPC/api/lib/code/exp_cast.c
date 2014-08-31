# include "code.h"
# include "expression.h"

inherit Expression;


private Type cast;		/* type to cast to */
private Expression sub;		/* expression to cast */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize cast expression
 */
static void create(Type type, Expression exp, varargs int line)
{
    ::create(EXP_CAST, line);
    cast = type;
    sub = exp;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for cast expression
 */
void code()
{
    emit("((");
    cast->code();
    emit(")");
    sub->code();
    emit(")");
}
