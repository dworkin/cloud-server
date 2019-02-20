# include "code.h"

inherit LPCStatement;


private LPCStatement statement;	/* code */

/*
 * initialize default statement
 */
static void create(LPCStatement stmt, varargs int line)
{
    ::create(line);
    statement = stmt;
}

/*
 * emit code for default statement
 */
void code(void)
{
    emit("default:", line());
    statement->code();
}
