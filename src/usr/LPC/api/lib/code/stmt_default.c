# include "code.h"

inherit LPCStatement;


private LPCStatement statement;	/* code */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize default statement
 */
static void create(LPCStatement stmt, varargs int line)
{
    ::create(line);
    statement = stmt;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for default statement
 */
void code()
{
    emit("default:", line());
    statement->code();
}
