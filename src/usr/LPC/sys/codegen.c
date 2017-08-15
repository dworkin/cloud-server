# include <String.h>
# include "code.h"


/*
 * generate LPC source from code
 */
StringBuffer *codegen(LPCCode program)
{
    program->start();
    program->code();
    return program->end();
}
