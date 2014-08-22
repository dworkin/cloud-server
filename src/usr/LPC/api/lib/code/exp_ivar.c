# include "code.h"
# include "expression.h"

inherit ExpVar;


private string label;	/* inherit label */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize inherited variable reference
 */
static void create(string str, string name, varargs int line)
{
    ::create(name, line);
    label = str;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for inherited variable reference
 */
void code()
{
    if (label) {
	emit(label, line());
    }
    emit("::", line());
    ::code();
}
