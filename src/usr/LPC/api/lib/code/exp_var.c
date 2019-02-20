# include "code.h"
# include "expression.h"

inherit LPCExpression;


private string name;	/* variable name */

/*
 * initialize variable reference
 */
static void create(string str, varargs int line)
{
    ::create(LPC_EXP_VAR, line);
    name = str;
}

/*
 * emit code for variable reference
 */
void code(void)
{
    emit(name, line());
}
