# include "code.h"

inherit LPCDeclaration;


/*
 * NAME:	code()
 * DESCRIPTION:	emit code for variable declaration
 */
void code()
{
    ::code();
    emit(";");
}
