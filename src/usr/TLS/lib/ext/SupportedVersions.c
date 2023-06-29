# include "Record.h"
# include "Extension.h"

inherit Data;


private string versions;

static create(string versions)
{
    ::versions = versions;
}

string transport()
{
    return len1Save(versions);
}


string versions()	{ return versions; }
