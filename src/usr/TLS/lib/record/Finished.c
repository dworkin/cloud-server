# include "Record.h"

inherit Data;


private string hash;

static void create(string hash)
{
    ::create(HANDSHAKE_FINISHED);
    ::hash = hash;
}

string transport()
{
    return hash;
}


string hash()	{ return hash; }
