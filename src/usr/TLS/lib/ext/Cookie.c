# include "Record.h"

inherit Data;


private string cookie;

static void create(string cookie)
{
    ::cookie = cookie;
}

string transport()
{
    return len2Save(cookie);
}


string cookie()		{ return cookie; }
