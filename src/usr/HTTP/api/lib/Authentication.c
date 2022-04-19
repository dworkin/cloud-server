# include "HttpHeader.h"

inherit HttpHeaderItem;

private inherit base64 "/lib/util/base64";


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
    return (authentication) ?
	    scheme + " " + base64::encode(authentication) :
	    scheme;
}


string scheme()		{ return scheme; }
mixed authentication()	{ return authentication; }
