# include "code.h"

inherit Statement;


private Declaration *locals;	/* local declarations */
private Statement sub;		/* linked list of statements inside the block */

/*
 * NAME:	create()
 * DESCRITION:	initialize statement block
 */
static void create(Declaration *decls, Statement stmt, varargs int line)
{
    ::create(line);
    locals = decls;
    sub = stmt;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for block
 */
void code()
{
    int i, sz;
    Statement stmt;

    emit("{");
    for (i = 0, sz = sizeof(locals); i < sz; i++) {
	locals[i]->code();
    }
    for (stmt = sub; stmt; stmt = stmt->next()) {
	stmt->code();
    }
    emit("}");
}
