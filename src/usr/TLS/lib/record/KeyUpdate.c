# include "Record.h"

inherit Data;


private int updateRequested;

static void create(int updateRequested)
{
    ::updateRequested = updateRequested;
}

string transport()
{
    string str;

    str = ".";
    str[0] = updateRequested;

    return str;
}


int updateRwquested()	{ return updateRequested; }
