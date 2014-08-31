# include "code.h"

inherit Declaration;


private Declaration *parameters;	/* function parameters */
private int ellipsis;			/* function ellipsis */
private StmtBlock body;			/* function body */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize function declaration
 */
static void create(Type type, string name, Declaration *decls, int flag,
		   StmtBlock stmt, varargs int line)
{
    ::create(type, name, line);
    parameters = decls;
    ellipsis = flag;
    body = stmt;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for function declaration
 */
void code()
{
    int i, sz;

    ::code();
    emit("(");
    for (i = 0, sz = sizeof(parameters) - 1; i < sz; i++) {
	parameters[i]->code();
	emit(",");
    }
    if (sz >= 0) {
	parameters[sz]->code();
	if (ellipsis) {
	    emit("...");
	}
    }
    emit(")");
    if (body) {
	body->code();
    } else {
	emit(";");
    }
}
