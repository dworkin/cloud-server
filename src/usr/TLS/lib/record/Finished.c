# include "Record.h"

inherit Data;


private string hash;	/* verification hash */

/*
 * initialize Finished
 */
static void create(string hash)
{
    ::create(HANDSHAKE_FINISHED);
    ::hash = hash;
}

/*
 * export as a blob
 */
string transport()
{
    return hash;
}


string hash()	{ return hash; }
