# include "Record.h"

inherit Data;


private string group;

static void create(string group)
{
    ::group = group;
}

string transport()
{
    return group;
}


string group()		{ return group; }
