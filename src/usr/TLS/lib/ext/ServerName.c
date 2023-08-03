# include "Record.h"

inherit Data;


private string *names;		/* list of names */

/*
 * initialize ServerName
 */
static void create(string *names)
{
    ::names = names;
}

/*
 * export as a blob
 */
string transport()
{
    int sz, i;
    string *list;

    list = allocate(sz = sizeof(names));
    for (i = 0; i < sz; i++) {
	list[i] = len2Save(names[i]);
    }
    return len2Save("\0" + implode(list, "\0"));
}


string *names()		{ return names; }
