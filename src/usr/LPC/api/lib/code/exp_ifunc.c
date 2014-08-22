# include "code.h"
# include "expression.h"

inherit ExpFuncall;


private string label;	/* inherit label */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize inherited function call
 */
static void create(string str, string name, Expression *list,
		   varargs int line)
{
    ::create(name, list, line);
    label = str;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for inherited function call
 */
void code()
{
    if (label) {
	emit(label, line());
    }
    emit("::", line());
    ::code();
}
