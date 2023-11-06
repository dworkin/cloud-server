# include "Record.h"

inherit Data;


private string hostName;		/* host name */

/*
 * initialize ServerName
 */
static void create(string hostName)
{
    ::hostName = hostName;
}

/*
 * export as a blob
 */
string transport()
{
    return (hostName) ? len2Save("\0" + len2Save(hostName)) : "";
}


string hostName()	{ return hostName; }
