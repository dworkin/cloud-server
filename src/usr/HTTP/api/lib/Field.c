# include <type.h>
# include "HttpField.h"

inherit HttpFieldItem;
private inherit "/lib/util/ascii";


private string name;	/* field name */
private string lcName;	/* lower case name */
private mixed value;	/* field value */
private string lcValue;	/* lower case value */
private mixed *params;	/* parameters */

/*
 * create field
 */
static void create(string name, mixed value, varargs mixed *params)
{
    ::name = name;
    lcName = lower_case(name);
    ::value = value;
    ::params = params;
}

/*
 * add items to a field value list
 */
void add(mixed *value, varargs mixed *params)
{
    if (typeof(::value) != T_ARRAY) {
	error("Field is not a list");
    }
    ::value += value;
    if (params) {
	::params += params;
    }
}

/*
 * assume that the value is an array of strings, see if it contains a string
 */
int listContains(string str)
{
    if (!lcValue) {
	lcValue = "\0" + lower_case(implode(value, "\0")) + "\0";
    }
    return sscanf(lcValue, "%*s\0" + str + "\0");
}

/*
 * transport field
 */
string transport()
{
    return name + ((value) ? ": " + transportValue(value, params) : ":");
}


string name()	{ return name; }
string lcName()	{ return lcName; }
mixed value()	{ return value; }
mixed *params()	{ return params; }
