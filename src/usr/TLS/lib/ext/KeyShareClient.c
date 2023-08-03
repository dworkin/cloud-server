# include "Record.h"

inherit Data;


private string **keyShares;	/* list of ({ group, key }) pairs */

/*
 * initialize KeyShare for client
 */
static void create(string **keyShares)
{
    ::keyShares = keyShares;
}

/*
 * export as a blob
 */
string transport()
{
    int sz, i, j;
    string *list;

    sz = sizeof(keyShares);
    list = allocate(2 * sz);
    for (i = 0, j = 0; i < sz; i++) {
	list[j++] = keyShares[i][0];
	list[j++] = len2Save(keyShares[i][1]);
    }

    return len2Save(implode(list, ""));
}


string **keyShares()	{ return keyShares; }
