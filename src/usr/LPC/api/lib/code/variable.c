# include "code.h"

inherit LPCDeclaration;


/*
 * emit code for variable declaration
 */
void code(void)
{
    ::code();
    emit(";");
}
