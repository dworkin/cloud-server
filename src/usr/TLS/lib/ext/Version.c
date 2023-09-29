# include "Record.h"

inherit Data;


private string version;		/* chosen version */

/*
 * initialize Version
 */
static create(string version)
{
    ::version = version;
}

/*
 * export as a blob
 */
string transport()
{
    return version;
}


string version()	{ return version; }
