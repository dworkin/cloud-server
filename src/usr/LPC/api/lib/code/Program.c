# include "Code.h"
# include <String.h>

inherit LPCCode;


private LPCInherit *inherits;		/* inherit statements */
private LPCDeclaration *globals;	/* function and variable declarations */

/*
 * initialize program
 */
static void create(LPCInherit *inh, LPCDeclaration *decl)
{
    ::create();
    inherits = inh;
    globals = decl;
}

/*
 * emit code for program
 */
void code()
{
    int i, sz;

    for (i = 0, sz = sizeof(inherits); i < sz; i++) {
	inherits[i]->code();
    }
    for (i = 0, sz = sizeof(globals); i < sz; i++) {
	globals[i]->code();
    }
}

/*
 * return program code in a StringBuffer
 */
StringBuffer codegen()
{
    start();
    code();
    return end();
}
