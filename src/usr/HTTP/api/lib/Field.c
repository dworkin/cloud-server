# include <type.h>
# include "HttpField.h"

inherit HttpFieldItem;


private string name;	/* field name */
private mixed value;	/* field value */

/*
 * create field
 */
static void create(string name, mixed value)
{
    ::name = name;
    ::value = value;
}

/*
 * add items to a field value list
 */
void add(mixed *value)
{
    if (typeof(::value) != T_ARRAY) {
	error("Field is not a list");
    }
    ::value += value;
}

/*
 * transport field
 */
string transport()
{
    return name + ((value) ? ": " + transportValue(value) : ":");
}


string name()	{ return name; }
mixed value()	{ return value; }
