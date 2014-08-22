# include "code.h"
# include "expression.h"

inherit Expression;


private Expression *elements;	/* mapping aggregate elements */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize mapping aggrgate
 */
static void create(Expression *arr, varargs int line)
{
    ::create(EXP_MAPPING, line);
    elements = arr;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for mapping aggregate
 */
void code()
{
    int i, sz;

    emit("([");
    for (i = 0, sz = sizeof(elements) - 2; i < sz; i += 2) {
	elements[i]->code();
	emit(":");
	elements[i + 1]->code();
	emit(",");
    }
    if (sz >= 0) {
	elements[i]->code();
	emit(":");
	elements[i + 1]->code();
    }
    emit("])");
}
