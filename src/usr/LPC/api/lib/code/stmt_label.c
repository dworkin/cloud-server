# include "code.h"

inherit LPCStatement;


private string label;		/* label */
private LPCStatement statement;	/* code */

/*
 * initialize labelled statement
 */
static void create(string str, LPCStatement stmt, varargs int line)
{
    ::create(line);
    label = str;
    statement = stmt;
}

/*
 * emit code for labelled statement
 */
void code()
{
    emit(label + ":", line());
    statement->code();
}
