# include "code.h"
# include "expression.h"

inherit LPCExpression;


private LPCExpression *elements;	/* mapping aggregate elements */

/*
 * initialize mapping aggrgate
 */
static void create(LPCExpression *arr, varargs int line)
{
    ::create(LPC_EXP_MAPPING, line);
    elements = arr;
}

/*
 * emit code for mapping aggregate
 */
void code(void)
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
