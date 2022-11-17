# include "Code.h"

inherit LPCStatement;


private LPCEntry *entries;	/* case label */
private LPCStatement statement;	/* case code */

/*
 * initialize entry point
 */
static void create(LPCEntry *list, LPCStatement stmt, varargs int line)
{
    ::create(line);
    entries = list;
    statement = stmt;
}

/*
 * emit code for entry point
 */
void code()
{
    int i, sz;

    for (i = 0, sz = sizeof(entries); i < sz; i++) {
	entries[i]->code();
    }
    statement->code();
}
