# include "code.h"

inherit LPCStatement;


private LPCDeclaration *locals;	/* local declarations */
private LPCStatement sub;	/* linked list of statements inside the block */

/*
 * initialize statement block
 */
static void create(LPCDeclaration *decls, LPCStatement stmt, varargs int line)
{
    ::create(line);
    locals = decls;
    sub = stmt;
}

/*
 * emit code for block
 */
void code(void)
{
    int i, sz;
    LPCStatement stmt;

    emit("{");
    for (i = 0, sz = sizeof(locals); i < sz; i++) {
	locals[i]->code();
    }
    for (stmt = sub; stmt; stmt = stmt->next()) {
	stmt->code();
    }
    emit("}");
}
