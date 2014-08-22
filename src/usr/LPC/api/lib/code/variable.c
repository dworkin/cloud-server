# include "code.h"

inherit Declaration;


/*
 * NAME:	code()
 * DESCRIPTION:	emit code for variable declaration
 */
void code()
{
    ::code();
    emit(";");
}
