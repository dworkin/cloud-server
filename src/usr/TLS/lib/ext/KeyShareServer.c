# include "Record.h"

inherit Data;


private string *keyShare;

static void create(string *keyShare)
{
    ::keyShare = keyShare;
}

string transport()
{
    return keyShare[0] + len2Save(keyShare[1]);
}


string *keyShare()	{ return keyShare; }
