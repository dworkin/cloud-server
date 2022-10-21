# include "HttpField.h"
# include <type.h>

inherit HttpFieldItem;


private string scheme;		/* authentication scheme */
private mixed authentication;	/* authentication item */

/*
 * create authentication item
 */
static void create(string scheme, varargs mixed authentication)
{
    ::scheme = scheme;
    ::authentication = authentication;
}

/*
 * transport this item
 */
string transport()
{
    switch (typeof(authentication)) {
    case T_NIL:
	return scheme;

    case T_STRING:
	return scheme + " " + authentication;

    case T_ARRAY:
	return scheme + " " + implode(authentication, ", ");
    }
}


string scheme()		{ return scheme; }
mixed authentication()	{ return authentication; }
