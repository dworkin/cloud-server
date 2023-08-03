# include "Record.h"

inherit Data;


private string group;	/* requested group */

/*
 * initialize KeyShare for HelloRetryRequest
 */
static void create(string group)
{
    ::group = group;
}

/*
 * export as a blob
 */
string transport()
{
    return group;
}


string group()		{ return group; }
