# include "Record.h"

inherit Data;


private string cookie;		/* cookie */

/*
 * initialize Cookie
 */
static void create(string cookie)
{
    ::cookie = cookie;
}

/*
 * export as a blob
 */
string transport()
{
    return len2Save(cookie);
}


string cookie()		{ return cookie; }
