# include "code.h"
# include "expression.h"

inherit LPCExpression;


private string name;		/* function name */
private LPCExpression *args;	/* arguments */
private int spread;		/* argument spread */

/*
 * initialize function call
 */
static void create(string str, LPCExpression *list, int flag, varargs int line)
{
    ::create(LPC_EXP_FUNCALL, line);
    name = str;
    args = list;
    spread = flag;
}

/*
 * emit code for function call
 */
void code(void)
{
    int i, sz;

    emit(name + "(", line());
    for (i = 0, sz = sizeof(args) - 1; i < sz; i++) {
	args[i]->code();
	emit(",");
    }
    if (sz >= 0) {
	args[sz]->code();
	if (spread) {
	    emit("...");
	}
    }
    emit(")");
}
