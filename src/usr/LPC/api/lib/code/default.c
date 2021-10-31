# include "code.h"

inherit LPCEntry;


/*
 * emit default label
 */
void code()
{
    emit("default");
    ::code();
}
