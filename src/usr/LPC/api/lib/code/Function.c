# include "Code.h"

inherit LPCDeclaration;


private LPCDeclaration *parameters;	/* function parameters */
private int ellipsis;			/* function ellipsis */
private LPCStmtBlock body;		/* function body */

/*
 * initialize function declaration
 */
static void create(LPCType type, string name, LPCDeclaration *decls, int flag,
		   LPCStmtBlock stmt, varargs int line)
{
    ::create(type, name, line);
    parameters = decls;
    ellipsis = flag;
    body = stmt;
}

/*
 * emit code for function declaration
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
