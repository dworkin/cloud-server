# include "code.h"

inherit Statement;


private string label;		/* label */
private Statement statement;	/* code */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize labelled statement
 */
static void create(string str, Statement stmt, varargs int line)
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
