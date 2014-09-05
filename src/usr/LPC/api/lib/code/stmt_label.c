# include "code.h"

inherit LPCStatement;


private string label;		/* label */
private LPCStatement statement;	/* code */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize labelled statement
 */
static void create(string str, LPCStatement stmt, varargs int line)
{
    ::create(line);
    label = str;
    statement = stmt;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for labelled statement
 */
void code()
{
    emit(label + ":", line());
    statement->code();
}
