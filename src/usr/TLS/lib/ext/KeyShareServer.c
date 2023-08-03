# include "Record.h"

inherit Data;


private string *keyShare;	/* ({ group, key }) */

/*
 * initialize KeyShare for ServerHello
 */
static void create(string *keyShare)
{
    ::keyShare = keyShare;
}

/*
 * export as a blob
 */
string transport()
{
    return keyShare[0] + len2Save(keyShare[1]);
}


string *keyShare()	{ return keyShare; }
