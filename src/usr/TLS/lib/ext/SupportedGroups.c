# include "Record.h"

inherit Data;


private string groups;

static void create(string groups)
{
    ::groups = groups;
}

string transport()
{
    return len2Save(groups);
}


string groups()		{ return groups; }
