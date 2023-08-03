# include <String.h>
# include "Record.h"

inherit Data;


private string data;	/* uninterpreted data */

/*
 * initialize UnknownExtension from a blob
 */
static void create(String blob, int offset, int end)
{
    data = substring(blob, offset, end - 1);
}

/*
 * export as a blob
 */
string transport()
{
    return data;
}


string data()	{ return data; }
