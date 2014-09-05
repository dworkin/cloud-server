# include "code.h"
# include "expression.h"

inherit LPCExpression;


private LPCExpression *elements;	/* array elements */ 

/*
 * NAME:	create()
 * DESCRIPTION:	initialize array aggregate
 */
static void create(LPCExpression *arr, varargs int line)
{
    ::create(LPC_EXP_ARRAY, line);
    elements = arr;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for array aggregate
 */
void code()
{
    int i, sz;

    emit("({");
    for (i = 0, sz = sizeof(elements) - 1; i < sz; i++) {
	elements[i]->code();
	emit(",");
    }
    if (sz >= 0) {
	elements[i]->code();
    }
    emit("})");
}
