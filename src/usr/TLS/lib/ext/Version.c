# include "Record.h"
# include "Extension.h"

inherit Data;


private string version;

static create(string version)
{
    ::version = version;
}

string transport()
{
    return version;
}


string version()	{ return version; }
