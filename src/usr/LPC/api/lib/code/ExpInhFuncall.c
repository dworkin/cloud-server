# include "Code.h"
# include "expression.h"

inherit LPCExpFuncall;


private string label;	/* inherit label */

/*
 * initialize inherited function call
 */
static void create(string str, string name, LPCExpression *list, int flag,
		   varargs int line)
{
    ::create(name, list, flag, line);
    label = str;
}

/*
 * emit code for inherited function call
 */
void code()
{
    if (label) {
	emit(label, line());
    }
    emit("::", line());
    ::code();
}
