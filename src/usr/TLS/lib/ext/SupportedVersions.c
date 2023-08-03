# include "Record.h"
# include "Extension.h"

inherit Data;


private string *versions;	/* list of supported versions */

/*
 * initialize SupportedVersions
 */
static create(string *versions)
{
    ::versions = versions;
}

/*
 * export as a blob
 */
string transport()
{
    return len1Save(implode(versions, ""));
}


string *versions()	{ return versions; }
