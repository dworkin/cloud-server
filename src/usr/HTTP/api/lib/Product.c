# include "HttpField.h"

inherit HttpFieldItem;


private string product;		/* product */
private string version;		/* product version */
private string *comments;	/* comments */

/*
 * create product item
 */
static void create(string product, varargs string version, string comments...)
{
    ::product = product;
    ::version = version;
    ::comments = comments;
}

/*
 * transport this item
 */
string transport()
{
    return product + ((version) ? "/" + version : "") +
		     ((sizeof(comments) != 0) ?
		       " " + implode(comments, " ") : "");
}


string product()	{ return product; }
string version()	{ return version; }
string *comments()	{ return comments; }
