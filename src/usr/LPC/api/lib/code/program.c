# include "code.h"

inherit LPCCode;


private LPCInherit *inherits;		/* inherit statements */
private LPCDeclaration *globals;	/* function and variable declarations */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize program
 */
static void create(LPCInherit *inh, LPCDeclaration *decl)
{
    ::create();
    inherits = inh;
    globals = decl;
}

/*
 * NAME:	code()
 * DESCRIPTION:	emit code for program
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
