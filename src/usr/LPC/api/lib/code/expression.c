# include "code.h"
# include "expression.h"
# include <type.h>

inherit LPCCode;


private mixed value;	/* expression value */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize expression
 */
static void create(mixed exp, varargs int line)
{
    ::create(line);
    value = exp;
}

/*
 * NAME:	value()
 * DESCRIPTION:	return value of this expression
 */
mixed value()
{
    return value;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for expression
 */
void code()
{
    emit("(");
    switch (typeof(value)) {
    case T_NIL:
	emit("nil", line());
	break;

    case T_INT:
	emit((string) value, line());
	break;

    case T_FLOAT:
	emitFloat(value, line());
	break;

    case T_STRING:
	emitString(value, line());
	break;
    }
    emit(")");
}
