# include "Code.h"

inherit LPCEntry;


private string label;

static void create(string str, varargs int line)
{
    ::create(line);
    label = str;
}

void code()
{
    emit(label);
    ::code();
}
