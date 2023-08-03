# include "Record.h"

inherit Data;


private int updateRequested;	/* remote update requested? */

/*
 * initialize KeyUpate
 */
static void create(int updateRequested)
{
    ::updateRequested = updateRequested;
}

/*
 * export as a blob
 */
string transport()
{
    string str;

    str = ".";
    str[0] = updateRequested;

    return str;
}


int updateRequested()	{ return updateRequested; }
