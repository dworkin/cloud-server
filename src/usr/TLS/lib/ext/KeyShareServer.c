# include "Record.h"

inherit Data;


private mixed *keyShare;	/* ({ group, key }) */

/*
 * initialize KeyShare for ServerHello
 */
static void create(mixed *keyShare)
{
    ::keyShare = keyShare;
}

/*
 * export as a blob
 */
string transport()
{
    return keyShare[0] + len2Save(keyShareSave(keyShare[1]));
}


mixed *keyShare()	{ return keyShare; }
