# include "code.h"
# include "expression.h"

inherit Expression;


private string name;		/* function name */
private Expression *args;	/* arguments */
private int spread;		/* argument spread */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize function call
 */
static void create(string str, Expression *list, int flag, varargs int line)
{
    ::create(EXP_FUNCALL, line);
    name = str;
    args = list;
    spread = flag;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for function call
 */
void code()
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
