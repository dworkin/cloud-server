# include "code.h"


/*
 * NAME:	codegen()
 * DESCRIPTION:	generate LPC source from code
 */
string *codegen(LPCCode program)
{
    program->start();
    program->code();
    return program->end();
}
