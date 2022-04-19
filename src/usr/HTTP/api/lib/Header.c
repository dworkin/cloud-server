# include <type.h>
# include "HttpHeader.h"

inherit HttpHeaderItem;


private string name;	/* header name */
private mixed value;	/* header value */

/*
 * create header
 */
static void create(string name, mixed value)
{
    ::name = name;
    ::value = value;
}

/*
 * add items to a header value list
 */
void add(mixed *value)
{
    if (typeof(::value) != T_ARRAY) {
	error("Header is not a list");
    }
    ::value += value;
}

/*
 * transport header
 */
string transport()
{
    return name + ((value) ? ": " + transportValue(value) : ":");
}


string name()	{ return name; }
mixed value()	{ return value; }
