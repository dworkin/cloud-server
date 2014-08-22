# include "code.h"


private Inherit *inherits;	/* inherit statements */
private Declaration *globals;	/* function and variable declarations */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize program
 */
static void create(Inherit *inh, Declaration *decl)
{
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
