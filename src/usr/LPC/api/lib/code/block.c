# include "code.h"

inherit Statement;


private Statement sub;	/* linked list of statements inside the block */

/*
 * NAME:	create()
 * DESCRITION:	initialize statement block
 */
static void create(Statement stmt, varargs int line)
{
    ::create(line);
    sub = stmt;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for block
 */
void code()
{
    Statement stmt;

    emit("{");
    for (stmt = sub; stmt; stmt = stmt->next()) {
	stmt->code();
    }
    emit("}");
}
