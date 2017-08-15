# include "compiler.h"

/*
 * initialize LPC compiler
 */
static void create()
{
    compile_object(LPC_COMPILER);
    compile_object(LPC_CODEGEN);
}
