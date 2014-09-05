# include "compiler.h"

/*
 * NAME:	create()
 * DESCRIPTION:	initialize LPC compiler
 */
static void create()
{
    compile_object(LPC_COMPILER);
    compile_object(LPC_CODEGEN);
}
