# include "Record.h"

inherit Data;


private string *groups;		/* list of supported groups */

/*
 * initialize SupportedGroups
 */
static void create(string *groups)
{
    ::groups = groups;
}

/*
 * export as a blob
 */
string transport()
{
    return len2Save(implode(groups, ""));
}


string *groups()		{ return groups; }
