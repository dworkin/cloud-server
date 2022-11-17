# include "Code.h"

inherit LPCDeclaration;


/*
 * emit code for variable declaration
 */
void code()
{
    ::code();
    emit(";");
}
