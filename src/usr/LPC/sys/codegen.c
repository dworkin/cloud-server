# include <StringBuffer.h>
# include "code.h"


/*
 * NAME:	codegen()
 * DESCRIPTION:	generate LPC source from code
 */
StringBuffer *codegen(LPCCode program)
{
    program->start();
    program->code();
    return program->end();
}
